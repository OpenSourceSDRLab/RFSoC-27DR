
#include "xrfdc.h"
#ifndef SRC_RFSOC_XRFDC_DCP_H_
#define SRC_RFSOC_XRFDC_DCP_H_
int RFdcDcp_initial(XRFdc *RFdcInstPtr, u16 RFdcDeviceId);
int RFdcDcp_Example(XRFdc *RFdcInstPtr, spi_device * spi);

#endif /* SRC_RFSOC_XRFDC_DCP_H_ */
