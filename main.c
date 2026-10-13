#include "token.h"
#include "parse.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct orderBy
{
    char *variable;
    char *order;
} orderBy;
typedef struct Limit
{
    unsigned int limit;
} Limit;
typedef struct Offset
{
    unsigned int offset;
} Offset;
typedef struct ProjectionExpression
{
    char *function;
    char *parameter;
    char *variable; // where its stored
} ProjectionExpression;
typedef struct RDFterm
{
    char *value;
    bool isVariable;
} RDFterm;
typedef struct TriplePattern
{
    RDFterm *terms;
    bool isOptional;

} TriplePattern;
typedef struct Select
{
    Variables variables;
    ProjectionExpression *projectionExpressions;
    TriplePattern *triplePatterns;
    bool isDistinct;

} Select;

orderBy getOrderBy(Tokens *tokens, unsigned int index);
Offset getOffset(Tokens *tokens, unsigned int index);
Limit getLimit(Tokens *tokens, unsigned int index);
Select getSelectTokens(Tokens *tokens, unsigned int index);
ProjectionExpression *getProjectionExpression(Tokens *tokens, unsigned int index);
void parseTokens(Tokens *tokens);

int main(void)
{
    char query[] =
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
void parseTokens(Tokens *tokens)
{
    orderBy orderby = {0};
    Limit limit = {0};
    Select select = {0};
    for (size_t i = 0; i < tokens->size; i++)
    {
        char *type = tokens->arrOfTokens[i].type;
        if (type == TOKEN_SELECT)
        {
            select = getSelectTokens(tokens, i);
        }
        else if (type == TOKEN_ORDER_BY)
        {
            orderby = getOrderBy(tokens, i);
        }
        else if (type == TOKEN_LIMIT)
        {
            limit = getLimit(tokens, i);
        }
    }
}
Limit getLimit(Tokens *tokens, unsigned int index)
{
}
orderBy getOrderBy(Tokens *tokens, unsigned int index)
{
    orderBy orderby = {0};

    for (size_t current = index + 1; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;

        if (type == TOKEN_ASC || type == TOKEN_DESC)
        {
            orderby.order = tokens->arrOfTokens[current].text;
        }
        else if (type == TOKEN_VARIABLE)
        {
            orderby.variable = tokens->arrOfTokens[current].text;
        }
        else if (type == TOKEN_RPAREN)
        {
            break;
        }
    }

    return orderby;
}
Select getSelectTokens(Tokens *tokens, unsigned int index)
{
    Select select = {0};
    select.variables bool insideBraces = false;

    for (size_t current = index + 1; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;
        if (insideBraces == false && type == TOKEN_LPAREN)
        {
            ProjectionExpression *expression = getProjectionExpression(tokens, current);
            select.projectionExpressions = expression;
        }
        if (condition)
        {
            /* code */
        }

        if (type == TOKEN_LBRACE)
        {
            insideBraces = true;
        }
    }
}
ProjectionExpression *getProjectionExpression(Tokens *tokens, unsigned int index)
{
    ProjectionExpression *expression = malloc(sizeof(ProjectionExpression *));

    for (size_t current = index + 1; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;
        if (type == TOKEN_COUNT)
        {
            expression->function = tokens->arrOfTokens[current].text;
            if (tokens->arrOfTokens[current + 1].type == TOKEN_LPAREN)
            {
                expression->parameter = tokens->arrOfTokens[current + 2].text;
            }
        }
        else if (type = TOKEN_AS)
        {
            expression->variable = tokens->arrOfTokens[current + 1].text;
            return expression;
        }

        else if (type == TOKEN_LBRACE)
        {
            return NULL;
        }
    }
}