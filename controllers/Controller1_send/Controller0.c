/*
 * File: Controller0.c
 *
 * Real-Time Workshop code generated for Simulink model Controller0.
 *
 * Model version                        : 1.1934
 * Real-Time Workshop file version      : 7.1  (R2008a)  23-Jan-2008
 * Real-Time Workshop file generated on : Thu Feb 02 15:31:04 2012
 * TLC version                          : 7.1 (Jan 18 2008)
 * C/C++ source code generated on       : Thu Feb 02 15:31:04 2012
 */

#include "Controller0.h"
#include "Controller0_private.h"

/* External inputs (root inport signals with auto storage) */
ExternalInputs_Controller0 Controller0_U;

/* External outputs (root outports fed by signals with auto storage) */
ExternalOutputs_Controller0 Controller0_Y;

/* Model step function */
void Controller0_step(void)
{
  /* local block i/o variables */
  real_T rtb_Gain;

  /* Gain Block: '<S2>/Gain'
   * About '<S2>/Gain :'
   *   Gain value: K


   */
  {
    static const int_T dims[3] = { 1, 5, 1 };

    rt_MatMultRR_Dbl((real_T *)&rtb_Gain, (real_T *)
                     &Controller0_ConstP.Gain_Gain[0],
                     (real_T *)&Controller0_U.x[0], &dims[0]);
  }

  /* Outport: '<Root>/U' incorporates:
   *  Constant: '<S2>/Feedforward'
   *  Sum: '<S2>/Sum'
   */
  Controller0_Y.U = 9.0974843180209870E+000 + rtb_Gain;
}

/* Model initialize function */
void Controller0_initialize(void)
{
  /* (no initialization code required) */
}

/* File trailer for Real-Time Workshop generated code.
 *
 * [EOF]
 */
