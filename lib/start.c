/*
 * Stub startup code
 *
 * This lets us allocate vector #0 (reset) in a tidy fashion, and
 * supply a default vector (#32) to be patched in by the flash tool.
 */

#include <stddef.h>
#include <mc9s08dz60.h>
#include <app.h>
#include <pt.h>
#include <HAL/_can.h>

/*
 * Startup trampoline.
 */
extern void _Startup(void);     /* from start08.c in MCU library code */

#pragma CODE_SEG .init
#pragma NO_FRAME
#pragma NO_EXIT
void
__interrupt VectorNumber_Vreset
__start(void)
{
    __asm   jmp _Startup;
}

/*
 * Supply a 'default' vector for the s-record patcher.
 * It will patch this value into any jump table slot that doesn't have
 * a vector explicitly set.
 */
#pragma CODE_SEG DEFAULT
static void
__interrupt 32
__default_vector(void)
{
    /* spin and wait for the watchdog */
    for (;;);
}

/*
 * Main application loop.
 */
void
main(void)
{
    /* do app and HAL init */
    app_init();

    for (;;) {
        __RESET_WATCHDOG();

        /* run the CAN listener thread */
        PT_RUN(_HAL_can_listen);

        /* run app thread(s) */
        PT_RUN(app_main);
    }
}
