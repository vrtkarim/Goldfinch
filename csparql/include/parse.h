#ifndef PARSE_H
#define PARSE_H

#include <stdbool.h>
#include <stddef.h>
#include "token.h"

typedef struct orderBy
{
    char *variable;
    char *order;
} orderBy;

typedef struct Limit
{
    unsigned int limit;
} Limit;

typedef struct Offset
{
    unsigned int offset;
} Offset;

typedef struct ProjectionExpression
{
    char *function;
    char *parameter;
    char *variable;
} ProjectionExpression;

typedef struct Variable
{
    char *value;
    bool isVariable;
} Variable;

typedef struct Variables
{
    Variable *arrOfVariables;
    size_t size;
    size_t capacity;
} Variables;

typedef struct RDFterm
{
    char *value;
    bool isVariable;
} RDFterm;

typedef struct TriplePattern
{
    RDFterm *terms;
    bool isOptional;
} TriplePattern;

typedef struct TriplePatterns
{
    TriplePattern *arrOfTriplePatterns;
    size_t size;
    size_t capacity;
} TriplePatterns;

typedef struct Select
{
    Variables *variables;
    ProjectionExpression *projectionExpressions;
    TriplePatterns *triplePatterns;
    bool isDistinct;
} Select;

orderBy getOrderBy(Tokens *tokens, unsigned int index);
Offset getOffset(Tokens *tokens, unsigned int index);
Limit getLimit(Tokens *tokens, unsigned int index);
unsigned int getSelectTokens(Tokens *tokens, unsigned int index, Select *select);
ProjectionExpression *getProjectionExpression(Tokens *tokens, unsigned int index);
void parseTokens(Tokens *tokens);
unsigned int rightBracesCount(Tokens *tokens);
Variables *createVariables(size_t capacity);
void addToVariables(Variable variable, Variables *variables);
bool variablesArrayGrow(Variables *variables);
void freeVariables(Variables *variables);
TriplePatterns *createTriplePatterns(size_t capacity);
void addToTriplePatterns(TriplePattern triplePattern, TriplePatterns *triplePatterns);
bool triplePatternsArrayGrow(TriplePatterns *triplePatterns);
void freeTriplePatterns(TriplePatterns *triplePatterns);
void printTriplePatterns(const TriplePatterns *triplePatterns);
void printSelect(const Select *select);
unsigned int getTriplePatterns(Tokens *tokens, unsigned int index, TriplePatterns *triplePatterns);
unsigned int getOptionalTriplePatterns(Tokens *tokens, unsigned int index, TriplePatterns *triplePatterns);
unsigned int getTriple(Tokens *tokens, unsigned int index, TriplePattern *triplePattern);

#endif
