#include <stdio.h>
#include <stdlib.h>
#include "config.h"
#include "stack.h"

stack_t *stack_push(stack_t *self, uint32_t val) {
	stack_t *head;
	if ((head = (stack_t *)malloc(sizeof(stack_t))) == NULL) {
		PRINT_ERROR(MSG_OUT_OF_MEMORY);
		abort();
	}
	head->v = val;
	head->next = self;
	return head;
}

stack_t *stack_pop(stack_t *self) {
	stack_t *head;
	if (self == NULL) {
		PRINT_ERROR(MSG_POPPED_EMPTY_STACK);
		abort();
	}
	head = self->next;
	free(self);
	return head;
}

int stack_size(stack_t *self) {
	int acc = 0;
	while (self != NULL) {
		acc++;
		self = self->next;
	}
	return acc;
}

void stack_liberate(stack_t *self) {
	stack_t *head;
	while (self != NULL) {
		head = self->next;
		free(self);
		self = head;
	}
}

