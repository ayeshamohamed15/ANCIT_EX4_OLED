
#ifndef DET_SIM_H
#define DET_SIM_H

#include <stdint.h>

#define DET_SIM_NO_ERROR       0U
#define DET_SIM_UNKNOWN_BLOCK  1U
#define DET_SIM_INVALID_PARAM  2U
#define DET_SIM_MEMIF_ERROR    3U
#define DET_SIM_TABLE_FULL     4U
#define DET_SIM_DUPLICATE_ID   5U

void Det_Sim_Init(void);

void Det_Sim_ReportError(uint16_t BlockId,
                         uint8_t ErrorId);

extern volatile uint16_t Det_LastBlockId;
extern volatile uint8_t Det_LastErrorId;
extern volatile uint32_t Det_ErrorCount;

#endif
