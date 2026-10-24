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
    TOKEN_ERROR,
    TOKEN_GROUP_BY,
    TOKEN_HAVING,
    TOKEN_ORDER_BY,
    TOKEN_LIMIT,
    TOKEN_OFFSET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_EQUAL,
    TOKEN_NOT_EQUAL,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_NUMBER,
    TOKEN_ASC,
    TOKEN_DESC,
    TOKEN_OPTIONAL,
    TOKEN_END_OPTIONAL,
    TOKEN_DISTINCT,
    TOKEN_COUNT,
    TOKEN_AS
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
char *normaliseSpacing(char *query);
Tokens *getTokens(char *query);
unsigned int parsePrefixes(size_t index, char *query, Prefixes *prefixes);
unsigned int parseSelect(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *Prefixes);
unsigned int parseGroupBy(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseHaving(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOffset(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseLimit(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOrderBy(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOptional(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
bool startsWith(char *token, char start);
bool endsWith(char *token, char end);
bool isMathOperator(const char *token);
bool isNumberToken(const char *token);
TokenType getOperatorType(const char *token);
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
void removeParentheses(char *text);

#endif