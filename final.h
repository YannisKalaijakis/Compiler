#include "manage.h"

typedef enum type_p {
	str,
	num,
	userfunc,
	libfunc,
	instr,
}type;

typedef enum vmopcode {
	assign_v,       add_v,          sub_v,
	mul_v,          div_v,          mod_v,
	uminus_v,       and_v,          or_v,
	not_v,          jeq_v,          jne_v,
	jle_v,          jge_v,          jlt_v,
	jgt_v,          call_v,         pusharg_v,
	funcenter_v,    funcexit_v,     newtable_v,
	tablegetelem_v, tablesetelem_v, nop_v,
}vmopcode;

typedef enum vmarg_t {
	label_a   = 0,
	global_a  = 1,
	formal_a  = 2,
	local_a   = 3,
	number_a  = 4,
	string_a  = 5,
	bool_a    = 6,
	nil_a   = 7,
	userfunc_a  = 8,
	libfunc_a = 9,
	retval_a  = 10
}vmarg_t;

typedef struct vmarg {
	vmarg_t   type;
	unsigned  val;
}vmarg;

typedef struct instruction {
	vmopcode  opcode;
	vmarg   *result;
	vmarg   *arg1;
	vmarg   *arg2;
	unsigned  line;
	int typeflag;
}instruction;

typedef struct avm_userfunc{
	unsigned  taddress;
	unsigned  totallocals;
	char*   id;
}avm_userfunc;

typedef struct incomplete_jump {
	unsigned instrNo;
	unsigned iaddress;
	struct incomplete_jump* next;
}incomplete_jump;

extern unsigned int Current_size_of_Str;
extern unsigned int Expand_size_of_Str;
extern unsigned int constString;
extern unsigned int totalConstString;
extern char **strArray;
extern  unsigned int currQuad;

extern quad* quads;

void generate_ADD(quad*);
void generate_SUB(quad*);
void generate_MUL(quad*);
void generate_DIV(quad*);
void generate_MOD(quad*);
void generate_NEWTABLE(quad*);
void generate_TABLEGETELEM(quad*);
void generate_TABLESETELEM(quad*);
void generate_ASSIGN(quad*);
void generate_NOP(quad*);
void generate_JUMP(quad*);
void generate_IF_EQ(quad*);
void generate_IF_NOTEQ(quad*);
void generate_IF_GREATER(quad*);
void generate_IF_GREATEREQ(quad*);
void generate_IF_LESS(quad*);
void generate_IF_LESSEQ(quad*);
void generate_NOT(quad*);
void generate_OR(quad*);
void generate_AND(quad*);
void generate_PARAM(quad*);
void generate_CALL(quad*);
void generate_GETRETVAL(quad*);
void generate_FUNCSTART(quad*);
void generate_RETURN(quad*);
void generate_FUNCEND(quad*);
void patch_incomplete_jumps();
void generate_UMINUS( quad* );
void generateALL(void);
void generate(vmopcode, quad*);
void make_operand(expr *, vmarg *);
unsigned consts_newstring(char *);
unsigned consts_newnumber(double );
unsigned libfuncs_newused(char *);
unsigned userfuncs_newfunc (avm_userfunc *);
char* getVmargType(vmarg_t type);
char* getVmOpcode(vmopcode opcode);
vmarg* make_retvaloperand(vmarg*);
