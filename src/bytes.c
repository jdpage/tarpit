#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "bytes.h"
#include "barf.h"

#define INITIAL_SIZE 16

bytes_t *bytes_make() {
	bytes_t *self;
	if ((self = (bytes_t *)malloc(sizeof(bytes_t))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}
	if ((self->p = (uint8_t *)malloc(INITIAL_SIZE * sizeof(uint8_t))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}
	self->size = INITIAL_SIZE;
	self->length = 0;
	return self;
}

void bytes_del(bytes_t *self) {
	free(self->p);
	free(self);
}

void bytes_set(bytes_t *self, size_t index, uint8_t value) {
	if (index >= self->length) {
		PRINT_ERROR(MSG_WRITE_OUT_OF_BOUNDS, (unsigned int)index, (unsigned int)self->length);
		barf();
		abort();
	}
	self->p[index] = value;
}

uint8_t bytes_get(bytes_t *self, size_t index) {
	if (index >= self->length) {
		PRINT_ERROR(MSG_READ_OUT_OF_BOUNDS, (unsigned int)index, (unsigned int)self->length);
		barf();
		abort();
	}
	return self->p[index];
}

void bytes_append(bytes_t *self, uint8_t value) {
	if (self->size == self->length) {
		if ((self->p = (uint8_t *)realloc(self->p, 2 * self->size * sizeof(uint8_t))) == NULL) {
			PRINT_ERROR(MSG_OUT_OF_MEMORY);
			abort();
		}
		self->size *= 2;
	}
	self->p[self->length++] = value;
}

void bytes_extend(bytes_t *self, uint8_t *data, size_t length) {
	if (self->size < self->length + length) {
		int newsize = self->size;
		while (newsize < self->length + length)
			newsize *= 2;
		if ((self->p = (uint8_t *)realloc(self->p, newsize * sizeof(uint8_t))) == NULL) {
			PRINT_ERROR(MSG_OUT_OF_MEMORY);
			abort();
		}
		self->size = newsize;
	}
	memcpy(self->p + self->length, data, length);
	self->length += length;
}

uint8_t bytes_pop(bytes_t *self) {
	return self->p[--self->length];
}
