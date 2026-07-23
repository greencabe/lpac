// SPDX-License-Identifier: AGPL-3.0-only

#pragma once

#include <stdbool.h>
#include <stdint.h>

bool notification_parse_sequence_number(const char *value, uint32_t *sequence_number);
