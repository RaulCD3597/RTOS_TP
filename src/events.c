/*
 * events.c
 *
 *  Created on: May 30, 2020
 *      Author: raul
 */
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

uint8_t getValueLength(uint8_t *value)
{
    uint8_t length = 0;
    while (isalnum(value[length]) || isspace(value[length]) || value[length] == '?' || value[length] == '!')
    {
        length++;
    }
    return length;
}