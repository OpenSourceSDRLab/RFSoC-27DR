
#include "rfsoc_cfg.h"
#include "platform_drivers.h"

extern uint64_t FS;
extern spi_device lmk04828_spi;
extern uint32_t txatt[8];
#include "spi_ctrl.h"

u32 XRFdc_ChangeNCOFreq(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,
		double Freq_set) {
	u32 Status;
	XRFdc_Mixer_Settings Mixer_Settings;
	Status = XRFdc_GetMixerSettings(InstancePtr, Type, Tile_Id, Block_Id,
			&Mixer_Settings);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	// Make sure the mixer settings update use the Tile event
	Mixer_Settings.EventSource = XRFDC_EVNT_SRC_SYSREF;
	Mixer_Settings.Freq = Freq_set; //unit is MHz
	Status = XRFdc_SetMixerSettings(InstancePtr, Type, Tile_Id, Block_Id,
			&Mixer_Settings);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	Status = XRFdc_ResetNCOPhase(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XST_SUCCESS)
		printf("Reset Type=%d NCO Tile#%d block%d fail\n", Type, Tile_Id,
				Block_Id);
	XRFdc_UpdateEvent(InstancePtr, Type, Tile_Id, Block_Id, XRFDC_EVENT_MIXER);
	gpio_set_value(LMK_SYNC, 1);
	gpio_set_value(LMK_SYNC, 0);
	return Status;
}

u32 XRFdc_ChangeNCOPhase(XRFdc *InstancePtr, u32 Type, u32 Tile_Id,
		u32 Block_Id, double Phase_offset) {
	u32 Status;
	XRFdc_Mixer_Settings Mixer_Settings;
	Status = XRFdc_GetMixerSettings(InstancePtr, Type, Tile_Id, Block_Id,
			&Mixer_Settings);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	// Make sure the mixer settings update use the Tile event
	Mixer_Settings.EventSource = XRFDC_EVNT_SRC_SYSREF;
	Mixer_Settings.PhaseOffset = Phase_offset; //double -180~180
	Status = XRFdc_SetMixerSettings(InstancePtr, Type, Tile_Id, Block_Id,
			&Mixer_Settings);
	if (Status != XRFDC_SUCCESS) {
		return XRFDC_FAILURE;
	}
	Status = XRFdc_ResetNCOPhase(InstancePtr, Type, Tile_Id, Block_Id);
	if (Status != XST_SUCCESS)
		printf("Reset Type=%d NCO Tile#%d block%d Phase_offset fail\n", Type,
				Tile_Id, Block_Id);
	XRFdc_UpdateEvent(InstancePtr, Type, Tile_Id, Block_Id, XRFDC_EVENT_MIXER);
	gpio_set_value(LMK_SYNC, 1);
	gpio_set_value(LMK_SYNC, 0);
	return Status;
}


u32 qmc_settings(XRFdc *RFdcInstPtr, u32 Type) {
	u32 Status;
	XRFdc_QMC_Settings QMCSettingsPtr;
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++) {
			Status = XRFdc_GetQMCSettings(RFdcInstPtr, Type, i, j * (Type + 1),
					&QMCSettingsPtr);
			printf("tile=%d,block=%d,EnableGain=%d\n", i, j,
					QMCSettingsPtr.EnableGain);
			printf("tile=%d,block=%d,GainCorrectionFactor=%f\n", i, j,
					QMCSettingsPtr.GainCorrectionFactor);
		if(Type==XRFDC_ADC_TILE){
			QMCSettingsPtr.EnableGain = 1;
			QMCSettingsPtr.GainCorrectionFactor = 1.99;
		}
			QMCSettingsPtr.EventSource = XRFDC_EVNT_SRC_SYSREF;
			Status = XRFdc_SetQMCSettings(RFdcInstPtr, Type, i, j * (Type + 1),
					&QMCSettingsPtr);
	}
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++)
			Status = XRFdc_UpdateEvent(RFdcInstPtr, Type, i, j * (Type + 1),
			XRFDC_EVENT_QMC);
	gpio_set_value(LMK_SYNC, 1);
	gpio_set_value(LMK_SYNC, 0);
	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 2; j++) {
			Status = XRFdc_GetQMCSettings(RFdcInstPtr, Type, i, j * (Type + 1),
					&QMCSettingsPtr);
			printf("tile=%d,block=%d,EnableGain=%d,Status=%d\n", i, j,
					QMCSettingsPtr.EnableGain, Status);
			printf("tile=%d,block=%d,GainCorrectionFactor=%f\n", i, j,
					QMCSettingsPtr.GainCorrectionFactor);
		}
	return Status;
}

