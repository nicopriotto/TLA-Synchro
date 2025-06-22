#include "SymbolTable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define BUILTIN_SCOPE_ID -1  
#define GLOBAL_SCOPE_ID 0   

static SymbolEntry *newEntry(const char *id, SymbolType t, int scope, const void *data);
static void freeEntry(SymbolEntry *e);

static Logger* _logger = NULL;

void initializeSymbolTableModule() {
    _logger = createLogger("SymbolTable");
}

void shutdownSymbolTableModule() {
    if (_logger != NULL) {
        destroyLogger(_logger);
        _logger = NULL;
    }
}

void initSymbolTable(SymbolTable* table) {
    if (!table) return;
    table->head = NULL;
    table->size = 0;
    
    insertSymbol(table, "print", SYMBOL_FUNCTION, BUILTIN_SCOPE_ID, NULL);
    insertSymbol(table, "sleep", SYMBOL_FUNCTION, BUILTIN_SCOPE_ID, NULL);
    insertSymbol(table, "up", SYMBOL_FUNCTION, BUILTIN_SCOPE_ID, NULL);
    insertSymbol(table, "down", SYMBOL_FUNCTION, BUILTIN_SCOPE_ID, NULL);
    insertSymbol(table, "thread", SYMBOL_FUNCTION, BUILTIN_SCOPE_ID, NULL);
    insertSymbol(table, "status", SYMBOL_INTEGER, BUILTIN_SCOPE_ID, NULL);
    
    logDebugging(_logger, "Symbol table initialized with built-in functions and variables");
}

boolean insertSymbol(SymbolTable* table, const char* identifier, SymbolType type, int scope, SymbolData* data) {
    if (!table || !identifier) return false;
    
    SymbolEntry* existing = findSymbol(table, identifier, scope);
    if (existing && existing->scope == scope) {
        logDebugging(_logger, "Symbol %s already exists in scope %d", identifier, scope);
        return false;
    }
    
    SymbolEntry* entry = malloc(sizeof(SymbolEntry));
    if (!entry) return false;
    
    entry->identifier = strdup(identifier);
    if (!entry->identifier) {
        free(entry);
        return false;
    }
    
    entry->type = type;
    entry->scope = scope;
    
    if (data) {
        entry->data = *data;
    } else {
        memset(&entry->data, 0, sizeof(SymbolData));
    }
    
    entry->next = table->head;
    table->head = entry;
    table->size++;
    
    logDebugging(_logger, "Inserted symbol '%s' of type %d in scope %d", identifier, type, scope);
    
    return true;
}

SymbolEntry* findSymbol(SymbolTable* table, const char* identifier, int scope) {
    if (!table || !identifier) return NULL;
    
    SymbolEntry* current = table->head;
    
    if (scope == -1) {
        while (current) {
            if (strcmp(current->identifier, identifier) == 0) {
                return current;
            }
            current = current->next;
        }
    } else {
        while (current) {
            if (strcmp(current->identifier, identifier) == 0 && current->scope == scope) {
                return current;
            }
            current = current->next;
        }
    }
    
    return NULL;
}

boolean updateSymbolValue(SymbolTable *table, const char *id, int scope, const void *data) {
    if (!table || !id || !data) return false;
    
    SymbolEntry *e = findSymbol(table, id, scope);
    if (!e) return false;
    
    if (e->type == SYMBOL_STRING && e->data.stringValue) {
        free(e->data.stringValue);
        e->data.stringValue = NULL;
    }
    
    switch (e->type) {
        case SYMBOL_INTEGER:
            e->data.intValue = *(const int*)data; 
            break;
        case SYMBOL_STRING: 
            e->data.stringValue = strdup((const char*)data); 
            if (!e->data.stringValue) return false; 
            break;
        case SYMBOL_FLOAT: 
            e->data.floatValue = *(const float*)data; 
            break;
        case SYMBOL_BOOLEAN: 
            e->data.boolValue = *(const boolean*)data; 
            break;
        case SYMBOL_SEMAPHORE: 
            e->data.intValue = *(const int*)data; 
            break;
        case SYMBOL_FUNCTION: 
            break;
    }
    return true;
}

