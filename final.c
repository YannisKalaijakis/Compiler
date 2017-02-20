#include "final.h"
#include <assert.h>

/**
* Pinakes statherwn timwn 
* kai sunartisewn
**/

unsigned int quadsnumber= 0;

/**
*   ARRAY OF INSTRUCTIONS
*/
unsigned int Current_size_of_Instruction= 0;
unsigned int Expand_size_of_Instruction=0;
instruction *instructions;
unsigned int instructionsSize;
unsigned int currInstruction;



incomplete_jump* ij_head =  (incomplete_jump *) 0;
unsigned ij_total = 0;

/**
*   ARRAY OF STRINGS
**/
char **strArray = NULL;
unsigned int Current_size_of_Str = 0;
unsigned int Expand_size_of_Str = 0;
unsigned int totalConstString = 0;
unsigned int strArraySize = 0;

/**
*   ARRAY OF NUMBERS
**/
double* numArray = NULL;
unsigned int Current_size_of_Num = 0;
unsigned int Expand_size_of_Num = 0;
unsigned int totalConstNum = 0;
unsigned int numArraySize = 0;

/**
*   ARRAY OF USER FUNCTIONS
**/
avm_userfunc **userFuncsArray = NULL;
unsigned int Current_size_of_Func = 0;
unsigned int Expand_size_of_Func = 0;
unsigned int userFuncsArraySize = 0;
unsigned int totalConstFunc = 0;

/**
*   ARRAY OF LIB FUNCTIONS
**/
char **libFuncsArray = NULL;
unsigned int Current_size_of_LibFunc = 0;
unsigned int Expand_size_of_LibFunc = 0;
unsigned int libFuncsArraySize = 0;
unsigned int totalConstLibFunc = 0;

avm_userfunc* createUserFunc(expr *e) {
    avm_userfunc *newFunc = malloc(sizeof(avm_userfunc));
    newFunc->taddress = e->sym->iaddress;
    newFunc->totallocals = e->totallocals;
    newFunc->id = strdup(e->sym->name);
    return newFunc;
}


void reset_operand(vmarg* arg) {
    if (arg == NULL )
        arg = malloc(sizeof(vmarg));
    arg->type = nil_a;
}

unsigned int  currprocessedquad()
{
    return quadsnumber;
}

void CurrentSize(type type) {

    if (type == str)
        Current_size_of_Str = (totalConstString)*sizeof(char *);
    else if (type == num)
        Current_size_of_Num = (totalConstNum*sizeof(char *));
    else if (type == userfunc)
        Current_size_of_Func = ( totalConstFunc*sizeof(char *));
    else if (type == instr)
        Current_size_of_Instruction = (instructionsSize*sizeof(char *));
}
  
void ExpandSize(type type) {

    if (type == str)
        Expand_size_of_Str = (1024*sizeof(strArray));
    else if (type == num)
        Expand_size_of_Num = (1024*sizeof(numArray));
    else if (type == userfunc)
        Expand_size_of_Func = (1024*sizeof(userFuncsArray));
    else if (type == instr)
        Expand_size_of_Instruction = (1024*sizeof(instructions));
}

unsigned consts_newstring(char * s){

    if (!strArray)
        strArray = malloc(sizeof(char *));

    int i = 0;
    for (; i<strArraySize; i++) {
        if (strArray[i] && strcmp((strArray[i]), s) == 0)
            return i;
    }
    strArray = (char **)realloc(strArray, (++strArraySize)*sizeof(char *));
    strArray[strArraySize-1] = strdup(s);
    return (strArraySize-1);
}


unsigned consts_newnumber(double n) {

   if (!numArray)
        numArray = malloc(sizeof(double));

    int i = 0;
    for (; i<numArraySize; i++) {
        if (numArray[i] && numArray[i] == n)
            return i;
    }
    numArray = (double *)realloc(numArray, (++numArraySize)*sizeof(double));
    numArray[numArraySize-1] = n;
    return (numArraySize-1);
}


