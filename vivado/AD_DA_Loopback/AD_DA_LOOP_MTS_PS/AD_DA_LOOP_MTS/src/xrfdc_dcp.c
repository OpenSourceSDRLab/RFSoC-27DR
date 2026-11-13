
#include "xrfdc.h"
#include "rfsoc_cfg.h"
#include "platform_drivers.h"
#include "spi_ctrl.h"

int RFdcDcp_initial(XRFdc *RFdcInstPtr,u16 RFdcDeviceId)
{
	s32 Status;
	XRFdc_Config *ConfigPtr;
//	XRFdc RFdcInst;      /* RFdc driver instance */
//	RFdcInstPtr = &RFdcInst;
#ifndef __BAREMETAL__
	struct metal_device *device;
	struct metal_io_region *io;
	int ret = 0;
#endif


	ConfigPtr = XRFdc_LookupConfig(RFdcDeviceId);
	if (ConfigPtr == NULL) {
		return XRFDC_FAILURE;
	}

	Status = XRFdc_CfgInitialize(RFdcInstPtr, ConfigPtr);
	if (Status != XRFDC_SUCCESS) {
		printf("RFdc Init Failure\n\r");
	}

#ifndef __BAREMETAL__
	ret = metal_device_open(BUS_NAME, RFDC_DEV_NAME, &device);
	if (ret) {
		printf("ERROR: Failed to open device %s.\n", DeviceName);
		return XRFDC_FAILURE;
	}

	/* Map RFDC device IO region */
	io = metal_device_io_region(device, 0);
	if (!io) {
		printf("ERROR: Failed to map RFDC regio for %s.\n",
				device->name);
		return XRFDC_FAILURE;
	}
	RFdcInstPtr->device = device;
	RFdcInstPtr->io = io;
#endif
	return 0;
}

static void dump_dac_cfg(XRFdc *RFdcInstPtr, u32 tile, u32 block)
{
    u32 status;
    u32 interp = 0;
    u32 fabwr = 0;
    u32 dpmode = 0;
    XRFdc_Mixer_Settings mix = {0};

    status = XRFdc_GetInterpolationFactor(RFdcInstPtr, tile, block, &interp);
    xil_printf("DAC tile=%lu block=%lu GetInterpolationFactor status=%lu val=%lu\r\n",
               tile, block, status, interp);

    status = XRFdc_GetFabWrVldWords(RFdcInstPtr, XRFDC_DAC_TILE, tile, block, &fabwr);
    xil_printf("DAC tile=%lu block=%lu GetFabWrVldWords status=%lu val=%lu\r\n",
               tile, block, status, fabwr);

    status = XRFdc_GetMixerSettings(RFdcInstPtr, XRFDC_DAC_TILE, tile, block, &mix);
    xil_printf("DAC tile=%lu block=%lu GetMixerSettings status=%lu\r\n",
               tile, block, status);
    xil_printf("  MixerMode=%lu  MixerType=%lu  CoarseMixFreq=%lu  EventSource=%lu\r\n",
               mix.MixerMode, mix.MixerType, mix.CoarseMixFreq, mix.EventSource);
    xil_printf("  Freq=%.3f  PhaseOffset=%.3f\r\n",
               mix.Freq, mix.PhaseOffset);
}


