/*******************************************************************************
 *   Filename:       osal_memory.c
 *   Revised:        $Date: 2013-04-17 $
 *   Revision:       $
 *   Writer:         Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *
 *
 *   All copyrights reserved to Wuming Shen.
 *
 *******************************************************************************/
/*******************************************************************************
 * INCLUDES
 */
#include <osal.h>
#include <osal_memory.h>

#if OSAL_MEM_EN > 0
/*******************************************************************************
 * CONSTANTS
 */
#define MAXMEMHEAP  2048                // 内存分配堆栈大小// Typically, 0.50-1.50K

#if ( MAXMEMHEAP >= 32768 )
  #error MAXMEMHEAP is too big to manage!
#endif

#if !defined ( OSALMEM_MIN_BLKSZ )
  #define OSALMEM_MIN_BLKSZ    16
#endif

#if !defined ( OSALMEM_SMALL_BLKSZ )
  #define OSALMEM_SMALL_BLKSZ  16
#endif

#if !defined ( OSALMEM_GUARD )
  #define OSALMEM_GUARD  TRUE  // TBD - Hacky workaround til Bugzilla 1252 is fixed!
  #define OSALMEM_READY  0xE2
#endif
/*******************************************************************************
 * TYPEDEFS
 */
typedef INT16U  osalMemHdr_t;

/*******************************************************************************
 * MACROS
 */
#define OSALMEM_IN_USE  0x8000

/* This number sets the size of the small-block bucket. Although profiling
 * shows max simultaneous alloc of 16x18, timing without profiling overhead
 * shows that the best worst case is achieved with the following.
 */
#define SMALLBLKHEAP    40

// To maintain data alignment of the pointer returned, reserve the greater
// space for the memory block header.
#define HDRSZ  ( (sizeof ( halDataAlign_t ) > sizeof( osalMemHdr_t )) ? \
                  sizeof ( halDataAlign_t ) : sizeof( osalMemHdr_t ) )
/*******************************************************************************
 * LOCAL VARIABLES
 */

#if ( OSALMEM_GUARD )
  static byte ready = 0;
#endif

static osalMemHdr_t *ff1;  // First free block in the small-block bucket.
static osalMemHdr_t *ff2;  // First free block after the small-block bucket.

#if defined( EXTERNAL_RAM )
  static byte  *theHeap = (byte *)EXT_RAM_BEG;
#else
  static halDataAlign_t   _theHeap[ MAXMEMHEAP / sizeof( halDataAlign_t ) ];
  static byte  *theHeap     = (byte *)_theHeap;
#endif

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * LOCAL FUNCTIONS
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */
void *osal_mem_alloc(INT16U size );
/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * @fn      osal_mem_init
 *
 * @brief   Initialize the heap memory management system.
 *
 * @param   void
 *
 * @return  void
 */
void osal_mem_init( void )
{
  osalMemHdr_t  *tmp;
  // Setup a NULL block at the end of the heap for fast comparisons with zero.
  tmp = (osalMemHdr_t *)theHeap + (MAXMEMHEAP / HDRSZ) - 1;
  *tmp = 0;

  // Setup a small-block bucket.
  tmp = (osalMemHdr_t *)theHeap;
  *tmp = SMALLBLKHEAP;

  // Setup the wilderness.
  tmp = (osalMemHdr_t *)theHeap + (SMALLBLKHEAP / HDRSZ);
  *tmp = ((MAXMEMHEAP / HDRSZ) * HDRSZ) - SMALLBLKHEAP - HDRSZ;

#if ( OSALMEM_GUARD )
  ready = OSALMEM_READY;
#endif

  // Setup a NULL block that is never freed so that the small-block bucket
  // is never coalesced with the wilderness.
  ff1 = tmp;
  ff2 = (osalMemHdr_t *)osal_mem_alloc( 0 );
  ff1 = (osalMemHdr_t *)theHeap;
}

/*******************************************************************************
 * @fn      osal_mem_kick
 *
 * @brief   Kick the ff1 pointer out past the long-lived OSAL Task blocks.
 *          Invoke this once after all long-lived blocks have been allocated -
 *          presently at the end of osal_init_system().
 *
 * @param   void
 *
 * @return  void
 *******************************************************************************/
