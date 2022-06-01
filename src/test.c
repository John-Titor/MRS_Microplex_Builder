
#include <Microplex_7X.h>

void
main(void)
{
    Microplex_7X_init();

    DO_HSD_1 = 1;
    for (;;) {
        _FEED_COP();
    }
}