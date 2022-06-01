
#include <mc9s08dz60.h>

#include <HAL/7X.h>
#include <lib.h>

void
main(void)
{
    HAL_init();
    HAL_timer_init();
    HAL_CAN_init(HAL_CAN_BR_500, HAL_CAN_FM_NONE, 0);

    print("testing");
    DO_HSD_1 = 1;
    for (;;) {
        __RESET_WATCHDOG();
    }
}