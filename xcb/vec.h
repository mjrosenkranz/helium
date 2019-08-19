#ifndef VEC_H
#define VEC_H

typedef struct {
	void **elements;
	unsigned int size;
	unsigned int mem;
} vector;

typedef struct {
	int val;
} tester;

vector *create_vector(void);
void add_to_vector(vector *vec, void *data);
void * remove_from_vector(vector *vec, unsigned int index);
void *get_from_vector(vector *vec, unsigned int index);
void destroy_vector(vector *vec);
void print_vectors(vector *vec);

#endif
