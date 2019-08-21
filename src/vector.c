#include <stdlib.h>
#include <stdio.h>

#include "vec.h"

vector *
create_vector(void) {
	vector *ret = malloc(sizeof(vector));
	ret->elements = malloc(sizeof(void *));
	ret->size = 0;
	ret->mem = 1;

	fprintf(stderr, "%s\n", "vector created");
	return ret;
}

void
add_to_vector(vector *vec, void *data){
	if (vec->size == vec->mem) {
		vec->mem *= 2;
		fprintf(stderr, "%s: %d\n", "doubling mem to", vec->mem);
		vec->elements = realloc(vec->elements, sizeof(void *) * vec->mem);
	}

	vec->elements[vec->size++] = data;
	fprintf(stderr, "%s\n", "written to vector");
}

void *
remove_from_vector(vector *vec, unsigned int index) {
	if (index >= vec->size) {
		fprintf(stderr, "index %d out of elements array size\n", index);
		return NULL;
	}
	void *ret = vec->elements[index];
	// move all the elements down one
	for (; index < vec->size; index++) {
		vec->elements[index] = vec->elements[index + 1];
	}
	// set the last element to null so we dont have duplicates
	vec->elements[vec->size--] == NULL;

	// get rid of memory we aint usin
	if (vec->size * 2 == vec->mem) {
		vec->mem /= 2;
		vec->elements = realloc(vec->elements, sizeof(void *) * vec->mem);
	}
	fprintf(stderr, "vector at %d removed\n", index);
	return ret;
}

void *
get_from_vector(vector *vec, unsigned int index) {
	if (index >= vec->size) {
		fprintf(stderr, "index %d out of elements array size\n", index);
		return NULL;
	}

	return vec->elements[index];
}

void
destroy_vector(vector *vec) {
	free(vec->elements);
	free(vec);
}