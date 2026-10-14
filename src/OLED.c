
#include "OLED.h"
#include "sdk_project_config.h"
#include <stdint.h>
#include <stddef.h>

/* SSD1306 OLED configuration */
#define OLED_CMD_MODE       0x00U
#define OLED_DATA_MODE      0x40U
#define OLED_I2C_INSTANCE   0U
#define OLED_TIMEOUT        100U
#define OLED_FONT_WIDTH     5U
#define OLED_CHAR_SPACING   1U

/* Send bytes to the OLED using LPI2C0 */
static uint8_t OLED_I2C_Write(const uint8_t *data, uint16_t length)
{
    status_t status;

    if ((data == NULL) || (length == 0U))
    {
        return 1U;
    }

    /*
     * SetSlaveAddr returns void in this SDK.
     * Do not assign its return value to status.
     */
    LPI2C_DRV_MasterSetSlaveAddr(
        OLED_I2C_INSTANCE,
        OLED_I2C_ADDRESS,
        false
    );

    status = LPI2C_DRV_MasterSendDataBlocking(
        OLED_I2C_INSTANCE,
        (uint8_t *)data,
        length,
        true,
        OLED_TIMEOUT
    );

    return (status == STATUS_SUCCESS) ? 0U : 1U;
}

/* Send one command */
static uint8_t OLED_WriteCommand(uint8_t command)
{
    uint8_t buffer[2];

    buffer[0] = OLED_CMD_MODE;
    buffer[1] = command;

    return OLED_I2C_Write(buffer, 2U);
}

/* Send display data in one I2C transaction */
static uint8_t OLED_WriteData(const uint8_t *data, uint16_t length)
{
    uint8_t buffer[129U];
    uint16_t i;

    if ((data == NULL) || (length == 0U) || (length > 128U))
    {
        return 1U;
    }

    buffer[0] = OLED_DATA_MODE;

    for (i = 0U; i < length; i++)
    {
        buffer[i + 1U] = data[i];
    }

    return OLED_I2C_Write(buffer, (uint16_t)(length + 1U));
}

/*
 * Five-column font.
 * Each byte represents a vertical column.
 * Bit 0 is the top pixel.
 */
static void OLED_GetCharacter(uint8_t character, uint8_t glyph[5])
{
    static const uint8_t digits[10][5] =
    {
        {0x3EU, 0x51U, 0x49U, 0x45U, 0x3EU}, /* 0 */
        {0x00U, 0x42U, 0x7FU, 0x40U, 0x00U}, /* 1 */
        {0x42U, 0x61U, 0x51U, 0x49U, 0x46U}, /* 2 */
        {0x21U, 0x41U, 0x45U, 0x4BU, 0x31U}, /* 3 */
        {0x18U, 0x14U, 0x12U, 0x7FU, 0x10U}, /* 4 */
        {0x27U, 0x45U, 0x45U, 0x45U, 0x39U}, /* 5 */
        {0x3CU, 0x4AU, 0x49U, 0x49U, 0x30U}, /* 6 */
        {0x01U, 0x71U, 0x09U, 0x05U, 0x03U}, /* 7 */
        {0x36U, 0x49U, 0x49U, 0x49U, 0x36U}, /* 8 */
        {0x06U, 0x49U, 0x49U, 0x29U, 0x1EU}  /* 9 */
    };

    static const uint8_t letters[26][5] =
    {
        {0x7EU,0x11U,0x11U,0x11U,0x7EU}, /* A */
        {0x7FU,0x49U,0x49U,0x49U,0x36U}, /* B */
        {0x3EU,0x41U,0x41U,0x41U,0x22U}, /* C */
        {0x7FU,0x41U,0x41U,0x22U,0x1CU}, /* D */
        {0x7FU,0x49U,0x49U,0x49U,0x41U}, /* E */
        {0x7FU,0x09U,0x09U,0x09U,0x01U}, /* F */
        {0x3EU,0x41U,0x49U,0x49U,0x7AU}, /* G */
        {0x7FU,0x08U,0x08U,0x08U,0x7FU}, /* H */
        {0x00U,0x41U,0x7FU,0x41U,0x00U}, /* I */
        {0x20U,0x40U,0x41U,0x3FU,0x01U}, /* J */
        {0x7FU,0x08U,0x14U,0x22U,0x41U}, /* K */
        {0x7FU,0x40U,0x40U,0x40U,0x40U}, /* L */
        {0x7FU,0x02U,0x0CU,0x02U,0x7FU}, /* M */
        {0x7FU,0x04U,0x08U,0x10U,0x7FU}, /* N */
        {0x3EU,0x41U,0x41U,0x41U,0x3EU}, /* O */
        {0x7FU,0x09U,0x09U,0x09U,0x06U}, /* P */
        {0x3EU,0x41U,0x51U,0x21U,0x5EU}, /* Q */
        {0x7FU,0x09U,0x19U,0x29U,0x46U}, /* R */
        {0x46U,0x49U,0x49U,0x49U,0x31U}, /* S */
        {0x01U,0x01U,0x7FU,0x01U,0x01U}, /* T */
        {0x3FU,0x40U,0x40U,0x40U,0x3FU}, /* U */
        {0x1FU,0x20U,0x40U,0x20U,0x1FU}, /* V */
        {0x3FU,0x40U,0x38U,0x40U,0x3FU}, /* W */
        {0x63U,0x14U,0x08U,0x14U,0x63U}, /* X */
        {0x07U,0x08U,0x70U,0x08U,0x07U}, /* Y */
        {0x61U,0x51U,0x49U,0x45U,0x43U}  /* Z */
    };

    uint8_t i;

    for (i = 0U; i < 5U; i++)
    {
        glyph[i] = 0x00U;
    }

    if ((character >= '0') && (character <= '9'))
    {
        for (i = 0U; i < 5U; i++)
        {
            glyph[i] = digits[character - '0'][i];
        }
    }
    else if ((character >= 'A') && (character <= 'Z'))
    {
        for (i = 0U; i < 5U; i++)
        {
            glyph[i] = letters[character - 'A'][i];
        }
    }
    else if ((character >= 'a') && (character <= 'z'))
    {
        for (i = 0U; i < 5U; i++)
        {
            glyph[i] = letters[character - 'a'][i];
        }
    }
    else
    {
        switch (character)
        {
            case ':':
                glyph[1] = 0x36U;
                glyph[2] = 0x36U;
                break;

            case '-':
                glyph[0] = 0x08U;
                glyph[1] = 0x08U;
                glyph[2] = 0x08U;
                glyph[3] = 0x08U;
                glyph[4] = 0x08U;
                break;

            case '_':
                glyph[0] = 0x40U;
                glyph[1] = 0x40U;
                glyph[2] = 0x40U;
                glyph[3] = 0x40U;
                glyph[4] = 0x40U;
                break;

            default:
                /* Space or unsupported character */
                break;
        }
    }
}

