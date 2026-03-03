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