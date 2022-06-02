
#include <mc9s08dz60.h>

#include <HAL/7X.h>

void
main(void)
{
    HAL_init();

    print("testing");
    DO_HSD_1 = 1;
    for (;;) {
        __RESET_WATCHDOG();
    }
}