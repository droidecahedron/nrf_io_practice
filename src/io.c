#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "inc/io.h"

#define BLINKYTHREAD_PRIORITY 3
#define STACKSIZE 256

#define button0_msk 1 << 11 // button0 is gpio pin 11 in .dts
#define button1_msk 1 << 12 // button1 is gpio pin 12 in the .dts
#define button2_msk 1 << 24 // button2 is gpio pin 24 in the .dts

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS(SW0_NODE, okay)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
#define SW1_NODE	DT_ALIAS(sw1)
#if !DT_NODE_HAS_STATUS(SW1_NODE, okay)
#error "Unsupported board: sw1 devicetree alias is not defined"
#endif
#define SW2_NODE	DT_ALIAS(sw2)
#if !DT_NODE_HAS_STATUS(SW2_NODE, okay)
#error "Unsupported board: sw2 devicetree alias is not defined"
#endif

static const struct gpio_dt_spec button0 = GPIO_DT_SPEC_GET_OR(SW0_NODE, gpios,
							      {0});
static const struct gpio_dt_spec button1 = GPIO_DT_SPEC_GET_OR(SW1_NODE, gpios,
							      {0});
static const struct gpio_dt_spec button2 = GPIO_DT_SPEC_GET_OR(SW2_NODE, gpios, 
								  {0});
static struct gpio_callback button_cb_data;

/*
 * The led0 devicetree alias is optional. If present, we'll use it
 * to turn on the LED whenever the button is pressed.
 */
static struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led0), gpios,
						     {0});
static struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led1), gpios,
						     {0});
static struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led2), gpios,
						     {0});
static struct gpio_dt_spec led3 = GPIO_DT_SPEC_GET_OR(DT_ALIAS(led3), gpios,
						     {0});                             


void button_pressed(const struct device *dev, struct gpio_callback *cb,
		    uint32_t pins)
{
	printk("Button pressed at %" PRIu32 "\n", k_cycle_get_32());
    printk("pins var: %d\n", pins);
	printk("1 << 11 = %d, 1 << 12 = %d, 1 << 24 = %d\n", (1 << 11), (1 << 12), (1 << 24));

    switch(pins)
    {
        case button0_msk: // 11 is button0's gpio pin# in the device tree source (.dts)
            printk("BUTTON0\n");
            break;

        case button1_msk:
            printk("BUTTON1\n");
            break;

		case button2_msk:
			printk("BUTTON2\n");
			break;

        default:
            printk("?");
    }
}

int init_gpio(void)
{
    int ret;

    /*
    !!!! BUTTONs !!!!
    */
   // Button0
    if (!gpio_is_ready_dt(&button0)) {
		printk("Error: button0 device %s is not ready\n",
		       button0.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button0, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button0.port->name, button0.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button0,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button0.port->name, button0.pin);
		return 0;
	}

    //! button1
    if (!gpio_is_ready_dt(&button1)) {
		printk("Error: button1 device %s is not ready\n",
		       button1.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button1, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button1.port->name, button1.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button1,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button1.port->name, button1.pin);
		return 0;
	}

	//! button2 (Note: you can do this in a loop and in a prettier way.)
	// you can peep dk_buttons_and_leds.h and .c to see.
	if (!gpio_is_ready_dt(&button2)) {
		printk("Error: button1 device %s is not ready\n",
		       button2.port->name);
		return 0;
	}

	ret = gpio_pin_configure_dt(&button2, GPIO_INPUT);
	if (ret != 0) {
		printk("Error %d: failed to configure %s pin %d\n",
		       ret, button2.port->name, button2.pin);
		return 0;
	}

	ret = gpio_pin_interrupt_configure_dt(&button2,
					      GPIO_INT_EDGE_TO_ACTIVE);
	if (ret != 0) {
		printk("Error %d: failed to configure interrupt on %s pin %d\n",
			ret, button2.port->name, button2.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button0.pin) | BIT(button1.pin) | BIT(button2.pin));
	gpio_add_callback(button0.port, &button_cb_data);
    gpio_add_callback(button1.port, &button_cb_data);
	gpio_add_callback(button2.port, &button_cb_data);
	printk("Set up button0 at %s pin %d\n", button0.port->name, button0.pin);
    printk("Set up button1 at %s pin %d\n", button1.port->name, button1.pin);
	printk("Set up button2 at %s pin %d\n", button2.port->name, button2.pin);

    /*
    !!!! LEDs !!!!
    */
    //LED0
	if (led0.port && !device_is_ready(led0.port)) {
		printk("Error %d: led0 device %s is not ready; ignoring it\n",
		       ret, led0.port->name);
		led0.port = NULL;
	}
	if (led0.port) {
		ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure led0 device %s pin %d\n",
			       ret, led0.port->name, led0.pin);
			led0.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led0.port->name, led0.pin);
		}
	}
    //LED1
    if (led1.port && !device_is_ready(led1.port)) {
		printk("Error %d: led1 device %s is not ready; ignoring it\n",
		       ret, led1.port->name);
		led1.port = NULL;
	}
	if (led1.port) {
		ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure led1 device %s pin %d\n",
			       ret, led1.port->name, led1.pin);
			led1.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led1.port->name, led1.pin);
		}
	}
    //LED2
    if (led2.port && !device_is_ready(led2.port)) {
		printk("Error %d: led2 device %s is not ready; ignoring it\n",
		       ret, led2.port->name);
		led2.port = NULL;
	}
	if (led2.port) {
		ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure led2 device %s pin %d\n",
			       ret, led2.port->name, led2.pin);
			led2.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led2.port->name, led2.pin);
		}
	}
    //LED3
    if (led3.port && !device_is_ready(led3.port)) {
		printk("Error %d: led3 device %s is not ready; ignoring it\n",
		       ret, led3.port->name);
		led3.port = NULL;
	}
	if (led3.port) {
		ret = gpio_pin_configure_dt(&led3, GPIO_OUTPUT);
		if (ret != 0) {
			printk("Error %d: failed to configure led3 device %s pin %d\n",
			       ret, led3.port->name, led3.pin);
			led3.port = NULL;
		} else {
			printk("Set up LED at %s pin %d\n", led3.port->name, led3.pin);
		}
	}

    return 1;
}

void blinkythread(void)
{
    /* If we have an LED, match its state to the button's. */
    int cnt = 0;
    while(1)
    {
        switch(cnt++ & 0x3)
        {
            case 0:
                gpio_pin_toggle_dt(&led0);
                break;
            case 1:
                gpio_pin_toggle_dt(&led1);
                break;
            case 2:
                gpio_pin_toggle_dt(&led3);
                break;
            case 3:
                gpio_pin_toggle_dt(&led2);
                break;
        }

        k_msleep(100);
    }
}


K_THREAD_DEFINE(blinkythread_id, STACKSIZE, blinkythread, NULL, NULL, NULL, BLINKYTHREAD_PRIORITY, 0, 0);