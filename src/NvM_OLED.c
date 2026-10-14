
#include <stdint.h>
#include "OLED.h"
#include "NvM_OLED.h"
#include "Det_Sim.h"

/* Test variables defined in main.c */
extern volatile uint8_t Test_SingleByte;
extern volatile uint8_t Test_20ByteBlock;
extern volatile uint8_t Test_UnknownRead;
extern volatile uint8_t Test_UnknownWrite;
extern volatile uint8_t Test_InvalidBounds;
extern volatile uint8_t Test_DuplicateID;
extern volatile uint8_t Test_TableFull;
extern volatile uint8_t Test_DataIntegrity;
extern volatile uint8_t Test_DET;
extern volatile uint8_t allTestsPassed;
extern volatile uint8_t failureStage;

/* Convert an integer to a null-terminated decimal string */
static void NumberToString(uint32_t number, char *buffer)
{
    char temp[10];
    uint8_t i = 0U;
    uint8_t j = 0U;

    if (number == 0U)
    {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while ((number > 0U) && (i < sizeof(temp)))
    {
        temp[i++] = (char)('0' + (number % 10U));
        number /= 10U;
    }

    while (i > 0U)
    {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}

void NvM_OLED_DisplayResults(void)
{
    char number[10];

    OLED_Clear();

    /* Use X = 8 for a safe left margin */
    OLED_ShowString(8U, 0U,
                    "NVM EXERCISE 4");

    OLED_ShowString(8U, 8U,
                    (allTestsPassed == 1U) ?
                    "RESULT: PASS" : "RESULT: FAIL");

    OLED_ShowString(8U, 16U,
                    (Test_SingleByte == 1U) ?
                    "1-BYTE: PASS" : "1-BYTE: FAIL");

    OLED_ShowString(8U, 24U,
                    (Test_20ByteBlock == 1U) ?
                    "20-BYTE: PASS" : "20-BYTE: FAIL");

    OLED_ShowString(8U, 32U,
                    (Test_DataIntegrity == 1U) ?
                    "INTEGRITY: PASS" : "INTEGRITY: FAIL");

    OLED_ShowString(8U, 40U,
                    (Test_DET == 1U) ?
                    "ERROR TESTS: PASS" : "ERROR TESTS: FAIL");

    NumberToString((uint32_t)Det_ErrorCount, number);
    OLED_ShowString(8U, 48U, "DET ERRORS:");
    OLED_ShowString(80U, 48U, number);

    NumberToString((uint32_t)failureStage, number);
    OLED_ShowString(8U, 56U, "FAIL STAGE:");
    OLED_ShowString(80U, 56U, number);
}