unsigned libfuncs_newused(char* s) {

    if (!libFuncsArray)
        libFuncsArray = malloc(sizeof(char *));

    int i = 0;
    for (; i<libFuncsArraySize; i++) {
        if (libFuncsArray[i] && strcmp(libFuncsArray[i], s) == 0)
            return i;
    }
    libFuncsArray = (char **)realloc(libFuncsArray, (++libFuncsArraySize)*sizeof(char *));
    libFuncsArray[libFuncsArraySize-1] = strdup(s);
    return (libFuncsArraySize-1);
}

unsigned userfuncs_newfunc(avm_userfunc *sym) {

    if (!userFuncsArray)
        userFuncsArray = malloc(sizeof(avm_userfunc*));

    int i = 0;
    for (; i<userFuncsArraySize; i++) {
        if (userFuncsArray[i] && strcmp(userFuncsArray[i]->id, sym->id) == 0)
            return i;
    }
    userFuncsArray = (avm_userfunc **)realloc(userFuncsArray, (++userFuncsArraySize)*sizeof(avm_userfunc*));
    userFuncsArray[userFuncsArraySize-1] = sym;
    return (userFuncsArraySize-1);
}


// void Emit(instruction *instr) {
    
//     if (!instructions)
//         instructions = malloc(sizeof(instruction));

//     // int i = 0;
//     // for (; i<instructionsSize; i++) {
//     //     if (instructions[i] && strcmp(instructions[i]->id, sym->id) == 0)
//     //         return i;
//     // }
//     instructions = (instruction *)realloc(instructions, (++instructionsSize)*sizeof(instruction));
//     instructions[instructionsSize-1] = *instr;
//     currInstruction++; //??????
// }

void Emit( instruction* InstuctionPtr) {
		instruction* lala=NULL;
		//printf(" %d",lala->arg1->type);
    if (instructionsSize == 0) {
        int i=0;
        CurrentSize (instr);
        instruction* tmp = malloc(sizeof(instruction)*1024);
        instructions= tmp;
        instructionsSize = 1024;
        instruction* newInstr= InstuctionPtr + currInstruction;
        currInstruction++;
        newInstr->opcode= InstuctionPtr->opcode;
        newInstr->arg1= InstuctionPtr->arg1;
        newInstr->arg2= InstuctionPtr->arg2;
        newInstr->result= InstuctionPtr->result;
        newInstr->line = InstuctionPtr->line;
    } else {
        if (instructionsSize == currInstruction) {
            ExpandSize(instr);
            instruction* tmp = malloc (Current_size_of_Instruction + Expand_size_of_Instruction);
            memcpy ( tmp, instructions, currInstruction* sizeof(instruction));
            free( instructions);
            instructions= tmp;
            instructionsSize += 1024;
        }
        instruction* newInstr= instructions + currInstruction;
        currInstruction++;
        newInstr->opcode= InstuctionPtr->opcode;
        newInstr->arg1= InstuctionPtr->arg1;
        newInstr->arg2= InstuctionPtr->arg2;
        newInstr->result= InstuctionPtr->result;
        newInstr->line = InstuctionPtr->line;
    }

}

typedef void (generator_func_t)(quad *);


generator_func_t* generators[ ] = {
        
    generate_ASSIGN,
    generate_ADD,
    generate_SUB,
    generate_MUL,
    generate_DIV,
    generate_MOD,
    generate_UMINUS,
    generate_AND,
    generate_OR,
    generate_NOT,
    generate_IF_EQ,
    generate_IF_NOTEQ,
    generate_IF_LESSEQ,
    generate_IF_GREATEREQ,
    generate_IF_LESS,
    generate_IF_GREATER,
    generate_CALL,
    generate_PARAM,
    generate_RETURN,
    generate_GETRETVAL,
    generate_FUNCSTART,
    generate_FUNCEND,
    generate_NEWTABLE,
    generate_JUMP,
    generate_TABLEGETELEM,
    generate_TABLESETELEM,
    generate_NOP       
} ;


void generateALL(void){
    unsigned i;
    for(i=0 ;i<currQuad;i++)
        (*generators[quads[i].op])(quads+i);
}

unsigned int nextInstructionLabel()
{
    return currInstruction;
}

void patch_incomplete_jumps() {
    
    if (ij_head==NULL)
        return;

    incomplete_jump *tmp = ij_head;
    for (tmp; tmp->next != NULL; tmp=tmp->next) {
        if (tmp->iaddress == currQuad)
            instructions[tmp->instrNo].result->val= currInstruction;
        else
            instructions[tmp->instrNo].result->val= quads[tmp->iaddress].taddress;
    }
}


