#define EXPAND_SIZE 1024
#define CURR_SIZE	(total*sizeof(quad))
#define NEW_SIZE	(EXPAND_SIZE *sizeof(quad)+ CURR_SIZE)
#include <assert.h>
#include "manage.h"

unsigned int currQuad=0;

void expand(void){
  assert(total == currQuad);

  quad* p = (quad*)malloc(NEW_SIZE);
  
  if(quads){
    memcpy(p,quads, CURR_SIZE);
    free(quads);
  }
  quads = p;
  total += EXPAND_SIZE;
}

void emit(iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label, unsigned line){

  if(currQuad ==  total)
    expand();

  quad* p = quads +currQuad++;
  p->arg1 =  arg1;
  p->arg2 =  arg2;
  if( result == NULL)
  {
  	//printf("\n DANGER mothafucka");
  }
  p->result = result;
  p->label = label;
  p->line = line;
  p->op = op;
}


scopespace_t currscopespace(void)
{
	if(scopeSpaceCounter == 1)
	return programvar;
	else
	{
		if(scopeSpaceCounter % 2 == 0)
			return formalarg;
		else
		{
			return functionlocal;	
		}      
	}
}

unsigned currscopeoffset(void) {
    switch(currscopespace())
    {
      case programvar	:  return programVarOffset;
      case functionlocal:  return functionLocalOffset;
      case formalarg	:  return formalArgOffset;
      default		: assert(0);
      
    }
  
}

void inccurrscopeoffset(void) 
{
    switch (currscopespace()) {
      case programvar	:	++programVarOffset;break;
      case functionlocal:	++functionLocalOffset;break;
      case formalarg	:	++formalArgOffset;
      default		:	assert(0);
    }
}

void enterscopespace(void)
{
  ++scopeSpaceCounter;
}

void exitscopespace(void)
{
  assert(scopeSpaceCounter > 1);
  --scopeSpaceCounter;
}

void resetformalargsoffset(void){
  formalArgOffset = 0;
}

void resetfunctionlocaloffset(void)
{
  functionLocalOffset = 0;
}

void restorecurrscopeoffset(unsigned n)
{
  switch(currscopespace())
  {
    case programvar		:  programVarOffset = n; break;
    case functionlocal	:  functionLocalOffset = n; break;
    case formalarg	:  formalArgOffset = n; break;
    default		:  assert(0);
  }
}

unsigned nextquadlabel(void)
{
  return currQuad;
}

void patchlabel(unsigned quadNo, unsigned label)
{
	assert(quadNo < currQuad);
	quads[quadNo].label = label;
}

char* newTempName(){
	char tmpStr[10] = "_t";
	char s[10] = "";
	sprintf(s, "%d", tempCounter++);
	return strdup(strcat(tmpStr, s));
}

void resetTemp(){
	tempCounter=0;
}

SymbolTableEntry* newTemp() {

	char* name = newTempName();
	SymbolTableEntry* symbol = LookUpScope(name, currScope());
	if (symbol == NULL) {
		symbol = (SymbolTableEntry*)malloc(sizeof( SymbolTableEntry));
		symbol->isActive = true;
		symbol->name = strdup(name);
		symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
		symbol->value.varVal->scope = scope;
		symbol->value.varVal->line = yylineno;
		symbol->ValType = VARIABLE;
		symbol->type = var_s;
		symbol->offset = currscopeoffset();
		symbol->space = currscopespace();
		inccurrscopeoffset();
		symbol->next = NULL;
		symbol->nextArgument = NULL;
		symbol->scopeNext = NULL;

		if (symbol->space != formalarg) {
			symbol->SymType = (scope>0)?LOCAL:GLOBAL;
		} else {
			symbol->SymType = FORMAL;
		}
	
		Insert(symbol, HT);

		return symbol;

	} else { return symbol; }
}

expr* lvalue_expr(SymbolTableEntry* sym){
	assert(sym);
	expr* e = (expr*) malloc(sizeof(expr));
	memset(e, 0, sizeof(expr));

	e->next = (expr*)0;
	e->sym = sym;
	
	switch(sym->type){
		case var_s:			e->type = var_e; break;
		case programfunc_s:	e->type = programfunc_e; break;
		case libraryfunc_s:	e->type = libraryfunc_e; break;
		default: assert(0);
	}
	
	return e;
}

expr* newexpr(expr_t t){
	expr* e = (expr*) malloc(sizeof(expr));
	memset(e, 0, sizeof(expr));
	e->type = t;
	return e;
}

expr* newexpr_conststring(char* s){
	expr* e = newexpr(conststring_e);
	e->strConst = strdup(s);
	e->sym = newTemp();
	e->type = conststring_e;
	return e;
}

