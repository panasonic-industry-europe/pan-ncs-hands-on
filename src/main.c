/*
 * Copyright (c) 2022, Panasonic Industrial Devices Europe GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/printk.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#define BT_GAP_ADV_BEACON_INT_MIN                 0x0640 * 4  /* 1 s      */
#define BT_GAP_ADV_BEACON_INT_MAX                 0x0780 * 4  /* 1.2 s    */

#define BT_LE_ADV_NCONN_BEACON BT_LE_ADV_PARAM(0, BT_GAP_ADV_BEACON_INT_MIN, BT_GAP_ADV_BEACON_INT_MAX, NULL)

/*
 * Set iBeacon demo advertisement data. These values are for
 * demonstration only and must be changed for production environments!
 *
 * UUID:  18ee1516-016b-4bec-ad96-bcb96d166e97
 * Major: 0
 * Minor: 0
 * RSSI:  -56 dBm
 */
static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_NO_BREDR),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, 0xa8, 0x58, 0x0b, 0x1f, 0x43, 0x02, 0x7d, 0xab, 0xfe, 0x4f, 0xf1, 0x64, 0x6f, 0x53, 0xad, 0x61),
	BT_DATA_BYTES(BT_DATA_MANUFACTURER_DATA,
		      0x3a, 0x00, /* Panasonic */)
};

static void bt_ready(int err)
{
	if (err) {
		return;
	}

	/* Start advertising */
	err = bt_le_adv_start(BT_LE_ADV_NCONN_BEACON, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		return;
	}
}

void main(void)
{
	int err;

	/* Initialize the Bluetooth Subsystem */
	err = bt_enable(bt_ready);
}
