#ifndef ENGINE_H
#define ENGINE_H
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "triplestore.h"

typedef struct Results
{
    size_t size;
    size_t capacity;
    Triple *triples;
} Results;

Results *getTriples(Triplestore *ts, char *subject, char *predicate, char *object, bool sv, bool pv, bool ov);
void addToTriples(Results *results, Triple triple);
bool triples_grow(Results *results);
void free_results(Results *results);

#endif
