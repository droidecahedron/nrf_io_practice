/*
 * Copyright (c) 2016 Open-RnD Sp. z o.o.
 * Copyright (c) 2020 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/util.h>
#include <zephyr/sys/printk.h>
#include <inttypes.h>

#include "inc/io.h"

#define SLEEP_TIME_MS	10

extern struct gpio_dt_spec led;
extern const struct gpio_dt_spec button;

int main(void)
{
	init_gpio();
	
	while (1)
	{
		k_msleep(SLEEP_TIME_MS);
	}

	return 0;
}
