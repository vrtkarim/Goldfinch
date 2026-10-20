#include "triplestore.h"
#include "fetcher.h"
#include "hashtable.h"
#include "engine.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/* the sv means that is the subject a variable */
#define TRIPLES_CAPACITY 10

static Results *create_results(void)
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

static bool find_term_id(HashTable *ht, const char *term, uint32_t *id)
{
    Entry *entry;

    if (ht == NULL || term == NULL)
    {
        return false;
    }

    entry = hashtable_find(ht, term);

    if (entry == NULL)
    {
        return false;
    }

    *id = entry->value;
    return true;
}

static bool results_add(Results *results, Triple triple)
{
    if (results == NULL)
    {
        return false;
    }

    if (results->size >= results->capacity && !triples_grow(results))
    {
        return false;
    }

    results->triples[results->size] = triple;
    results->size += 1;
    return true;
}

Results *getTriples(Triplestore *ts, char *subject, char *predicate, char *object, bool sv, bool pv, bool ov)
{
    HashTable *ht;
    Results *results;
    uint32_t subject_id = 0;
    uint32_t predicate_id = 0;
    uint32_t object_id = 0;

    if (ts == NULL || ts->dictionary == NULL)
    {
        return NULL;
    }

    ht = ts->dictionary->ht;
    results = create_results();

    if (results == NULL)
    {
        return NULL;
    }

    /*
     * Resolve each constant term once. A term that is not in the
     * dictionary cannot match any triple, so return an empty result
     * instead of dereferencing a NULL hash-table entry.
     */
    if ((!sv && !find_term_id(ht, subject, &subject_id)) ||
        (!pv && !find_term_id(ht, predicate, &predicate_id)) ||
        (!ov && !find_term_id(ht, object, &object_id)))
    {
        return results;
    }

    for (size_t i = 0; i < ts->size; i++)
    {
        Triple *triple = &ts->triples[i];

        if ((!sv && triple->subject != subject_id) ||
            (!pv && triple->predicate != predicate_id) ||
            (!ov && triple->object != object_id))
        {
            continue;
        }

        if (!results_add(results, *triple))
        {
            free_results(results);
            return NULL;
        }
    }

    return results;
}

bool triples_grow(Results *results)
{
    size_t new_capacity;
    Triple *new_triples;

    if (results == NULL || results->capacity > SIZE_MAX / 2)
    {
        return false;
    }

    new_capacity = results->capacity == 0 ? TRIPLES_CAPACITY : results->capacity * 2;

    if (new_capacity > SIZE_MAX / sizeof(*new_triples))
    {
        return false;
    }

    new_triples = realloc(results->triples, new_capacity * sizeof(*new_triples));

    if (new_triples == NULL)
    {
        return false;
    }

    results->triples = new_triples;
    results->capacity = new_capacity;
    return true;
}

void addToTriples(Results *results, Triple triple)
{
    (void)results_add(results, triple);
}

void free_results(Results *results)
{
    if (results == NULL)
    {
        return;
    }

    free(results->triples);
    free(results);
}
