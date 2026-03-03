#ifndef __ARRAY_H__
#define __ARRAY_H__

#include <stdlib.h>

typedef struct Array {
	int count;
	int capacity;
	void** items;
} Array;

Array* Array_new(void);
void Array_free(Array* self);
void Array_push(Array* self, void* item);
void* Array_get(Array* self, int index);
void Array_remove(Array* self, void* item);
void Array_clear(Array* self);

#endif // __ARRAY_H__