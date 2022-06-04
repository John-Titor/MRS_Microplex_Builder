
#include <mc9s08dz60.h>

#include <HAL/7X.h>
#include <lib.h>

void
main(void)
{
    HAL_init();

    print("testing");
    HAL_pin_set(OUT_1, true);

    for (;;) {
        __RESET_WATCHDOG();
    }
}