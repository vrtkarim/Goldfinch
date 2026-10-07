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
typedef struct Prefix
{
    char *prefix;
    char *uri;
} Prefix;
typedef struct Prefixes
{
    Prefix *arrOfPrefixes;
    size_t size;
    size_t capacity;
} Prefixes;
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

Tokens* getTokens(char *query);
unsigned int parsePrefixes(size_t index, char *query, Prefixes *prefixes);
unsigned int parseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *Prefixes);
bool startsWith(char *token, char start);
bool endsWith(char *token, char end);
Tokens *createTokens(size_t capacity);
void addToTokens(Token token, Tokens *tokens);
bool tokensArrayGrow(Tokens *tokens);
Prefixes *createPrefixes(size_t capacity);
void addToPrefixes(Prefix prefix, Prefixes *prefixes);
bool prefixesArrayGrow(Prefixes *prefixes);
const char *getPrefixUri(const Prefixes *prefixes, const char *prefix);
void freePrefixes(Prefixes *prefixes);
char *trimWhitespace(char *text);
void removeColon(char *text);
void remove_angle_brackets(char *text);
const char *tokenTypeName(TokenType type);
void printTokens(const Tokens *tokens);
void freeTokens(Tokens *tokens);
char *qnameToUri(char *qname, Prefixes *prefixes);
bool isItQname(char *qname);

#endif