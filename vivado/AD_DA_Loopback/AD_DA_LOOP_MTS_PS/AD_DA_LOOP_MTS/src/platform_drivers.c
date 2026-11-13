#include "platform_drivers.h"
#include "parameters.h"
#include <stdio.h>
/******************************************************************************/
/************************ Variables Definitions *******************************/
/******************************************************************************/
spi_device lmk04828_spi = {
		SPI_BASEADDR, 	// base_address
		SPI_DEVICE_ID,  // device_id
		1, 				// chip_select
		0, 				// cpha
		0, 				// cpol
		0				// id_no
};

XGpioPs_Config			*gpio_config;
XGpioPs					gpio_instance;
static XSpiPs_Config 	*spi_config;
static XSpiPs			spi_instance;

/***************************************************************************//**
 * @brief spi_init
 *******************************************************************************/
int32_t spi_init(uint16_t device_id, XSpiPs *spi_instance)
{
#ifdef _XPARAMETERS_PS_H_
	spi_config = XSpiPs_LookupConfig(device_id);
	if (spi_config == NULL)
		return -1;

	if (XSpiPs_CfgInitialize(spi_instance, spi_config, spi_config->BaseAddress) != 0)
		return -1;
	//	spi_instance->Config.DeviceId=device_id;
#endif
	return 0;
}

/***************************************************************************//**
 * @brief platform_init
 *******************************************************************************/
int32_t platform_init(void)
{
	gpio_init(GPIO_DEVICE_ID);
	spi_init(SPI_DEVICE_ID, &spi_instance);
	gpio_direction_output(LMK_REF_SELECT0, 0);//TCXO
	gpio_direction_output(LMK_REF_SELECT1, 1);
	gpio_direction_output(LMK_SYNC, 0);
	gpio_direction_output(LED_0, 0);
	gpio_direction_output(LED_1, 0);
	gpio_direction_output(LED_2, 0);
	gpio_direction_output(LED_3, 0);
	gpio_direction_output(LMK_RESET, 1);
	mdelay(500);
	gpio_direction_output(LMK_RESET, 0);
	return 0;
}

/***************************************************************************//**
 * @brief spi_write_and_read
 *******************************************************************************/
int32_t spi_write_and_read(spi_device *dev,
					   	   uint8_t *data,
						   uint8_t bytes_number)
{
	uint32_t initss;
	initss = XSpiPs_ReadReg(dev->base_address, XSPIPS_CR_OFFSET);
	initss = initss & (uint32_t)(~XSPIPS_CR_SSCTRL_MASK);
	initss = initss | (0x7 << XSPIPS_CR_SSCTRL_SHIFT);
	XSpiPs_WriteReg(dev->base_address, XSPIPS_CR_OFFSET, initss);
	XSpiPs_SetOptions(&spi_instance, XSPIPS_MASTER_OPTION |
			XSPIPS_DECODE_SSELECT_OPTION | XSPIPS_FORCE_SSELECT_OPTION |
			((dev->cpol == 1) ? XSPIPS_CLK_ACTIVE_LOW_OPTION : 0) |
			((dev->cpha == 1) ? XSPIPS_CLK_PHASE_1_OPTION : 0));
	XSpiPs_SetSlaveSelect(&spi_instance, (uint8_t) 0x7);
	XSpiPs_SetClkPrescaler(&spi_instance, XSPIPS_CLK_PRESCALE_64);
	XSpiPs_SetSlaveSelect(&spi_instance,  0xf & ~dev->chip_select);
	XSpiPs_PolledTransfer(&spi_instance, data, data, bytes_number);
	XSpiPs_SetSlaveSelect(&spi_instance,  (uint8_t) 0x7);

	return 0;
}
/***************************************************************************//**
 * @brief gpio_init
 *******************************************************************************/
int32_t gpio_init(uint16_t device_id)
{
	gpio_config = XGpioPs_LookupConfig(device_id);
	XGpioPs_CfgInitialize(&gpio_instance, gpio_config, gpio_config->BaseAddr);

	return 0;
}

/***************************************************************************//**
 * @brief gpio_direction_output
 *******************************************************************************/
int32_t gpio_direction_output(uint8_t gpio, uint8_t value)
{
	XGpioPs_SetDirectionPin(&gpio_instance, gpio, 1);
	XGpioPs_SetOutputEnablePin(&gpio_instance, gpio, 1);
	XGpioPs_WritePin(&gpio_instance, gpio, value);

	return 0;
}

/***************************************************************************//**
 * @brief gpio_set_value
 *******************************************************************************/
int32_t gpio_set_value(uint8_t gpio, uint8_t value)
{
	XGpioPs_WritePin(&gpio_instance, gpio, value);

	return 0;
}

void udelay(uint32_t usecs)
{
	usleep(usecs);
}

/**
 * @brief Generate miliseconds delay.
 * @param msecs - Delay in miliseconds.
 * @return None.
 */
void mdelay(uint32_t msecs)
{
	usleep(msecs * 1000);
}

void pl_resetn(){ // drive the MPSoC pl_resetn0 output low, then high (active-low reset of the PL)
	Xil_Out32(0XFF0A0054, 0x00000000U);
	usleep(10);
	Xil_Out32(0XFF0A0054, 0x80000000U);
}

