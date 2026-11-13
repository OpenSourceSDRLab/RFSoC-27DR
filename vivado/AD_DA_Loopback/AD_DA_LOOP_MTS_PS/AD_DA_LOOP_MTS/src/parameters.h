
#ifndef PARAMETERS_H_
#define PARAMETERS_H_

#define SPI_BASEADDR				XPAR_PSU_SPI_1_BASEADDR
#define GPIO_BASEADDR				XPAR_PSU_GPIO_0_BASEADDR

#define SPI_DEVICE_ID				XPAR_XSPIPS_0_DEVICE_ID
#define GPIO_DEVICE_ID              XPAR_PSU_GPIO_0_DEVICE_ID
#define RFDC_DEVICE_ID 				XPAR_XRFDC_0_DEVICE_ID


#define GPIO_OFFSET					78
#define LMK_RESET   				17
#define LMK_REF_SELECT1				16
#define LMK_REF_SELECT0 			15
#define LMK_SYNC					GPIO_OFFSET + 0
#define LED_0						GPIO_OFFSET + 1
#define LED_1						GPIO_OFFSET + 2
#define LED_2						GPIO_OFFSET + 3
#define LED_3						GPIO_OFFSET + 4

#define SUCCESS						0
#define FAILURE						-1


#endif /* PARAMETERS_H_ */
