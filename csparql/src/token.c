#include "token.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#define TOKENS_CAPACITY 10

Tokens *getTokens(char *query)
{
    Tokens *tokens = createTokens(TOKENS_CAPACITY);
    unsigned int current = 0;
    for (size_t index = 0; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ' || query[index] == '\0')
        {
            getToken(current, index, query, tokens);
            current = index + 1;
        }
    }

    return tokens;
}
void getToken(unsigned int current, unsigned int index, char *query, Tokens *tokens)
{
    size_t length = index - current;
    char *token = malloc((length + 1) * sizeof(*token));
    if (token == NULL)
    {
        return;
    }
    memcpy(token, query + current, length);
    token[length] = '\0';
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

    if (endsWith(token, '}'))
    {
        tokenToAdd = (Token){
            .type = TOKEN_RBRACE,
            .text = token};
    }
    addToTokens(tokenToAdd, tokens);
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
void freeTokens(Tokens* tokens){
    if(tokens==NULL){
        return;
    }
    free(tokens->arrOfTokens);
    free(tokens);
    tokens==NULL;

};