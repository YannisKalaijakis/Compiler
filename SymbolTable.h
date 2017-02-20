#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#define capacity 300

typedef enum scopespace_t {
	programvar,
	functionlocal,
	formalarg
}scopespace_t;

typedef enum symbol_t{
	var_s,
	programfunc_s,
	libraryfunc_s
}symbol_t;

enum SymbolType{
	GLOBAL,
	LOCAL,
	FORMAL,
	USERFUNC,
	LIBFUNC
};

enum ValueType{VARIABLE, FUNCTION};

/*** STRUCT ARG LIST ***/
typedef struct ArgList{
	struct SymbolTableEntry* head;
}ArgumentList;


/*** STRUCT VARIABLE ***/
typedef struct Variable{
	unsigned int scope;
	unsigned int line;
}Variable;


/*** STRUCT FUNCTION ***/
typedef struct Function{
	ArgumentList* args;
	unsigned int scope;
	unsigned int line;
}Function;


/*** STRUCT SYMBOLTABLEENTRY ***/
typedef struct SymbolTableEntry{
	char* name;
	int test;
	bool isActive;
	scopespace_t space;
	unsigned offset;
	unsigned iaddress;
	union{ 
		Variable *varVal;
		Function *funcVal;
	}value;
	enum ValueType ValType;
	enum SymbolType SymType;
	symbol_t type;
	struct SymbolTableEntry* next;
	struct SymbolTableEntry* scopeNext;
	struct SymbolTableEntry* nextArgument;	
}SymbolTableEntry;

extern SymbolTableEntry *HT[capacity]; 
extern SymbolTableEntry **arrayScope;
extern int hashTableSize;
extern int countFuncs;
extern int scope;
extern bool isFunction;
extern bool forStatement;



void CreateHashTable();//ex
void Insert(SymbolTableEntry *, SymbolTableEntry **);//ex
void HideFromHash(const char *, int);
void Hide(int);
void InsertToScope(SymbolTableEntry *, int);
void createLibFuncs();//ex
void printSymbols();//ex
bool SearchArgList(ArgumentList *, SymbolTableEntry *);
bool isLibFunc(const char *);
int currScope();
unsigned int hash(const char *);//ex
const char* getValType(enum ValueType);
const char* getSymType(enum SymbolType);
SymbolTableEntry* LookUp(const char *);//ex
SymbolTableEntry* LookUpScope(const char *, int);//ex
SymbolTableEntry* isVisible(SymbolTableEntry *, int);
SymbolTableEntry* InsertHash(SymbolTableEntry *, SymbolTableEntry *);
SymbolTableEntry* makeSymbolCopy(SymbolTableEntry *);
ArgumentList* InsertArgument(ArgumentList *, SymbolTableEntry *);
