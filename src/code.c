#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "code.h"

#define DATA_BIT_WIDTH 8

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, 7 - idx);

    return result;
}

void decimal_to_binary(int decimal_in, char *binary_out, int num_bits)
{
    int idx = num_bits - 1;
    memset(binary_out, '\0', sizeof(binary_out));
    while (idx >= 0) {
        if ((int)pow(2, idx) > decimal_in) {
            binary_out[num_bits - 1 - idx] = '0';
        } else {
            binary_out[num_bits - 1 - idx] = '1';
            decimal_in = decimal_in - (int)pow(2, idx);
            if (decimal_in == 0)
                decimal_in--;
        }
        idx--;
    }
}
