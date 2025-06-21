#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "../Logger.h"
#include "../Type.h"

// Forward declarations
typedef struct SymbolEntry SymbolEntry;
typedef struct SymbolTable SymbolTable;

// Symbol types
typedef enum {
    SYMBOL_INTEGER,
    SYMBOL_FLOAT,
    SYMBOL_STRING,
    SYMBOL_BOOLEAN,
    SYMBOL_FUNCTION,
    SYMBOL_SEMAPHORE
} SymbolType;

// Symbol data union
typedef union {
    int intValue;
    float floatValue;
    char* stringValue;
    boolean boolValue;
} SymbolData;

// Symbol table entry
struct SymbolEntry {
    char* identifier;
    SymbolType type;
    int scope;
    SymbolData data;
    SymbolEntry* next;
};

// Symbol table structure
struct SymbolTable {
    SymbolEntry* head;
    int size;
};

// Module functions
void initializeSymbolTableModule();
void shutdownSymbolTableModule();

// Symbol table functions
void initSymbolTable(SymbolTable* table);
boolean insertSymbol(SymbolTable* table, const char* identifier, SymbolType type, int scope, SymbolData* data);
SymbolEntry* findSymbol(SymbolTable* table, const char* identifier, int scope);
void removeScopeSymbols(SymbolTable* table, int scope);
void removeScopesAbove(SymbolTable* table, int maxScope);
void dumpSymbolTable(SymbolTable* table);
void freeSymbolTable(SymbolTable* table);

#endif
