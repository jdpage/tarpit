#pragma once
#include <stdint.h>

typedef struct stack {
	uint32_t v;
	struct stack *next;
} stack_t;

stack_t *stack_push(stack_t *self, uint32_t val);
stack_t *stack_pop(stack_t *self);
int stack_size(stack_t *self);
void stack_liberate(stack_t *self);
