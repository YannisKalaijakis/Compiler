#include "SymbolTable.h"

SymbolTableEntry **arrayScope=NULL;
SymbolTableEntry* HT[capacity];
ArgumentList* AL;
int countFuncs=0;
int scopeCnt=0;
bool isFunction = false;
bool  forStatement=false;

/* return a hash function */
unsigned int hash(const char* name){

  unsigned int ui;
  unsigned int uiHash = 0U;

  for (ui = 0U; name[ui] != '\0'; ui++)
    uiHash = uiHash * 579 + name[ui];
  return uiHash%capacity;
}






/* Create a hash table */
void CreateHashTable()
{
	int i = 0;
	int size = capacity;
	// SymbolTableEntry** HT=(SymbolTableEntry**)malloc(sizeof(SymbolTableEntry*)*size);
	while ( i < size)
	{
		HT[i]= NULL;
		i++;
	}
	//initialize lib funcs
	createLibFuncs();
}

/* insert a symbol to the hash table */
void Insert(SymbolTableEntry* newSymbol, SymbolTableEntry** HT)
{
	if(newSymbol != NULL)
	{

		if(newSymbol->ValType == VARIABLE)
		{
			unsigned int index = hash(newSymbol->name);
			HT[index] = InsertHash(HT[index], newSymbol);
		}
		if(newSymbol->ValType == FUNCTION)
		{
			unsigned int index = hash(newSymbol->name);
			HT[index] = InsertHash(HT[index], newSymbol);
		}
		
	}
}

/* look up for a symbol in HASH TABLE */
SymbolTableEntry* LookUp(const char* symbol2find){

	unsigned int index = hash(symbol2find);
	SymbolTableEntry* symbol = NULL;
	
	symbol = HT[0];
	while(symbol != NULL){
		if(strcmp(symbol->name, symbol2find) == 0){
			return symbol;
		}
		symbol = symbol->next;
	}
	
	symbol = HT[index];
	while(symbol != NULL){
		if(symbol->ValType == VARIABLE){
			if(strcmp(symbol->name, symbol2find) == 0){
				if(symbol->isActive){
					return symbol;
				}
			}
		}
		
		if(symbol->ValType == FUNCTION){
			if(strcmp(symbol->name, symbol2find)==0){
				if(symbol->isActive){
					return symbol;
				}
			}
		}
		symbol = symbol->next;
	}
	return NULL;
}

/* look up for a symbol in SCOPE TABLE */
SymbolTableEntry* LookUpScope(const char *symbol2find, int scope){

	SymbolTableEntry* symbol = NULL;

	if(arrayScope && arrayScope[scope] != NULL){

		symbol = arrayScope[scope];

		while(symbol != NULL ){
			if(symbol->ValType == VARIABLE){
				if(symbol->value.varVal->scope == scope){
					if(strcmp(symbol2find , symbol->name) == 0){
						if(symbol->isActive){
							return symbol;
						}
					}
				}
			}
				
			if(symbol->ValType == FUNCTION){
				if(symbol->value.funcVal->scope == scope){
					if(strcmp(symbol2find , symbol->name) == 0){
						if(symbol->isActive){
							return symbol;
						}
					}
				}
			}
			symbol = symbol->next;
		}

		return NULL;	// not found

	}else{
		if(!arrayScope){
			fprintf(stdout, "\n\t[Warning]: Scope table is empty!\n");
			return NULL;

		}else{ // arrayScope[scope] is NULL
			return NULL;
		}
	}

}

/* check if we have access to a symbol */
SymbolTableEntry* isVisible(SymbolTableEntry *symbol2find, int scope){

	SymbolTableEntry* symbol = NULL;
	const char *name2find = strdup(symbol2find->name);
	int symbolScope = ((symbol2find->ValType==FUNCTION) ? symbol2find->value.funcVal->scope: symbol2find->value.varVal->scope);

	if(!arrayScope && (scope>scopeCnt)){
		return NULL;
	}


    int currScope = scope;

    if( scope == symbolScope)
    {
    	if((symbol=LookUpScope(name2find, currScope)) != NULL)
		{
			if(symbol->isActive){

				return symbol;
			}

		}
    }


	 if(symbol2find->ValType == FUNCTION){
	 	for(;currScope>=0;currScope--){
	 		if((symbol=LookUpScope(name2find, currScope))!=NULL){
	 			if(symbol->ValType == FUNCTION){
	 				return symbol;
	 			}
	 		}
	 	}
	 }
	else{
		//found and its active
		for(;currScope>=0;currScope--){
	 		if((symbol=LookUpScope(name2find, currScope))!=NULL){

				if(symbol->isActive){

					if((isFunction==false)) { // ean einai true simainei oti proigeitai function

						return symbol;

					}else if( (symbol->SymType == GLOBAL) && (isFunction==false)){
						
						return symbol;

					}

					if( (isFunction == true) && (forStatement == true) )
					{
						return symbol;
					}

					if( (isFunction == true ) && (countFuncs == 1) )
					{
						return symbol;
					}
				}
			}
		}
	}

	//default
	return NULL;
}