int RFdcDcp_Example(XRFdc *RFdcInstPtr, spi_device * spi) {
	u32 factor;

	s32 status_adc, status_dac, i;

	struct metal_init_params init_param = METAL_INIT_DEFAULTS;
	if (metal_init(&init_param)) {
		printf("ERROR: Failed to run metal initialization\n");
		return XRFDC_FAILURE;
	}
	metal_set_log_level(METAL_LOG_DEBUG);

	/* ADC MTS Settings */
	XRFdc_MultiConverter_Sync_Config ADC_Sync_Config;

	/* DAC MTS Settings */
	XRFdc_MultiConverter_Sync_Config DAC_Sync_Config;

	/* Run MTS for the ADC & DAC */
	lmk04828_set_sysref_sync_cont(spi);
	mdelay(1);
	gpio_set_value(LMK_SYNC, 1);
	mdelay(10);

	int sync = -1;
	while (sync != 0) {
		sync = 0;
		printf("\n\r=== Run ADC Sync ===\n\r");
		/* Initialize ADC MTS Settings */
		XRFdc_MultiConverter_Init(&ADC_Sync_Config, NULL, NULL, 0);
		ADC_Sync_Config.Tiles = 0xF; /* Sync ADC tiles 0, 1, 2, 3 */
		ADC_Sync_Config.Target_Latency = -1;
		status_adc = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_ADC_TILE,&ADC_Sync_Config);
		if (status_adc == XRFDC_MTS_OK) {
			printf("INFO : ADC Multi-Tile-Sync completed successfully\n\r");
		} else {
			printf("INFO : ADC Multi-Tile-Sync failed\n\r");
			sync = -1;
			gpio_set_value(LMK_SYNC, 0);
			mdelay(10);
			gpio_set_value(LMK_SYNC, 1);
			mdelay(10);
			printf("status_adc_MTSerror =%3d\n\r", status_adc);
			return -1;
		}
		for (i = 0; i < 4; i++) {
			if ((1 << i) & ADC_Sync_Config.Tiles) {
				if (ADC_Sync_Config.Offset[i] > 1) { //!= ADC_Sync_Config.Offset[0]
					printf("ADC%d: Latency(T1) =%3d, Adjusted DelayOffset=%3d\n\r", i, ADC_Sync_Config.Latency[i],ADC_Sync_Config.Offset[i]);
					sync = -1;
					gpio_set_value(LMK_SYNC, 0);
					mdelay(10);
					gpio_set_value(LMK_SYNC, 1);
					mdelay(10);
					return -1;
				}
			}
		}
	}

	sync = -1;
	while (sync != 0) {
			sync = 0;
			printf("\n\r=== Run DAC Sync ===\n\r");
			XRFdc_MultiConverter_Init(&DAC_Sync_Config, NULL, NULL, 0);
			DAC_Sync_Config.Tiles = 0x3; /* Sync DAC tiles 0 and 1 */
			DAC_Sync_Config.Target_Latency = -1;
			status_dac = XRFdc_MultiConverter_Sync(RFdcInstPtr, XRFDC_DAC_TILE,
					&DAC_Sync_Config);
			if (status_dac == XRFDC_MTS_OK) {
				printf("INFO : DAC Multi-Tile-Sync completed successfully\n\r");
			} else {
				printf("INFO : DAC Multi-Tile-Sync failed\n\r");
				sync = -1;
				gpio_set_value(LMK_SYNC, 0);
				mdelay(10);
				gpio_set_value(LMK_SYNC, 1);
				mdelay(10);
				printf("status_dac_MTSerror =%5d\n\r", status_dac);
				return -1;
			}
			for (i = 0; i < 2; i++) {
				if ((1 << i) & DAC_Sync_Config.Tiles) {
					if (DAC_Sync_Config.Offset[i] != DAC_Sync_Config.Offset[0]) {
						printf("DAC%d: Latency(T1) =%3d, Adjusted DelayOffset=%3d\n\r", i, DAC_Sync_Config.Latency[i],DAC_Sync_Config.Offset[i]);
						sync = -1;
						gpio_set_value(LMK_SYNC, 0);
						mdelay(10);
						gpio_set_value(LMK_SYNC, 1);
						mdelay(10);
						return -1;
					}
				}
			}
		}
	gpio_set_value(LMK_SYNC, 0);

	/****************************************************************************/
	//Report Overall Latency in T1 (Sample Clocks) and
	// Offsets (in terms of PL words) added to each FIFO
	/****************************************************************************/
	printf("\n\n\r=== Multi-Tile Sync Report ===\n\r");
	for (i = 0; i < 2; i++) {
		if ((1 << i) & DAC_Sync_Config.Tiles) {
			XRFdc_GetInterpolationFactor(RFdcInstPtr, i, 0, &factor);
			printf("DAC%d: Latency(T1) =%3d, Adjusted Delay"
					"Offset(T%d) =%3d\n\r", i, DAC_Sync_Config.Latency[i],
					factor, DAC_Sync_Config.Offset[i]);
		}
	}
	for (i = 0; i < 4; i++) {
		if ((1 << i) & ADC_Sync_Config.Tiles) {
			XRFdc_GetDecimationFactor(RFdcInstPtr, i, 0, &factor);
			printf("ADC%d: Latency(T1) =%3d, Adjusted Delay"
					"Offset(T%d) =%3d\n\r", i, ADC_Sync_Config.Latency[i],
					factor, ADC_Sync_Config.Offset[i]);
		}
	}
	return 0;
}


