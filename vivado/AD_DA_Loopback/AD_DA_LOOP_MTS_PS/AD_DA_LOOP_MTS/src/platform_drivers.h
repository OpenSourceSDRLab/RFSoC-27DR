#ifndef PLATFORM_DRIVERS_H_
#define PLATFORM_DRIVERS_H_

#define XILINX
/******************************************************************************/
/***************************** Include Files **********************************/
/******************************************************************************/
#include <sleep.h>
#include <xspips.h>
#include <xgpiops.h>
#include "parameters.h"
/******************************************************************************/
/********************** Macros and Constants Definitions **********************/
/******************************************************************************/

/******************************************************************************/
/*************************** Types Declarations *******************************/
/******************************************************************************/
typedef struct {
	uint32_t	base_address;
	uint32_t	device_id;
	uint8_t		chip_select;
	uint32_t	cpha;
	uint32_t	cpol;
	uint8_t		id_no;
} spi_device;


/******************************************************************************/
/************************ Functions Declarations ******************************/
/******************************************************************************/
#define ARRAY_SIZE(ar)		(sizeof(ar)/sizeof(ar[0]))

#define AXI_REG_WRITE(BaseAddress, RegOffset, Data) \
	Xil_Out32((BaseAddress) + (RegOffset), (u32)(Data))
#define AXI_REG_READ(BaseAddress, RegOffset) \
	Xil_In32((BaseAddress) + (RegOffset))

int32_t platform_init(void);
int32_t spi_write_and_read(spi_device *dev,uint8_t *data,uint8_t bytes_number);

int32_t gpio_init(uint16_t device_id);
int32_t gpio_direction_output(uint8_t gpio,uint8_t value);
int32_t gpio_set_value(uint8_t gpio,uint8_t value);
void udelay(uint32_t usecs);
void mdelay(uint32_t msecs);
void pl_resetn();

#endif
