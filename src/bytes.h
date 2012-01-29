#pragma once
#include <stdint.h>

typedef struct bytes {
	uint8_t *p;
	size_t length;
	size_t size;
} bytes_t;

bytes_t *bytes_make();
void bytes_del(bytes_t *self);
void bytes_set(bytes_t *self, size_t index, uint8_t value);
uint8_t bytes_get(bytes_t *self, size_t index);
void bytes_append(bytes_t *self, uint8_t value);
void bytes_extend(bytes_t *self, uint8_t *data, size_t length);
uint8_t bytes_pop(bytes_t *self);
