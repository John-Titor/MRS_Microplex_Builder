#include <mc9s08dz60.h>
#include <HAL/_reset.h>

#pragma NO_EXIT
#pragma NO_RETURN
#pragma NO_FRAME
void
HAL_reset(void)
{
    /* reset immediately */
    __asm DCW 0x9e00;
}

HAL_reset_reason_t
HAL_reset_reason(void)
{
    if (SRS_POR) {
        return HAL_RESET_POWER_ON;
    }

    if (SRS_PIN) {
        return HAL_RESET_RESET;
    }

    if (SRS_COP) {
        return HAL_RESET_WATCHDOG;
    }

    if (SRS_ILOP || SRS_ILAD) {
        return HAL_RESET_ILLEGAL;
    }

    if (SRS_LOC) {
        return HAL_RESET_CLOCK;
    }

    if (SRS_LVD) {
        return HAL_RESET_LVD;
    }

    return HAL_RESET_UNKNOWN;
}