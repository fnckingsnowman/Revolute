/*
 * Copyright (c) 2016 Intel Corporation.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/usb/usb_device.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <stdio.h>
#include <zephyr/init.h> 
#include <zephyr/usb/class/usb_hid.h>


#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>
#include <zephyr/usb/class/usb_cdc.h>

static K_SEM_DEFINE(usb_sem, 1, 1);	/* starts off "available" */
static const uint8_t hid_kbd_report_desc[] = HID_KEYBOARD_REPORT_DESC();

BUILD_ASSERT(DT_NODE_HAS_COMPAT(DT_CHOSEN(zephyr_console), zephyr_cdc_acm_uart),
         "Console device is not ACM CDC UART device");

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
USBD_CONFIGURATION_DEFINE(config_1,
              USB_SCD_SELF_POWERED,
              200);

USBD_DESC_LANG_DEFINE(sample_lang);
USBD_DESC_STRING_DEFINE(sample_mfr, "ZEPHYR", 1);
USBD_DESC_STRING_DEFINE(sample_product, "Zephyr USBD ACM console", 2);
USBD_DESC_STRING_DEFINE(sample_sn, "0123456789ABCDEF", 3);

USBD_DEVICE_DEFINE(sample_usbd,
           DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
           0x2fe3, 0x0001);

static int enable_usb_device_next(void)
{
    int err;

    err = usbd_add_descriptor(&sample_usbd, &sample_lang);
    if (err) {
        return err;
    }

    err = usbd_add_descriptor(&sample_usbd, &sample_mfr);
    if (err) {
        return err;
    }

    err = usbd_add_descriptor(&sample_usbd, &sample_product);
    if (err) {
        return err;
    }

    err = usbd_add_descriptor(&sample_usbd, &sample_sn);
    if (err) {
        return err;
    }

    err = usbd_add_configuration(&sample_usbd, &config_1);
    if (err) {
        return err;
    }

    err = usbd_register_class(&sample_usbd, "cdc_acm_0", 1);
    if (err) {
        return err;
    }

    err = usbd_init(&sample_usbd);
    if (err) {
        return err;
    }

    err = usbd_enable(&sample_usbd);
    if (err) {
        return err;
    }

    return 0;
}
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK_NEXT) */

void print_as5600_value(const struct device *dev)
{
    int ret;
    struct sensor_value rot_raw;
    
    
    ret = sensor_sample_fetch_chan(dev,SENSOR_CHAN_ROTATION);
    if (ret != 0){
            printk("ono dis not good, ur err code is :,%d\n", ret);
        }
    sensor_channel_get(dev,SENSOR_CHAN_ROTATION, &rot_raw);
    
    printk("angle: %d %d\n ", rot_raw.val1, rot_raw.val2);
    // printk("%d",rot_raw.val1)
}
static void in_ready_cb(const struct device *dev)
{
	ARG_UNUSED(dev);

	k_sem_give(&usb_sem);
}
static const struct hid_ops ops = {
	.int_in_ready = in_ready_cb,
};

static void status_cb(enum usb_dc_status_code status, const uint8_t *param)
{
	printk("Status %d", status);
}

int main(void)
{

// const struct device *i2c_dev =
// DEVICE_DT_GET(DT_NODELABEL(i2c0));

//static const struct device *bme_1 = DEVICE_DT_GET(DT_NODELABEL(bme688_1));
//const struct device *dek = DEVICE_DT_GET_ANY(ams_as5600);
//const struct device *const dek = DEVICE_DT_GET(DT_NODELABEL(ams_as5600));



//const struct i2c_dt_spec *i2c_port I2C_DT_SPEC_GET(),
//static const struct i2c_dt_spec i2c_port = I2C_DT_SPEC_GET(I2C0_NODE);


	const struct device *const as = DEVICE_DT_GET(DT_INST(0,ams_as5600));
    const struct device *const dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_console));
	const struct device *const hid0_dev = device_get_binding("HID_0");

    uint32_t dtr = 0;

#if defined(CONFIG_USB_DEVICE_STACK_NEXT)
    if (enable_usb_device_next()) {
        return 0;
    }
#else
    if (usb_enable(NULL)) {
        return 0;
    }
#endif

    /* Poll if the DTR flag was set */
    while (!dtr) {
        uart_line_ctrl_get(dev, UART_LINE_CTRL_DTR, &dtr);
        /* Give CPU resources to low priority threads. */
        k_sleep(K_MSEC(100));
    }

    if (as == NULL || !device_is_ready(as)) {
        printk("\nono bad stuff sad no device tree\n");
        return;
    }

    
    printk("device is %p, name is %s\n", as, as->name);

    usb_hid_register_device(hid0_dev, hid_kbd_report_desc,
				sizeof(hid_kbd_report_desc), &ops);

	

      

    while (1) {
    
        printk("Hello World! %s\n", CONFIG_ARCH);
        k_sleep(K_MSEC(100));
        
        print_as5600_value(as);
        //const struct as5600_dev_cfg_usr *dev_cfg = dev->config;
        uint8_t rep[] = {0x00, 0x00, 0x00, 0x00,
						      0x00, 0x00, 0x00, 0x00};

		rep[7] = HID_KEY_Z;

		hid_int_ep_write(hid0_dev,rep,sizeof(rep), NULL);
        //printk("Device %d\n", yes);
        //printk("Device %s\n", &dev_cfg->i2c_port);
        


    
    }
}
