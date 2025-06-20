#include "SymbolTable.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static SymbolEntry *newEntry(const char *id, SymbolType t, int scope, const void *data);
static void freeEntry(SymbolEntry *e);

void initSymbolTable(SymbolTable *table) {
    table->head = NULL;
}

boolean insertSymbol(SymbolTable *table, const char *id, SymbolType t, int scope, const void *data) {
    if (!table || !id) return false;
    if (findSymbol(table, id, scope)) return false;
    SymbolEntry *e = newEntry(id, t, scope, data);
    if (!e) return false;
    e->next = table->head;
    table->head = e;
    return true;
}

SymbolEntry *findSymbol(const SymbolTable *table, const char *id, int scopeWanted) {
    SymbolEntry *best = NULL;
    for (SymbolEntry *p = table->head; p; p = p->next) {
        if (strcmp(p->identifier, id) != 0) continue;
        if (scopeWanted < 0) {
            if (!best || p->scope > best->scope) best = p;
        } else if (p->scope == scopeWanted) {
            return p;
        }
    }
    return best;
}

boolean updateSymbolValue(SymbolTable *table, const char *id, int scope, const void *data) {
    SymbolEntry *e = findSymbol(table, id, scope);
    if (!e) return false;
    if (e->type == SYMBOL_STRING) {
        free(e->data.stringValue);
        e->data.stringValue = NULL;
    }
    switch (e->type) {
        case SYMBOL_INTEGER:
            e->data.integerValue = *(const int*)data; 
            break;
        case SYMBOL_STRING: 
            e->data.stringValue = strdup((const char*)data); 
            if (!e->data.stringValue) return false; 
            break;
        case SYMBOL_FLOAT: 
            e->data.floatValue = *(const float*)data; 
            break;
        case SYMBOL_BOOLEAN: 
            e->data.booleanValue = *(const boolean*)data; 
            break;
        case SYMBOL_SEMAPHORE: 
            e->data.semaphoreValue = *(const int*)data; 
            break;
        case FUNCTION: 
            e->data.functionType = *(const SymbolType*)data; 
            break;
    }
    return true;
}

boolean removeSymbol(SymbolTable *table, const char *id, int scope) {
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
    if (targetPrev) targetPrev->next = target->next;
    else table->head = target->next;
    freeEntry(target);
    return true;
}

void removeScopeSymbols(SymbolTable *table, int scope) {
    SymbolEntry *prev = NULL, *curr = table->head;
    while (curr) {
        SymbolEntry *next = curr->next;
        if (curr->scope == scope) {
            if (prev) prev->next = next;
            else table->head = next;
            freeEntry(curr);
        } else {
            prev = curr;
        }
        curr = next;
    }
}

void freeSymbolTable(SymbolTable *table) {
    SymbolEntry *curr = table->head;
    while (curr) {
        SymbolEntry *next = curr->next;
        freeEntry(curr);
        curr = next;
    }
    table->head = NULL;
}

void dumpSymbolTable(const SymbolTable *table) {
    for (SymbolEntry *e = table->head; e; e = e->next) {
        printf("[%d] %s\n", e->scope, e->identifier);
    }
}

static SymbolEntry *newEntry(const char *id, SymbolType t, int scope, const void *data) {
    SymbolEntry *e = malloc(sizeof(SymbolEntry));
    if (!e) return NULL;
    e->identifier = strdup(id);
    if (!e->identifier) { free(e); return NULL; }
    e->type = t;
    e->scope = scope;
    e->next = NULL;
    switch (t) {
        case SYMBOL_INTEGER: e->data.integerValue = *(const int*)data; break;
        case SYMBOL_STRING: e->data.stringValue = strdup((const char*)data); if (!e->data.stringValue) { freeEntry(e); return NULL; } break;
        case SYMBOL_FLOAT: e->data.floatValue = *(const float*)data; break;
        case SYMBOL_BOOLEAN: e->data.booleanValue = *(const boolean*)data; break;
        case SYMBOL_SEMAPHORE: e->data.semaphoreValue = *(const int*)data; break;
        case FUNCTION: e->data.functionType = *(const SymbolType*)data; break;
    }
    return e;
}

static void freeEntry(SymbolEntry *e) {
    if (!e) return;
    free(e->identifier);
    if (e->type == SYMBOL_STRING) free(e->data.stringValue);
    free(e);
}