/* hide a symbol in specific scope from HASH TABLE */
void HideFromHash(const char *name, int scope){

	unsigned int index = hash(name);
	SymbolTableEntry *symbol = NULL;
	symbol = HT[index];

	if(symbol != NULL){

		while(symbol !=NULL){

			if(symbol->ValType == VARIABLE){

				if(symbol->value.varVal->scope == scope){
					if(strcmp(name, symbol->name) == 0 ){
						symbol->isActive = false;
						return;
					}
				}

			}else if(symbol->ValType == FUNCTION){

				if(symbol->value.funcVal->scope == scope){
					if( strcmp (name, symbol->name) == 0 ){
						symbol->isActive = false;
						return;
					}
				}
			}
			symbol = symbol->next;
		}
	}
}

/* hide all symbols in specific scope from SCOPE TABLE*/
void Hide(int scope){

	SymbolTableEntry *symbol=NULL;

	if(arrayScope && arrayScope[scope] != NULL){

		symbol = arrayScope[scope];

		//iterate through the scope list
		while(symbol != NULL){

			symbol->isActive = false;
			
			if(symbol->ValType == VARIABLE){
				HideFromHash(symbol->name, scope);
			}else if(symbol->ValType == FUNCTION){
				HideFromHash(symbol->name, scope);
			}

			symbol = symbol->next;
		}

	}else{
		if(!arrayScope){
			fprintf(stdout, "\t[Warning]: Scope table is empty!\n");
		}
	}
}