void osal_mem_kick( void )
{
  halIntState_t  cpu_sr;
  CPU_CRITICAL_ENTER();  // Hold off interrupts.

  /* Logic in osal_mem_free() will ratchet ff1 back down to the first free
   * block in the small-block bucket.
   */
  ff1 = ff2;
  CPU_CRITICAL_EXIT();  // Re-enable interrupts.
}

/*******************************************************************************
 * @fn      osal_mem_alloc
 *
 * @brief   Implementation of the allocator functionality.
 *
 * @param   size - number of bytes to allocate from the heap.
 *
 * @return  void * - pointer to the heap allocation; NULL if error or failure.
 *******************************************************************************/
void *osal_mem_alloc(INT16U size )
{
    osalMemHdr_t   *prev;
    osalMemHdr_t   *hdr;
    halIntState_t   cpu_sr;
    INT16U          tmp;
    byte  coal = 0;

 #if ( OSALMEM_GUARD )
    // Try to protect against premature use by HAL / OSAL.
    if ( ready != OSALMEM_READY ) {
        osal_mem_init();
    }
#endif
    size += HDRSZ;
    // Calculate required bytes to add to 'size' to align to halDataAlign_t.
    if ( sizeof( halDataAlign_t ) == 2 ) {
        size += (size & 0x01);
    } else if ( sizeof( halDataAlign_t ) != 1 ) {
        byte mod = size % sizeof( halDataAlign_t );

        if ( mod != 0 ) {
            size += (sizeof( halDataAlign_t ) - mod);
        }
    }

    CPU_CRITICAL_ENTER();  // Hold off interrupts.

    // Smaller allocations are first attempted in the small-block bucket.
    if ( size <= OSALMEM_SMALL_BLKSZ ) {
        hdr = ff1;
    } else {
        hdr = ff2;
    }
    tmp = *hdr;

    do {
        if ( tmp & OSALMEM_IN_USE ) {
            tmp ^= OSALMEM_IN_USE;
            coal = 0;
        }  else {
            if ( coal != 0 ) {
                *prev += *hdr;
                if ( *prev >= size )  {
                    hdr = prev;
                    tmp = *hdr;
                    break;
                }
            } else {
                if ( tmp >= size ) {
                    break;
                }
                coal = 1;
                prev = hdr;
            }
        }

        hdr = (osalMemHdr_t *)((byte *)hdr + tmp);

        tmp = *hdr;
        if ( tmp == 0 ) {
            hdr = ((void *)NULL);
            break;
        }
    } while ( 1 );

    if ( hdr != ((void *)NULL)) {
        tmp -= size;
        // Determine whether the threshold for splitting is met.
        if ( tmp >= OSALMEM_MIN_BLKSZ ) {
            // Split the block before allocating it.
            osalMemHdr_t *next = (osalMemHdr_t *)((byte *)hdr + size);
            *next = tmp;
            *hdr = (size | OSALMEM_IN_USE);
        } else {
            *hdr |= OSALMEM_IN_USE;
        }
        hdr++;
    }
    CPU_CRITICAL_EXIT();  // Re-enable interrupts.
    return (void *)hdr;
}

/*******************************************************************************
 * @fn      osal_mem_free
 *
 * @brief   Implementation of the de-allocator functionality.
 *
 * @param   ptr - pointer to the memory to free.
 *
 * @return  void
 *******************************************************************************/
void osal_mem_free( void *ptr )
{
    osalMemHdr_t   *currHdr;
    halIntState_t   cpu_sr;

#if ( OSALMEM_GUARD )
    // Try to protect against premature use by HAL / OSAL.
    if ( ready != OSALMEM_READY ) {
        osal_mem_init();
    }
#endif

    CPU_CRITICAL_ENTER();  // Hold off interrupts.
    currHdr = (osalMemHdr_t *)ptr - 1;
    *currHdr &= ~OSALMEM_IN_USE;

    if ( ff1 > currHdr ) {
        ff1 = currHdr;
    }
    CPU_CRITICAL_EXIT();  // Re-enable interrupts.
}

/*******************************************************************************
 * 				     end of file                               *
 *******************************************************************************/
#endif