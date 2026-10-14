#ifndef MEMIF_SIM_H
#define MEMIF_SIM_H

#include <stdint.h>

/* Simulated EEPROM configuration */
#define MEMIF_SIM_EEPROM_SIZE 512U
#define MEMIF_SIM_PAGE_SIZE   8U

/* Initialize simulated EEPROM */
void MemIf_Sim_Init(void);

/* Read data from simulated EEPROM */
uint8_t MemIf_Sim_Read(uint16_t Offset,
                       uint8_t *DestPtr,
                       uint16_t Length);

/* Write data to simulated EEPROM */
uint8_t MemIf_Sim_Write(uint16_t Offset,
                        const uint8_t *SrcPtr,
                        uint16_t Length);

#endif /* MEMIF_SIM_H */
