
#include "xparameters.h"
#include "rfsoc_cfg.h"
#include "platform_drivers.h"
#include "spi_ctrl.h"
#include "xtime_l.h"
#include "xrfdc.h"
#include "xrfdc_dcp.h"
#include "xsdps.h"
#include "xadc.h"

/************************** Variable Definitions ****************************/
uint32_t axi_lite_addr = XPAR_AXI_LITE_CTRL_0_BASEADDR;
extern spi_device lmk04828_spi;
XRFdc RFdcInstPtr;
XRFdc_Mixer_Settings Mixer_Settings;

uint64_t FS, dacFS;
uint16_t auxdac=1600;
uint32_t ref_ext_en = 0, sample_rate, val_out, Tile_Id, Block_Id, nco_nyquist;
double NCO_Freq_set = 1000, Freq_set = 10;	//MHZ
float  Temperature;


uint32_t txatt[8] = { 40500, 40500, 40500, 40500, 40500, 40500, 40500, 40500 };
int Status, lock_pll1 = -1, lock_pll2 = -1, cyc = 0;


int main(void)
{
	printf("RFSOC-27DR-Test-Start\n");
	platform_init();

/**************************************************************************/
/*****      configure system clock					                  *****/
/**************************************************************************/
	lmk04828_init(&lmk04828_spi); 		//256MHZ
	lmk04828_update_man_dac(&lmk04828_spi, auxdac);

	printf("waiting lmk04828 locked......\r\n");
	while ((lock_pll1 != ref_ext_en) || lock_pll2 != 1 || cyc < 100) {
		lock_pll1 = lmk04828_spi_read(&lmk04828_spi, 0X182) >> 1 & 0x1;
		lock_pll2 = lmk04828_spi_read(&lmk04828_spi, 0X183) >> 1 & 0x1;
		if (lock_pll2 == 1 && (lock_pll1 == ref_ext_en)) {
			lock_pll2 = 1;
			lock_pll1 = ref_ext_en;
			cyc++;
		} else {
			lock_pll2 = 0;
			lock_pll1 = (~ref_ext_en) & 0x1;
			cyc = 0;
		}
	}
	printf("read from lmk04828 pll1_lock=0x%x, pll12_lock=0x%x\n", lock_pll1, lock_pll2);
	//sync
	gpio_set_value(LMK_SYNC, 0);
	mdelay(1);
	gpio_set_value(LMK_SYNC, 1);
	mdelay(1);
	gpio_set_value(LMK_SYNC, 0);

	lmk04828_update_sysref_pulse(&lmk04828_spi);
	gpio_set_value(LMK_SYNC, 0);
	lmk04828_set_sysref_shots(&lmk04828_spi, 0);//
	mdelay(500);

	pl_resetn();
	mdelay(10);
	AXI_REG_WRITE(axi_lite_addr, 0 * 4, 0);			//reset dds

/**************************************************************************/
/*****      configure RFdc      					                  *****/
/**************************************************************************/
	Status = RFdcDcp_initial(&RFdcInstPtr, RFDC_DEVICE_ID);
	for (int i = 0; i < 4; i++) {
		XRFdc_Reset(&RFdcInstPtr, XRFDC_ADC_TILE, i);
	}
	for (int i = 0; i < 2; i++){
		XRFdc_Reset(&RFdcInstPtr, XRFDC_DAC_TILE, i);
	}
	//Enable Status
	Status = AXI_REG_READ(XPAR_XRFDC_0_BASEADDR, XRFDC_ADC_PATHS_ENABLED_OFFSET);
	printf("ADC_PATHS_ENABLED=%x\n", Status);
	Status = AXI_REG_READ(XPAR_XRFDC_0_BASEADDR, XRFDC_DAC_PATHS_ENABLED_OFFSET);
	printf("DAC_PATHS_ENABLED=%x\n", Status);
	//Read FS
	FS = RFdcInstPtr.ADC_Tile[0].PLL_Settings.SampleRate * 1e9;
	dacFS = RFdcInstPtr.DAC_Tile[0].PLL_Settings.SampleRate * 1e9;
	printf("DAC converter samplerate=%ldHz\n", dacFS);
	printf("ADC converter samplerate=%ldHz\n", FS);
	//Read PLL Status
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_ADC_TILE, 0, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_ADC_TILE0=%d\n", val_out);
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_ADC_TILE, 1, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_ADC_TILE1=%d\n", val_out);
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_ADC_TILE, 2, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_ADC_TILE2=%d\n", val_out);
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_ADC_TILE, 3, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_ADC_TILE3=%d\n", val_out);
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_DAC_TILE, 0, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_DAC_TILE0=%d\n", val_out);
	XRFdc_GetPLLLockStatus(&RFdcInstPtr, XRFDC_DAC_TILE, 1, &val_out);
	printf("XRFdc_GetPLLLockStatus XRFDC_DAC_TILE1=%d\n", val_out);
	//Config NCO
	if (NCO_Freq_set * 1e6 < FS / 2)
		nco_nyquist = 1;
	else {
		nco_nyquist = 2;
		NCO_Freq_set = -NCO_Freq_set;
	}
	for (Tile_Id = 0; Tile_Id < 2; Tile_Id++) {
		for (Block_Id = 0; Block_Id < 4; Block_Id++) {
			Status = XRFdc_SetNyquistZone(&RFdcInstPtr, XRFDC_DAC_TILE, Tile_Id,Block_Id, nco_nyquist);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_SetNyquistZone fail\n",Tile_Id, Block_Id);
			Status = XRFdc_ChangeNCOFreq(&RFdcInstPtr, XRFDC_DAC_TILE, Tile_Id,Block_Id, NCO_Freq_set);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_ChangeNCOFreq fail\n",Tile_Id, Block_Id);
			Status = XRFdc_GetMixerSettings(&RFdcInstPtr, XRFDC_DAC_TILE, Tile_Id,Block_Id, &Mixer_Settings);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_GetMixerSettings fail\n",Tile_Id, Block_Id);
			printf("DAC NCO frequency of Tile#%d Block%d is set to %f\n",Tile_Id, Block_Id, Mixer_Settings.Freq);
		}
	}
	for (Tile_Id = 0; Tile_Id < 4; Tile_Id++) {
		for (Block_Id = 0; Block_Id < 2; Block_Id++) {
			Status = XRFdc_SetNyquistZone(&RFdcInstPtr, XRFDC_ADC_TILE, Tile_Id,Block_Id, nco_nyquist);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_SetNyquistZone fail\n",Tile_Id, Block_Id);
			Status = XRFdc_ChangeNCOFreq(&RFdcInstPtr, XRFDC_ADC_TILE, Tile_Id,Block_Id, NCO_Freq_set);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_ChangeNCOFreq fail\n",Tile_Id, Block_Id);
			Status = XRFdc_GetMixerSettings(&RFdcInstPtr, XRFDC_ADC_TILE,Tile_Id, Block_Id, &Mixer_Settings);
			if (Status != XST_SUCCESS)
				printf("Tile_Id=%d,Block_Id=%d,XRFdc_GetMixerSettings fail\n",Tile_Id, Block_Id);
			printf("ADC NCO frequency of Tile#%d Block%d is set to %f\n",Tile_Id, Block_Id, Mixer_Settings.Freq);
		}
	}
	//MTS
	// Status=-1;
	// while (Status != XRFDC_SUCCESS) {
	// 	Status = RFdcDcp_Example(&RFdcInstPtr, &lmk04828_spi);
	// 	if (Status != XRFDC_SUCCESS) {
	// 		printf("RFdcDcp initial failed\n");
	// 	}
	// }
	printf("RFSOC initial Successful\n");
/**************************************************************************/
/*****      configure other     					                  *****/
/**************************************************************************/
	printf("The lasted build at %s %s\n", __TIME__, __DATE__);

	AXI_REG_WRITE(axi_lite_addr, 0 * 4, 1);			//set dds
	AXI_REG_WRITE(axi_lite_addr, 1 * 4, 1);			//config done
	AXI_REG_WRITE(axi_lite_addr, 2 * 4, Freq_set*1000000*65536/(FS/4) );		//phase inc
	//XADC
	Status = xadc_initial(XPAR_PSU_AMS_DEVICE_ID);
	if (Status == 0) {
		printf("Xadc Setup Successful\r\n");
	}

	while (1) {
		xadc_read_temp(&Temperature);
		printf("RFSOC Temperature=%d\r\n", (uint16_t)Temperature);
		gpio_set_value(LED_0, 1);
		gpio_set_value(LED_1, 1);
		gpio_set_value(LED_2, 1);
		gpio_set_value(LED_3, 1);
		mdelay(500);
		gpio_set_value(LED_0, 0);
		gpio_set_value(LED_1, 0);
		gpio_set_value(LED_2, 0);
		gpio_set_value(LED_3, 0);
		mdelay(500);
	}

	return XRFDC_SUCCESS;
}
