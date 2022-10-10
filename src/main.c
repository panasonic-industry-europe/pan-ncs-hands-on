/*
 * Copyright (c) 2022, Panasonic Industrial Devices Europe GmbH
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/zephyr.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

static struct sensor_value sensor_values[9];

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define SERVICE_UUID 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
                     0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x75
#define CHARACTERISTIC_UUID_PM1_0 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
                                  0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x76
#define CHARACTERISTIC_UUID_PM2_5 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
                                  0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x77
#define CHARACTERISTIC_UUID_PM10_0 0xd4, 0x86, 0x48, 0x24, 0x54, 0xB3, 0x43, 0xA1, \
                                   0xBC, 0x20, 0x97, 0x8F, 0xC3, 0x76, 0xC2, 0x78

#define BT_UUID_SERVICE BT_UUID_DECLARE_128(SERVICE_UUID)
#define BT_UUID_SERVICE_PM1_0 BT_UUID_DECLARE_128(CHARACTERISTIC_UUID_PM1_0)
#define BT_UUID_SERVICE_PM2_5 BT_UUID_DECLARE_128(CHARACTERISTIC_UUID_PM2_5)
#define BT_UUID_SERVICE_PM10_0 BT_UUID_DECLARE_128(CHARACTERISTIC_UUID_PM10_0)

BT_GATT_SERVICE_DEFINE(service,
                       BT_GATT_PRIMARY_SERVICE(BT_UUID_SERVICE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_SERVICE_PM1_0, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, NULL, NULL, NULL),
                       BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_SERVICE_PM2_5, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, NULL, NULL, NULL),
                       BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
                       BT_GATT_CHARACTERISTIC(BT_UUID_SERVICE_PM10_0, BT_GATT_CHRC_NOTIFY, BT_GATT_PERM_READ, NULL, NULL, NULL),
                       BT_GATT_CCC(NULL, BT_GATT_PERM_READ | BT_GATT_PERM_WRITE), );

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL, SERVICE_UUID),
};

static struct bt_conn *current_conn = NULL;

static void connected(struct bt_conn *conn, uint8_t err)
{
    if (err)
    {
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
    .disconnected = disconnected};

static int send_data(struct bt_conn *conn, const void *data, uint16_t len, const struct bt_uuid *uuid)
{
    if(current_conn == NULL) {
        /* Not connected */
        return 0;
    }

    /* Look up the attribute data for later use. */
    const struct bt_gatt_attr *attr = bt_gatt_find_by_uuid(NULL, 0, uuid);
    if (!attr)
    {
        /* fatal error */
        printk("Fatal error, attribute not found");
        return -1;
    }

    /* Check whether notifications are enabled or not for this attribute */
    if (!bt_gatt_is_subscribed(conn, attr, BT_GATT_CCC_NOTIFY))
    {
        /* Do not signal an error to the upper layer if sending is not enabled. */
        return 0;
    }

    struct bt_gatt_notify_params params =
        {
            .uuid = uuid,
            .attr = attr,
            .data = data,
            .len = len,
            .func = NULL,
            .user_data = NULL,
        };

    /* Finally send the notification. */
    if (bt_gatt_notify_cb(conn, &params))
    {
        printk("bt_gatt_notify_cb() failed");
        return -1;
    }

    return 0;
}

void main(void)
{
    /*Databuffer for nus output message*/
    uint8_t sendBufferPm_1_0[20];
    uint8_t sendBufferPm_2_5[20];
    uint8_t sendBufferPm_10_0[20];
    uint16_t messageLengthPm_1_0 = 0;
    uint16_t messageLengthPm_2_5 = 0;
    uint16_t messageLengthPm_10_0 = 0;

    printk("Hello World! %s\n", CONFIG_BOARD);

    const struct device *const dev = DEVICE_DT_GET_ONE(panasonic_sngcja5);

    bt_conn_cb_register(&conn_callbacks);

    bt_enable(NULL);

    bt_le_adv_start(BT_LE_ADV_CONN, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));

    while (42)
    {
        sensor_sample_fetch(dev);
        sensor_channel_get(dev, SENSOR_CHAN_ALL, &sensor_values);

        messageLengthPm_1_0 = snprintf(sendBufferPm_1_0, sizeof(sendBufferPm_1_0), "%i.%i", sensor_values[6].val1, sensor_values[6].val2);
        messageLengthPm_2_5 = snprintf(sendBufferPm_2_5, sizeof(sendBufferPm_2_5), "%i.%i", sensor_values[6].val1, sensor_values[6].val2);
        messageLengthPm_10_0 = snprintf(sendBufferPm_10_0, sizeof(sendBufferPm_10_0), "%i.%i", sensor_values[6].val1, sensor_values[6].val2);

        send_data(current_conn, sendBufferPm_1_0, messageLengthPm_1_0, BT_UUID_SERVICE_PM1_0);
        send_data(current_conn, sendBufferPm_2_5, messageLengthPm_2_5, BT_UUID_SERVICE_PM2_5);
        send_data(current_conn, sendBufferPm_10_0, messageLengthPm_10_0, BT_UUID_SERVICE_PM10_0);

        printk("pm1_0: %i.%i\n", sensor_values[6].val1, sensor_values[6].val2);
        printk("pm2_5: %i.%i\n", sensor_values[7].val1, sensor_values[7].val2);
        printk("pm10_0: %i.%i\n", sensor_values[8].val1, sensor_values[8].val2);

        k_msleep(1000);
    }
}
