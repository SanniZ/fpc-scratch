/*
 * FPC Fingerprint sensor device driver
 *
 * This driver will control the platform resources that the FPC fingerprint
 * sensor needs to operate. The major things are probing the sensor to check
 * that it is actually connected and let the Kernel know this and with that also
 * enabling and disabling of regulators, enabling and disabling of platform
 * clocks, controlling GPIOs such as SPI chip select, sensor reset line, sensor
 * IRQ line, MISO and MOSI lines.
 *
 * The driver will expose most of its available functionality in sysfs which
 * enables dynamic control of these features from eg. a user space process.
 *
 * The sensor's IRQ events will be pushed to Kernel's event handling system and
 * are exposed in the drivers event node. This makes it possible for a user
 * space process to poll the input node and receive IRQ events easily. Usually
 * this node is available under /dev/input/eventX where 'X' is a number given by
 * the event system. A user space process will need to traverse all the event
 * nodes and ask for its parent's name (through EVIOCGNAME) which should match
 * the value in device tree named input-device-name.
 *
 * This driver will NOT send any SPI commands to the sensor it only controls the
 * electrical parts.
 *
 *
 * Copyright (c) 2015 Fingerprint Cards AB <tech@fingerprints.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License Version 2
 * as published by the Free Software Foundation.
 */

#include <linux/clk.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/pm_wakeup.h>
#include <linux/pm_runtime.h>
#include <linux/wakelock.h>
#include <linux/platform_device.h>
#include "fpc_irq.h"

static void gpio_set(unsigned gpio, int val)
{
	gpio_set_value(gpio, val);
}

static int gpio_get(unsigned gpio)
{
	return gpio_get_value(gpio);
}

void spi_enable_clk(void)
{
	/* In X86 platform, the clock is embedded in SPI controller.
	 * Hence, no need enable clock here
	 */
	return;
}

void spi_disable_clk(void)
{
	/* In X86 platform, the clock is embedded in SPI controller.
	 * Hence, no need disable clock here
	 */
	return;
}

static ssize_t clk_enable_set(struct fpc_data *fpc, const char *buf, size_t count)
{
	if (buf[0] == '1')
		spi_enable_clk();
	else if (buf[0] == '0')
		spi_disable_clk();
	return count;
}

static void irq_handler(int irq, struct fpc_data *fpc)
{
	struct device *dev = fpc->dev;
	static int current_level = 0; // We assume low level from start
	current_level = !current_level;

	dev_dbg(dev, "FPC sensor IRQ generated\n");
}

static int configure(struct fpc_data *fpc, int *irq_num, int *irq_trig_flags)
{
	struct device *dev = fpc->dev;
	int rc = 0;

	dev_err(dev, "%s\n", __func__);
	*irq_num = gpio_to_irq(fpc->irq_gpio);
	*irq_trig_flags = IRQF_TRIGGER_RISING;

	return rc;
}

static int init(struct fpc_data *fpc)
{
	struct device *dev = fpc->dev;

	return 0;
}

static struct fpc_gpio_info drv_ops = {
	.init = init,
	.configure = configure,
	.get_val = gpio_get,
	.set_val = gpio_set,
	.clk_enable_set = clk_enable_set,
	.irq_handler = irq_handler,
};

static struct of_device_id drv_of_match[] = {
	{ .compatible = "fpc,fpc_irq", },
	{}
};
MODULE_DEVICE_TABLE(of, drv_of_match);

static int drv_probe(struct platform_device *pldev)
{
	int rc;

	rc = fpc_probe(pldev, &drv_ops);

	return rc;
}

static struct platform_driver driver = {
	.driver = {
		.name	= "fpc_irq",
		.owner	= THIS_MODULE,
		.of_match_table = drv_of_match,
	},
	.probe	= drv_probe,
	.remove	= fpc_remove
};

void fpc1020_dev_release(struct device *dev)
{
	dev_dbg(dev, "%s() is called()\n", __func__);
	return;
}

static struct device_node fpc1020_node = {
	.name = "fpc_irq",
};

static struct platform_device fpc1020_device = {
	.name = "fpc_irq",
	.id   = -1,
	.num_resources = 0,
	.dev = {
		.release = fpc1020_dev_release,
		.of_node = &fpc1020_node,
	},
};

static struct platform_device *devices[] __initdata = {
	&fpc1020_device,
};

static int __init fpc_init(void)
{
	int rval;

	rval = platform_add_devices(devices, ARRAY_SIZE(devices));
	if (rval)
		return rval;

	return platform_driver_register(&driver);
}

static void __exit fpc_exit(void)
{
	platform_driver_unregister(&driver);
	platform_device_unregister(&fpc1020_device);
}

module_init(fpc_init);
module_exit(fpc_exit);

MODULE_LICENSE("GPL");
