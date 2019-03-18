#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/mutex.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/wakelock.h>
#include <linux/regulator/consumer.h>

#include "fpc_irq.h"

#define FPC_RESET_LOW_US 5000
#define FPC_RESET_HIGH1_US 100
#define FPC_RESET_HIGH2_US 5000

#define FPC_TTW_HOLD_TIME 1000
#define SUPPLY_1V8	1800000UL
#define SUPPLY_3V3	3300000UL
#define SUPPLY_TX_MIN	SUPPLY_3V3
#define SUPPLY_TX_MAX	SUPPLY_3V3
/* GPIO mapping should be implemented by
  * Device Tree or ACPI. In INTEL platform,
  * ACPI will be used.
  *
  * NOTE: IRQ and RST GPIO number will be
  * got manually at the first version. After ACPI
  * table has implemented description for FPC IRQ
  * and RST GPIO, then we will add code in fpc
  * driver to parse the ACPI table so as to get
  * the GPIO number.
  */
#define FPC_IRQ_GPIO_NO 434
#define FPC_RST_GPIO_NO 435

static int hw_reset(struct  fpc_data *fpc)
{
	int irq_gpio;
	struct device *dev = fpc->dev;

	fpc->hwabs->set_val(fpc->rst_gpio, 1);
	usleep_range(FPC_RESET_HIGH1_US, FPC_RESET_HIGH1_US + 100);

	fpc->hwabs->set_val(fpc->rst_gpio, 0);
	usleep_range(FPC_RESET_LOW_US, FPC_RESET_LOW_US + 100);

	fpc->hwabs->set_val(fpc->rst_gpio, 1);
	usleep_range(FPC_RESET_HIGH2_US, FPC_RESET_HIGH2_US + 100);

	irq_gpio = fpc->hwabs->get_val(fpc->irq_gpio);
	dev_err(dev, "IRQ after reset %d\n", irq_gpio);

	return 0;
}

static ssize_t hw_reset_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	int rc;
	struct  fpc_data *fpc = dev_get_drvdata(dev);

	if (!strncmp(buf, "reset", strlen("reset"))) {
		rc = hw_reset(fpc);
		return rc ? rc : count;
	}
	else
		return -EINVAL;


}
static DEVICE_ATTR(hw_reset, S_IWUSR, NULL, hw_reset_set);

/**
 * sysfs node for controlling whether the driver is allowed
 * to wake up the platform on interrupt.
 */
static ssize_t wakeup_enable_set(struct device *dev,
	struct device_attribute *attr, const char *buf, size_t count)
{
	struct  fpc_data *fpc = dev_get_drvdata(dev);

	if (!strncmp(buf, "enable", strlen("enable")))
	{
		fpc->wakeup_enabled = true;
		smp_wmb();
	}
	else if (!strncmp(buf, "disable", strlen("disable")))
	{
		fpc->wakeup_enabled = false;
		smp_wmb();
	}
	else
		return -EINVAL;

	return count;
}
static DEVICE_ATTR(wakeup_enable, S_IWUSR, NULL, wakeup_enable_set);

/**
 * sysf node to check the interrupt status of the sensor, the interrupt
 * handler should perform sysf_notify to allow userland to poll the node.
 */
static ssize_t irq_get(struct device *device,
			struct device_attribute *attribute,
			char* buffer)
{
	struct fpc_data *fpc = dev_get_drvdata(device);
	int irq = gpio_get_value(fpc->irq_gpio);

	return scnprintf(buffer, PAGE_SIZE, "%i\n", irq);
}

/**
 * writing to the irq node will just drop a printk message
 * and return success, used for latency measurement.
 */
static ssize_t irq_ack(struct device *device,
			struct device_attribute *attribute,
			const char *buffer, size_t count)
{
	struct fpc_data *fpc = dev_get_drvdata(device);
	dev_dbg(fpc->dev, "%s\n", __func__);

	return count;
}

static DEVICE_ATTR(irq, S_IRUSR | S_IWUSR, irq_get, irq_ack);

static ssize_t clk_enable_set(struct device *device,
		struct device_attribute *attr, const char *buf, size_t count)
{
	struct fpc_data *fpc = dev_get_drvdata(device);

	if (!fpc->hwabs->clk_enable_set)
		return count;

	return fpc->hwabs->clk_enable_set(fpc, buf, count);
}

static DEVICE_ATTR(clk_enable, S_IWUSR, NULL, clk_enable_set);