expr* newexpr_constbool(unsigned char s){
	expr* e = newexpr(constbool_e);
	e->boolConst = s;
	e->sym = newTemp();
	e->type = constbool_e;
	return e;
}

expr* newexpr_constnum( double s){
	expr* e = newexpr(constnum_e);
	e->numConst = s;
	e->sym = newTemp();
	e->type = constnum_e;
	return e;
}



expr* emit_iftableitem(expr* e){
	if(e->type != tableitem_e){
		return e;
	}else{
		expr* result = newexpr(var_e);
		result->sym = newTemp();
		emit(tablegetelem, e, e->index, result, 0, yylineno);
		return result;
	}
}

expr* member_item(expr* lvalue,char* name){
	lvalue = emit_iftableitem(lvalue);
	expr* item = newexpr(tableitem_e);
	item->sym = lvalue->sym;
	item->index = newexpr_conststring(name);
	
	return item;
}

expr* make_call(expr* lvalue, ArgumentList* elist){

	expr* func  =  emit_iftableitem(lvalue);
	expr *exp, *result;
	SymbolTableEntry* tmp =  elist->head;
	
	for(; tmp != NULL; tmp=tmp->nextArgument){
		
		if(tmp->type == programfunc_s){
			exp = newexpr(programfunc_e);
		}else if(tmp->type == libraryfunc_s){
			exp = newexpr(libraryfunc_e);
		}else if(tmp->type == var_s){
			exp = newexpr(var_e);
		}
		exp->sym = tmp;
		emit(param, NULL, NULL, exp, 0, yylineno);
	}		

	emit(call,func, NULL,NULL,0, yylineno);
	result = newexpr(var_e);
	result->sym = newTemp();
	emit(getretval,result, NULL,NULL,0, yylineno);
	return result;
}


void checkuminus(expr* e){
	if (e->type == constbool_e ||
		e->type == conststring_e ||
		e->type == nil_e ||
		e->type == newtable_e ||
		e->type == programfunc_e ||
		e->type == libraryfunc_e ||
		e->type == boolexpr_e){
		//comperror("Illegal expr to unary -");
	}
}

unsigned int istempname(char* s){
	return (strcmp(s,"_")? 0:1);
}

unsigned int istempexpr(expr* e){
	//??-not sure
	return( (e->sym) && (e->sym->type == var_s) && (istempname(e->sym->name)));
}


void print_quads()
{
	//struct quad *q;
	char Strings[26][13] = {"ASSIGN", "ADD", "SUB","MUL","DIV","MOD",
	"UMINUS","AND",	"OR","NOT","IF_EQ",	"IF_NOTEQ","IF_LESSEQ",	"IF_GREATEREQ",	"IF_LESS",
	"IF_GREATER","CALL","PARAM","RET","GETRETVAL",	"FUNCSTART","FUNCEND","TABLECREATE","JUMP",
	"TABLEGETELEM",	"TABLESETELEM"};

	int i = 0;
	int count = 1;
	FILE *fp;

	if (!(fp = fopen("quads.txt","w+"))) {
		fprintf(stderr, "[Error]: Could not open file 'quads.txt'.\n");
		exit(1);
	}

	fprintf(fp, "%-4s %-15s%-20s%-20s%-20s%-10s%-5s\n\n", "No","OP", "ARG1", "ARG2", "RESULT", "LABEL", "LINE");

	for (; i<currQuad; ++i) {
		fprintf(fp,"%-3d: ", count++);
		fprintf(fp, "%-15s", Strings[quads[i].op]);
		fprintf(fp, "%-20s", Expression2String(quads[i].arg1));
		fprintf(fp, "%-20s", Expression2String(quads[i].arg2));
		fprintf(fp, "%-20s", Expression2String(quads[i].result));
		fprintf(fp, "%-10d", quads[i].label);
        fprintf(fp, "%-5d\n",quads[i].line);
	}
	fflush(fp);
}

const char* Expression2String(expr* ex){

	if(ex == NULL){
		return "NULL";
	}

	if(ex->type == constnum_e){

		char TempString[50]= "";
		char TempString2[50]= "";
		sprintf(TempString,"%lf", ex->numConst);
		return strcat(TempString2,TempString);;

	}else if(ex->type == constbool_e){
		return ex->boolConst?"TRUE":"FALSE";

	}else if(ex->type == conststring_e){
		return ex->strConst;

	}else if(ex->type == nil_e){
		return "NIL";

	}else{

		if(ex->sym->name != NULL){
			return  ex->sym->name;     		
		}
		return "NULL_VARIABLE_NAME";
	}
 }
     
     
   
     
     
	
