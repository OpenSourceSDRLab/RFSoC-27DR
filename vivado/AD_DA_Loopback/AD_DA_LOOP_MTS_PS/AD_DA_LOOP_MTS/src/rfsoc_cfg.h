

#ifndef SRC_RFSOC_RFSOC_CFG_H_
#define SRC_RFSOC_RFSOC_CFG_H_
#include "xrfdc.h"

u32 XRFdc_ChangeNCOFreq(XRFdc *InstancePtr, u32 Type, u32 Tile_Id, u32 Block_Id,double Freq_set);
u32 XRFdc_ChangeNCOPhase(XRFdc *InstancePtr, u32 Type, u32 Tile_Id,u32 Block_Id, double Phase_offset);
u32 qmc_settings(XRFdc *RFdcInstPtr, u32 type);

#endif /* SRC_RFSOC_RFSOC_CFG_H_ */
