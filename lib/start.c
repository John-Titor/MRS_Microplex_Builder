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
extern void _Startup(void);     // from start08.c in MCU library code

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
    static struct pt _can_listener;

    /* do app and HAL init */
    app_init();

    for (;;) {
        uint8_t i;

        __RESET_WATCHDOG();

        /* run the CAN listener thread */
        _HAL_can_listen(&_can_listener);

        /* run app threads */
        for (i = 0; app_thread_table[i].func != NULL; i++) {
            app_thread_table[i].func(&app_thread_table[i].pt);
        }
    }
}