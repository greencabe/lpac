// SPDX-License-Identifier: AGPL-3.0-only

#include "notification_sequence.h"

#include <stddef.h>
#include <stdint.h>

bool notification_parse_sequence_number(const char *value, uint32_t *sequence_number) {
    uint32_t parsed = 0;

    if (value == NULL || value[0] == '\0' || sequence_number == NULL) {
        return false;
    }

    for (const unsigned char *p = (const unsigned char *)value; *p != '\0'; p++) {
        uint32_t digit;

        if (*p < '0' || *p > '9') {
            return false;
        }

        digit = (uint32_t)(*p - '0');
        if (parsed > (UINT32_MAX - digit) / 10U) {
            return false;
        }
        parsed = parsed * 10U + digit;
    }

    *sequence_number = parsed;
    return true;
}
