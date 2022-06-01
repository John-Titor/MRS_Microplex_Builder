/*
 * Approximation of the standard boolean type.
 *
 * Use with caution; never compare with true/false.
 */

#pragma ONCE

typedef unsigned char   bool;
#define false           ((bool)0)
#define true            (!false)