void make_operand(expr* e, vmarg* arg) {

	switch (e->type) {

        case var_e:
        case tableitem_e:
        case arithexpr_e:
        case boolexpr_e:
        //??????????
        case assignexpr_e:
        //???????
        case newtable_e: {

            assert(e->sym);
            arg->val = e->sym->offset;
            
            switch (e->sym->space) {
                case programvar:    arg->type = global_a;   break;
                case functionlocal: arg->type = local_a;    break;
                case formalarg:     arg->type = formal_a;   break;
                default: assert(0);
            }
            break; // from case newtable_e
        }
        case constbool_e:
        {
            arg->val = e->boolConst;
            arg->type = bool_a;
            break;
        }
        case conststring_e:
        {
            arg->val = consts_newstring(e->strConst);
    		arg->type = string_a;
            break;
        }
        case constnum_e:
        {
            arg->val = consts_newnumber(e->numConst);
            arg->type = number_a;
            break;

        }
        case nil_e:
        {
            arg->type = nil_a;
            break;
        }

        case programfunc_e: 
        {
            arg->type = userfunc_a;
            avm_userfunc *userFunc = createUserFunc(e);
            arg->val = userfuncs_newfunc(userFunc);
            //arg->val = e->sym->iaddress;
            break;

        }
        case libraryfunc_e: 
        {
            arg->type = libfunc_a;
            arg->val = libfuncs_newused(e->sym->name);
            break;
        }
        default: assert(0);         
    }     
}

void make_numberoperand(vmarg* arg, double val) {
    
    if (arg == NULL ) 
        arg = malloc(sizeof(vmarg));
    arg->val = consts_newnumber(val);
    arg->type = number_a;
}

void make_booloperand(vmarg* arg, unsigned val) {
    
    if (arg == NULL)
        arg = malloc(sizeof(vmarg));
    arg->val =  val;
    arg->type = bool_a;
}

vmarg* make_retvaloperand(vmarg* arg){
    if (arg == NULL){
            arg = malloc(sizeof(vmarg));}
    arg->type =  retval_a;
    return arg;
}


void add_incomplete_jump(unsigned instrNo, unsigned iaddress) {

    incomplete_jump* inst = malloc(sizeof(incomplete_jump));
    inst->instrNo = instrNo;
    inst->iaddress = iaddress;
    inst->next = ij_head;
    ij_head = inst;
    ij_total++;
}

void generate(vmopcode  op, quad *quad){


	instruction* inst =(instruction*) malloc(sizeof(instruction));
    expr* tmp=(expr*)malloc(sizeof(expr));
	
    inst->opcode = op;
    inst->arg1 =(vmarg*) malloc(sizeof(vmarg));
    inst->arg2 =(vmarg*) malloc(sizeof(vmarg));
    inst->result =(vmarg*) malloc(sizeof(vmarg));

   	tmp=quad->arg1;
   	printf ("\nTHE LINE IM PARSING is %d",quad->line);

    printf("\nSTELNWWWW: %d----%d\n", quad->arg1->type, inst->arg1->type);
    printf("\nTHE NAME OF THE arg1 inserting is %s with %lf",quad->arg1->sym->name, quad->arg1->numConst);
    if( quad->arg2 != NULL)
    {
    	printf("\nTHE NAME OF THE arg2 inserting is %s",quad->arg2->sym->name);
    }
    printf("\nTHE NAME OF THE arg3 inserting is %s",quad->result->sym->name);
    make_operand(quad->arg1, inst->arg1);
    if( quad->arg2 != NULL)
    {
    	make_operand(quad->arg2, inst->arg2);
    }
    make_operand(quad->result, inst->result);
    //if( inst->arg1->type == conststring_e  )
    //{
    	inst->typeflag= 0; //string;
    //}
    //else if( inst->arg1->type ==  constnum_e)
    //{
    	inst->typeflag= 1; //integer
    //}

    //if( inst->arg2->type == conststring_e  )
    //{
    	inst->typeflag= 0; //string;
    //}
    //else if( inst->arg2->type ==  constnum_e)
    //{
    	inst->typeflag= 1; //integer
    //}
    quad->taddress = nextInstructionLabel();
    Emit(inst);
    free(inst);
}

