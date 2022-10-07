/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <bluetooth/services/nus.h>

static struct sensor_value sensor_values[9];

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN	(sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_NUS_VAL),
};

static struct bt_conn *current_conn = NULL;

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		printk("Connection failed (err %u)", err);
		return;
	}
	printk("Connected");
	current_conn = bt_conn_ref(conn);
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	printk("Disconnected (reason %u)", reason);
	bt_conn_unref(current_conn);
	current_conn = NULL;
}

static struct bt_conn_cb conn_callbacks = {
	.connected = connected,
	.disconnected = disconnected
};

void main(void)
{
	/*Databuffer for nus output message*/
	uint8_t sendBuffer[200];
	uint16_t messageLength = 0;

	printk("Hello World! %s\n", CONFIG_BOARD);

	const struct device *const dev = DEVICE_DT_GET_ONE(panasonic_sngcja5);

	bt_conn_cb_register(&conn_callbacks);

	bt_enable(NULL);

	bt_nus_init(NULL);

	bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

	while (42)
	{
		sensor_sample_fetch(dev);
		sensor_channel_get(dev, SENSOR_CHAN_ALL, &sensor_values);

		memset(sendBuffer, 0, sizeof(sendBuffer));

		messageLength = sprintf( sendBuffer, "PM1.0:%i.%i;PM2.5:%i.%i;PM10:%i.%i", \
						sensor_values[6].val1, sensor_values[6].val2,				\
						sensor_values[7].val1, sensor_values[7].val2,				\
						sensor_values[8].val1, sensor_values[8].val2);

		bt_nus_send(current_conn, &sendBuffer[0], messageLength);

		printk("pc0_5: %i.%i\n", sensor_values[0].val1, sensor_values[0].val2);
		printk("pc1_0: %i.%i\n", sensor_values[1].val1, sensor_values[1].val2);
		printk("pc2_5: %i.%i\n", sensor_values[2].val1, sensor_values[2].val2);
		printk("pc5_0: %i.%i\n", sensor_values[3].val1, sensor_values[3].val2);
		printk("pc7_5: %i.%i\n", sensor_values[4].val1, sensor_values[4].val2);
		printk("pc10_0: %i.%i\n", sensor_values[5].val1, sensor_values[5].val2);
		printk("pm1_0: %i.%i\n", sensor_values[6].val1, sensor_values[6].val2);
		printk("pm2_5: %i.%i\n", sensor_values[7].val1, sensor_values[7].val2);
		printk("pm10_0: %i.%i\n", sensor_values[8].val1, sensor_values[8].val2);

		k_msleep(1000);
	}
	
}
