#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "final.h"


//ex
#define AVM_STACKSIZE 4096
//ex
#define AVM_WIPEOUT(m) memset(&(m) , 0, sizeof(m))
//ex
#define AVM_TABLE_HASHSIZE 211
//ex
#define AVM_NUMACTUALS_OFFSET +4
//ex
#define AVM_SAVEDPC_OFFSET +3
//ex
#define AVM_SAVEDTOP_OFFSET +2
//ex
#define AVM_SAVEDTOPSP_OFFSET +1
//ex
#define AVM_STACKENV_SIZE	4
//ex 
#define AVM_MAX_INSTRUCTIONS (unsigned) nop_v
//ex 
#define AVM_ENDING_PC codeSize


FILE *fp;
//ex with other names in lecture 13 page 17/35
unsigned TotalGlobals;

unsigned TotalNums;
double *Nums;

unsigned TotalStrings;
char **Strings;

unsigned Totalused;

unsigned TotalLibs;
char **LibFuncs;

typedef void (*execute_func_t)(instruction*);

typedef void (*library_func_t)(void);
extern void libfunc_print(void);
extern void libfunc_totalarguments(void);
extern void libfunc_typeof(void);


library_func_t libraryFunctions[]  = {
	libfunc_print,
	libfunc_totalarguments,
	libfunc_typeof
};

//ex without typedef 
typedef  enum avm_memcell_t{
	number_m = 0,
	string_m = 1,
	bool_m = 2,
	table_m = 3,
	userfunc_m = 4,
	libfunc_m = 5,
	nil_m = 6,
	undef_m = 7
}avm_memcell_t;

//ex
typedef struct avm_memcell{
	avm_memcell_t type;
	union{
		double numVal;
		char* strVal;
		unsigned char boolVal;
		struct avm_table* tableVal;
		unsigned funcVal;
		char* libfuncVal;
	}data;
}avm_memcell;

typedef char* (*tostring_func_t)(avm_memcell* );

//ex all 
extern char* number_tostring(avm_memcell*);//finished
extern char* string_tostring(avm_memcell*);//finished
extern char* table_tostring(avm_memcell*);//finished
extern char* bool_tostring(avm_memcell* m);//finished
extern char* userfunc_tostring(avm_memcell*);//finished
extern char* libfunc_tostring(avm_memcell*);//finished
extern char* nil_tostring(avm_memcell*);//finished
extern char* undef_tostring(avm_memcell*);//finished
////////////////////////////////////////////////////////////
void PrintInstr(instruction* instr, FILE *fout);

//ex
tostring_func_t tostringFuncs[] ={
	number_tostring,
	string_tostring,
	bool_tostring,
	table_tostring,
	userfunc_tostring,
	libfunc_tostring,
	nil_tostring,
	undef_tostring
};

typedef unsigned char(*tobool_func_t)(avm_memcell*);
typedef double(*arithmetic_func_t)(double x, double y);
typedef void (*memclear_func_t)(avm_memcell*);
extern void memclear_string(avm_memcell* m);
extern void memclear_table(avm_memcell* m);

//ex
memclear_func_t memclearFuncs[] = {
  0,/*number*/
	memclear_string,
  0,/*bool*/
	memclear_table,
  0,/*userfunc*/
  0,/*livfunc*/
  0,/*nil*/
  0,/*undef*/
};

//ex
avm_memcell stack[AVM_STACKSIZE];

//ex
typedef struct avm_table_bucket{
	struct avm_memcell key;
	struct avm_memcell value;
	struct avm_table_bucket *next;
	
}avm_table_bucket;

//ex
typedef struct avm_table{
	unsigned refCounter;
	struct avm_table_bucket* strIndexed[AVM_TABLE_HASHSIZE];
	struct avm_table_bucket*  numIndexed[AVM_TABLE_HASHSIZE];

	unsigned total;

}avm_table;

avm_userfunc *userFunctions;

char* libNames[] = {
	"print",
	"totalarguments",
	"typeof"
};

char* typeStrings[]={
	"number",
	"string",
	"bool",
	"table",
	"userfunc",
	"libfunc",
	"nil",
	"undef"
};


extern unsigned char number_tobool(avm_memcell* m);
extern unsigned char string_tobool(avm_memcell* m);
extern unsigned char bool_tobool(avm_memcell* m);
extern unsigned char table_tobool(avm_memcell* m);
extern unsigned char userfunc_tobool(avm_memcell* m);
extern unsigned char libfunc_tobool(avm_memcell* m);
extern unsigned char nil_tobool(avm_memcell* m);
extern unsigned char undef_tobool(avm_memcell* m);