/* insert a symbol to scope array */
void InsertToScope(SymbolTableEntry *newSymbol, int scopeIndex){

 	 if(arrayScope==NULL){
		// arrayScope = (SymbolTableEntry**)malloc((scopeIndex+1)*sizeof(SymbolTableEntry*));
		// scopeCnt = scopeIndex+1;
		arrayScope=(SymbolTableEntry**)malloc(100*sizeof(SymbolTableEntry*));
		scopeCnt=99;

	}else{
		if(scopeIndex>=scopeCnt){//(sizeof(arrayScope)/4)<scopeIndex
			arrayScope = (SymbolTableEntry**)realloc(arrayScope, (scopeIndex+1)*sizeof(SymbolTableEntry*));
		    scopeCnt = scopeIndex+1;
		}
	}
	
	if(newSymbol!=NULL){
		if(arrayScope[scopeIndex]==NULL){
			arrayScope[scopeIndex]=(SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
		    newSymbol->next = NULL;
		    arrayScope[scopeIndex] = newSymbol;

		}else{
			newSymbol->next = arrayScope[scopeIndex];
			arrayScope[scopeIndex] = newSymbol;
		}
	}
}


bool SearchArgList(ArgumentList *List, SymbolTableEntry *symbol2find){	

	SymbolTableEntry* ptr = NULL;

	if((List != NULL) && (List->head != NULL)){
		for(ptr = List->head; ptr; ptr=ptr->nextArgument){
			if(!strcmp(ptr->name, symbol2find->name)){
				if(ptr->type == symbol2find->type){
					return true;
				}
			}
		}
	}
	return false;
}

void printArgList(ArgumentList* List){	

	SymbolTableEntry* ptr = NULL;

	if((List != NULL) && (List->head != NULL)){
		for(ptr = List->head; ptr; ptr=ptr->nextArgument){
			printf("\n ## %s\n", ptr->name);
		}
	}
}

ArgumentList* InsertArgument(ArgumentList* List, SymbolTableEntry* ArgumentIns){

	if(List == NULL){
		List = (ArgumentList*)malloc(sizeof(ArgumentList));
	}
	if(ArgumentIns!=NULL){
		if(List->head==NULL){
			ArgumentIns->nextArgument = NULL;
			List->head = ArgumentIns;
		}else{
			ArgumentIns->nextArgument = List->head;
			List->head = ArgumentIns;
		}
	}

	return List;
}

/* make a copy of given symbol */
SymbolTableEntry* makeSymbolCopy(SymbolTableEntry *symbol){

	SymbolTableEntry *copy = (SymbolTableEntry *)malloc(sizeof(SymbolTableEntry));
	copy->name = symbol->name;
	copy->test = symbol->test;
	copy->isActive = symbol->isActive;
	copy->space = symbol->space;
	copy->offset = symbol->offset;
	copy->iaddress = symbol->iaddress;

	if(symbol->ValType == VARIABLE){
		copy->ValType = VARIABLE;
		copy->value.varVal = (Variable*)malloc(sizeof(Variable));
		copy->value.varVal->scope = symbol->value.varVal->scope;
		copy->value.varVal->line = symbol->value.varVal->line;
	}else{
		copy->ValType = FUNCTION;
		copy->value.funcVal = (Function*)malloc(sizeof(Function));
		copy->value.funcVal->scope = symbol->value.funcVal->scope;
		copy->value.funcVal->line = symbol->value.funcVal->line;
	}

	copy->SymType = symbol->SymType;
	copy->type = symbol->type;
	copy->next = symbol->next;
	copy->scopeNext = symbol->scopeNext;
	copy->nextArgument = NULL;

	return copy;
}

/* insert a symbol to hash & scope table */
SymbolTableEntry* InsertHash(SymbolTableEntry* BUCKET, SymbolTableEntry* newSymbol){

	if(newSymbol != NULL){
		if(BUCKET == NULL){
			BUCKET = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
			newSymbol->next = NULL;
			BUCKET = newSymbol;
		}else{
			newSymbol->next = BUCKET;
			BUCKET = newSymbol;
		}

		if(newSymbol->ValType == VARIABLE){
			InsertToScope(newSymbol, newSymbol->value.varVal->scope);
		
		}else if(newSymbol->ValType == FUNCTION){
			BUCKET->value.funcVal->args = NULL;
			InsertToScope(newSymbol, newSymbol->value.funcVal->scope);
		}
	}
	return BUCKET;
}


/*create a node foe lib func - helps for initialization */
void createLibFuncs(){
	
	int i=0 ;
	for (; i<12; i++) {
		
		SymbolTableEntry* Libnode = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
		Libnode->isActive = true;
		Libnode->value.funcVal=(Function*)malloc(sizeof(Function));
		Libnode->SymType = LIBFUNC;
		Libnode->ValType = FUNCTION;
		Libnode->type = libraryfunc_s;
		Libnode->iaddress = i;
		Libnode->value.funcVal->scope = 0;
		Libnode->value.funcVal->line= 0;
		Libnode->next= NULL;
		Libnode->nextArgument= NULL;
		Libnode->scopeNext= NULL;

		if(i == 0)
		{ Libnode->name = strdup("print");}
		else if(i == 1)
		{Libnode->name = strdup("input");}
		else if(i == 2)
		{ Libnode->name = strdup("objectmemberkeys");}
		else if(i == 3)
		{ Libnode->name = strdup("objecttotalmembers");}
		else if(i == 4)
		{ Libnode->name = strdup("objectcopy");}
		else if(i == 5)
		{ Libnode->name = strdup("totalarguments");}
		else if(i == 6)
		{ Libnode->name = strdup("argument");}
		else if(i == 7)
		{ Libnode->name = strdup("typeof");}
		else if(i == 8)
		{ Libnode->name = strdup("strtonum");}
		else if(i == 9)
		{ Libnode->name = strdup("sqrt");}
		else if(i == 10)
		{ Libnode->name = strdup("cos");}
		else
		{ Libnode->name = strdup("sin");}
		
		HT[0]= InsertHash(HT[0],Libnode);
			
	}
}

const char* getValType(enum ValueType ValType){
	switch(ValType){
		case VARIABLE: 	return "VARIABLE";
		case FUNCTION: 	return "FUNCTION";
		default:		return "DEFAULT";
	}
}

const char* getSymType(enum SymbolType SymType){
	switch(SymType){
		case GLOBAL:	return "GLOBAL";
		case LOCAL:		return "LOCAL";
		case FORMAL:	return "FORMAL";
		case USERFUNC:	return "USERFUNC";
		case LIBFUNC:	return "LIBFUNC";
		default:		return "DEFAULT";
	}
}

bool isLibFunc(const char* funcname){

	if(strcmp(funcname, "print") == 0){
		return true;
	}else if(strcmp(funcname, "input") == 0){
		return true;
	}else if(strcmp(funcname, "objectmemberkeys") == 0){
		return true;
	}else if(strcmp(funcname, "objecttotalmembers") == 0){
		return true;
	}else if(strcmp(funcname, "objectcopy") == 0){
		return true;
	}else if(strcmp(funcname, "totalarguments") == 0){
		return true;
	}else if(strcmp(funcname, "argument") == 0){
		return true;
	}else if(strcmp(funcname, "typeof") == 0){
		return true;
	}else if(strcmp(funcname, "strtonum") == 0){
		return true;
	}else if(strcmp(funcname, "sqrt") == 0){
		return true;
	}else if(strcmp(funcname, "cos") == 0){
		return true;
	}else if(strcmp(funcname, "sin") == 0){
		return true;
	}else{
		return false;
	}
}

int currScope(){
	return scope;
}


void printSymbols()
{
	SymbolTableEntry* ptr= NULL;
	unsigned int i=0;

	printf("\n\n==========================\n");
	printf("\tSYMBOLS");
	printf("\n==========================\n");
	
	for(i = 0; i <= scopeCnt; i++){
		//printf("the size : %lu" ,(sizeof(arrayScope)));
		if((ptr=arrayScope[i])!=NULL){
		
			printf("\nscope %d:\n---------\n", i);
			
			while(ptr != NULL){
			
				if(ptr->ValType == VARIABLE){

					printf("%-20s(line: %d, offset: %d, %s, %s)\n", 
							ptr->name,
							ptr->value.varVal->line,
							ptr->offset,
							getValType(ptr->ValType),
							getSymType(ptr->SymType));
				}else{

					printf("%-20s(line: %d, address: %d, %s, %s)\n", 
							ptr->name,
							ptr->value.funcVal->line,
							ptr->iaddress,
							getValType(ptr->ValType),
							getSymType(ptr->SymType));
				}
				ptr=ptr->next;
			}
			printf("\n");
		}
	}
}