static struct attribute *fpc_attributes[] = {
	&dev_attr_hw_reset.attr,
	&dev_attr_wakeup_enable.attr,
	&dev_attr_clk_enable.attr,
	&dev_attr_irq.attr,
	NULL
};

static const struct attribute_group const fpc_attribute_group = {
	.attrs = fpc_attributes,
};

static irqreturn_t fpc_irq_handler(int irq, void *handle)
{
	struct fpc_data *fpc = handle;
	if (fpc->hwabs->irq_handler)
		fpc->hwabs->irq_handler(irq, fpc);

	/* Make sure 'wakeup_enabled' is updated before using it
	** since this is interrupt context (other thread...) */
	smp_rmb();

	if (fpc->wakeup_enabled) {
		wake_lock_timeout(&fpc->ttw_wl,
					msecs_to_jiffies(FPC_TTW_HOLD_TIME));
	}

	sysfs_notify(&fpc->dev->kobj, NULL, dev_attr_irq.attr.name);

	return IRQ_HANDLED;
}

int fpc_probe(struct platform_device *pldev,
		struct fpc_gpio_info *fpc_gpio_ops)
{
	struct device *dev = &pldev->dev;
	struct device_node *node = dev->of_node;
	struct fpc_data *fpc;
	int irqf;
	int irq_num;
	int rc;

	dev_err(dev, "%s\n", __func__);

	fpc = devm_kzalloc(dev, sizeof(*fpc), GFP_KERNEL);
	if (!fpc) {
		dev_err(dev,
			"failed to allocate memory for struct fpc_data\n");
		rc = -ENOMEM;
		goto exit;
	}

	fpc->dev = dev;
	dev_set_drvdata(dev, fpc);
	fpc->pldev = pldev;
	fpc->hwabs = fpc_gpio_ops;

	if (!node) {
		dev_err(dev, "no of node found\n");
		rc = -EINVAL;
		goto exit;
	}

	rc = fpc->hwabs->init(fpc);

	if (rc) {
		printk(KERN_INFO "error\n");
		goto exit;
	}

	/* TODO: Get the gpio pin number used for irq from ACPI */
	fpc->irq_gpio = FPC_IRQ_GPIO_NO;
	fpc->rst_gpio = FPC_RST_GPIO_NO;

	rc = devm_gpio_request_one(dev, fpc->irq_gpio,GPIOF_IN, "fpc_irq");
	if (rc < 0) {
		dev_err(dev, "Requesting GPIO for IRQ failed with %d.\n", rc);
		goto exit;
	}

	rc = devm_gpio_request_one(dev, fpc->rst_gpio, GPIOF_OUT_INIT_LOW, "fpc_rst");
	if (rc < 0) {
		dev_err(dev, "Requesting GPIO for RST failed with %d.\n", rc);
		goto exit;
	}

	rc = fpc->hwabs->configure(fpc, &irq_num, &irqf);

	if (rc < 0)
		goto exit;

	dev_dbg(dev, "Using GPIO#%d as IRQ.\n", fpc->irq_gpio);
	dev_dbg(dev, "Using GPIO#%d as RST.\n", fpc->rst_gpio);

	fpc->wakeup_enabled = false;

	irqf |= IRQF_ONESHOT;
	irqf |= IRQF_NO_SUSPEND;
	device_init_wakeup(dev, 1);
	rc = devm_request_threaded_irq(dev, irq_num,
			NULL, fpc_irq_handler, irqf,
			dev_name(dev), fpc);
	if (rc) {
		dev_err(dev, "could not request irq %d\n", irq_num);
		goto exit;
	}
	dev_dbg(dev, "requested irq %d\n", irq_num);

	/* Request that the interrupt should be wakeable */
	enable_irq_wake(irq_num);
	wake_lock_init(&fpc->ttw_wl, WAKE_LOCK_SUSPEND, "fpc_ttw_wl");

	rc = sysfs_create_group(&dev->kobj, &fpc_attribute_group);
	if (rc) {
		dev_err(dev, "could not create sysfs\n");
		goto exit;
	}

	(void)hw_reset(fpc);
	dev_info(dev, "%s: ok\n", __func__);
exit:
	return rc;
}

int fpc_remove(struct platform_device *pldev)
{
	struct  fpc_data *fpc = dev_get_drvdata(&pldev->dev);

	sysfs_remove_group(&pldev->dev.kobj, &fpc_attribute_group);
	wake_lock_destroy(&fpc->ttw_wl);
	dev_info(&pldev->dev, "%s\n", __func__);

	return 0;
}

MODULE_LICENSE("GPL");
