#ifndef	PWM_H
#define	PWM_H
/*******************************************************************************
 *   Filename:       bsp_touch.h
 *   Revised:        All copyrights reserved to Wuming Shen.
 *   Date:           2014-03-24
 *   Revision:       v1.0
 *   Writer:	     Wuming Shen.
 *
 *   Description:
 *
 *   Notes:
 *					QQ:276193028
 *     				E-mail:shenchangwei945@163.com
 *
 *******************************************************************************/

/*******************************************************************************
 * INCLUDES
 */
 #include  <os_cpu.h>

/*******************************************************************************
 * CONSTANTS
 */
/***********************************************
 * √Ë ˆ£∫
 */
#define PWM_CH_NBR          4

#define PWM_CH_BEEP         1

#define PWM_CH_FAN          0

#define PWM_CH_LED_R        1
#define PWM_CH_LED_G        2
#define PWM_CH_LED_B        3
/*******************************************************************************
 * MACROS
 */

/*******************************************************************************
 * TYPEDEFS
 */
typedef struct {
  TIM_TypeDef*      TIMx;
  float             freq[PWM_CH_NBR];
  uint32_t          period[PWM_CH_NBR];
  float             duty[PWM_CH_NBR];
  uint8_t           maxDuty[PWM_CH_NBR];
  uint32_t          compare[PWM_CH_NBR];
  uint8_t           status[PWM_CH_NBR];
  uint8_t           chEn[PWM_CH_NBR];
} StrPwm;

extern StrPwm   pwm;
extern StrPwm   LedPwm;
extern StrPwm   BeepPwm;
#define FanPwm  LedPwm

/*******************************************************************************
 * GLOBAL VARIABLES
 */

/*******************************************************************************
 * GLOBAL FUNCTIONS
 */

/*******************************************************************************
 * EXTERN VARIABLES
 */

/*******************************************************************************
 * EXTERN FUNCTIONS
 */
StrPwm	  *FanPWM_Init				    (void);
void	   FanPWM_SetFreq		        (uint8_t ch, float freq);
void       FanPWM_SetDuty               (uint8_t ch, float duty);
void       FanPWM_SetProperty           (uint8_t ch, float freq, float duty);

void       FanPWM_Start                 (void);
void       FanPWM_Stop                  (void);

StrPwm	  *LedPWM_Init				    (void);
void	   LedPWM_SetFreq		        (uint8_t ch, float freq);
void       LedPWM_SetDuty               (uint8_t ch, float duty);
void       LedPWM_SetProperty           (uint8_t ch, float freq, float duty);

void       LedPWM_Start                 (void);
void       LedPWM_Stop                  (void);


StrPwm	  *BeepPWM_Init				    (void);
StrPwm	  *PWM_Init				        (StrPwm *pwm);
void	   PWM_SetFreq		            (StrPwm *pwm, uint8_t ch, float freq);
void       PWM_SetDuty                  (StrPwm *pwm, uint8_t ch, float duty);
void       PWM_SetProperty              (StrPwm *pwm, uint8_t ch, float freq, float duty);

void       PWM_Start                    (StrPwm *pwm);
void       PWM_Stop                     (StrPwm *pwm);


void        LED_SetColor                (INT32U color );
/*******************************************************************************
 * 				end of file
 *******************************************************************************/
#endif	/* GLOBLES_H */