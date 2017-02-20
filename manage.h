#include "SymbolTable.h"
#include "stack.h"


typedef enum iopcode {
	assign ,	add,	sub,
	mul,		div_,	mod,
	uminus,	and,	or,
	not,		if_eq,	if_noteq,
	if_lesseq,	if_greatereq,	if_less,
	if_greater,	call,	param,
	ret,		getretval,	funcstart,
	funcend,	tablecreate, jump,
	tablegetelem,	tablesetelem
}iopcode;

typedef enum expr_t{
	var_e,
	tableitem_e,
	programfunc_e,
	libraryfunc_e,
	arithexpr_e,
	boolexpr_e,
	assignexpr_e,
	newtable_e,
	constnum_e,
	constbool_e,
	conststring_e,
	nil_e
}expr_t;


typedef struct expr{
	expr_t type;
	SymbolTableEntry* sym;
	struct expr* index;
	double numConst;
	char *strConst;
	unsigned char boolConst;
	int totallocals;
	struct expr* next;
	stack_t* trueList;
	stack_t* falseList;
}expr;

typedef struct quad{
	iopcode  op;
	expr*	result;
	expr*	arg1;
	expr*	arg2;
	unsigned label;
	unsigned line;
	unsigned taddress;
}quad;

typedef struct statement
{
   stack_t* breaklist;
   stack_t* continuelist;
}statements;


//METHOD STRUCT*
typedef struct methodCall
{
   char* name;
   bool method;
   ArgumentList*  elist;
   
}method;

typedef struct forvalue
{
     int test;
     int enter;
}forValue;

// //indelem STRUCT*
 typedef struct indexElement
 {
	 expr* index;
	 expr* object;
	 struct indexElement *next;
		
 }indexElement;

//indexedList STRUCT*
typedef struct indexedList
{
  indexElement* head;
  int listSize;
  
}indexedList;

extern quad* quads;
extern unsigned programVarOffset;
extern unsigned functionLocalOffset;
extern unsigned formalArgOffset;
extern unsigned scopeSpaceCounter;
extern unsigned total;
extern unsigned int currQuad;
extern unsigned int tempCounter;
extern int yylineno;

void expand(void);
void emit(iopcode op, expr* arg1, expr* arg2, expr* result, unsigned label, unsigned line);
void inccurrscopeoffset(void);
void enterscopespace(void);
void exitscopespace(void);
void resetformalargsoffset(void);
void resetfunctionlocaloffset(void);
void restorecurrscopeoffset(unsigned n);
void patchlabel(unsigned quadNo, unsigned label);
void resetTemp();
void checkuminus(expr* e);
void print_quads();
unsigned currscopeoffset(void);
unsigned nextquadlabel(void);
unsigned int istempname(char* s);
unsigned int istempexpr(expr* e);
char* newTempName();
const char* Expression2String(expr* ex);
expr* lvalue_expr(SymbolTableEntry* sym);
expr* newexpr(expr_t t);
expr* newexpr_conststring(char* s);
expr* newexpr_constnum( double s);
expr* newexpr_constbool(unsigned char s);
expr* emit_iftableitem(expr* e);
expr* make_call(expr* lvalue, ArgumentList* elist);
expr* member_item(expr* lvalue,char* name);
scopespace_t currscopespace(void);
SymbolTableEntry* newTemp();
