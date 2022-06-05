
#include <stdbool.h>
#include <stddef.h>

#include <mc9s08dz60.h>

#include <app.h>
#include <lib.h>
#include <pt.h>
#include <HAL/7X.h>

static void _main_thread(struct pt *pt);

app_thread_t    app_thread_table[] = {
    { _main_thread },
    { NULL }
};

void
app_init(void)
{
    HAL_init();
    print("start");

    HAL_pin_set_duty(OUT_2, 50);
}

bool
app_can_filter(uint32_t id)
{
    // check whether the bootrom code wants this message
    if (MRS_bootrom_filter(id)) {
        return true;
    }
    return false;
}

void
app_can_receive(const HAL_CAN_message_t *msg)
{
    // let the bootrom code look at this message
    if (MRS_bootrom_rx(msg)) {
        return;
    }
}

void
app_can_idle(bool is_idle)
{
    (void)is_idle;
}

static void
_main_thread(struct pt *pt)
{
    static HAL_timer_t t;

    pt_begin(pt);
    HAL_timer_register(t);
    HAL_timer_reset(t, 1000);

    print("main");
    for (;;) {
        if (HAL_timer_expired(t)) {
            HAL_timer_reset(t, 1000);
            print("tick");
        } else {
//            print("tock %lu", HAL_timer_us());
        }
        pt_yield(pt);
    }
    pt_end(pt);
}