#include "triplestore.h"
#include "engine.h"
#include "fetcher.h"
#include "token.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void getTokenstemp(char *query);
size_t parsePrefixes(size_t index, char *query, Prefixes *prefixes);
unsigned int parseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *Prefixes);
int main(void)
{
    char query[] =
        "PREFIX ex:<http://example.com/> "
        "PREFIX ex2: <http://ezzxample2.com/>"
        " SELECT ?person WHERE { ?person ex2:knows ex:alice . }";

    getTokenstemp(query);
    return 0;
}
void getTokenstemp(char *query)
{
    Prefixes *prefixes = createPrefixes(5);
    Tokens *tokens = createTokens(10);
    unsigned int current = 0;
    for (size_t index = 0; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ' || query[index] == '\0')
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';
            printf("token: %s\n", token);

            if (strcasecmp(token, "prefix") == 0)
            {
                index = parsePrefixes(index, query, prefixes);
            }
            if (strcasecmp(token, "select") == 0)
            {
                index = parseSelect(current, index, query, tokens, prefixes);
            }

            current = index;
        }
    }
    printTokens(tokens);
}
size_t parsePrefixes(size_t index, char *query, Prefixes *prefixes)
{
    size_t current = index;
    Prefix prefix;
    char *uri = NULL;
    char *pref = NULL;
    int i = 1;
    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ':' && i < 2)
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';

            if (i == 1)
            {
                pref = trimWhitespace(token);
                current = index;
                i++;
            }
        }
        else if (query[index] == ' ' && i == 2)
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';
            if (endsWith(token, '>'))
            {
                removeColon(token);
                uri = trimWhitespace(token);

                remove_angle_brackets(uri);
                current = index;
                i++;
                addToPrefixes((Prefix){
                                  .prefix = pref,
                                  .uri = uri},
                              prefixes);
            }
        }
        else if (i > 2)
        {
            current = index;
            break;
        }
    }
    return index;
}
unsigned int parseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes)
{

    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ' || query[index] == '\0')
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';
            printf("token: %s\n", token);
            token = trimWhitespace(token);
            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};

            if (strcasecmp("SELECT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_SELECT,
                    .text = token};
            }
            if (strcasecmp("WHERE", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_WHERE,
                    .text = token};
            }
            if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            if (startsWith(token, '<'))
            {
                remove_angle_brackets(token);

                tokenToAdd = (Token){
                    .type = TOKEN_IRI,
                    .text = token};
            }
            if (startsWith(token, '.'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DOT,
                    .text = token};
            }
            if (startsWith(token, '{'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LBRACE,
                    .text = token};
            }

            if (isItQname(token))
            {
                char *iri = qnameToUri(token, prefixes);

                tokenToAdd = (Token){
                    .type = TOKEN_IRI,
                    .text = iri};
            }
            if (endsWith(token, '}'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_RBRACE,
                    .text = token};
            }
            addToTokens(tokenToAdd, tokens);
            current = index;
        }
    }
    return index;
}