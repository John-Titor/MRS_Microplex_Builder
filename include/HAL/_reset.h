/** @file
 * SoC reset and reset reason API.
 */

#pragma ONCE

/** Hardware reset reasons */
typedef enum {
    HAL_RESET_POWER_ON,     /**< module was powered on */
    HAL_RESET_RESET,        /**< module was internally reset */
    HAL_RESET_WATCHDOG,     /**< watchdog timer triggered reset */
    HAL_RESET_ILLEGAL,      /**< illegal instruction or address caused reset */
    HAL_RESET_CLOCK,        /**< clock oscillator failed */
    HAL_RESET_LVD,          /**< low-voltage dropout caused reset */
    HAL_RESET_UNKNOWN       /**< reset register contents invalid */
} HAL_reset_reason_t;

/**
 * Reset the system. Does not return
 */
extern void HAL_reset(void);

/**
 * Fetch the most recent reset reason.
 *
 * @return      The cause of the most recent reset.
 */
extern HAL_reset_reason_t HAL_reset_reason(void);
