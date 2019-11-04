///------------------------------------------------------------------------------
/// \file    dio_drv.c
///
/// \brief   Driver for "X3" (4 digital I/O) on TouchPanel devices
///
/// \author  Ralf Gliese, elrest Automationssysteme GmbH
///------------------------------------------------------------------------------


#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mutex.h>
#include <linux/mod_devicetable.h>
#include <linux/spi/spi.h>
#include <linux/of_gpio.h>
#include <linux/delay.h>

struct dio_data {
	unsigned char dma_buffer[2];
	int gpio_load;
	int gpio_reset;
	struct mutex lock;
	unsigned char tx_data;
};

static unsigned char reverse_low_nibble(unsigned char input)
{
	static const unsigned char lookup[] = {
		0x0, 0x8, 0x4, 0xc, 0x2, 0xa, 0x6, 0xe,
		0x1, 0x9, 0x5, 0xd, 0x3, 0xb, 0x7, 0xf
	};
	return lookup[input & 0xf] | (input & 0xf0);
};

/* SPI Write/Read cycle */
static int dio_spi_exchange(struct spi_device *spi, int data)
{
	struct dio_data *dio = spi_get_drvdata(spi);
	struct spi_message msg;
	struct spi_transfer xfer;
	int ret;

	if (data >= 0)
		dio->tx_data = reverse_low_nibble(data);

	dio->dma_buffer[0] = 0;
	dio->dma_buffer[1] = dio->tx_data;

	/* Latch input data */
	gpio_set_value(dio->gpio_load, 0);
	udelay(10);
	gpio_set_value(dio->gpio_load, 1);

	/* Initialize the SPI message and transfer data structures */
	spi_message_init(&msg);
	memset(&xfer, 0, sizeof xfer);
	xfer.tx_buf = xfer.rx_buf = dio->dma_buffer;
	xfer.len = sizeof dio->dma_buffer;
	spi_message_add_tail(&xfer, &msg);

	/* Send the message and wait for completion */
	ret = spi_sync(spi, &msg);
	if (ret != 0) {
		dev_err(&spi->dev, "SPI transmission failed\n");
		return 0;
	}
	dev_dbg(&spi->dev, "sent %02x %02x, received %02x %02x\n",
			dio->tx_data, dio->tx_data,
			dio->dma_buffer[0], dio->dma_buffer[1]);
	return reverse_low_nibble(dio->dma_buffer[0]) | dio->dma_buffer[1] << 8;
}

static ssize_t dio_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct spi_device *spi = to_spi_device(dev);
	struct dio_data *dio = spi_get_drvdata(spi);
	int ret;

	ret = mutex_lock_interruptible(&dio->lock);
	if (ret)
		return ret;
	ret = dio_spi_exchange(spi, -1);
	mutex_unlock(&dio->lock);
	return sprintf(buf, "%d\n", ret);
}

static ssize_t dio_store(struct device *dev, struct device_attribute *attr,
		  const char *buf, size_t count)
{
	struct spi_device *spi = to_spi_device(dev);
	struct dio_data *dio = spi_get_drvdata(spi);
	unsigned long data;
	int ret;

	ret = kstrtoul(buf, 0, &data);
	if (ret < 0)
		return ret;
	if (data > 0xf)
		return -EINVAL;

	ret = mutex_lock_interruptible(&dio->lock);
	if (ret)
		return ret;
	dio_spi_exchange(spi, data);
	mutex_unlock(&dio->lock);
	return count;
}

static DEVICE_ATTR_RW(dio);

static int alloc_gpio(struct device *dev, const char *name)
{
	int ret, gpio = of_get_named_gpio(dev->of_node, name, 0);

	if (gpio < 0) {
		dev_err(dev, "GPIO '%s' not found in device tree\n", name);
		return gpio;
	}
	ret = devm_gpio_request_one(dev, gpio, GPIOF_OUT_INIT_HIGH, name);
	if (ret < 0) {
		dev_err(dev, "Cannot request GPIO '%s' %d\n", name, gpio);
		return ret;
	}
	dev_dbg(dev, "GPIO '%s' has Number %d\n", name, gpio);
	return gpio;
}

static int dio_drv_probe(struct spi_device *spi)
{
	struct device *dev = &spi->dev;
	struct dio_data *dio;

	dio = devm_kzalloc(dev, sizeof *dio, GFP_KERNEL);
	if (!dio)
		return -ENOMEM;

	dio->gpio_load = alloc_gpio(dev, "gpio-load");
	if (dio->gpio_load < 0)
		return dio->gpio_load;

	dio->gpio_reset = alloc_gpio(dev, "gpio-reset");
	if (dio->gpio_reset < 0)
		return dio->gpio_reset;

	mutex_init(&dio->lock);
	spi_set_drvdata(spi, dio);

	/* Initialize and enable output lines */
	dio_spi_exchange(spi, 0);
	gpio_set_value(dio->gpio_reset, 0);

	return device_create_file(dev, &dev_attr_dio);
}

static int dio_drv_remove(struct spi_device *spi)
{
	struct dio_data *dio = spi_get_drvdata(spi);

	gpio_set_value(dio->gpio_reset, 1);
	device_remove_file(&spi->dev, &dev_attr_dio);
	return 0;
}

static const struct spi_device_id dio_spi_ids[] = {
	{ "dio_spi", 0 },
	{ },
};
MODULE_DEVICE_TABLE(spi, dio_spi_ids);

static struct spi_driver dio_driver = {
	.driver = {
		.name	= "dio_spi",
		.owner	= THIS_MODULE,
	},
	.id_table = dio_spi_ids,
	.probe	= dio_drv_probe,
	.remove = dio_drv_remove,
};

module_spi_driver(dio_driver);

MODULE_AUTHOR("elrest");
MODULE_DESCRIPTION("dio_spi driver");
MODULE_LICENSE("GPL");