void generate_relational(vmopcode op, quad *quad){
       
    instruction *inst = malloc(sizeof(instruction));
    inst->arg1 = malloc(sizeof(vmarg));
    inst->arg2 = malloc(sizeof(vmarg));
    inst->result = malloc(sizeof(vmarg));
    inst->opcode = op;
    inst->line = quad->line;      
    inst->result->type = label_a;

    make_operand(quad->arg1, inst->arg1);
    make_operand(quad->arg2, inst->arg2);
    

    if(quad->label<currprocessedquad())
        inst->result->val = quads[quad->label].taddress;
    else
        add_incomplete_jump(nextInstructionLabel(),quad->label);

    quad->taddress = nextInstructionLabel();
    Emit(inst);    
}

void generate_ADD (quad* quad)
{
	generate(add_v, quad);
}

void generate_SUB (quad* quad)
{
	generate(sub_v, quad);
}

void generate_MUL (quad* quad)
{
	generate(mul_v, quad);
}

void generate_DIV (quad* quad)
{
	generate(div_v, quad);
}

void generate_MOD (quad* quad)
{
	generate(mod_v, quad);
}

void generate_NEWTABLE (quad* quad)
{
	generate(newtable_v, quad);
}

void generate_TABLEGETELEM (quad* quad)
{
	generate(tablegetelem_v, quad);
}

void generate_TABLESETELEM (quad* quad)
{
	generate(tablesetelem_v, quad);
}

void generate_ASSIGN (quad* quad)
{
	
	generate(assign_v, quad);
}

void generate_NOP (quad* quad)
{
    instruction *t = malloc(sizeof(instruction));
    t->opcode = nop_v;
    Emit(t);
}

void generate_UMINUS( quad* quad)
{
    quad->arg2= newexpr_constnum ((-1));
    generate(mul_v,quad);

}

void generate_JUMP (quad* quad)
{
    quad->arg1 = newexpr_constbool(true);
    quad->arg2 = newexpr_constbool(true);
    generate_relational(jeq_v , quad);
}
void generate_IF_EQ (quad* quad)
{
    generate_relational(jeq_v , quad);
}

void generate_IF_NOTEQ (quad* quad)
{
    generate_relational(jne_v , quad);
}
void generate_IF_GREATER (quad* quad)
{
    generate_relational(jgt_v , quad);
}

void generate_IF_GREATEREQ (quad* quad)
{
    generate_relational(jge_v , quad);
}


void generate_IF_LESS (quad* quad)
{
    generate_relational(jlt_v , quad);
}

void generate_IF_LESSEQ (quad* quad)
{
    generate_relational(jle_v , quad);
}


void generate_NOT (quad* quad) {

    quad->taddress = nextInstructionLabel();
    instruction *inst = malloc(sizeof(instruction));

    inst->opcode = jeq_v;
    make_operand(quad->arg1, inst->arg1);
    make_booloperand(inst->arg2, false);
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+3;
    Emit(inst);

    inst->opcode = assign_v;
    make_booloperand(inst->arg1, false);
    reset_operand(inst->arg2);
    make_operand(quad->result, inst->result);
    Emit(inst);


    inst->opcode = jeq_v;
    reset_operand(inst->arg1);    
    reset_operand(inst->arg2);
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+2;
    Emit(inst);


    inst->opcode = assign_v;
    make_booloperand(inst->arg1, true);
    reset_operand(inst->arg2);
    make_operand(quad->result, inst->result);
    Emit(inst);
}

void generate_OR (quad* quad) {

    quad->taddress = nextInstructionLabel();
    instruction *inst = malloc(sizeof(instruction));

    inst->opcode = jeq_v;
    make_operand(quad->arg1, inst->arg1);
    make_booloperand(inst->arg2, true);
    inst->result = malloc(sizeof(vmarg));
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+4;
    Emit(inst);

    make_operand(quad->arg2, inst->arg1);
    inst->result->val = nextInstructionLabel()+3;
    Emit(inst);


    inst->opcode = assign_v;
    make_booloperand(inst->arg1, false);
    reset_operand(inst->arg2);
    make_operand(quad->result, inst->result);
    Emit(inst);


    inst->opcode = jeq_v;
    reset_operand(inst->arg1);    
    reset_operand(inst->arg2);
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+2;
    Emit(inst);


    inst->opcode = assign_v;
    make_booloperand(inst->arg1, true);
    reset_operand(inst->arg2);
    make_operand(quad->result, inst->result);
    Emit(inst);
}


