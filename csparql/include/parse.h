#ifndef PARSE_H
#define PARSE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <token.h>

typedef enum Position
{
    POSITION_SUBJECT,
    POSITION_PREDICATE,
    POSITION_OBJECT
} Position;

typedef enum QueryType
{
    QUERY_SELECT,
    QUERY_CONSTRUCT,
    QUERY_ASK,
    QUERY_DESCRIBE,
    QUERY_INSERT_DATA,
    QUERY_DELETE_DATA,
    QUERY_DELETE_WHERE,
    QUERY_DELETE_INSERT
} QueryType;

typedef struct TriplePattern
{
    char *subject;
    char *predicate;
    char *object;
    bool sv;
    bool pv;
    bool ov;

} TriplePattern;
typedef struct Variable
{
    char *value;
    Position position;
} Variable;

typedef struct Variables
{

    Variable *vs;
    size_t count;
    size_t capacity;
} Variables;

typedef struct ParsedQuery
{
    QueryType type;
    Variables variables;
    TriplePattern triplePattern;
} ParsedQuery;

Variables *getVariablesAndPositions(Tokens *tokens, TriplePattern *triple);
bool addVariable(Variable variable, Variables *variables);
bool variablesArrayGrow(Variables *variables);
QueryType getType(Tokens *tokens);
TriplePattern *getTriplePattern(Tokens *tokens);
ParsedQuery *parseQuery(Tokens *tokens);
void freeVariables(Variables *variables);
#endif