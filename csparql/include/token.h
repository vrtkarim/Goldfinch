#ifndef TOKEN_H
#define TOKEN_H
#include <stdbool.h>
#include <stddef.h>
typedef enum
{
    TOKEN_SELECT,
    TOKEN_WHERE,
    TOKEN_VARIABLE,
    TOKEN_IRI,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_DOT,
    TOKEN_EOF,
    TOKEN_ERROR
} TokenType;
typedef struct
{
    TokenType type;
    char *text;
} Token;

typedef struct Tokens
{
    Token *arrOfTokens;
    size_t size;
    size_t capacity;
} Tokens;

Tokens *getTokens(char *query);
void getToken(unsigned int current, unsigned int index, char *query, Tokens *tokens);
bool startsWith(char *token, char start);
bool endsWith(char *token, char end);
Tokens *createTokens(size_t capacity);
void addToTokens(Token token, Tokens *tokens);
bool tokensArrayGrow(Tokens *tokens);
void remove_angle_brackets(char *text);
void freeTokens(Tokens* tokens);
#endif