#include <stdio.h>
#include <mc9s08dz60.h>
#include <lib.h>
#include <HAL/_can.h>

void
print(const char *format, ...)
{
    va_list args;

    set_printf(HAL_can_putchar);
    va_start(args, format);
    (void)vprintf(format, args);
    va_end(args);
    HAL_can_putchar('\n');
}

void
printn(const char *format, ...)
{
    va_list args;

    set_printf(HAL_can_putchar);
    va_start(args, format);
    (void)vprintf(format, args);
}

void
hexdump(uint8_t *addr, unsigned int count)
{
    while (count) {
        uint8_t i;

        printn("%p:", addr);

        for (i = 0; count && (i < 16); i++) {
            printn(" %02x", *addr++);
            count--;
        }

        print("");
        __RESET_WATCHDOG();
    }
}

void
__require_abort(const char *file, int line)
{
    /* turn off interrupts, we are dead here */
    __asm SEI;

    print("ABORT: %s:%d", file, line);

    for (;;);
}

