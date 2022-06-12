/** @file
 *
 * Reset reason and activation abstraction.
 */

#pragma ONCE

typedef enum {
    HAL_RESET_POWER_ON,
    HAL_RESET_RESET,
    HAL_RESET_WATCHDOG,
    HAL_RESET_ILLEGAL,
    HAL_RESET_CLOCK,
    HAL_RESET_LVD,
    HAL_RESET_UNKNOWN
} HAL_reset_reason_t;

extern void HAL_reset(void);
extern HAL_reset_reason_t HAL_reset_reason(void);