void generate_AND (quad* quad) {

    quad->taddress = nextInstructionLabel();
    instruction *inst = malloc(sizeof(instruction));
    inst->arg1 = malloc(sizeof(vmarg));
    inst->arg2 = malloc(sizeof(vmarg));
    inst->result = malloc(sizeof(vmarg));
    inst->opcode = jeq_v;

    make_operand(quad->arg1, inst->arg1);
    make_booloperand(inst->arg2, false);
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+4;
    Emit(inst);

    make_operand(quad->arg1, inst->arg2);
    inst->result->val = nextInstructionLabel()+3;
    Emit(inst);


    inst->opcode = assign_v;
    make_booloperand(inst->arg1, true);
    reset_operand(inst->arg2);
    
    (quad->result, inst->result);
    Emit(inst);


    inst->opcode = jeq_v;
    reset_operand(inst->arg1);    
    reset_operand(inst->arg2);
    inst->result->type = label_a;
    inst->result->val = nextInstructionLabel()+2;
    Emit(inst);


    inst->opcode = assign_v;
    make_booloperand(inst->arg1, false);
    reset_operand(inst->arg2);
    make_operand(quad->result, inst->result);
    Emit(inst);
}

void generate_PARAM (quad* quad) {

    quad->taddress= nextInstructionLabel();
    instruction* instr = malloc(sizeof(instruction));
    instr->opcode = pusharg_v;
    instr->arg1 = malloc(sizeof(vmarg));
    make_operand(quad->result, instr->arg1);
    Emit(instr);
    
}

void generate_CALL (quad* quad){

    quad->taddress= nextInstructionLabel();
    instruction* inst= malloc(sizeof(instruction));
    inst->opcode = call_v;
    inst->arg1=(vmarg*)malloc(sizeof(vmarg));
    make_operand(quad->arg1, inst->arg1);
    Emit(inst);
}

void generate_GETRETVAL (quad* quad)
{
    quad->taddress = nextInstructionLabel();
    instruction* inst = malloc(sizeof(instruction));
    inst->opcode = assign_v;
    inst->result = malloc(sizeof(vmarg));
    make_operand(quad->arg1, inst->result);
    //////////
    inst->arg1 = make_retvaloperand(inst->arg1);
    /////
    Emit(inst);
}

void generate_FUNCSTART (quad* quad)
{
    SymbolTableEntry* entry = malloc(sizeof(SymbolTableEntry));
    instruction* inst = malloc (sizeof(instruction));
    entry = quad->result->sym;
    entry->iaddress = nextInstructionLabel();
    quad->taddress = nextInstructionLabel();
    // TODO push(funcstack, entry) 27/36
    inst->opcode = funcenter_v;
    make_operand(quad->result, inst->result);
    Emit(inst);
}

void generate_RETURN (quad* quad)
{
    quad->taddress = nextInstructionLabel();
    instruction* inst = malloc(sizeof(instruction));
    inst->opcode = assign_v;
    inst->arg1 = malloc(sizeof(vmarg));
    inst->arg2 = malloc(sizeof(vmarg));
    inst->result = malloc(sizeof(vmarg));
    inst->result = make_retvaloperand(inst->result);
    make_operand(quad->arg1, inst->arg1);
    Emit(inst);
    // TODO  top(funcstack) 27/36
    inst->opcode = jeq_v;
    reset_operand(inst->arg1);
    reset_operand(inst->arg2);
    inst->result->type = label_a;
    Emit(inst);
}

void generate_FUNCEND (quad* quad)
{
    // TODO  pop(funcstack) 27/36
    quad->taddress = nextInstructionLabel();
    instruction* inst = malloc(sizeof(instruction));
    inst->opcode = funcexit_v;
    inst->result = malloc(sizeof(vmarg));
    make_operand(quad->result, inst->result);
    Emit(inst);
}

