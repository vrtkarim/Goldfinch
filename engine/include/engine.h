#ifndef ENGINE_H
#define ENGINE_H
#include <stdint.h>
#include <stddef.h>
#include "triplestore.h"
#include "parse.h"

typedef struct Results
{
    size_t size;
    size_t capacity;
    Triple *triples;
} Results;

typedef struct NumericRDFterm
{
    uint32_t value;
    bool isVariable;
} NumericRDFterm;

typedef struct NumericTriplePattern
{
    NumericRDFterm terms[3];
    bool isOptional;
} NumericTriplePattern;

typedef struct NumericTriplePatterns
{
    NumericTriplePattern *arrOfTriplePatterns;
    size_t size;
    size_t capacity;
} NumericTriplePatterns;

Results *initResults(void);
Results *getTriples(Triplestore *ts, TriplePatterns *triplepatterns);
void addToResults(Results *results, Triple triple);
bool growResults(Results *results);
void freeResults(Results *results);
bool mapTriplePattern(
    const TriplePattern *pattern,
    HashTable *hashTable,
    NumericTriplePattern *mappedPattern);
bool mapTriplePatterns(
    const TriplePatterns *triplePatterns,
    HashTable *hashTable,
    NumericTriplePatterns *mappedPatterns);

#endif