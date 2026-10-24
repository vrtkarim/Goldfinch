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

            trimWhitespace(token);
            if (token == NULL || token[0] == '\0')
            {
                free(token);
                current = index;
                continue;
            }

            if (strcasecmp(token, "prefix") == 0)
            {
                index = parsePrefixes(index, query, prefixes);
            }
            else if (strcasecmp(token, "select") == 0)
            {
                index = parseSelect(current, index, query, tokens, prefixes);
            }
            else if (strcasecmp(token, "group") == 0)
            {
                index = parseGroupBy(current, index, query, tokens, prefixes);
            }
            else if (strcasecmp(token, "having") == 0)
            {
                index = parseHaving(current, index, query, tokens, prefixes);
            }
            else if (strcasecmp(token, "limit") == 0)
            {
                index = parseLimit(current, index, query, tokens, prefixes);
            }
            else if (strcasecmp(token, "offset") == 0)
            {
                index = parseOffset(current, index, query, tokens, prefixes);
            }
            else if (strcasecmp(token, "order") == 0)
            {
                index = parseOrderBy(current, index, query, tokens, prefixes);
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
    int braceDepth = 0;
    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ')
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';

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
            else if (strcasecmp("DISTINCT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DISTINCT,
                    .text = token};
            }
            else if (strcasecmp("COUNT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_COUNT,
                    .text = token};
            }
            else if (strcasecmp("AS", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_AS,
                    .text = token};
            }
            else if (strcasecmp("WHERE", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_WHERE,
                    .text = token};
            }
            else if (strcasecmp("OPTIONAL", token) == 0)
            {
                index = parseOptional(current, index, query, tokens, prefixes);
                current = index;
                continue;
            }
            else if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            else if (startsWith(token, '<'))
            {
                remove_angle_brackets(token);

                tokenToAdd = (Token){
                    .type = TOKEN_IRI,
                    .text = token};
            }
            else if (startsWith(token, '.'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DOT,
                    .text = token};
            }
            else if (strcmp(token, "{") == 0)
            {
                braceDepth++;
                tokenToAdd = (Token){
                    .type = TOKEN_LBRACE,
                    .text = token};
            }
            else if (strcmp(token, "}") == 0)
            {
                braceDepth--;
                tokenToAdd = (Token){
                    .type = TOKEN_RBRACE,
                    .text = token};
                addToTokens(tokenToAdd, tokens);

                if (braceDepth <= 0)
                {
                    return index;
                }
                current = index;
                continue;
            }
            else if (isItQname(token))
            {
                char *iri = qnameToUri(token, prefixes);

                tokenToAdd = (Token){
                    .type = TOKEN_IRI,
                    .text = iri};
            }
            if (tokenToAdd.text != NULL && tokenToAdd.text[0] != '\0')
            {
                addToTokens(tokenToAdd, tokens);
            }
            current = index;
        }
    }
    return index;
}
unsigned int parseGroupBy(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
{
    int byExistence = 1;
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
            if (token == NULL || token[0] == '\0')
            {
                free(token);
                current = index;
                continue;
            }

            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};

            if (strcasecmp(token, "by") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_GROUP_BY,
                    .text = "GROUP BY"};

                byExistence++;
            }
            else if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            else if (!startsWith(token, '?') && byExistence > 1)
            {
                return current;
            }
            if (strcasecmp(token, "group") != 0)
            {
                addToTokens(tokenToAdd, tokens);
            }
            current = index;
        }
    };
}
unsigned int parseHaving(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
            if (token == NULL || token[0] == '\0')
            {
                free(token);
                current = index;
                continue;
            }

            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};
            if (strcasecmp(token, "having") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_HAVING,
                    .text = token};
            }
            if (strcasecmp(token, "(") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LPAREN,
                    .text = token};
            }
            else if (strcasecmp(token, ")") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_RPAREN,
                    .text = token};
            }
            else if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            else if (isMathOperator(token))
            {
                tokenToAdd = (Token){
                    .type = getOperatorType(token),
                    .text = token};
            }
            addToTokens(tokenToAdd, tokens);

            if (endsWith(token, ')'))
            {
                return index;
            }
            current = index;
        }
    }
}
unsigned int parseOffset(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
            if (token == NULL || token[0] == '\0')
            {
                free(token);
                current = index;
                continue;
            }
            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};
            if (strcasecmp(token, "offset") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_OFFSET,
                    .text = token};
            }
            else if (isNumberToken(token))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_NUMBER,
                    .text = token};

                addToTokens(tokenToAdd, tokens);
                return index;
            }

            addToTokens(tokenToAdd, tokens);
            current = index;
        }
    }
    return index;
}
unsigned int parseLimit(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
{
    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ')
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';
            printf("token: %s\n", token);
            token = trimWhitespace(token);
            if (token == NULL || token[0] == '\0')
            {
                free(token);
                current = index;
                continue;
            }
            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};
            if (strcasecmp(token, "limit") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LIMIT,
                    .text = token};
            }
            else if (isNumberToken(token))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_NUMBER,
                    .text = token};

                addToTokens(tokenToAdd, tokens);
                return index;
            }

            addToTokens(tokenToAdd, tokens);
            current = index;
        }
    }
    return index;
}
unsigned int parseOrderBy(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
            if (strcasecmp(token, "order") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_ORDER_BY,
                    .text = "ORDER BY"};
            }
            else if (strcasecmp(token, "desc") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DESC,
                    .text = token};
            }
            else if (strcasecmp(token, "asc") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_ASC,
                    .text = token};
            }
            else if (strcasecmp(token, "(") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LPAREN,
                    .text = token};
            }
            else if (strcasecmp(token, ")") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_RPAREN,
                    .text = token};
            }
            else if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            else if (isMathOperator(token))
            {
                tokenToAdd = (Token){
                    .type = getOperatorType(token),
                    .text = token};
            }
            if (strcasecmp(token, "by") != 0)
            {
                addToTokens(tokenToAdd, tokens);
            }

            if (endsWith(token, ')'))
            {
                return index;
            }
            current = index;
        }
    }
}

