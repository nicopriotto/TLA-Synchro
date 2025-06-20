#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H
#include "../Type.h"
#include <stddef.h>

typedef enum {
    SYMBOL_INTEGER,
    SYMBOL_STRING,
    SYMBOL_FLOAT,
    SYMBOL_BOOLEAN,
    SYMBOL_SEMAPHORE,
    SYMBOL_FUNCTION
} SymbolType;

typedef struct SymbolEntry {
    char *identifier;
    SymbolType type;
    int scope;
    union {
        int integerValue;
        char *stringValue;
        float floatValue;
        boolean booleanValue;
        int semaphoreValue;
        SymbolType functionType;
    } data;
    struct SymbolEntry *next;
} SymbolEntry;

typedef struct {
    SymbolEntry *head;
} SymbolTable;

void initSymbolTable(SymbolTable *table);
boolean insertSymbol(SymbolTable *table,const char *identifier,SymbolType type,int scope,const void *data);
SymbolEntry *findSymbol(const SymbolTable *table,const char *identifier,int scope);
boolean updateSymbolValue(SymbolTable *table,const char *identifier,int scope,const void *newData);
boolean removeSymbol(SymbolTable *table,const char *identifier,int scope);
void removeScopeSymbols(SymbolTable *table,int scope);
void freeSymbolTable(SymbolTable *table);
void dumpSymbolTable(const SymbolTable *table);

#endif
