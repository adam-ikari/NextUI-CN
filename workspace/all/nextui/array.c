#include "array.h"
#include <stdlib.h>
#include <string.h>

Array* Array_new(void) {
	Array* self = malloc(sizeof(Array));
	self->count = 0;
	self->capacity = 8;
	self->items = malloc(sizeof(void*) * self->capacity);
	return self;
}

void Array_free(Array* self) {
	if (self) {
		if (self->items) {
			free(self->items);
		}
		free(self);
	}
}

void Array_push(Array* self, void* item) {
	if (!self) return;
	
	if (self->count >= self->capacity) {
		self->capacity *= 2;
		self->items = realloc(self->items, sizeof(void*) * self->capacity);
	}
	self->items[self->count++] = item;
}

void* Array_get(Array* self, int index) {
	if (!self || index < 0 || index >= self->count) {
		return NULL;
	}
	return self->items[index];
}

void Array_clear(Array* self) {
	if (!self) return;
	self->count = 0;
}

void Array_remove(Array* self, void* item) {
	if (!self || !item || self->count == 0) return;
	
	int i = 0;
	while (i < self->count && self->items[i] != item) {
		i++;
	}
	
	if (i < self->count) {
		// Shift items to fill the gap
		for (int j = i; j < self->count - 1; j++) {
			self->items[j] = self->items[j + 1];
		}
		self->count--;
	}
}