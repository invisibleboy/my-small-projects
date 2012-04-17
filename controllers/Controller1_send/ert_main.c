/*
 * File: ert_main.c
 *
 * Real-Time Workshop code generated for Simulink model Controller0.
 *
 * Model version                        : 1.1934
 * Real-Time Workshop file version      : 7.1  (R2008a)  23-Jan-2008
 * Real-Time Workshop file generated on : Thu Feb 02 15:31:04 2012
 * TLC version                          : 7.1 (Jan 18 2008)
 * C/C++ source code generated on       : Thu Feb 02 15:31:04 2012
 */

//#include <stdio.h>                     /* This ert_main.c example uses printf/fflush */
#include "Controller0.h"               /* Model's header file */
#include "rtwtypes.h"                  /* MathWorks types */

static boolean_T OverrunFlag = 0;

/* Associating rt_OneStep with a real-time clock or interrupt service routine
 * is what makes the generated code "real-time".  The function rt_OneStep is
 * always associated with the base rate of the model.  Subrates are managed
 * by the base rate from inside the generated code.  Enabling/disabling
 * interrupts and floating point context switches are target specific.  This
 * example code indicates where these should take place relative to executing
 * the generated code step function.  Overrun behavior should be tailored to
 * your application needs.  This example simply sets an error status in the
 * real-time model and returns from rt_OneStep.
 */
void rt_OneStep(void)
{
  /* Disable interrupts here */

  /* Check for overrun */
  if (OverrunFlag++) {
    rtmSetErrorStatus(Controller0_M, "Overrun");
    return;
  }

  /* Save FPU context here (if necessary) */
  /* Re-enable timer or interrupt here */
  /* Set model inputs here */

  /* Step the model */
  Controller0_step();

  /* Get model outputs here */

  /* Indicate task complete */
  OverrunFlag--;

  /* Disable interrupts here */
  /* Restore FPU context here (if necessary) */
  /* Enable interrupts here */
}

/* The example "main" function illustrates what is required by your
 * application code to initialize, execute, and terminate the generated code.
 * Attaching rt_OneStep to a real-time clock is target specific.  This example
 * illustates how you do this relative to initializing the model.
 */
int_T main(int_T argc, const char_T *argv[])
{
  /* Initialize model */
  Controller0_initialize();

  /* Attach rt_OneStep to a timer or interrupt service routine with
   * period 0.6 seconds (the model's base sample time) here.  The
   * call syntax for rt_OneStep is
   *
   *  rt_OneStep();
   */
  //printf("Warning: The simulation will run forever.\n");
 //fflush (NULL);

  Controller0_U.x[0] = 0.2;
  Controller0_U.x[1] = 0.1;
  Controller0_U.x[2] = -0.1;
  Controller0_U.x[3] = 0.1;
  Controller0_U.x[4] = 0.2;
  // Compuation of control input Controller_Y.U
  Controller0_step();
  //printf("In/put to plant: %f \n",  Controller0_Y.U);

  return 0;
}

/* File trailer for Real-Time Workshop generated code.
 *
 * [EOF]
 */