void createBinaryFile() {
    FILE *fp = fopen("finalCode.abc","wb");

    unsigned magicNumber = 340200501;
    fwrite(&magicNumber, sizeof(unsigned), 1, fp);

    fwrite(&programVarOffset, sizeof(unsigned), 1,fp);
    
    fwrite(&strArraySize, sizeof(unsigned), 1, fp);

    unsigned int i, size;
    for (i=0; i<strArraySize; ++i) {
        size = (strlen(strArray[i]))+1;
        fwrite(&size, sizeof(unsigned int), 1, fp);
        fwrite(strArray[i], sizeof(char), size, fp);
    }

    fwrite(&numArraySize, sizeof(unsigned), 1, fp);
    for (i=0; i<numArraySize; ++i)
        fwrite(&numArray[i], sizeof(double), 1, fp);

    avm_userfunc *func = NULL;

    fwrite(&userFuncsArraySize, sizeof(unsigned), 1, fp);
    for (i=0; i<userFuncsArraySize; ++i) {
        func = userFuncsArray[i];
        fwrite(&(func->taddress), sizeof(unsigned int), 1, fp);
        fwrite(&(func->totallocals), sizeof(unsigned int), 1, fp);
        size = (strlen(func->id))+1;
        fwrite(&size, sizeof(unsigned int), 1 , fp);
        fwrite(func->id, sizeof(char), size, fp);
    }

    fwrite(&libFuncsArraySize, sizeof(unsigned), 1,fp);
    for (i=0; i<libFuncsArraySize; ++i) {
        size = (strlen(libFuncsArray[i]))+1;
        fwrite(&size, sizeof(unsigned int), 1 , fp);
        fwrite(libFuncsArray[i], sizeof(char), size, fp);
    }
    ////ALLAGHHHHHHHHHH v
    size = currInstruction-1;
    fwrite(&size, sizeof(unsigned), 1,fp);

    vmarg* NilArgument = malloc(sizeof(vmarg));
    NilArgument->type = nil_a;
    instruction* insT = NULL;

    for (i=1; i<currInstruction; i++) {

        insT = &instructions[i];

        if (insT == NULL)
            continue;

        fwrite(&insT->opcode , sizeof(int), 1, fp);
        printf(">>>>>>>>%s\n", getVmOpcode(insT->opcode));
        if (insT->arg1 == NULL) {
            fwrite(&NilArgument->type, sizeof(int), 1, fp);
            fwrite(&NilArgument->type, sizeof(unsigned), 1, fp);
        } else {
        	printf( "\n the instruction ====ARG1 is %s",getVmargType(instructions[i].arg1->type));
            fwrite(&instructions[i].arg1->type, sizeof(int), 1, fp);
            fwrite(&instructions[i].arg1->val, sizeof(unsigned), 1, fp);
        }
        if (instructions[i].arg2 == NULL) {
            fwrite(&NilArgument->type, sizeof(int), 1, fp);
            fwrite(&NilArgument->type, sizeof(unsigned), 1, fp);
        } else {
        	printf( "\n the instruction ** **ARG2 is %s",getVmargType(instructions[i].arg2->type));
            fwrite(&instructions[i].arg2->type, sizeof(int), 1, fp);
            fwrite(&instructions[i].arg2->val, sizeof(unsigned), 1, fp);
        }

        if (instructions[i].result == NULL) {
            fwrite(&NilArgument->type, sizeof(int), 1, fp);
            fwrite(&NilArgument->type, sizeof(unsigned), 1, fp);
        } else {
        	printf( "\n the instruction $$$$$ RESULT is %s",getVmargType(instructions[i].result->type));
            fwrite(&instructions[i].result->type, sizeof(int), 1, fp);
            fwrite(&instructions[i].result->val, sizeof(unsigned), 1, fp);
        }
        fwrite(&instructions[i].line, sizeof(unsigned), 1, fp);
    }
}

