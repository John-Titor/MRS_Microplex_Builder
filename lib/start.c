//
// Stub startup code
//
// This lets us allocate vector #0 (reset) in a tidy fashion, and
// supply a default vector (#32) to be patched in by the flash tool.
//

extern void _Startup(void);     // from start08.c in MCU library code

#pragma CODE_SEG .init
#pragma NO_FRAME
#pragma NO_EXIT
void 
__interrupt 0
__start(void)
{
    __asm   jmp _Startup;
}

#pragma CODE_SEG DEFAULT
static void
__interrupt 32
__default_vector(void)
{
    // spin and wait for the watchdog
    for (;;);
}