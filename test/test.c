#include <stdbool.h>
#include <stddef.h>

#include <mc9s08dz60.h>

#include <app.h>
#include <blink_keypad.h>
#include <lib.h>
#include <pt.h>
#include <HAL/7X.h>

void
app_init(void)
{
    HAL_init();
    /*MRS_set_software_version(GIT_VERSION); */

    print("start %s", GIT_VERSION);

    HAL_pin_set_duty(OUT_2, 50);
}

bool
app_can_filter(uint32_t id)
{   
    if (MRS_bootrom_filter(id) ||   /* bootrom interested? */
        bk_can_filter(id)) {        /* blink keypad handler interested? */
        return true;
    }

    return false;                   /* not interested */
}

void
app_can_receive(const HAL_can_message_t *msg)
{
    if (MRS_bootrom_rx(msg) ||      /* handled by bootrom code? */
        bk_can_receive(msg)) {      /* handled by blink keypad? */
        return;
    }

    /* we could do something here */
}

void
app_can_idle(bool is_idle)
{
    (void)is_idle;
}

PT_DEFINE(app_main)
{
    static HAL_timer_t t;

    pt_begin(pt);
    HAL_timer_register(t);
    HAL_timer_reset(t, 1000);

    print("%s v%u #%lx", 
          MRS_parameters.Name,
          MRS_parameters.HardwareVersion,
          MRS_parameters.SerialNumber);
    {
        uint16_t v;
        v = HAL_eeprom_read16(0x3f0);
        print("0x3f0: %x", v);
        HAL_eeprom_write16(0x3f0, v + 1);
        v = HAL_eeprom_read16(0x3f0);
        print("0x3f0: %x", v);
    }

    for (;;) {
        if (HAL_timer_expired(t)) {
            HAL_timer_reset(t, 1000);
            print("tick");
        }
        PT_RUN(blink_keypad);
        pt_yield(pt);
    }
    pt_end(pt);
}
