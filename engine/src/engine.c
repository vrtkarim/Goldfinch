#include "triplestore.h"
#include "fetcher.h"
#include "hashtable.h"
#include "engine.h"
#include "parse.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
/* the sv means that is the subject a variable */
#define TRIPLES_CAPACITY 10

Results *initResults(void)
{
    Results *results = malloc(sizeof(*results));

    if (results == NULL)
    {
        return NULL;
    }

    results->triples = malloc(TRIPLES_CAPACITY * sizeof(*results->triples));
    if (results->triples == NULL)
    {
        free(results);
        return NULL;
    }

    results->size = 0;
    results->capacity = TRIPLES_CAPACITY;
    return results;
}

Results *getTriples(Triplestore *ts, TriplePatterns *triplepatterns)
{
    /* typedef struct TriplePattern
    {
        RDFterm *terms;
        bool isOptional;
    } TriplePattern; */

    /* typedef struct RDFterm
    {
        char *value;
        bool isVariable;
    } RDFterm; */

    Triple *data = ts->triples;
    HashTable *ht = ts->dictionary->ht;
    Results *results = initResults();
    NumericTriplePatterns *mappedPatterns = malloc(sizeof(*mappedPatterns));

    if (results == NULL)
    {
        return NULL;
    }

    if (!mapTriplePatterns(
            triplepatterns,
            ht,
            mappedPatterns))
    {
        freeResults(results);
        return NULL;
    }

    for (size_t triplePatternCursor = 0;
         triplePatternCursor < mappedPatterns->size;
         triplePatternCursor++)
    {
        
        for (size_t cursor = 0; cursor < ts->size; cursor++)
        {
            Triple triple = ts->triples[cursor];

            bool subjectMatches =
                mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[0].isVariable ||
                mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[0].value == triple.subject;

            bool predicateMatches =
                mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[1].isVariable ||
                mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[1].value == triple.predicate;

            bool objectMatches =
               mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[2].isVariable ||
                mappedPatterns->arrOfTriplePatterns[triplePatternCursor].terms[2].value == triple.object;

            if (subjectMatches &&
                predicateMatches &&
                objectMatches)
            {
                addToResults(results, triple);
            }
        }
        if(triplePatternCursor==1){
            break; 
        }
    }

    free(mappedPatterns->arrOfTriplePatterns);
    free(mappedPatterns);
    return results;
}
bool growResults(Results *results)
{
    size_t newcapacity = results->capacity + TRIPLES_CAPACITY;
    Triple *new = realloc(results->triples, newcapacity * sizeof(Triple *));
    /* array size 1, zdtilo 2   0 1 2 , size 3, */
    if (new != NULL)
    {
        results->triples = new;
        results->capacity += TRIPLES_CAPACITY;
        free(new);
        return true;
    }
    else
    {
        return false;
    }
}
void addToResults(Results *results, Triple triple)
{
    results->triples[results->size] = triple;
    results->size += 1;
    printf("size of triples in engine: %u\n", results->size);
    if (results->size >= results->capacity)
    {
        if (growResults(results))
        {
            /* code */
        }
        else
        {
            return;
        }
    }
}
void freeResults(Results *results)
{
    if (results == NULL)
    {
        return;
    }

    free(results->triples);

    free(results);
}
bool mapTriplePatternToTriple(
    const TriplePattern *pattern,
    HashTable *hashTable,
    Triple *triple)
{
    if (pattern == NULL ||
        pattern->terms == NULL ||
        hashTable == NULL ||
        triple == NULL)
    {
        return false;
    }

    uint32_t *values[3] = {
        &triple->subject,
        &triple->predicate,
        &triple->object};

    for (size_t index = 0; index < 3; index++)
    {
        const RDFterm *term = &pattern->terms[index];

        if (term->isVariable)
        {
            *values[index] = 0;
            continue;
        }

        if (term->value == NULL)
        {
            return false;
        }

        Entry *entry = hashtable_find(hashTable, term->value);

        if (entry == NULL)
        {
            return false;
        }

        *values[index] = entry->value;
    }

    return true;
}
bool mapTriplePattern(
    const TriplePattern *pattern,
    HashTable *hashTable,
    NumericTriplePattern *mappedPattern)
{
    if (pattern == NULL ||
        pattern->terms == NULL ||
        hashTable == NULL ||
        mappedPattern == NULL)
    {
        return false;
    }

    mappedPattern->isOptional = pattern->isOptional;

    for (size_t index = 0; index < 3; index++)
    {
        const RDFterm *term = &pattern->terms[index];

        mappedPattern->terms[index].isVariable = term->isVariable;

        if (term->isVariable)
        {
            mappedPattern->terms[index].value = 0;
            continue;
        }

        if (term->value == NULL)
        {
            return false;
        }

        Entry *entry = hashtable_find(hashTable, term->value);

        if (entry == NULL)
        {
            return false;
        }

        mappedPattern->terms[index].value = entry->value;
    }

    return true;
}

bool mapTriplePatterns(
    const TriplePatterns *triplePatterns,
    HashTable *hashTable,
    NumericTriplePatterns *mappedPatterns)
{
    if (triplePatterns == NULL ||
        hashTable == NULL ||
        mappedPatterns == NULL)
    {
        return false;
    }

    size_t capacity = triplePatterns->capacity;

    if (capacity == 0)
    {
        capacity = 1;
    }

    mappedPatterns->arrOfTriplePatterns =
        malloc(capacity * sizeof(*mappedPatterns->arrOfTriplePatterns));

    if (mappedPatterns->arrOfTriplePatterns == NULL)
    {
        return false;
    }

    mappedPatterns->size = 0;
    mappedPatterns->capacity = capacity;

    for (size_t index = 0; index < triplePatterns->size; index++)
    {
        const TriplePattern *pattern =
            &triplePatterns->arrOfTriplePatterns[index];

        NumericTriplePattern *mappedPattern =
            &mappedPatterns->arrOfTriplePatterns[mappedPatterns->size];

        if (!mapTriplePattern(
                pattern,
                hashTable,
                mappedPattern))
        {
            free(mappedPatterns->arrOfTriplePatterns);
            mappedPatterns->arrOfTriplePatterns = NULL;
            mappedPatterns->size = 0;
            mappedPatterns->capacity = 0;
            return false;
        }

        mappedPatterns->size++;
    }

    return true;
}