char* getVmOpcode(vmopcode opcode) {

    switch (opcode) {
        case assign_v:      return "assign";        break;
        case add_v:         return "add";           break;
        case sub_v:         return "sub";           break;
        case mul_v:         return "mul";           break;
        case div_v:         return "div";           break;
        case mod_v:         return "mod";           break;
        case uminus_v:      return "uminus";        break;
        case and_v:         return "and";           break;
        case not_v:         return "not";           break;
        case jeq_v:         return "jeq";           break;
        case or_v:          return "or";            break;
        case jne_v:         return "jne";           break;
        case jle_v:         return "jle";           break;
        case jge_v:         return "jge";           break;
        case jlt_v:         return "jlt";           break;
        case jgt_v:         return "jgt";           break;
        case call_v:        return "call";          break;
        case pusharg_v:     return "pusharg";       break;
        case funcenter_v:   return "funcenter";     break;
        case funcexit_v:    return "funcexit";      break;
        case newtable_v:    return "newtable";      break;
        case tablegetelem_v:return "tablegetelem";  break;
        case tablesetelem_v:return "tablesetelem";  break;
        case nop_v:         return "nop";           break;
        default:            return "";              break;
    }   
}

char* getVmargType(vmarg_t type) {

    switch (type) {
        case label_a:   return "label";     break;
        case global_a:  return "global";    break;
        case formal_a:  return "formal";    break;
        case local_a:   return "local";     break;
        case number_a:  return "number";    break;
        case string_a:  return "string";    break;
        case bool_a:    return "bool";      break;
        case nil_a:     return "nil";       break;
        case userfunc_a:return "userfunc";  break;
        case libfunc_a: return "libfunc";   break;
        case retval_a:  return "retval";    break;
        default:        return "";          break;
    }
}

