/*

A part of the Bound-T test program tp_bro_int.
This is the example program in the Bound-T brochure.

$Id: routines.c,v 1.1 2007/06/14 11:04:31 niklas Exp $
*/


#include "types.h"
#include "routines.h"


void Count25 (uint *x)
{
   count_t u = 25;

   for (; u > 0; u -= 2)
   {
      *x = *x + u;
   }
}


void Count (count_t u, uint *x)
{
   for (; u > 0; u -= 2)
   {
      *x = *x + u;
   }
}


void Foo (count_t num, uint *x)
{
   Count (num + 3, x);
   /* The loop in Count depends on Count.u = num + 3. */
}


void Foo7 (uint *x)
{
   *x = *x + 10;

   Count (7, x);
   /* The loop in Count depends on Count.u = 7. */

   *x = *x - 8;
}


void Solve (uint *x)
{
   count_t i;
   volatile count_t k;

   for (i = 0; i < 8; i++)
   /* The bounds on this loop are static. */
   {
      k = Ones (*x);
      /* k is now the number of '1' bits in *x.          */
      /* This would be quite hard to analyse statically. */

      if (k == 0) break;
      /* This can make the for-loop stop before its full    */
      /* number of iterations. Bound-T uses the full number */
      /* for the Worst Case Time.                           */

      Count (k, x);
      /* The loop in Count depends on Count.u = k, which is  */
      /* hard to analyse statically. An assertion is needed. */
   }
}


count_t Ones (uint x)
{
   count_t v = 0;

   while (x)
   /* This is not a 'counter' loop, so Bound-T cannot find */
   /* its bounds automatically. An assertion is needed.    */
   {
      if (x & 1) v ++;
      x >>= 1;
   }

   return v;
}
