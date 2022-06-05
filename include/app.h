/** @file
 *
 * Application framework
 */

#pragma ONCE

#include <stdbool.h>

#include <pt.h>
#include <HAL/_can.h>

typedef struct {
    void (*func)(struct pt *pt);
    struct pt pt;
} app_thread_t;

/**
 * Application thread list.
 *
 * Threads are called each time around the main loop; threads should
 * yield rather than busy-waiting in order to avoid blocking CAN
 * message handling.
 *
 * List must be terminated by an entry with a NULL @p func field.
 */
extern app_thread_t app_thread_table[];

/**
 * Application initialisation hook.
 *
 * Called once at application startup. Should initialize the
 * HAL by calling @p HAL_init() as well as performing app logic
 * initialisation.
 */
extern void app_init(void);

/**
 * Application CAN filter.
 *
 * Called at interrupt time when a message is received.
 *
 * To support MRS ROM messages, the filter should call
 * @p mrs_bootrom_filter() and return true if it does.
 *
 * TBD: list other library CAN filters here.
 *
 * @param id        CAN message id.
 * @return          true if the message should be queued for processing,
 *                  false if it has been handled or should be discarded.
 */
extern bool app_can_filter(uint32_t id);

/**
 * Application CAN receive callback.
 *
 * Called by the CAN listener thread when a message is received.
 * ROM protocol messages are handled internally and will not be
 * presented to this callback.
 *
 * To support MRS ROM messages, the handler should pass the message to
 * @p mrs_bootrom_rx() and ignore the message if it returns true.
 *
 * TBD: list other library CAN handlers here.
 *
 * @param buf       CAN message.
 */
extern void app_can_receive(const HAL_CAN_message_t *buf);

/**
 * Application CAN timeout.
 *
 * Called on the main loop when CAN traffic either stops or starts.
 *
 * Note that traffic filtered by hardware will be ignored, but messages
 * rejected by @p app_can_filter are counted as activity.
 *
 * @bool is_idle        true if the CAN bus has gone idle, false
 *                      if traffic has resumed.
 */
extern void app_can_idle(bool is_idle);
