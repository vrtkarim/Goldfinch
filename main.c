#include "token.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    char query[] =
        "PREFIX foaf: <http://xmlns.com/foaf/0.1/> "
        "SELECT DISTINCT ?person"
        " WHERE { "
        " ?person foaf:name ?name . "
        " ?personaa foaf:age ?name . "
        "OPTIONAL { "
        "?person foaf:knows ?friend . "
        " ?person foaf:knows ?friend . "
        " ?person foaf:knows ?friend . "
        "} "

        "} "
        "GROUP BY ?person "
        "HAVING (COUNT(?friend) > 0) "
        "ORDER BY DESC(?friendCount) "
        "LIMIT 10 "
        "OFFSET 0";

    char *normalizedQuery = normaliseSpacing(query);
    if (normalizedQuery == NULL)
    {
        return 1;
    }

    Tokens *parsedTokens = getTokens(normalizedQuery);
    if (parsedTokens == NULL)
    {
        free(normalizedQuery);
        return 1;
    }

    printTokens(parsedTokens);
    parseTokens(parsedTokens);

    freeTokens(parsedTokens);
    free(normalizedQuery);
    return 0;
}
