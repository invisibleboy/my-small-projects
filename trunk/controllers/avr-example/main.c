/*

Main function for Bound-T test program tp_bro_int.

This is the example program in the Bound-T brochure.

This main() function contains mainly calls to functions.

$Id: main.c,v 1.1 2007/06/14 11:04:30 niklas Exp $
*/


#include "types.h"
#include "routines.h" 


int main (void)
{
   uint x = 1;

   Count25 (&x);
   /* This can be bounded automatically. */

   Foo7 (&x);
   /* This can be bounded automatically because Foo7 calls */
   /* Count with a static value for Count.u.               */

   Foo (6, &x);
   /* This can be bounded automatically, because Foo.num = 6,  */
   /* which makes Count.u = 9, which bounds the loop in Count. */

   Solve (&x);
   /* The loop in Solve can be bounded automatically, but not */
   /* the loop in Count when called from Solve.               */

   while (1) x++;
   /* This is a simple eternal loop. Bound-T will report it */
   /* and include one execution in the WCET.                */

}
