
#include <mc9s08dz60.h>

#include <Microplex_7X.h>
#include <lib.h>
#include <mscan.h>
#include <timer.h>

void
main(void)
{
    Microplex_7X_init();
    CAN_init(CAN_BR_500, CAN_FM_NONE, 0);

    print("testing");
    DO_HSD_1 = 1;
    for (;;) {
        __RESET_WATCHDOG();
    }
}