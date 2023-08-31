#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <inttypes.h>
#include <soc.h>

#include <zephyr/logging/log.h>
// #include <zephyr/sys/printk.h>

#include "inc/io.h"

LOG_MODULE_REGISTER(IO_C);

//--- Blinky Thread Defines
#define BLINKYTHREAD_PRIORITY 3
#define STACKSIZE 256

#define BLINKY_SLEEP_FAST 100
#define BLINKY_SLEEP_SLOW 250

//--- DK GPIO Defines
#define BUTTONS_NODE DT_PATH(buttons)
#define LEDS_NODE DT_PATH(leds)

#define GPIO0_DEV DEVICE_DT_GET(DT_NODELABEL(gpio0))
#define GPIO1_DEV DEVICE_DT_GET_OR_NULL(DT_NODELABEL(gpio1))

/* GPIO0, GPIO1 and GPIO expander devices require different interrupt flags. */
#define FLAGS_GPIO_0_1_ACTIVE GPIO_INT_LEVEL_ACTIVE
#define FLAGS_GPIO_EXP_ACTIVE (GPIO_INT_EDGE | GPIO_INT_HIGH_1 | GPIO_INT_LOW_0 | GPIO_INT_ENABLE)

static const struct gpio_dt_spec buttons[] = {
#if DT_NODE_EXISTS(BUTTONS_NODE)
    DT_FOREACH_CHILD(BUTTONS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static const struct gpio_dt_spec leds[] = {
#if DT_NODE_EXISTS(LEDS_NODE)
    DT_FOREACH_CHILD(LEDS_NODE, GPIO_SPEC_AND_COMMA)
#endif
};

static struct gpio_callback button_callback;

// --- app logic
volatile bool dir = true;    // true = forward, false = reverse
volatile bool speed = false; // true = fast, false = slow.
uint8_t cnt = 0;             // led position

// --- Timer for double press detection
uint8_t clicks = 0; // sentinel var for timer logic
K_TIMER_DEFINE(click_timer, NULL, NULL);

void button_pressed(const struct device *dev, struct gpio_callback *cb,
                    uint32_t pins)
{
    LOG_INF("Button pressed at %" PRIu32, k_cycle_get_32());
    LOG_INF("pins var: %d", pins);

    switch (pins)
    {
    case dk_button1_msk: // 11 is button0's gpio pin# in the device tree source (.dts)
        LOG_INF("BUTTON1");
        if (clicks == 0)
        {
            clicks++;
            /* start one shot timer that expires after 1000 ms */
            k_timer_start(&click_timer, K_MSEC(1000), K_NO_WAIT);
        }
        else
        {
            /* check timer status */
            if (k_timer_status_get(&click_timer) > 0)
            {
                /* timer has expired */
                LOG_INF("Took you a while to press the second time.");
            }
            else if (k_timer_remaining_get(&click_timer) == 0)
            {
                /* timer was stopped (by someone else) before expiring */
            }
            else
            {
                /* timer is still running */
                speed = !speed; // double click = speed change
            }
            clicks = 0;
        }
        break;

    case dk_button2_msk:
        LOG_INF("BUTTON2");

        // clear LEDs
        for (size_t led_idx = 0; led_idx < ARRAY_SIZE(leds); led_idx++)
        {
            gpio_pin_set_dt(&leds[led_idx], 0);
        }
        // reverse direction, set countdown or up start val.
        dir = !dir;
        (dir) ? (cnt = 0) : (cnt = ARRAY_SIZE(leds) - 1);
        break;

    case dk_button3_msk:
        LOG_INF("BUTTON3");
        break;

    case dk_button4_msk:
        LOG_INF("BUTTON4");
        break;

    default:
        LOG_INF("?");
    }
}

//! (Note: you can do this in a loop and in a prettier way.)
// you can peep dk_buttons_and_leds.h and .c to see. this is not very dry.
int init_gpio(void)
{
    int err;

    //--- LEDs
    for (size_t i = 0; i < ARRAY_SIZE(leds); i++)
    {
        err = gpio_pin_configure_dt(&leds[i], GPIO_OUTPUT);
        if (err)
        {
            LOG_ERR("Cannot configure LED gpio");
            return err;
        }
    }

    //--- Buttons
    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        /* Enable pull resistor towards the inactive voltage. */
        gpio_flags_t flags =
            buttons[i].dt_flags & GPIO_ACTIVE_LOW ? GPIO_PULL_UP : GPIO_PULL_DOWN;
        err = gpio_pin_configure_dt(&buttons[i], GPIO_INPUT | flags);

        if (err)
        {
            LOG_ERR("Cannot configure button gpio");
            return err;
        }
    }

    uint32_t pin_mask = 0;

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        err = gpio_pin_interrupt_configure_dt(&buttons[i],
                                              GPIO_INT_EDGE_TO_ACTIVE);
        if (err)
        {
            LOG_ERR("Cannot disable callbacks()");
            return err;
        }

        pin_mask |= BIT(buttons[i].pin);
    }

    gpio_init_callback(&button_callback, button_pressed, pin_mask);

    for (size_t i = 0; i < ARRAY_SIZE(buttons); i++)
    {
        err = gpio_add_callback(buttons[i].port, &button_callback);
        if (err)
        {
            LOG_ERR("Cannot add callback");
            return err;
        }
    }

    // log infodump
    LOG_INF("GPIO initialized");
    for (size_t idx = 0; idx < ARRAY_SIZE(buttons); idx++)
    {
        LOG_INF("Set up button%d at %s pin%d",
                idx, buttons[idx].port->name, buttons[idx].pin);
    }
    for (size_t idx = 0; idx < ARRAY_SIZE(leds); idx++)
    {
        LOG_INF("Set up led%d at %s pin%d",
                idx, leds[idx].port->name, leds[idx].pin);
    }

    // helpful to translate pins var
    LOG_INF("1 << 11 = %d, 1 << 12 = %d", (1 << 11), (1 << 12));
    LOG_INF("1 << 24 = %d, 1 << 25 = %d", (1 << 24), (1 << 25));
    return err;
}

void blinkythread(void)
{
    while (1)
    {
        // 1->2->4->3 deliberate, looks better on DK
        switch (cnt & 0x3)
        {
        case 0:
            gpio_pin_toggle_dt(&leds[0]);
            break;
        case 1:
            gpio_pin_toggle_dt(&leds[1]);
            break;
        case 2:
            gpio_pin_toggle_dt(&leds[3]);
            break;
        case 3:
            gpio_pin_toggle_dt(&leds[2]);
            break;
        }
        (dir) ? cnt++ : cnt--;
        (speed) ? k_msleep(BLINKY_SLEEP_FAST) : k_msleep(BLINKY_SLEEP_SLOW);
    }
}

K_THREAD_DEFINE(blinkythread_id, STACKSIZE, blinkythread, NULL, NULL, NULL, BLINKYTHREAD_PRIORITY, 0, 0);
