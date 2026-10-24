#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parseTokens(Tokens *tokens)
{
    orderBy orderby = {0};
    Limit limit = {0};
    Select *select = malloc(sizeof(Select));
    size_t i = 0;
    for (i; i < tokens->size; i++)
    {
        TokenType type = tokens->arrOfTokens[i].type;
        if (type == TOKEN_SELECT)
        {
            i = getSelectTokens(tokens, i, select);
        }
    }
    printSelect(select);
}
Limit getLimit(Tokens *tokens, unsigned int index)
{
}
orderBy getOrderBy(Tokens *tokens, unsigned int index)
{
    orderBy orderby = {0};

    for (size_t current = index + 1; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;

        if (type == TOKEN_ASC || type == TOKEN_DESC)
        {
            orderby.order = tokens->arrOfTokens[current].text;
        }
        else if (type == TOKEN_VARIABLE)
        {
            orderby.variable = tokens->arrOfTokens[current].text;
        }
        else if (type == TOKEN_RPAREN)
        {
            break;
        }
    }

    return orderby;
}
unsigned int rightBracesCount(Tokens *tokens)
{
    int count = 0;
    for (int index = 0; index < tokens->size; index++)
    {
        TokenType type = tokens->arrOfTokens[index].type;
        if (type == TOKEN_LBRACE)
        {
            count++;
        }
    }
    return count;
}
unsigned int getSelectTokens(Tokens *tokens, unsigned int index, Select *select)
{

    select->variables = createVariables(2);
    select->triplePatterns = createTriplePatterns(2);

    bool beforeWhere = true;
    size_t current = index;
    int braceDepth = 0;

    for (current; current < tokens->size; current++)
    {

        TokenType type = tokens->arrOfTokens[current].type;
        printf("type=%s, beforeWhere=%s\n",
               tokenTypeName(type),
               beforeWhere ? "true" : "false");

        if (type == TOKEN_LBRACE)
        {
            braceDepth++;
        }
        else if (type == TOKEN_RBRACE)
        {
            braceDepth--;
        }

        if (type == TOKEN_DISTINCT)
        {
            select->isDistinct = true;
            continue;
        }
        if (type == TOKEN_WHERE)
        {
            beforeWhere = false;
            continue;
        }

        if (type == TOKEN_VARIABLE && beforeWhere == true)
        {
            Variable var = {
                0};
            var.value = tokens->arrOfTokens[current].text;
            var.isVariable = startsWith(var.value, '?');
            addToVariables(var, select->variables);
            continue;
        }
        else if (type == TOKEN_OPTIONAL)
        {

            current = getOptionalTriplePatterns(tokens, current, select->triplePatterns);
        }
        if (type == TOKEN_VARIABLE && braceDepth > 0)
        {
            current = getTriplePatterns(tokens, current, select->triplePatterns);
        }
        if (type == TOKEN_VARIABLE && braceDepth == 0)
        {
            return current;
        }
    }
    return current;
}
ProjectionExpression *getProjectionExpression(Tokens *tokens, unsigned int index)
{
    ProjectionExpression *expression = malloc(sizeof(ProjectionExpression));

    for (size_t current = index; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;
        if (type == TOKEN_COUNT)
        {
            expression->function = tokens->arrOfTokens[current].text;
            if (tokens->arrOfTokens[current + 1].type == TOKEN_LPAREN)
            {
                expression->parameter = tokens->arrOfTokens[current + 2].text;
            }
        }
        else if (type == TOKEN_AS)
        {
            expression->variable = tokens->arrOfTokens[current + 1].text;
            return expression;
        }

        else if (type == TOKEN_LBRACE)
        {
            return NULL;
        }
    }
}
Variables *createVariables(size_t capacity)
{
    Variables *variables = malloc(sizeof(*variables));

    if (variables == NULL)
    {
        return NULL;
    }

    if (capacity == 0)
    {
        capacity = 1;
    }

    variables->capacity = capacity;
    variables->size = 0;
    variables->arrOfVariables = malloc(capacity * sizeof(*variables->arrOfVariables));

    if (variables->arrOfVariables == NULL)
    {
        free(variables);
        return NULL;
    }

    return variables;
}

void addToVariables(Variable variable, Variables *variables)
{
    if (variables == NULL)
    {
        return;
    }

    if (variables->size >= variables->capacity && !variablesArrayGrow(variables))
    {
        return;
    }

    variables->arrOfVariables[variables->size] = variable;
    variables->size++;
}

bool variablesArrayGrow(Variables *variables)
{
    if (variables == NULL)
    {
        return false;
    }

    size_t newCapacity = variables->capacity == 0 ? 1 : variables->capacity * 2;
    Variable *newVariables = realloc(
        variables->arrOfVariables,
        newCapacity * sizeof(*variables->arrOfVariables));

    if (newVariables == NULL)
    {
        return false;
    }

    variables->arrOfVariables = newVariables;
    variables->capacity = newCapacity;
    return true;
}

void freeVariables(Variables *variables)
{
    if (variables == NULL)
    {
        return;
    }

    free(variables->arrOfVariables);
    free(variables);
}

TriplePatterns *createTriplePatterns(size_t capacity)
{
    TriplePatterns *triplePatterns = malloc(sizeof(*triplePatterns));

    if (triplePatterns == NULL)
    {
        return NULL;
    }

    if (capacity == 0)
    {
        capacity = 1;
    }

    triplePatterns->capacity = capacity;
    triplePatterns->size = 0;
    triplePatterns->arrOfTriplePatterns = malloc(
        capacity * sizeof(*triplePatterns->arrOfTriplePatterns));

    if (triplePatterns->arrOfTriplePatterns == NULL)
    {
        free(triplePatterns);
        return NULL;
    }

    return triplePatterns;
}

