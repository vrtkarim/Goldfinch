#include "token.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#define TOKENS_CAPACITY 10

Tokens *getTokens(char *query)
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
    return tokens;
}
unsigned int parsePrefixes(size_t index, char *query, Prefixes *prefixes)
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
bool startsWith(char *token, char start)
{
    if (token == NULL)
    {
        return false;
    }
    if (start == token[0])
    {
        return true;
    }
    return false;
}
bool endsWith(char *token, char end)
{
    if (token == NULL)
    {
        return false;
    }
    size_t length = strlen(token);
    if (end == token[length - 1])
    {
        return true;
    }
    return false;
}
Tokens *createTokens(size_t capacity)
{
    Tokens *tokens = malloc(sizeof(*tokens));

    if (tokens == NULL)
    {
        return NULL;
    }

    tokens->capacity = capacity;
    tokens->size = 0;
    tokens->arrOfTokens = malloc(capacity * sizeof(*tokens->arrOfTokens));

    if (tokens->arrOfTokens == NULL)
    {
        free(tokens);
        return NULL;
    }

    return tokens;
}
void addToTokens(Token token, Tokens *tokens)
{
    (tokens->arrOfTokens)[tokens->size] = token;
    tokens->size += 1;
    if (tokens->size >= tokens->capacity)
    {
        if (tokensArrayGrow(tokens))
        {
            /* code */
        }
        else
        {
            return;
        }
    }
}
bool tokensArrayGrow(Tokens *tokens)
{
    size_t newcapacity = 2 * (tokens->capacity);
    Token *new = realloc(
        tokens->arrOfTokens,
        newcapacity * sizeof(*tokens->arrOfTokens));
    if (new == NULL)
    {
        return false;
    }
    else
    {
        tokens->arrOfTokens = new;
        tokens->capacity = newcapacity;
        return true;
    }
}
Prefixes *createPrefixes(size_t capacity)
{
    Prefixes *prefixes = malloc(sizeof(*prefixes));

    if (prefixes == NULL)
    {
        return NULL;
    }

    prefixes->capacity = capacity;
    prefixes->size = 0;
    prefixes->arrOfPrefixes = malloc(capacity * sizeof(*prefixes->arrOfPrefixes));

    if (prefixes->arrOfPrefixes == NULL)
    {
        free(prefixes);
        return NULL;
    }

    return prefixes;
}
void addToPrefixes(Prefix prefix, Prefixes *prefixes)
{
    if (prefixes == NULL || prefixes->size >= prefixes->capacity)
    {
        return;
    }

    prefixes->arrOfPrefixes[prefixes->size] = prefix;
    prefixes->size += 1;

    if (prefixes->size >= prefixes->capacity)
    {
        prefixesArrayGrow(prefixes);
    }
}
bool prefixesArrayGrow(Prefixes *prefixes)
{
    size_t newcapacity = 2 * prefixes->capacity;
    Prefix *new = realloc(
        prefixes->arrOfPrefixes,
        newcapacity * sizeof(*prefixes->arrOfPrefixes));

    if (new == NULL)
    {
        return false;
    }

    prefixes->arrOfPrefixes = new;
    prefixes->capacity = newcapacity;
    return true;
}
const char *getPrefixUri(const Prefixes *prefixes, const char *prefix)
{
    if (prefixes == NULL || prefix == NULL)
    {
        return NULL;
    }

    for (size_t index = 0; index < prefixes->size; index++)
    {
        if (strcmp(prefixes->arrOfPrefixes[index].prefix, prefix) == 0)
        {
            return prefixes->arrOfPrefixes[index].uri;
        }
    }

    return NULL;
}
void freePrefixes(Prefixes *prefixes)
{
    if (prefixes == NULL)
    {
        return;
    }

    free(prefixes->arrOfPrefixes);
    free(prefixes);
}
char *trimWhitespace(char *text)
{
    if (text == NULL)
    {
        return NULL;
    }

    char *start = text;
    while (isspace((unsigned char)*start))
    {
        start++;
    }

    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)end[-1]))
    {
        end--;
    }

    *end = '\0';

    if (start != text)
    {
        memmove(text, start, (size_t)(end - start) + 1);
    }

    return text;
}
void removeColon(char *text)
{
    if (text == NULL)
    {
        return;
    }

    char *colon = strchr(text, ':');
    if (colon != NULL)
    {
        memmove(colon, colon + 1, strlen(colon));
    }
}
void remove_angle_brackets(char *text)
{
    size_t length;

    if (text == NULL)
    {
        return;
    }

    length = strlen(text);

    if (length >= 2 &&
        text[0] == '<' &&
        text[length - 1] == '>')
    {
        memmove(text, text + 1, length - 2);
        text[length - 2] = '\0';
    }
}
const char *tokenTypeName(TokenType type)
{
    switch (type)
    {
    case TOKEN_SELECT:
        return "TOKEN_SELECT";
    case TOKEN_WHERE:
        return "TOKEN_WHERE";
    case TOKEN_VARIABLE:
        return "TOKEN_VARIABLE";
    case TOKEN_IRI:
        return "TOKEN_IRI";
    case TOKEN_LBRACE:
        return "TOKEN_LBRACE";
    case TOKEN_RBRACE:
        return "TOKEN_RBRACE";
    case TOKEN_DOT:
        return "TOKEN_DOT";
    case TOKEN_EOF:
        return "TOKEN_EOF";
    case TOKEN_ERROR:
        return "TOKEN_ERROR";
    default:
        return "TOKEN_UNKNOWN";
    }
}
void printTokens(const Tokens *tokens)
{
    if (tokens == NULL)
    {
        return;
    }

    for (size_t index = 0; index < tokens->size; index++)
    {
        Token *token = &tokens->arrOfTokens[index];
        printf("token[%zu]: type=%s, text=%s\n",
               index,
               tokenTypeName(token->type),
               token->text);
    }
}
char *qnameToUri(char *qname, Prefixes *prefixes)
{
    char *prefix;
    char *localname;
    size_t current = 0;
    int index = 1;
    for (size_t i = 0; i < strlen(qname) + 1; i++)
    {
        if (qname[i] == ':' || qname[i] == '\0')
        {
            size_t length = i - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, qname + current, length);
            token[length] = '\0';
            if (index == 1)
            {
                prefix = token;
                index++;
            }
            else if (index == 2)
            {
                removeColon(token);
                localname = token;
            }
            current = i;
        }
    }
    const char *baseUri = getPrefixUri(prefixes, prefix);

    if (baseUri == NULL || localname == NULL)
    {
        return NULL;
    }

    char *uri = malloc(strlen(baseUri) + strlen(localname) + 1);

    if (uri == NULL)
    {
        return NULL;
    }

    strcpy(uri, baseUri);
    strcat(uri, localname);
    printf("gotten iri: %s", uri);
    return uri;
}
bool isItQname(char *qname)
{
    for (size_t i = 0; i < strlen(qname) + 1; i++)
    {
        if (qname[i] == ':')
        {
            return true;
        }
    }
    return false;
}
void freeTokens(Tokens *tokens)
{
    if (tokens == NULL)
    {
        return;
    }
    free(tokens->arrOfTokens);
    free(tokens);
    tokens == NULL;
};