static unsigned int ParsePrefixes(size_t index, char *query, Prefixes *prefixes);
static unsigned int ParseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes);

static Tokens *GetTokens(char *query)
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
                index = ParsePrefixes(index, query, prefixes);
            }
            if (strcasecmp(token, "select") == 0)
            {
                index = ParseSelect(current, index, query, tokens, prefixes);
            }

            current = index;
        }
    }
    return tokens;
}
static unsigned int ParsePrefixes(size_t index, char *query, Prefixes *prefixes)
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
static unsigned int ParseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes)
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
            if (strcasecmp("DISTINCT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DISTINCT,
                    .text = token};
            }
            if (strcasecmp("COUNT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_COUNT,
                    .text = token};
            }
            if (strcasecmp("AS", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_AS,
                    .text = token};
            }
            if (strcasecmp("WHERE", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_WHERE,
                    .text = token};
            }
            if (strcasecmp("LIMIT", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LIMIT,
                    .text = token};
            }
            if (strcasecmp("HAVING", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_HAVING,
                    .text = token};
            }
            if (strcasecmp("ORDER", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_ORDER_BY,
                    .text = token};
            }
            if (strcasecmp("OFFSET", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_OFFSET,
                    .text = token};
            }
            if (strcasecmp("ASC", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_ASC,
                    .text = token};
            }
            if (strcasecmp("DESC", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DESC,
                    .text = token};
            }
            if (isMathOperator(token))
            {
                tokenToAdd = (Token){
                    .type = getOperatorType(token),
                    .text = token};
            }
            if (isNumberToken(token))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_NUMBER,
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
            if (strcasecmp(token, "}") == 0)
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
bool isMathOperator(const char *token)
{
    if (token == NULL)
    {
        return false;
    }

    return getOperatorType(token) != TOKEN_ERROR;
}
bool isNumberToken(const char *token)
{
    if (token == NULL || token[0] == '\0')
    {
        return false;
    }

    for (size_t i = 0; i < strlen(token); i++)
    {
        if (!isdigit((unsigned char)token[i]))
        {
            return false;
        }
    }

    return true;
}
TokenType getOperatorType(const char *token)
{
    if (token == NULL)
    {
        return TOKEN_ERROR;
    }

    if (strcmp(token, ">") == 0)
    {
        return TOKEN_GREATER;
    }
    if (strcmp(token, ">=") == 0)
    {
        return TOKEN_GREATER_EQUAL;
    }
    if (strcmp(token, "<") == 0)
    {
        return TOKEN_LESS;
    }
    if (strcmp(token, "<=") == 0)
    {
        return TOKEN_LESS_EQUAL;
    }
    if (strcmp(token, "=") == 0)
    {
        return TOKEN_EQUAL;
    }
    if (strcmp(token, "!=") == 0)
    {
        return TOKEN_NOT_EQUAL;
    }
    if (strcmp(token, "+") == 0)
    {
        return TOKEN_PLUS;
    }
    if (strcmp(token, "-") == 0)
    {
        return TOKEN_MINUS;
    }
    if (strcmp(token, "*") == 0)
    {
        return TOKEN_MULTIPLY;
    }
    if (strcmp(token, "/") == 0)
    {
        return TOKEN_DIVIDE;
    }
    if (strcmp(token, "%") == 0)
    {
        return TOKEN_MODULO;
    }

    return TOKEN_ERROR;
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
    case TOKEN_GROUP_BY:
        return "TOKEN_GROUP_BY";
    case TOKEN_HAVING:
        return "TOKEN_HAVING";
    case TOKEN_ORDER_BY:
        return "TOKEN_ORDER_BY";
    case TOKEN_LIMIT:
        return "TOKEN_LIMIT";
    case TOKEN_OFFSET:
        return "TOKEN_OFFSET";
    case TOKEN_LPAREN:
        return "TOKEN_LPAREN";
    case TOKEN_RPAREN:
        return "TOKEN_RPAREN";
    case TOKEN_GREATER:
        return "TOKEN_GREATER";
    case TOKEN_GREATER_EQUAL:
        return "TOKEN_GREATER_EQUAL";
    case TOKEN_LESS:
        return "TOKEN_LESS";
    case TOKEN_LESS_EQUAL:
        return "TOKEN_LESS_EQUAL";
    case TOKEN_EQUAL:
        return "TOKEN_EQUAL";
    case TOKEN_NOT_EQUAL:
        return "TOKEN_NOT_EQUAL";
    case TOKEN_PLUS:
        return "TOKEN_PLUS";
    case TOKEN_MINUS:
        return "TOKEN_MINUS";
    case TOKEN_MULTIPLY:
        return "TOKEN_MULTIPLY";
    case TOKEN_DIVIDE:
        return "TOKEN_DIVIDE";
    case TOKEN_MODULO:
        return "TOKEN_MODULO";
    case TOKEN_NUMBER:
        return "TOKEN_NUMBER";
    case TOKEN_ASC:
        return "TOKEN_ASC";
    case TOKEN_DESC:
        return "TOKEN_DESC";
    case TOKEN_OPTIONAL:
        return "TOKEN_OPTIONAL";
    case TOKEN_END_OPTIONAL:
        return "TOKEN_END_OPTIONAL";
    case TOKEN_DISTINCT:
        return "TOKEN_DISTINCT";
    case TOKEN_COUNT:
        return "TOKEN_COUNT";
    case TOKEN_AS:
        return "TOKEN_AS";

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
#include <string.h>

void removeParentheses(char *text)
{
    if (text == NULL)
    {
        return;
    }

    size_t len = strlen(text);

    while (len > 0 && (text[0] == '(' || text[0] == ')'))
    {
        memmove(text, text + 1, len);
        len--;
        text[len] = '\0';
    }

    while (len > 0 && (text[len - 1] == '(' || text[len - 1] == ')'))
    {
        text[len - 1] = '\0';
        len--;
    }
}

char *normaliseSpacing(char *query)
{
    if (query == NULL)
    {
        return NULL;
    }

    size_t len = strlen(query);
    char *out = malloc((len * 2 + 1) * sizeof(*out));
    if (out == NULL)
    {
        return NULL;
    }

    size_t j = 0;

    for (size_t i = 0; i < len; i++)
    {
        char c = query[i];

        if (c == '{' || c == '}' || c == '(' || c == ')')
        {
            if (j > 0 && out[j - 1] != ' ')
            {
                out[j++] = ' ';
            }

            out[j++] = c;

            if (i + 1 < len && query[i + 1] != ' ' &&
                query[i + 1] != '{' && query[i + 1] != '}' &&
                query[i + 1] != '(' && query[i + 1] != ')')
            {
                out[j++] = ' ';
            }
        }
        else
        {
            out[j++] = c;
        }
    }

    out[j] = '\0';
    return out;
}
unsigned int parseOptional(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
{
    int braceDepth = 0;

    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ' || query[index] == '\0')
        {
            size_t length = index - current;
            char *token = malloc((length + 1) * sizeof(*token));
            memcpy(token, query + current, length);
            token[length] = '\0';
            token = trimWhitespace(token);
            Token tokenToAdd = {
                .type = TOKEN_ERROR,
                .text = token};

            if (strcasecmp("OPTIONAL", token) == 0)
            {
                tokenToAdd.type = TOKEN_OPTIONAL;
            }

            else if (startsWith(token, '?'))
            {
                tokenToAdd.type = TOKEN_VARIABLE;
            }
            else if (startsWith(token, '<'))
            {
                remove_angle_brackets(token);
                tokenToAdd.type = TOKEN_IRI;
            }
            else if (startsWith(token, '.'))
            {
                tokenToAdd.type = TOKEN_DOT;
            }
            else if (strcmp(token, "{") == 0)
            {
                braceDepth++;
                tokenToAdd.type = TOKEN_LBRACE;
            }
            else if (strcmp(token, "}") == 0)
            {
                braceDepth--;
                tokenToAdd.type = TOKEN_RBRACE;
            }
            else if (isItQname(token))
            {
                tokenToAdd.type = TOKEN_IRI;
                tokenToAdd.text = qnameToUri(token, prefix);
            }

            if (tokenToAdd.text != NULL && tokenToAdd.text[0] != '\0')
            {
                addToTokens(tokenToAdd, tokens);
            }

            if (strcmp(token, "}") == 0 && braceDepth == 0)
            {
                Token endofoptional = {
                    .type = TOKEN_END_OPTIONAL,
                    .text = "Here optional ends."};
                addToTokens(endofoptional, tokens);
                return index + 1;
            }

            current = index;
        }
    }

    return index;
}