void addToTriplePatterns(
    TriplePattern triplePattern,
    TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return;
    }

    if (triplePatterns->size >= triplePatterns->capacity &&
        !triplePatternsArrayGrow(triplePatterns))
    {
        return;
    }

    triplePatterns->arrOfTriplePatterns[triplePatterns->size] = triplePattern;
    triplePatterns->size++;
}

bool triplePatternsArrayGrow(TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return false;
    }

    size_t newCapacity = triplePatterns->capacity == 0
                             ? 1
                             : triplePatterns->capacity * 2;
    TriplePattern *newTriplePatterns = realloc(
        triplePatterns->arrOfTriplePatterns,
        newCapacity * sizeof(*triplePatterns->arrOfTriplePatterns));

    if (newTriplePatterns == NULL)
    {
        return false;
    }

    triplePatterns->arrOfTriplePatterns = newTriplePatterns;
    triplePatterns->capacity = newCapacity;
    return true;
}

void freeTriplePatterns(TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return;
    }

    free(triplePatterns->arrOfTriplePatterns);
    free(triplePatterns);
}

void printSelect(const Select *select)
{
    if (select == NULL)
    {
        printf("Select is NULL.\n");
        return;
    }

    printf("\n========== SELECT ==========\n");
    printf("DISTINCT: %s\n", select->isDistinct ? "true" : "false");

    // Print variables
    if (select->variables != NULL && select->variables->size > 0)
    {
        printf("\n--- Variables (%zu) ---\n", select->variables->size);
        for (size_t i = 0; i < select->variables->size; i++)
        {
            Variable v = select->variables->arrOfVariables[i];
            printf("  var[%zu]: value=%s, isVariable=%s\n",
                   i,
                   v.value ? v.value : "(null)",
                   v.isVariable ? "true" : "false");
        }
    }
    else
    {
        printf("\n--- No variables ---\n");
    }

    // Print projection expression

    // Print triple patterns
    printTriplePatterns(select->triplePatterns);

    printf("============================\n");
}

void printTriplePatterns(const TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        printf("No triple patterns found.\n");
        return;
    }

    printf("\n=== Triple Patterns (%zu) ===\n", triplePatterns->size);

    for (size_t patternIndex = 0;
         patternIndex < triplePatterns->size;
         patternIndex++)
    {
        const TriplePattern *pattern =
            &triplePatterns->arrOfTriplePatterns[patternIndex];

        printf("Triple[%zu]: optional=%s\n",
               patternIndex,
               pattern->isOptional ? "true" : "false");

        if (pattern->terms == NULL)
        {
            printf("  no terms\n");
            continue;
        }

        for (size_t termIndex = 0; termIndex < 3; termIndex++)
        {
            const RDFterm *term = &pattern->terms[termIndex];
            printf("  term[%zu]: value=%s, isVariable=%s\n",
                   termIndex,
                   term->value == NULL ? "(null)" : term->value,
                   term->isVariable ? "true" : "false");
        }
    }
}

unsigned int getOptionalTriplePatterns(Tokens *tokens, unsigned int index, TriplePatterns *TriplePatterns)
{
    size_t current = index;
    for (current; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;

        if (type == TOKEN_VARIABLE || type == TOKEN_IRI)
        {
            TriplePattern *trp = malloc(sizeof(TriplePattern *));
            trp->isOptional = true;
            current = getTriple(tokens, current, trp);
            addToTriplePatterns(*trp, TriplePatterns);
        }

        if (type == TOKEN_END_OPTIONAL)
        {
            return current;
        }
    }
};
unsigned int getTriplePatterns(Tokens *tokens, unsigned int index, TriplePatterns *TriplePatterns)
{

    /* typedef struct RDFterm
    {
        char *value;
        bool isVariable;
    } RDFterm;
    ----------------------------------------------------
    typedef struct TriplePattern
    {
        RDFterm *terms;
        bool isOptional;

    } TriplePattern;
    ---------------------------------------------------
    typedef struct TriplePatterns
    {
        TriplePattern *arrOfTriplePatterns;
        size_t size;
        size_t capacity;
    } TriplePatterns;
    ---------------------------------------------------
    NOW I WANNA GET THE TRIPLES, IS FIRST TERM, VARIABLE, URI,
    ADD IT AT FIRST, BREAK WHEN THERE IS A DOT
    GET OPTIONAL IF OPTIONAL
    SHOULD I USE RECURSIVE PARSING TO GET ALL THE TRIPLES
     */
    size_t current = index;
    for (current; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;
        if (type == TOKEN_VARIABLE || type == TOKEN_IRI)
        {
            TriplePattern *trp = malloc(sizeof(TriplePattern *));
            trp->isOptional = false;
            current = getTriple(tokens, current, trp);
            addToTriplePatterns(*trp, TriplePatterns);
        }

        if (type == TOKEN_DOT)
        {

            return current;
        }
    }

    return current;
}
unsigned int getTriple(Tokens *tokens, unsigned int index, TriplePattern *tp)

{

    tp->terms = malloc(3 * sizeof(RDFterm));
    int n = 0;
    for (index; index < tokens->size; index++)
    {
        RDFterm rdfterm;
        rdfterm.value = tokens->arrOfTokens[index].text;
        TokenType type = tokens->arrOfTokens[index].type;

        if (type == TOKEN_VARIABLE)
        {
            rdfterm.isVariable = true;
        }
        else
        {
            rdfterm.isVariable = false;
        }
        tp->terms[n] = rdfterm;
        n++;

        if (n == 3)
        {

            return index;
        }
    }
}