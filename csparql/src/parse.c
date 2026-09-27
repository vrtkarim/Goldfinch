#include <parse.h>
#include <stdlib.h>
#include <string.h>

ParsedQuery *parseQuery(Tokens *tokens)
{
    ParsedQuery *parsedquery = malloc(sizeof(*parsedquery));

    if (parsedquery == NULL)
    {
        return NULL;
    }

    TriplePattern *tp = getTriplePattern(tokens);
    Variables *vs = getVariablesAndPositions(tokens, tp);

    if (tp == NULL || vs == NULL)
    {
        free(tp);
        free(vs);
        free(parsedquery);
        return NULL;
    }

    QueryType type = getType(tokens);
    parsedquery->triplePattern = *tp;
    parsedquery->variables = *vs;
    parsedquery->type = type;
    free(tp);
    free(vs);
    return parsedquery;
}

Variables *getVariablesAndPositions(Tokens *tokens, TriplePattern *triple)
{
    Variables *variables = malloc(sizeof(*variables));

    if (variables == NULL || tokens == NULL || triple == NULL)
    {
        free(variables);
        return NULL;
    }

    variables->vs = malloc(3 * sizeof(*variables->vs));
    variables->count = 0;
    variables->capacity = 3;

    for (size_t i = 0; i < tokens->size; i++)
    {
        Token *token = &tokens->arrOfTokens[i];

        if (token->type != TOKEN_VARIABLE)
        {
            continue;
        }

        Variable variable = {
            .value = token->text,
            .position = POSITION_SUBJECT};

        if (triple->subject != NULL &&
            strcmp(token->text, triple->subject) == 0)
        {
            variable.position = POSITION_SUBJECT;
            addVariable(variable, variables);
        }
        if (triple->predicate != NULL &&
            strcmp(token->text, triple->predicate) == 0)
        {
            variable.position = POSITION_PREDICATE;
            addVariable(variable, variables);
        }
        if (triple->object != NULL &&
            strcmp(token->text, triple->object) == 0)
        {
            variable.position = POSITION_OBJECT;
            addVariable(variable, variables);
        }
    }

    return variables;
}
QueryType getType(Tokens *tokens)
{
    return QUERY_SELECT; // working on select, no need to be implemented;
};
bool addVariable(Variable variable, Variables *variables)
{
    if (variables == NULL)
    {
        return false;
    }

    if (variables->count >= variables->capacity)
    {
        if (!variablesArrayGrow(variables))
        {
            return false;
        }
    }

    variables->vs[variables->count] = variable;
    variables->count++;
    return true;
}
bool variablesArrayGrow(Variables *variables)
{
    size_t newcapacity = 2 * variables->capacity;
    Variable *new = realloc(
        variables->vs,
        newcapacity * sizeof(*variables->vs));

    if (new == NULL)
    {
        return false;
    }

    variables->vs = new;
    variables->capacity = newcapacity;
    return true;
}
TriplePattern *getTriplePattern(Tokens *tokens)
{
    TriplePattern *triple = malloc(sizeof(*triple));

    if (triple == NULL || tokens == NULL)
    {
        free(triple);
        return NULL;
    }

    triple->subject = NULL;
    triple->predicate = NULL;
    triple->object = NULL;
    triple->sv = false;
    triple->pv = false;
    triple->ov = false;

    for (size_t i = 0; i < tokens->size; i++)
    {
        if (tokens->arrOfTokens[i].type == TOKEN_LBRACE)
        {
            size_t subject_index = i + 1;
            size_t predicate_index = i + 2;
            size_t object_index = i + 3;

            if (object_index >= tokens->size)
            {
                break;
            }

            triple->subject = tokens->arrOfTokens[subject_index].text;
            triple->predicate = tokens->arrOfTokens[predicate_index].text;
            triple->object = tokens->arrOfTokens[object_index].text;
            triple->sv = tokens->arrOfTokens[subject_index].type == TOKEN_VARIABLE;
            triple->pv = tokens->arrOfTokens[predicate_index].type == TOKEN_VARIABLE;
            triple->ov = tokens->arrOfTokens[object_index].type == TOKEN_VARIABLE;
            break;
        }
    }
    return triple;
}
void freeVariables(Variables *variables)
{
    if (variables == NULL)
    {
        return;
    }
    free(variables->vs);
    variables->vs = NULL;
    free(variables);
    variables = NULL;
};
