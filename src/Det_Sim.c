
#include "Det_Sim.h"

volatile uint16_t Det_LastBlockId = 0U;
volatile uint8_t Det_LastErrorId = DET_SIM_NO_ERROR;
volatile uint32_t Det_ErrorCount = 0U;

void Det_Sim_Init(void)
{
    Det_LastBlockId = 0U;
    Det_LastErrorId = DET_SIM_NO_ERROR;
    Det_ErrorCount = 0U;
}

void Det_Sim_ReportError(uint16_t BlockId,
                         uint8_t ErrorId)
{
    Det_LastBlockId = BlockId;
    Det_LastErrorId = ErrorId;
    Det_ErrorCount++;
}
