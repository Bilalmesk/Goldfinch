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
typedef struct Variable
{
    char *value;
    bool isVariable;
} Variable;
typedef struct Variables
{
    Variable *arrOfVariables;
    size_t size;
    size_t capacity;
} Variables;
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
typedef struct TriplePatterns
{
    TriplePattern *arrOfTriplePatterns;
    size_t size;
    size_t capacity;
} TriplePatterns;
typedef struct Select
{
    Variables *variables;
    ProjectionExpression *projectionExpressions;
    TriplePatterns *triplePatterns;
    bool isDistinct;

} Select;

orderBy getOrderBy(Tokens *tokens, unsigned int index);
Offset getOffset(Tokens *tokens, unsigned int index);
Limit getLimit(Tokens *tokens, unsigned int index);
Select getSelectTokens(Tokens *tokens, unsigned int index);
ProjectionExpression *getProjectionExpression(Tokens *tokens, unsigned int index);
void parseTokens(Tokens *tokens);
Variables *createVariables(size_t capacity);
void addToVariables(Variable variable, Variables *variables);
bool variablesArrayGrow(Variables *variables);
void freeVariables(Variables *variables);
TriplePatterns *createTriplePatterns(size_t capacity);
void addToTriplePatterns(TriplePattern triplePattern, TriplePatterns *triplePatterns);
bool triplePatternsArrayGrow(TriplePatterns *triplePatterns);
void freeTriplePatterns(TriplePatterns *triplePatterns);

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
    select.variables = createVariables(2);
    bool insideBraces = false;

    for (size_t current = index + 1; current < tokens->size; current++)
    {
        TokenType type = tokens->arrOfTokens[current].type;
        if (insideBraces == false && type == TOKEN_LPAREN)
        {
            ProjectionExpression *expression = getProjectionExpression(tokens, current);
            select.projectionExpressions = expression;
        }
        if (type == TOKEN_VARIABLE)
        {
            Variable var = {
                0};
            var.value = tokens->arrOfTokens[current].text;
            var.isVariable = startsWith(var.value, '?');
            addToVariables(var, select.variables);
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
Variables *createVariables(size_t capacity)
{
    Variables *variables = malloc(sizeof(*variables));

    if (variables == NULL)
    {
        return NULL;
    }

    if (capacity == 0)
    {
        capacity = 1;
    }

    variables->capacity = capacity;
    variables->size = 0;
    variables->arrOfVariables = malloc(capacity * sizeof(*variables->arrOfVariables));

    if (variables->arrOfVariables == NULL)
    {
        free(variables);
        return NULL;
    }

    return variables;
}

void addToVariables(Variable variable, Variables *variables)
{
    if (variables == NULL)
    {
        return;
    }

    if (variables->size >= variables->capacity && !variablesArrayGrow(variables))
    {
        return;
    }

    variables->arrOfVariables[variables->size] = variable;
    variables->size++;
}

bool variablesArrayGrow(Variables *variables)
{
    if (variables == NULL)
    {
        return false;
    }

    size_t newCapacity = variables->capacity == 0 ? 1 : variables->capacity * 2;
    Variable *newVariables = realloc(
        variables->arrOfVariables,
        newCapacity * sizeof(*variables->arrOfVariables));

    if (newVariables == NULL)
    {
        return false;
    }

    variables->arrOfVariables = newVariables;
    variables->capacity = newCapacity;
    return true;
}

void freeVariables(Variables *variables)
{
    if (variables == NULL)
    {
        return;
    }

    free(variables->arrOfVariables);
    free(variables);
}

TriplePatterns *createTriplePatterns(size_t capacity)
{
    TriplePatterns *triplePatterns = malloc(sizeof(*triplePatterns));

    if (triplePatterns == NULL)
    {
        return NULL;
    }

    if (capacity == 0)
    {
        capacity = 1;
    }

    triplePatterns->capacity = capacity;
    triplePatterns->size = 0;
    triplePatterns->arrOfTriplePatterns = malloc(
        capacity * sizeof(*triplePatterns->arrOfTriplePatterns));

    if (triplePatterns->arrOfTriplePatterns == NULL)
    {
        free(triplePatterns);
        return NULL;
    }

    return triplePatterns;
}

void addToTriplePatterns(
    TriplePattern triplePattern,
    TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return;
    }

    if (triplePatterns->size >= triplePatterns->capacity &&
        !triplePatternsArrayGrow(triplePatterns))
    {
        return;
    }

    triplePatterns->arrOfTriplePatterns[triplePatterns->size] = triplePattern;
    triplePatterns->size++;
}

bool triplePatternsArrayGrow(TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return false;
    }

    size_t newCapacity = triplePatterns->capacity == 0
                             ? 1
                             : triplePatterns->capacity * 2;
    TriplePattern *newTriplePatterns = realloc(
        triplePatterns->arrOfTriplePatterns,
        newCapacity * sizeof(*triplePatterns->arrOfTriplePatterns));

    if (newTriplePatterns == NULL)
    {
        return false;
    }

    triplePatterns->arrOfTriplePatterns = newTriplePatterns;
    triplePatterns->capacity = newCapacity;
    return true;
}

void freeTriplePatterns(TriplePatterns *triplePatterns)
{
    if (triplePatterns == NULL)
    {
        return;
    }

    free(triplePatterns->arrOfTriplePatterns);
    free(triplePatterns);
}