/* Initialize SSD1306 OLED */
void OLED_Init(void)
{
    OLED_WriteCommand(0xAEU); /* Display OFF */

    OLED_WriteCommand(0xD5U);
    OLED_WriteCommand(0x80U);

    OLED_WriteCommand(0xA8U);
    OLED_WriteCommand(0x3FU);

    OLED_WriteCommand(0xD3U);
    OLED_WriteCommand(0x00U);

    OLED_WriteCommand(0x40U);

    OLED_WriteCommand(0x8DU);
    OLED_WriteCommand(0x14U);

    /* Horizontal addressing mode */
    OLED_WriteCommand(0x20U);
    OLED_WriteCommand(0x00U);

    OLED_WriteCommand(0xA1U);
    OLED_WriteCommand(0xC8U);

    OLED_WriteCommand(0xDAU);
    OLED_WriteCommand(0x12U);

    OLED_WriteCommand(0x81U);
    OLED_WriteCommand(0x7FU);

    OLED_WriteCommand(0xD9U);
    OLED_WriteCommand(0xF1U);

    OLED_WriteCommand(0xDBU);
    OLED_WriteCommand(0x40U);

    OLED_WriteCommand(0xA4U);
    OLED_WriteCommand(0xA6U);

    OLED_WriteCommand(0xAFU); /* Display ON */

    OLED_Clear();
}

/* Clear the complete 128 x 64 display */
void OLED_Clear(void)
{
    uint8_t blank[128U];
    uint8_t page;
    uint8_t i;

    for (i = 0U; i < 128U; i++)
    {
        blank[i] = 0x00U;
    }

    for (page = 0U; page < 8U; page++)
    {
        OLED_WriteCommand((uint8_t)(0xB0U + page));
        OLED_WriteCommand(0x00U);
        OLED_WriteCommand(0x10U);
        OLED_WriteData(blank, 128U);
    }
}

/* Display text with a 6-pixel character width */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str)
{
    uint8_t glyph[5];
    uint8_t characterData[6];
    uint16_t column;
    uint8_t i;

    if (str == NULL)
    {
        return;
    }

    if ((y >= OLED_HEIGHT) || ((y % 8U) != 0U))
    {
        return;
    }

    column = x;

    while (*str != '\0')
    {
        if ((column + 6U) > OLED_WIDTH)
        {
            break;
        }

        OLED_GetCharacter((uint8_t)*str, glyph);

        for (i = 0U; i < 5U; i++)
        {
            characterData[i] = glyph[i];
        }

        characterData[5] = 0x00U;

        /* Select display page and starting column */
        OLED_WriteCommand((uint8_t)(0xB0U + (y / 8U)));
        OLED_WriteCommand((uint8_t)(column & 0x0FU));
        OLED_WriteCommand(
            (uint8_t)(0x10U | ((column >> 4U) & 0x0FU))
        );

        OLED_WriteData(characterData, 6U);

        column += 6U;
        str++;
    }
}