//ex
tobool_func_t toboolFuncs[] = {
	number_tobool,
	string_tobool,
	bool_tobool,
	table_tobool,
	userfunc_tobool,
	libfunc_tobool,
	nil_tobool,
	undef_tobool
};
//ex all
extern void execute_assign(instruction*);//finished
extern void execute_add(instruction*);//finished
extern void execute_sub(instruction*);//finished
extern void execute_mul(instruction*);//finished
extern void execute_div(instruction*);//finished
extern void execute_mod(instruction*);//finished
extern void execute_uminus(instruction*);//finished
///////////////////////////////////////////
extern void execute_and(instruction*);
extern void execute_or(instruction*);
extern void execute_not(instruction*);
/////////////////////////////////////////////
extern void execute_jeq(instruction*);//finished
extern void execute_jne(instruction*);//finished
extern void execute_jle(instruction*);//finished
extern void execute_jge(instruction*);//finished
extern void execute_jlt(instruction*);//finished
extern void execute_jgt(instruction*);//finished
//////////////////////////////////////////////
extern void execute_call(instruction*);
extern void execute_pusharg(instruction*);
extern void execute_funcenter(instruction*);
extern void execute_funcexit(instruction*);
/////////////////////////////////////////////////
extern void execute_newtable(instruction*);
extern void execute_tablegetelem(instruction*);
extern void execute_tablesetelem(instruction*);
//////////////////////////////////////////////////
extern void execute_nop(instruction*);
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//ex 
execute_func_t executeFuncs[] = {
	execute_assign,
	execute_add,
	execute_sub,
	execute_mul,
	execute_div,
	execute_mod,
	execute_uminus,
	execute_and,
	execute_or,
	execute_not,
	execute_jeq,
	execute_jne,
	execute_jle,
	execute_jge,
	execute_jlt,
	execute_jgt,
	execute_call,
	execute_pusharg,
	execute_funcenter,
	execute_funcexit,
	execute_newtable,
	execute_tablegetelem,
	execute_tablesetelem,
	execute_nop
};





static void avm_initstack(void);//finished
// extern void avm_error(char* format, ..);
extern void avm_calllibfunc(char* funcName);
extern void avm_callsaveenviroment(void);
extern char* avm_tostring(avm_memcell*);
unsigned char avm_tobool(avm_memcell *);
// extern void avm_warning(char* format, ...);
extern void avm_assign(avm_memcell* lv, avm_memcell* rv);
void avm_tablesetelem(avm_table* table, avm_memcell* index, avm_memcell* content);
void memcell_value_number(avm_memcell *, int, avm_memcell*, avm_table*);
void memcell_value_string(avm_memcell* key, int IsCleared,avm_memcell* tmp, avm_table* table);
avm_memcell* memcell_value_not_nil(avm_memcell* value);//finished

avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg);
extern avm_userfunc* avm_getfuncinfo(unsigned address);//finished
avm_table* avm_tablenew(void);
void avm_tabledestroy(avm_table* t);//finished
void avm_tablebucketsdestroy(avm_table_bucket** p);
avm_memcell* avm_tablegetelem(avm_table* table,avm_memcell* index);
// void avm_tablesetelem(avm_memcell* key, avm_memcell* value);
void avm_memcellclear(avm_memcell* m);

void avm_registerlibfunc(char* id, library_func_t addr);//finished
extern avm_memcell* getelemstr(avm_table *table, avm_memcell *key);
avm_memcell* getelemnum(avm_table *table, avm_memcell *key);

// unsigned consts_newstring(char* s);
// unsigned consts_newnumber(double n);
// unsigned libfuncs_newused(char *s);
// unsigned userfuncs_newfunc(SymbolTableEntry* sym);

library_func_t avm_getlibraryfunc(char* id);//finished
void Libfunc_TypeOfavm_tostring(void);//finished
void libfunc_totalarguments(void);




//ex all
double add_impl(double x, double y);//finished
double sub_impl(double x, double y);//finished
double mul_impl(double x, double y);//finished
double div_impl(double x, double y);//finished
double mod_impl(double x, double y);//finished
/////////////////////////////////////////////////////////////////////////////



double consts_getnumber(unsigned index);//finished

char* consts_getstring(unsigned index);//finished
char* libfuncs_getused(unsigned index);//finished
char *Avm_ToString(avm_memcell *m);//finished



//ex
arithmetic_func_t arithmeticFuncs[] = {
	add_impl,
	sub_impl,
	mul_impl,
	div_impl,
	mod_impl
};