void printfReadbleInstructions() {

    FILE *fp = fopen("ReadableFinal.txt", "w+");
    int i = 0;
    
    unsigned magicNumber = 340200501;
    fprintf(fp, "magic number: %u\n", magicNumber);

    fprintf(fp, "program vars: %u\n", programVarOffset);

    fprintf(fp, "total strings: %u\n", strArraySize);
    unsigned int size;
    for (i=0; i<strArraySize; i++)
        fprintf(fp,"strings[%d]: %s (size: %zu)\n", i, strArray[i], strlen(strArray[i])+1);

    fprintf(fp,"total numbers: %u\n", numArraySize);
    for (i=0; i<numArraySize; ++i)
       fprintf(fp,"numbers[%d]: %lf\n", i, numArray[i]);


    avm_userfunc *tmpFunc= NULL;
    fprintf(fp,"total userfuncs: %u\n", userFuncsArraySize);
    for (i=0; i<userFuncsArraySize; ++i) {
        tmpFunc = userFuncsArray[i];
        fprintf(fp,"userfuncs[%d]: %s (size: %zu, addr: %u, locals: %u)\n", i, tmpFunc->id, strlen(tmpFunc->id)+1, tmpFunc->taddress, tmpFunc->totallocals);
    }

    fprintf(fp,"total libfuncs: %u\n", libFuncsArraySize);
    for (i=0; i<libFuncsArraySize; ++i)
        fprintf(fp,"libfuncs[%d]: %s (size: %zu)\n", i, libFuncsArray[i], strlen(libFuncsArray[i])+1);

    fprintf(fp, "\n%-4s%-15s%-20s%-20s%-20s\n\n", "#","OP", "ARG1", "ARG2", "RESULT");

    for (i=1; i<currInstruction; i++) {

        fprintf(fp,"%-4d", i);      
        fprintf(fp, "%-15s", getVmOpcode(instructions[i].opcode));

        if (instructions[i].arg1 != NULL) {
            fprintf(fp, "%d(%s), ", instructions[i].arg1->type, getVmargType(instructions[i].arg1->type));
        	switch (instructions[i].arg1->type) {
        		case number_a:
        		{
        			fprintf(fp, "%u:%lf", instructions[i].arg1->val, numArray[instructions[i].arg1->val]);
        			break;
        		}
        		case string_a:
        		{
        			fprintf(fp, "%u:%s", instructions[i].arg1->val, strArray[instructions[i].arg1->val]);
        			break;
        		}
        		case userfunc_a:
        		{
                    int j, tme = 0;
                    for (; j<userFuncsArraySize; ++j) {
        				if (instructions[i].arg1->val == j) {	
        					fprintf(fp, "%u:%s", j, userFuncsArray[j]->id);
        					break;
        				}
        			}
        		}
        		case libfunc_a:
        		{
        			fprintf(fp, "%u:%s", instructions[i].arg1->val, libFuncsArray[instructions[i].arg1->val]);
        			break;
        		}
        		default:
                {
                    //8ELEI KOITAGMAAAA - ALLAGH
                    //
                    fprintf(fp, "%u", instructions[i].arg1->val);
                    /*if( instructions[i].typeflag ==0 ) //string
                	{
                		fprintf(fp, "%u:%s", instructions[i].arg1->val, strArray[instructions[i].arg1->val]);
                	}
                	else if( instructions[i].typeflag ==1 )
                	{
                		fprintf(fp, "%u:%lf", instructions[i].arg1->val, numArray[instructions[i].arg1->val]);
                	}
*/
                	
        			break;
                }
        	}

        } 
        else fprintf(fp, "NULL");

        fprintf(fp, "\t\t|");

        if (instructions[i].arg2 != NULL) {
            fprintf(fp, "%d(%s), ", instructions[i].arg2->type, getVmargType(instructions[i].arg2->type));
            switch (instructions[i].arg2->type){
                case number_a:
                {
                    fprintf(fp, "%u:%lf", instructions[i].arg2->val, numArray[instructions[i].arg2->val]);
                    break;
                }
                case string_a:
                {
                    fprintf(fp, "%u:%s", instructions[i].arg2->val, strArray[instructions[i].arg2->val]);
                    break;
                }
                case userfunc_a:
                {
                    int j, tme = 0;
                    for (; j<userFuncsArraySize; ++j) {
                        if (instructions[i].arg2->val == j) {   
                            fprintf(fp, "%u:%s", j, userFuncsArray[j]->id);
                            break;
                        }
                    }
                }
                case libfunc_a:
                {
                    fprintf(fp, "%u:%s", instructions[i].arg2->val, libFuncsArray[instructions[i].arg2->val]);
                    break;
                }
                default:
                {
                    //8ELEI KOITAGMAAAA - ALLAGH
                    //fprintf(fp, "%u:%lf", instructions[i].arg2->val, numArray[instructions[i].arg2->val]);
//                    fprintf(fp,"DEFAULT ARG2");
                        fprintf(fp, "%u", instructions[i].arg2->val);
                	/*if( instructions[i].typeflag ==0 ) //string
                	{
						fprintf(fp, "%u:%s", instructions[i].arg2->val, strArray[instructions[i].arg2->val]);
                	}
                	else if( instructions[i].typeflag ==1 )
                	{	
                		fprintf(fp, "%u:%lf", instructions[i].arg2->val, numArray[instructions[i].arg2->val]);
                	}
                    */break;
                }
            }
        }
        else fprintf(fp, "NULL");

        fprintf(fp, "\t\t|");

        if (instructions[i].result != NULL) {

            fprintf(fp, "%d(%s), ", instructions[i].result->type, getVmargType(instructions[i].result->type));
            switch (instructions[i].result->type){
                case number_a:
                {
                    fprintf(fp, "%u:%lf", instructions[i].result->val, numArray[instructions[i].result->val]);
                    break;
                }
                case string_a:
                {
                    fprintf(fp, "%u:\"%s\"", instructions[i].result->val, strArray[instructions[i].result->val]);
                    break;
                }
                case userfunc_a:
                {
                    int j, tme = 0;
                    for (; j<userFuncsArraySize; ++j) {
                        if (instructions[i].result->val == j) {   
                            fprintf(fp, "%u:\"%s\"", j, userFuncsArray[j]->id);
                            break;
                        }
                    }
                }
                case libfunc_a:
                {
                    fprintf(fp, "%u:%s", instructions[i].result->val, libFuncsArray[instructions[i].result->val]);
                    break;
                }
                default:
                {
                	//fprintf(fp,"DEFAULT RESULT");
                        fprintf(fp, "%u", instructions[i].result->val);	
                    //fprintf(fp, "%d", instructions[i].result->val);
                    break;
                }
            }
        }
        else fprintf(fp, "NULL");

        fprintf(fp, "\n");
    }
    fclose(fp);
}
