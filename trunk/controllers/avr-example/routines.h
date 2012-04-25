/*

A part of the Bound-T test program tp_bro_int.
This is the example program in the Bound-T brochure.

$Id: routines.h,v 1.1 2007/06/14 11:04:32 niklas Exp $
*/


#include "types.h"

extern void Count25 (uint *x);
/* Demonstrates a loop with static bounds. */

extern void Count (count_t u, uint *x);
/* Demonstrates a loop that depends on the parameter u. */

extern void Foo (count_t num, uint *x);
/* Demonstrates a call to Count that makes Count.u depend on */
/* Foo.num, so that the loop in Count depends on Foo.num.    */

extern void Foo7 (uint *x);
/* Demonstrates a call to Foo with a static value for Foo.num  */
/* which passes on to Count.u and lets Bounds-T bound the loop */
/* in Count.                                                   */

extern void Solve (uint *x);
/* Demonstrates a for-loop with static bounds.               */
/* Demonstrates bounding a call (on Count) using assertions. */

extern count_t Ones (uint x);
/* Returns the number of '1' bits in x.                      */
/* Demonstrates a 'while' loop for which Bound-T cannot find */
/* a loop-bound automatically.                               */
