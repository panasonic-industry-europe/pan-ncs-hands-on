/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>

static struct sensor_value sensor_values[9];

void main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD);

	const struct device *const dev = DEVICE_DT_GET_ONE(panasonic_sngcja5);

	while (42)
	{
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_ALL, &sensor_values);

		printk("pc0_5: %i.%i\n", sensor_values[0].val1, sensor_values[0].val2);
		printk("pc1_0: %i.%i\n", sensor_values[1].val1, sensor_values[1].val2);
		printk("pc2_5: %i.%i\n", sensor_values[2].val1, sensor_values[2].val2);
		printk("pc5_0: %i.%i\n", sensor_values[3].val1, sensor_values[3].val2);
		printk("pc7_5: %i.%i\n", sensor_values[4].val1, sensor_values[4].val2);
		printk("pc10_0: %i.%i\n", sensor_values[5].val1, sensor_values[5].val2);
		printk("pm1_0: %i.%i\n", sensor_values[6].val1, sensor_values[6].val2);
		printk("pm2_5: %i.%i\n", sensor_values[7].val1, sensor_values[7].val2);
		printk("pm10_0: %i.%i\n", sensor_values[8].val1, sensor_values[8].val2);
	}
	
}
