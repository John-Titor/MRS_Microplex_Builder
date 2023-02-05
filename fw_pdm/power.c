/*
 * Master power switch logic.
 */

#include "defs.h"

/*
 * PlantUML

@startuml

state P_INIT
state P_INIT_DONE <<choice>>
state P_ON_WAIT
state P_ON
state P_OFF_WAIT
state P_OFF

[*] -[bold]> P_INIT

note right of P_INIT
 KL15 and KILL inputs need time for the ADC filters
 to stabilise. KILL pull-up must be on for this to
 work correctly.
 Turn on the LED so that we know the system is in 
 this state. It will either stay on, or flash if we
 go to P_OFF.
end note
P_INIT: entry / AI_3_PU = 1
P_INIT: entry / LED = 1
P_INIT: entry / T30 = 0
P_INIT: entry / T15 = 0
P_INIT -down[bold]-> P_INIT_DONE : timeout 100ms
note on link
 ADC sample buffer is 8 deep, 12 channels sampled 
 at 1ms intervals, so should settle in ~100ms.
end note

' powerup behaviour
P_INIT_DONE -> P_OFF : !KL15
P_INIT_DONE -[#red]> P_OFF : KILL
P_INIT_DONE -down[bold]-> P_ON_WAIT : KL15

note right of P_ON_WAIT
 Turn on T30 first, allow system to settle.
end note
P_ON_WAIT: entry / T30 = 1
P_ON_WAIT -> P_OFF : !KL15
P_ON_WAIT -[#red]> P_OFF : KILL
P_ON_WAIT -down[bold]-> P_ON : timeout 100ms

note right of P_ON
 Put system into "fully operational" state,
 remain here until killed or KL15 goes away.
end note
P_ON: entry / T15 = 1
P_ON: entry / system_run = true
P_ON: exit / system_run = false
P_ON -down[#red]-> P_OFF_WAIT : KILL
P_ON -down[bold]-> P_OFF_WAIT : !KL15

note right of P_OFF_WAIT
 Wait here until DDE turns off, but time
 out in the case where it doesn't.
end note
P_OFF_WAIT: entry / LED = 0
P_OFF_WAIT: entry / T15 = 0
P_OFF_WAIT -down> P_OFF : timeout 500ms
P_OFF_WAIT -down[bold]-> P_OFF : !S_BLOW

note right of P_OFF
 CAN transmission stops working when CAN_STB_N is cleared,
 so we must defer this until the system has stopped.
 Clearing DO_POWER allows the system to power off if/when
 KL15 is de-asserted.
 KL15 must be de-asserted to leave this state, i.e. power
 cycle is required.
end note
P_OFF: entry / T30 = 0
P_OFF: entry / DO_POWER = 0
P_OFF: entry / CAN_STB_N = 0
P_OFF: LED = flash
P_OFF -[bold]-> [*] : !KL15

state "???" as P_INVALID
P_INVALID -> P_OFF : Invalid state detected

note as N1
 **Bold** paths are the "normal" usage flow.
 <color:red>Red</color> paths are the "kill" flow.
 Other paths are exceptional cases.
end note

@enduml

*/

static enum {
    P_INIT,
    P_INIT_DONE,
    P_ON_WAIT,
    P_ON,
    P_OFF_WAIT,
    P_OFF
} power_state;

static HAL_timer_t power_timeout;

static void
enter_p_init(void)
{
    power_state = P_INIT;

    /* delay until ADC results stabilise */
    HAL_timer_reset(power_timeout, 100);

    /* enable the kill line pull-up */
    AI_3_PU = 1;

    /* ensure outputs are in the desired state */
    HAL_pin_set(OUT_POWER_LED, true);
    HAL_pin_set(OUT_T30, false);
    HAL_pin_set(OUT_T15, false);
}

static void
enter_p_init_done(void)
{
    power_state = P_INIT_DONE;
}

static void
enter_p_on_wait(void)
{
    power_state = P_ON_WAIT;

    /* delay after turning T30 on to allow system to settle */
    HAL_timer_reset(power_timeout, CONFIG_POWER_ON_DELAY);

    /* turn on main power */
    HAL_pin_set(OUT_T30, true);
}

static void
enter_p_on(void)
{
    power_state = P_ON;

    /* turn on T15, wake all modules in system */
    HAL_pin_set(OUT_T15, true);
}

static void
enter_p_off_wait(void)
{
    power_state = P_OFF_WAIT;

    /* set timeout - must turn off when this expires */
    HAL_timer_reset(power_timeout, CONFIG_POWER_OFF_DELAY);

    /* Turn off T15 and LED */
    HAL_pin_set(OUT_POWER_LED, false);
    HAL_pin_set(OUT_T15, false);
}

static void
enter_p_off(void)
{
    power_state = P_OFF;

    /* turn off main power */
    HAL_pin_set(OUT_T30, false);

    /* allow KL15 de-assertion to power down the system */
    DO_POWER = 0;

    /* turn off CAN tranceiver, disable CAN wakeup */
    CAN_STB_N = 0;
}

static void
do_p_off(void)
{
    static bool blink_state = false;

    if (HAL_timer_expired(power_timeout)) {
        HAL_timer_reset(power_timeout, CONFIG_POWER_BLINK_INTERVAL);
        HAL_pin_set(OUT_POWER_LED, blink_state);

        /* toggle CAN_STB_N in case we failed to go to sleep */
        CAN_STB_N = blink_state;

        /* make sure DO_POWER is clear */
        DO_POWER = 0;

        blink_state = !blink_state;
    }
}

bool
power_system_should_run(void)
{
    return power_state == P_ON;
}

PT_DEFINE(power)
{
    pt_begin(pt);
    HAL_timer_register(power_timeout);

    enter_p_init();

    for (;;) {
        bool kl15 = (HAL_pin_get_mV(KL15) > CONFIG_POWER_OFF_THRESHOLD);
        bool kill = (HAL_pin_get_mV(IN_KILL) > CONFIG_POWER_OFF_THRESHOLD);
        bool s_blow = (HAL_pin_get_mV(IN_S_BLOW) < CONFIG_DDE_SWITCH_MV);

        switch (power_state) {
        case P_INIT:
            if (HAL_timer_expired(power_timeout)) {
                enter_p_init_done();
            }
            break;

        case P_INIT_DONE:
            if (kill) {
                enter_p_off();
            } else if (!kl15) {
                enter_p_off();
            } else {
                enter_p_on_wait();
            }
            break;

        case P_ON_WAIT:
            if (kill) {
                enter_p_off();
            } else if (HAL_timer_expired(power_timeout)) {
                enter_p_on();
            }
            break;

        case P_ON:
            if (kill || !kl15) {
                enter_p_off_wait();
            }
            break;

        case P_OFF_WAIT:
            if (!s_blow || HAL_timer_expired(power_timeout)) {
                enter_p_off();
            }
            break;

        case P_OFF:
            do_p_off();
            break;

        default:
            /* this is fatal */
            enter_p_off();
        }
        pt_yield(pt);
    }
    pt_end(pt);
}
