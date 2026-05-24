#include "stm32f10x.h"

void Int2Str(uint8_t *p_str, uint32_t intnum)
{
    uint32_t i, div = 1000000000;

    for (i = 0; i < 10; i++)
    {
        p_str[i] = '0' + (intnum / div);
        intnum = intnum % div;
        div /= 10;
    }
    p_str[10] = '\0';
}

uint32_t Str2Int(uint8_t *inputstr, uint32_t *intnum)
{
    uint32_t i = 0, res = 0;

    while ((inputstr[i] >= '0') && (inputstr[i] <= '9'))
    {
        res = (res * 10) + (inputstr[i] - '0');
        i++;
    }
    *intnum = res;
    return i;
}