boolean removeSymbol(SymbolTable *table, const char *id, int scope) {
    if (!table || !id) return false;
    
    SymbolEntry *prev = NULL, *curr = table->head, *targetPrev = NULL, *target = NULL;
    
    while (curr) {
        boolean match = strcmp(curr->identifier, id) == 0 && (scope == -1 || curr->scope == scope);
        if (match) {
            if (scope == -1) {
                if (!target || curr->scope > target->scope) {
                    target = curr;
                    targetPrev = prev;
                }
            } else {
                target = curr;
                targetPrev = prev;
                break;
            }
        }
        prev = curr;
        curr = curr->next;
    }
    
    if (!target) return false;
    
    if (targetPrev) {
        targetPrev->next = target->next;
    } else {
        table->head = target->next;
    }
    
    freeEntry(target);
    table->size--;
    return true;
}

void removeScopeSymbols(SymbolTable* table, int scope) {
    if (!table) return;
    
    SymbolEntry* current = table->head;
    SymbolEntry* prev = NULL;
    
    while (current) {
        if (current->scope == scope) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->head = current->next;
            }
            
            SymbolEntry* toDelete = current;
            current = current->next;

            free(toDelete->identifier);
            free(toDelete);
            table->size--;
            
            logDebugging(_logger, "Removed all symbols from scope %d", scope);
        } else {
            prev = current;
            current = current->next;
        }
    }
}

void removeScopesAbove(SymbolTable* table, int maxScope) {
    if (!table) return;
    
    SymbolEntry* current = table->head;
    SymbolEntry* prev = NULL;
    int removedCount = 0;
    
    while (current) {
        if (current->scope > maxScope) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->head = current->next;
            }
            
            SymbolEntry* toDelete = current;
            current = current->next;
            
            free(toDelete->identifier);
            free(toDelete);
            table->size--;
            removedCount++;
        } else {
            prev = current;
            current = current->next;
        }
    }
    
    if (removedCount > 0) {
        logDebugging(_logger, "Removed %d symbols from scopes above %d", removedCount, maxScope);
    }
}

void freeSymbolTable(SymbolTable* table) {
    if (!table) return;
    
    SymbolEntry* current = table->head;
    while (current) {
        SymbolEntry* next = current->next;
        free(current->identifier);
        free(current);
        current = next;
    }
    
    table->head = NULL;
    table->size = 0;
    
    logDebugging(_logger, "Symbol table freed");
}

void dumpSymbolTable(SymbolTable* table) {
    if (!table) return;
    
    printf("=== Symbol Table Dump ===\n");
    
    SymbolEntry* current = table->head;
    while (current) {
        const char* scopeName;
        if (current->scope == -1) {
            scopeName = "BUILTIN";
        } else if (current->scope == 0) {
            scopeName = "GLOBAL";
        } else {
            scopeName = "LOCAL";
        }
        
        printf("[Scope %d (%s)] %s (type: %d)\n", 
               current->scope, scopeName, current->identifier, current->type);
        current = current->next;
    }
    
    printf("========================\n");
}

static SymbolEntry *newEntry(const char *id, SymbolType t, int scope, const void *data) {
    SymbolEntry *e = malloc(sizeof(SymbolEntry));
    if (!e) return NULL;
    
    e->identifier = strdup(id);
    if (!e->identifier) { 
        free(e); 
        return NULL; 
    }
    
    e->type = t;
    e->scope = scope;
    e->next = NULL;
    
    switch (t) {
        case SYMBOL_INTEGER: 
            e->data.intValue = data ? *(const int*)data : 0; 
            break;
        case SYMBOL_STRING: 
            if (data) {
                e->data.stringValue = strdup((const char*)data); 
                if (!e->data.stringValue) { 
                    freeEntry(e); 
                    return NULL; 
                }
            } else {
                e->data.stringValue = NULL;
            }
            break;
        case SYMBOL_FLOAT: 
            e->data.floatValue = data ? *(const float*)data : 0.0f; 
            break;
        case SYMBOL_BOOLEAN: 
            e->data.boolValue = data ? *(const boolean*)data : false; 
            break;
        case SYMBOL_SEMAPHORE: 
            e->data.intValue = data ? *(const int*)data : 0; 
            break;
        case SYMBOL_FUNCTION: 
            break;
    }
    
    return e;
}

static void freeEntry(SymbolEntry *e) {
    if (!e) return;
    
    if (e->identifier) {
        free(e->identifier);
    }
    
    if (e->type == SYMBOL_STRING && e->data.stringValue) {
        free(e->data.stringValue);
    }
    
    free(e);
}
