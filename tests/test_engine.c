#include "engine.h"
#include "triplestore.h"

#include <stdio.h>

#define CHECK(condition)                                                     \
    do                                                                       \
    {                                                                        \
        if (!(condition))                                                    \
        {                                                                    \
            fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__,          \
                    #condition);                                             \
            return 1;                                                        \
        }                                                                    \
    } while (0)

/*
 * The store contains 21 triples so that the all-variable query has to
 * grow the result array beyond its initial capacity.
 *
 *   s1 p1 o1
 *   s2 p1 o1
 *   s1 p2 o2
 *   s3..s20 p1 o1
 */
static Triplestore *make_store(void)
{
    Triplestore *ts = create();

    if (ts == NULL)
    {
        return NULL;
    }

    insert(ts, "s1", "p1", "o1");
    insert(ts, "s2", "p1", "o1");
    insert(ts, "s1", "p2", "o2");

    for (int i = 3; i <= 20; i++)
    {
        char subject[16];
        snprintf(subject, sizeof(subject), "s%d", i);
        insert(ts, subject, "p1", "o1");
    }

    return ts;
}

static int test_exact_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;
    uint32_t subject;
    uint32_t predicate;
    uint32_t object;

    CHECK(ts != NULL);
    results = getTriples(ts, "s1", "p1", "o1", false, false, false);
    CHECK(results != NULL);
    CHECK(results->size == 1);

    subject = dictionary_get_id(ts->dictionary, "s1");
    predicate = dictionary_get_id(ts->dictionary, "p1");
    object = dictionary_get_id(ts->dictionary, "o1");
    CHECK(results->triples[0].subject == subject);
    CHECK(results->triples[0].predicate == predicate);
    CHECK(results->triples[0].object == object);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_subject_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, "s1", NULL, NULL, false, true, true);
    CHECK(results != NULL);
    CHECK(results->size == 2);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_predicate_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, NULL, "p1", NULL, true, false, true);
    CHECK(results != NULL);
    CHECK(results->size == 20);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_object_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, NULL, NULL, "o2", true, true, false);
    CHECK(results != NULL);
    CHECK(results->size == 1);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_subject_predicate_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, "s1", "p1", NULL, false, false, true);
    CHECK(results != NULL);
    CHECK(results->size == 1);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_subject_object_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, "s1", NULL, "o2", false, true, false);
    CHECK(results != NULL);
    CHECK(results->size == 1);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_predicate_object_lookup(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, NULL, "p2", "o2", true, false, false);
    CHECK(results != NULL);
    CHECK(results->size == 1);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_all_variables_growth(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);
    results = getTriples(ts, NULL, NULL, NULL, true, true, true);
    CHECK(results != NULL);
    CHECK(results->size == 21);
    CHECK(results->capacity >= results->size);

    free_results(results);
    triplestore_free(ts);
    return 0;
}

static int test_missing_terms(void)
{
    Triplestore *ts = make_store();
    Results *results;

    CHECK(ts != NULL);

    results = getTriples(ts, "missing", NULL, NULL, false, true, true);
    CHECK(results != NULL);
    CHECK(results->size == 0);
    free_results(results);

    results = getTriples(ts, NULL, "missing", NULL, true, false, true);
    CHECK(results != NULL);
    CHECK(results->size == 0);
    free_results(results);

    results = getTriples(ts, NULL, NULL, "missing", true, true, false);
    CHECK(results != NULL);
    CHECK(results->size == 0);
    free_results(results);

    results = getTriples(ts, "missing", "missing", "missing", false, false, false);
    CHECK(results != NULL);
    CHECK(results->size == 0);
    free_results(results);

    triplestore_free(ts);
    return 0;
}

static int test_null_inputs(void)
{
    Triplestore empty = {0};
    Triple triple = {1, 2, 3};
    Results no_results = {0};

    CHECK(getTriples(NULL, NULL, NULL, NULL, true, true, true) == NULL);
    CHECK(getTriples(&empty, NULL, NULL, NULL, true, true, true) == NULL);

    addToTriples(NULL, triple);
    free_results(NULL);

    CHECK(triples_grow(NULL) == false);

    no_results.capacity = SIZE_MAX / 2 + 1;
    CHECK(triples_grow(&no_results) == false);
    CHECK(no_results.triples == NULL);

    return 0;
}

int main(void)
{
    int failures = 0;

#define RUN(test)                                                            \
    do                                                                       \
    {                                                                        \
        if (test() == 0)                                                     \
        {                                                                    \
            printf("PASS: %s\n", #test);                                     \
        }                                                                    \
        else                                                                 \
        {                                                                    \
            failures++;                                                      \
        }                                                                    \
    } while (0)

    RUN(test_exact_lookup);
    RUN(test_subject_lookup);
    RUN(test_predicate_lookup);
    RUN(test_object_lookup);
    RUN(test_subject_predicate_lookup);
    RUN(test_subject_object_lookup);
    RUN(test_predicate_object_lookup);
    RUN(test_all_variables_growth);
    RUN(test_missing_terms);
    RUN(test_null_inputs);

#undef RUN

    if (failures != 0)
    {
        fprintf(stderr, "%d test(s) failed\n", failures);
        return 1;
    }

    printf("All tests passed\n");
    return 0;
}
