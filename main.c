#include "triplestore.h"
#include "engine.h"
#include "fetcher.h"
#include "token.h"
#include "executor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Tokens *getTokenstemp(char *query);
unsigned int parsePrefixestemp(size_t index, char *query, Prefixes *prefixes);
unsigned int parseGroupBytemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseHavingtemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOffsettemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseLimittemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOrderBytemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix);
unsigned int parseOptionaltemp(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes);
unsigned int parseSelecttemp(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *Prefixes);
int main(void)
{
    char query[] =
        "PREFIX ex:<http://example.com/> "
        "PREFIX ex2: <http://ezzxample2.com/>"
        " SELECT ?person ?age"
        " WHERE { ?person ex2:age ?age . }"
        " GROUP BY ?person ?age"
        " HAVING (?age > 30)"
        " ORDER BY DESC(?age)"
        " LIMIT 10"
        " OFFSET 0";
    char advancedQuery[] =
        "PREFIX foaf: <http://xmlns.com/foaf/0.1/> "
        "SELECT DISTINCT ?person (COUNT(?friend) AS ?friendCount) "
        "WHERE { "
        "?person foaf:name ?name . "
        "OPTIONAL { ?person foaf:knows ?friend . } "
        "} "
        "GROUP BY ?person "
        "HAVING (COUNT(?friend) > 0) "
        "ORDER BY DESC(?friendCount) "
        "LIMIT 10 "
        "OFFSET 0";
    char *normalised = normaliseSpacing(query);
    getTokenstemp(normalised);
    char *advancedNormalised = normaliseSpacing(advancedQuery);
    getTokenstemp(advancedNormalised);

    return 0;
}
Tokens *getTokenstemp(char *query)
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
            if (strcasecmp(token, "prefix") == 0)
            {
                index = parsePrefixestemp(index, query, prefixes);
            }
            else if (strcasecmp(token, "select") == 0)
            {
                index = parseSelecttemp(current, index, query, tokens, prefixes);
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
    printTokens(tokens);
    return tokens;
}
unsigned int parsePrefixestemp(size_t index, char *query, Prefixes *prefixes)
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
unsigned int parseSelecttemp(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes)
{

    for (index; index < strlen(query) + 1; index++)
    {
        if (query[index] == ' ' || query[index] == '}')
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
            else if (strcasecmp(token, "distinct") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_DISTINCT,
                    .text = token};
            }
            else if (strcasecmp(token, "count") == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_COUNT,
                    .text = token};
            }
            else if (strcasecmp("WHERE", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_WHERE,
                    .text = token};
            }
            else if (strcasecmp("optional", token) == 0)
            {
                index = parseOptionaltemp(current, index, query, tokens, prefixes);
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
            else if (startsWith(token, '{'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LBRACE,
                    .text = token};
            }
            else if (isItQname(token))
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
                addToTokens(tokenToAdd, tokens);

                return index;
            }
            if (strcasecmp("token", token) != 0)
            {
                addToTokens(tokenToAdd, tokens);
            }
            current = index;
        }
    }
    return index;
}

unsigned int parseGroupBytemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
unsigned int parseHavingtemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
{
    /* HAVING (?age > 30)*/
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
            if (strcasecmp(token, "having"))
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
unsigned int parseOffsettemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
};
unsigned int parseLimittemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
};
unsigned int parseOrderBytemp(size_t current, size_t index, char *query, Tokens *tokens, Prefixes *prefix)
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
};
unsigned int parseOptionaltemp(unsigned int current, unsigned int index, char *query, Tokens *tokens, Prefixes *prefixes)
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
            if (strcasecmp("optional", token) == 0)
            {
                tokenToAdd = (Token){
                    .type = TOKEN_OPTIONAL,
                    .text = token};
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
            else if (startsWith(token, '{'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_LBRACE,
                    .text = token};
            }

            else if (isItQname(token))
            {
                char *iri = qnameToUri(token, prefixes);

                tokenToAdd = (Token){
                    .type = TOKEN_IRI,
                    .text = iri};
            }
            else if (endsWith(token, '}'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_RBRACE,
                    .text = token};
                addToTokens(tokenToAdd, tokens);

                return index;
            }
            else if (startsWith(token, '?'))
            {
                tokenToAdd = (Token){
                    .type = TOKEN_VARIABLE,
                    .text = token};
            }
            if (endsWith(token, '}'))
            {
                return index;
            }
            addToTokens(tokenToAdd, tokens);
            current = index;
        }
    }
};