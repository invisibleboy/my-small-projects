/*
 * File: Controller0.h
 *
 * Real-Time Workshop code generated for Simulink model Controller0.
 *
 * Model version                        : 1.1934
 * Real-Time Workshop file version      : 7.1  (R2008a)  23-Jan-2008
 * Real-Time Workshop file generated on : Thu Feb 02 15:31:04 2012
 * TLC version                          : 7.1 (Jan 18 2008)
 * C/C++ source code generated on       : Thu Feb 02 15:31:04 2012
 */

#ifndef RTW_HEADER_Controller0_h_
#define RTW_HEADER_Controller0_h_
#ifndef Controller0_COMMON_INCLUDES_
# define Controller0_COMMON_INCLUDES_
#include <stddef.h>
#include "rtwtypes.h"
//#include "rtlibsrc.h"
#include "rt_matrixlib.h"
#endif                                 /* Controller0_COMMON_INCLUDES_ */

#include "Controller0_types.h"

/* Macros for accessing real-time model data structure */
#ifndef rtmGetErrorStatus
# define rtmGetErrorStatus(rtm)        ((void*) 0)
#endif

#ifndef rtmSetErrorStatus
# define rtmSetErrorStatus(rtm, val)   ((void) 0)
#endif

/* Constant parameters (auto storage) */
typedef struct {
  /* Expression: K
   * '<S2>/Gain'
   */
  real_T Gain_Gain[5];
} ConstParam_Controller0;

/* External inputs (root inport signals with auto storage) */
typedef struct {
  real_T x[5];                         /* '<Root>/x' */
} ExternalInputs_Controller0;

/* External outputs (root outports fed by signals with auto storage) */
typedef struct {
  real_T U;                            /* '<Root>/U' */
} ExternalOutputs_Controller0;

/* External inputs (root inport signals with auto storage) */
extern ExternalInputs_Controller0 Controller0_U;

/* External outputs (root outports fed by signals with auto storage) */
extern ExternalOutputs_Controller0 Controller0_Y;

/* Constant parameters (auto storage) */
extern const ConstParam_Controller0 Controller0_ConstP;

/* Model entry point functions */
extern void Controller0_initialize(void);
extern void Controller0_step(void);

/*-
 * The generated code includes comments that allow you to trace directly
 * back to the appropriate location in the model.  The basic format
 * is <system>/block_name, where system is the system number (uniquely
 * assigned by Simulink) and block_name is the name of the block.
 *
 * Note that this particular code originates from a subsystem build,
 * and has its own system numbers different from the parent model.
 * Refer to the system hierarchy for this subsystem below, and use the
 * MATLAB hilite_system command to trace the generated code back
 * to the parent model.  For example,
 *
 * hilite_system('Model/Controller')    - opens subsystem Model/Controller
 * hilite_system('Model/Controller/Kp') - opens and selects block Kp
 *
 * Here is the system hierarchy for this model
 *
 * '<Root>' : Model
 * '<S1>'   : Model/Controller
 * '<S2>'   : Model/Controller/controller
 */
#endif                                 /* RTW_HEADER_Controller0_h_ */

/* File trailer for Real-Time Workshop generated code.
 *
 * [EOF]
 */
