/*
 * Approximation of the standard boolean type.
 *
 * Use with caution; never compare with true/false.
 */

#ifndef _STDBOOL_H
#define _STDBOOL_H

typedef unsigned char   bool;
#define false           ((bool)0)
#define true            (!false)

#endif // _STDBOOL_H