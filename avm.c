#include "avm.h"
///////////////////////////////////
//ex 
avm_memcell ax, bx, cx;
//ex 
avm_memcell retval;
//ex 
unsigned char executionFinished = 0;
//ex
unsigned pc = 0;
//ex lecture 15 slide 14/36
unsigned currLine = 0;
unsigned codeSize = 0;
//ex 
instruction* code =  (instruction*) 0;

//ex
unsigned tp,topsp;

extern FILE* fp;

//ex
static void avm_initstack(void){
	unsigned i;
	for(i = 0;i<AVM_STACKSIZE;++i)
	{
		AVM_WIPEOUT(stack[i]); 
		stack[i].type = undef_m;
	}
	ax = stack[AVM_STACKSIZE-1];
	bx= cx = retval = ax;
}

//ex
void avm_initialize(void)
{
	avm_initstack();
	
	avm_registerlibfunc("print", libfunc_print);
	
	avm_registerlibfunc("typeof", libfunc_typeof);
	
	
}

//ex
void avm_tabledestroy(avm_table* t)
{
	avm_tablebucketsdestroy(t->strIndexed);
	avm_tablebucketsdestroy(t->numIndexed);
	free(t);
}

//ex
void avm_tablebucketsdestroy(avm_table_bucket** p){
	unsigned i;
	for(i =0; i< AVM_TABLE_HASHSIZE;++i, ++p){
		avm_table_bucket* b = *p;
		for(; b!=NULL; b= b->next){

			avm_table_bucket* del = b;
			
			avm_memcellclear(&del->key);
			avm_memcellclear(&del->value);
			free(del);
			}
			p[i] = (avm_table_bucket* ) 0;
		}
	
}

//ex
void avm_tablebucketsinit(avm_table_bucket** p){
	unsigned i;
	for(i =0;i< AVM_TABLE_HASHSIZE;++i)
		p[i] = (avm_table_bucket* ) 0;
	
}

//ex
avm_table* avm_tablenew(void){
		avm_table* t = (avm_table*)malloc(sizeof(avm_table));
		AVM_WIPEOUT(*t);
		
		t->refCounter =  t->total = 0;
		
		avm_tablebucketsinit(t->numIndexed);
		avm_tablebucketsinit(t->strIndexed);
		return t; 
}


//ex
void avm_tableincrefcounter(avm_table* t){
	++t->refCounter;
	
}

//ex
void avm_tabledecrefcounter(avm_table* t){
	assert(t->refCounter >0);
	if(!--t->refCounter)
		avm_tabledestroy(t);
}

//ex in lecture 15, slide 8/36
avm_memcell* avm_translate_operand(vmarg* arg, avm_memcell* reg){
	printf("********************%d\n", arg->type);
	switch(arg->type){

		case global_a:
			return &stack[AVM_STACKSIZE-1-arg->val];
		
		case local_a: 
			return &stack[topsp-arg->val];
		
		case formal_a:
			return &stack[topsp+AVM_STACKENV_SIZE+1+arg->val];
		
		case retval_a:
			return &retval;

		case number_a: {
			reg->type = number_m;
			reg->data.numVal = consts_getnumber(arg->val);
			return reg;
		}
	
		case string_a: {
			reg->type = string_m;
			reg->data.strVal =  strdup(consts_getstring(arg->val));
			return reg;
		}

		case bool_a: {
			reg->type = bool_m;
			reg->data.boolVal =  arg->val;
			return reg;
		}

		case nil_a: {
			reg = (avm_memcell*)malloc(sizeof(avm_memcell));
			reg->type =  nil_m;
			return reg;
		}

		case userfunc_a: {
			reg->type =  userfunc_m;
			reg->data.funcVal = arg->val;
			return reg;
		}

		case libfunc_a: {
			reg->type =  libfunc_m;
			reg->data.libfuncVal =  libfuncs_getused(arg->val);
			return reg;

		}

		default: assert(0);
	}

}

void execute_and(instruction* i){ }

void execute_or(instruction* i){ }

void execute_not(instruction* i){ }

void execute_nop(instruction *i){ }

//ex 
void execute_cycle(void){

	if (executionFinished)
		return;
	else{

		if (pc == AVM_ENDING_PC) {
			executionFinished = 1;
			return;

		}else{
			assert(pc < AVM_ENDING_PC);
			instruction* instr = code +pc;
			assert(instr->opcode >= 0 && instr->opcode <= AVM_MAX_INSTRUCTIONS);

			if(instr->line)
				currLine =  instr->line;
			unsigned oldPC = pc;
			(*executeFuncs[instr->opcode])(instr);

			if(pc == oldPC)
				++pc;
		}
	}
}

//pseudocode
void avm_memcellclear(avm_memcell* m){
	if(m->type !=  undef_m){
		memclear_func_t f = memclearFuncs[m->type];
		if(f)
			(*f)(m);
		m->type = undef_m;
	}
	
}

//ex
void memclear_string(avm_memcell* m){
	assert(m->data.strVal);
	free(m->data.strVal);
}

//ex
void memclear_table(avm_memcell* m){
	assert(m->data.tableVal);
	avm_tabledecrefcounter(m->data.tableVal);
}


char *avm_tostring(avm_memcell *m)
{
	assert(m->type>=0 && m->type <=undef_m);
	return (*tostringFuncs[m->type])(m);
}

//ex
void execute_assign(instruction* instr) {

	avm_memcell* lv = avm_translate_operand(instr->result, (avm_memcell*)0);
	avm_memcell* rv = avm_translate_operand(instr->arg1, &ax);

	assert(lv && ( (&stack[AVM_STACKSIZE-1] >= lv) && (lv > &stack[tp]) || (lv = &retval)));
	assert(rv);
	avm_assign(lv,rv);
	printf(">>>>>>>>>>>>>>>%lf\n", lv->data.numVal);
}

//ex in lecture 15 slide 18/36
void avm_assign(avm_memcell* lv, avm_memcell* rv) {
	
	if(lv == rv)
		return;
	if(lv->type == table_m && rv->type == table_m && lv->data.tableVal == rv->data.tableVal)
		return;
	if(rv->type == undef_m)
		{	
			fprintf(stdout,"assigning from 'undef' content! ");}
	
	avm_memcellclear(lv);
	memcpy(lv,rv,sizeof(avm_memcell));
	
	if(lv->type ==  string_m || lv->type == table_m) {
		switch (lv->type) {
			case string_m: {
				lv->data.strVal = strdup(rv->data.strVal);
				break;
			}
			case table_m: {
				avm_tabledecrefcounter(lv->data.tableVal);
				break;
			}
			default: {return;}
		}
	}
}

//ex
void execute_call(instruction* instr){

	avm_memcell* func =  avm_translate_operand(instr->result, &ax);
	assert(func);
	avm_callsaveenviroment();
	
	switch(func->type){
		case userfunc_m: {
			pc =  func->data.funcVal;
			assert(pc < AVM_ENDING_PC);
			assert(code[pc].opcode == funcenter_v);
			break;
		}

		case string_m: {
			avm_calllibfunc(func->data.strVal);
			break;
		}

		case libfunc_m: {
			avm_calllibfunc(func->data.libfuncVal);
			break;
		}

		default: {
			char* s = avm_tostring(func);
			fprintf(stdout,"call: cannot bind '%s' to function!", s);
			free(s);
			executionFinished = 1;
		}
	}
}

//ex in 15 lecture, slide 20/36
unsigned totalActuals = 0;

//ex
void avm_dec_top(void){
	if(!tp)
	{
		fprintf(stdout,"stack overflow");
		executionFinished =  1;
	}
	else
		--tp;
	
}

//ex
void avm_push_envvalue(unsigned val){
	stack[tp].type  = number_m;
	stack[tp].data.numVal = val;
	avm_dec_top();
}

//ex
void avm_callsaveenviroment(void)
{
	avm_push_envvalue(totalActuals);
	avm_push_envvalue(pc+1);
	avm_push_envvalue(tp + totalActuals + 2);
	avm_push_envvalue(topsp);
}


//ex
void execute_funcenter(instruction* instr)
{
	avm_memcell* func =  avm_translate_operand(instr->result, &ax);
	assert(func);
	assert(pc == func->data.funcVal);
	
	totalActuals = 0;
	
	avm_userfunc* funcInfo =  avm_getfuncinfo(pc);
	
	topsp =  tp;
	
	tp =  tp - funcInfo->totallocals;
}


//ex
unsigned avm_get_envvalue(unsigned i)
{
	assert(stack[i].type =  number_m);
	unsigned val =  (unsigned)stack[i].data.numVal;
	assert(stack[i].data.numVal == ((double) val));
	return val;
	
}


//ex
void execute_funcexit(instruction* unused){
	unsigned oldTop =  tp;
	tp =  avm_get_envvalue(topsp + AVM_SAVEDTOP_OFFSET);
	pc = avm_get_envvalue(topsp + AVM_SAVEDPC_OFFSET);
	topsp = avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
	
	while(oldTop++ <= tp)
	{
		avm_memcellclear(&stack[oldTop]);
	}
}

//ex
void avm_calllibfunc(char* id)
{
	library_func_t f =  avm_getlibraryfunc(id);
	
	if(!f){
		fprintf(stdout,"Unsupported lib func '%s' called!", id);
		executionFinished = 1;
	}
	else{
		topsp =  tp;
		totalActuals =  0;
		
		(*f)();
		
		if(!executionFinished)
		{
			execute_funcexit((instruction*) 0);
		}
	}
}

//ex
unsigned avm_totalactuals(void)
{
	return avm_get_envvalue(topsp + AVM_NUMACTUALS_OFFSET);
}

//ex
avm_memcell* avm_getactual(unsigned i)
{
	assert(i < avm_totalactuals());
	return &stack[topsp + AVM_STACKSIZE +1 +i];
}

//ex
void libfunc_print(void){

	unsigned n = avm_totalactuals();
	unsigned i;
	for(i=0; i<n; i++) {
		char* s = avm_tostring(avm_getactual(i));
		puts(s);
		free(s);
	}
}


//ex
void execute_pusharg(instruction* instr)
{
	avm_memcell* arg =  avm_translate_operand(instr-> arg1, &ax);
	assert(arg);
	
	avm_assign(&stack[tp], arg);
	++totalActuals;
	avm_dec_top();
}




//ex
void execute_arithmetic(instruction* instr)
{
	avm_memcell* lv = avm_translate_operand(instr->result, (avm_memcell*) 0);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);

	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[tp] || lv == &retval));
	assert(rv1 && rv2);
	
	if(rv1->type != number_m || rv2->type != number_m)
	{
		fprintf(stdout,"not a number in arithmetic!");
		executionFinished =  1;
	}
	
	else
	{
		arithmetic_func_t op =  arithmeticFuncs[instr->opcode - add_v];
		avm_memcellclear(lv);
		lv->type =  number_m;
		lv->data.numVal =  (*op)(rv1->data.numVal, rv2->data.numVal);
	}
}


//ex
unsigned char avm_tobool(avm_memcell* m)
{
	assert(m->type >= 0 && m->type < undef_m);
	return (*toboolFuncs[m->type])(m);
}



//ex except switch lecture 15 slide 29/36
void execute_jeq (instruction* instr) {

	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type ==  undef_m || rv2->type == undef_m)
		fprintf(stdout,"'undef' involved in equality!");
	else {

		if(rv1->type ==  nil_m || rv2->type == nil_m)
			result = rv1->type ==  nil_m && rv2->type == nil_m;
		else {
			if(rv1->type ==  bool_m || rv2->type == bool_m)
				result = (avm_tobool(rv1) == avm_tobool(rv2));
			else {
				if(rv1->type !=  rv2->type)
					fprintf(stdout,"%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv2->type]);
				else {
					switch (rv1->type) {
						case string_m: {
							result = (strcmp(rv1->data.strVal, rv2->data.strVal) == 0);
							break;
						}
						case number_m: {
							result = (rv1->data.numVal == rv2->data.numVal);
							break;
						}
						case table_m: {
							result = (rv1->data.numVal == rv2->data.numVal);
							break;
						}
						default: { result = 0; }
					}
				}
			}
		}
	}
	if(!executionFinished && result)
		pc =  instr->result->val;
}


//ex
void Libfunc_TypeOf(void)
{
	 unsigned n = avm_totalactuals();        

	 if(n!=1)
			printf("The expected argument found \"typeof\"at line:%d \n", currLine);
	 if(n == 1){
			avm_memcellclear(&retval);
			retval.type = string_m;
			char* r = typeStrings[avm_getactual(0)->type];
			retval.data.strVal = strdup(r);
	 }
}


//ex
void execute_newtable(instruction* instr)
{
	avm_memcell* lv =  avm_translate_operand(instr->result, (avm_memcell*) 0);
	
	 assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[tp] || lv == &retval));
	avm_memcellclear(lv);
	
	lv->type = table_m;
	lv->data.tableVal =  avm_tablenew();
	avm_tableincrefcounter(lv->data.tableVal);
}

//ex
void execute_tablegetelem(instruction* instr)
{
	avm_memcell* lv = avm_translate_operand(instr->result, (avm_memcell*) 0);
	avm_memcell* t =  avm_translate_operand(instr->arg1, (avm_memcell*) 0);
	avm_memcell* i = avm_translate_operand(instr->arg2, &ax);
	
	assert(lv && (&stack[AVM_STACKSIZE-1] >= lv && lv > &stack[tp] || lv ==  &retval));
	assert(t && &stack[AVM_STACKSIZE-1] >= t && t> &stack[tp]);
	assert(i);
	
	avm_memcellclear(lv);
	lv->type = nil_m;
	
	if(t->type != table_m)
	{
		fprintf(stdout,"illegal use of type %s as table!", typeStrings[t->type]);
	}
	else{
		avm_memcell* content =  avm_tablegetelem(t->data.tableVal, i);
		
		if(content)
			avm_assign(lv, content);
		else
		{
			char* ts =  avm_tostring(t);
			char* is =  avm_tostring(i);
			
			fprintf(stdout,"%s[%s] not found !", ts, is);
			
			free(ts);
			free(is);
		}
	}
}

//ex
void execute_tablesetelem(instruction* instr)
{
	avm_memcell* t = avm_translate_operand(instr->result, (avm_memcell*) 0);
	avm_memcell* i = avm_translate_operand(instr->arg1, &ax);
	avm_memcell* c = avm_translate_operand(instr->arg2, &bx);
	
	assert(t && &stack[AVM_STACKSIZE-1] >= t && t > &stack[tp]);
	assert(i && c);
	
	if(t->type != table_m)
		fprintf(stdout,"illegal use of type %s as table!!!!", typeStrings[t->type]);
	else
		avm_tablesetelem(t->data.tableVal, i,c);
}


void libfunc_typeof(void)
{
	unsigned n = avm_totalactuals();
	
	if(n!= 1)
		fprintf(stdout,"one argument (not %d) expected in 'typeof'  !", n);
	else
	{
		avm_memcellclear(&retval);
		retval.type = string_m;
		retval.data.strVal = strdup(typeStrings[avm_getactual (0)->type]);
	}
}

//ex alll
double add_impl(double x, double y){return x+y;}//finished
double sub_impl(double x, double y){return x-y;}//finished
double mul_impl(double x, double y){return x*y;}//finished

double div_impl(double x, double y)
{
	assert(y);
	return x/y;
}

double mod_impl(double x, double y)
{
	assert(y);
	return (unsigned)x % (unsigned)y;
}
////////////////////////////////////////////////////////////////////////////////////

//pseudocode
int  avmbinaryfile(void) {

	unsigned tmpMagic;
	unsigned ProgramVars;
	int t,t2;
	char * tmp;
	size_t check;


	

	if(!fread(&tmpMagic,sizeof(unsigned),1,fp)&& tmpMagic!=340200501)
	
		return 0;

	else
		printf("\nOk... Starting reading binary file \n");

	/*if(!fread(&ProgramVars,sizeof(unsigned),1,fp))
	{
		printf("651");
		return 0;
	}*/
	if(!fread(&TotalStrings,sizeof(unsigned),1,fp))
	{
		printf("648");
		return 0;
	}

	Strings = malloc(TotalStrings*sizeof(char *));
	printf(" OK Strings init %d\n",TotalStrings);
	for (t=0; t<TotalStrings; ++t){
		
		if(!fread(&t2,sizeof(int),1,fp))
		{	
			printf("656");
			return 0;
		}
		printf("The size is :%u\n",t2 );
		tmp = (char *)malloc(sizeof(char)*t2);
		int k=0;
		while (k<t2) {
			//printf("The k is %d",k);
			if (fread(&tmp[k],sizeof(char),1,fp) != 1)
				{printf("663\n");
								return 0;}
			k++;
		}
		printf("\n The string tmp was read was %s\n",tmp);
		Strings[t] = strdup(tmp);//malloc(t2* sizeof(char))
		
		printf("\n The string read was %s\n",Strings[t]);
	}

	if(!fread(&TotalNums,sizeof(unsigned),1,fp))
		{printf("673\n");
				return 0;}

	Nums = malloc(TotalNums*sizeof(double));
	for (t=0; t<TotalNums; t++) {
		if(!fread(&Nums[t],sizeof(double),1,fp))
			{printf("679\n");
						return 0;}
	}

	if(!fread(&Totalused,sizeof(unsigned),1,fp))
		{printf("684\n");
				return 0;}

	userFunctions = malloc(Totalused*sizeof(avm_userfunc));
	for (t=0; t<Totalused; t++){
		if(!fread(&userFunctions[t].taddress, sizeof(unsigned), 1, fp))
			{printf("690\n");
						return 0;}
		if(!fread(&userFunctions[t].totallocals, sizeof(unsigned), 1, fp))
			{printf("693\n");
						return 0;}
		if(!fread(&t2,sizeof(unsigned), 1, fp))
			{printf("696\n");
						return 0;}
		userFunctions[t].id = malloc(t2*sizeof(char));
		if(!fread(&userFunctions[t].id,sizeof(char), t2, fp))
			{printf("700\n");
						return 0;}
	}

	if(!fread(&TotalLibs,sizeof(unsigned),1,fp))
		{printf("705\n");
				return 0;}

	LibFuncs = malloc(TotalLibs*sizeof(char *));
	for(t=0; t<TotalLibs; t++) {
		if(!fread(&t2,sizeof(unsigned), 1, fp))
			{printf("711\n");
						return 0;}
		LibFuncs[t] = malloc(t2*sizeof(char));
		fread(LibFuncs[t],sizeof(char), t2, fp); 
	}

	//read code
	int i;
	instruction *instr,*codeRead;
	vmarg *vmargm;
	if((fread(&codeSize, sizeof (unsigned), 1, fp)) !=1)
		{printf("722\n");
				return 0;
		}
 
	codeRead = malloc(sizeof(instruction)*codeSize);
	
	for(i=0; i<codeSize; i++){
		
		//codeRead[i] =  malloc(sizeof(instruction));
		if((fread(&codeRead[i].opcode, sizeof (int), 1, fp)) !=1)
			{printf("732\n");
						return 0;}

		codeRead[i].arg1 = malloc(sizeof(vmarg));
		if((fread(&codeRead[i].arg1->type, sizeof (int), 1, fp)) !=1)
			{printf("737\n");
						return 0;}
		
		if((fread(&codeRead[i].arg1->val, sizeof (unsigned), 1, fp)) !=1)
			{printf("741\n");
						return 0;}
		
		codeRead[i].arg2 = malloc(sizeof(vmarg));
		if((fread(&codeRead[i].arg2->type, sizeof (int), 1, fp)) !=1)
			{printf("746\n");
						return 0;}
		
		if((fread(&codeRead[i].arg2->val, sizeof (unsigned), 1, fp)) !=1)
			{printf("750\n");
						return 0;}
		
		codeRead[i].result = malloc(sizeof(vmarg));
		if((fread(&codeRead[i].result->type, sizeof (int), 1, fp)) !=1)
			{printf("755\n");
						return 0;}
		
		if((fread(&codeRead[i].result->val, sizeof (unsigned), 1, fp)) !=1)
			{printf("759\n");
						return 0;
					}
		if((fread(&codeRead[i].line, sizeof (unsigned), 1, fp)) !=1)
			{printf("763\n");
						return 0;}
	}
	code = codeRead;
}

char* number_tostring(avm_memcell* m){
	char s[100];
	sprintf(s,"%f",m->data.numVal);
	return strdup(s);

}

char* string_tostring(avm_memcell* m){     
	 return strdup(m->data.strVal);
}

char* bool_tostring(avm_memcell* m){
	 if(m->data.boolVal == false)
			return strdup("FALSE");
	 else 
			return strdup("TRUE");
}



avm_userfunc* avm_getfuncinfo(unsigned int address)
{
	int i=0;
	for (; i<Totalused; i++) {
		if(userFunctions[i].taddress == address)
			break;
	}

	if (i == Totalused)
		return NULL;// (avm_userfunc*)0

	if (i != Totalused)
		return(&userFunctions[i]);
}

char* userfunc_tostring(avm_memcell* m){
	 
	 avm_userfunc* userfun = (avm_userfunc *)malloc(sizeof(avm_userfunc));
	 userfun =  avm_getfuncinfo(m->data.funcVal);
	 
	 char* R = strdup(userfun->id);
	 free(userfun);
	 return strdup(R);
}

char* libfunc_tostring(avm_memcell* m){
		char* R = strdup(m->data.libfuncVal);
	 return R;
}

char* nil_tostring(avm_memcell* m){
	 char* R= "NIL";
	 return strdup(R);
}

char* undef_tostring(avm_memcell* m){
	 char* R = "UNDEFINED";
	 return strdup(R);
}

void execute_add(instruction * i)
{

		execute_arithmetic(i);
}


void execute_sub(instruction * i)
{

		execute_arithmetic(i);
}


void execute_mul(instruction * i)
{
		execute_arithmetic(i);
}
void execute_div(instruction * i)
{
		execute_arithmetic(i);
}
void execute_mod(instruction * i)
{
		execute_arithmetic(i);
}
void execute_uminus(instruction *i )
{
	assert(0);
}






unsigned char number_tobool(avm_memcell* m)
{
		return m->data.numVal != 0;
}

unsigned char string_tobool(avm_memcell* m)
{
	return m->data.strVal[0] != 0;
}

unsigned char bool_tobool(avm_memcell* m)
{
	return m->data.boolVal;
}

unsigned char table_tobool(avm_memcell* m)
{
	return 1;
}

unsigned char userfunc_tobool(avm_memcell* m)
{
	return 1;
}

unsigned char libfunc_tobool(avm_memcell* m)
{
	return 1;
}

unsigned char nil_tobool(avm_memcell* m)
{
	return 0;
}

unsigned char undef_tobool(avm_memcell* m)
{
	assert(0);
	return 0;
}







void  execute_jne(instruction* instr)
{
	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	
	if(rv1->type ==  undef_m || rv2->type == undef_m)
		fprintf(stdout,"'undef' involved in equality!");
	else
	{
		if(rv1->type ==  nil_m || rv2->type == nil_m)
			result = rv1->type ==  nil_m && rv2->type == nil_m;
		else
		{
			if(rv1->type ==  bool_m || rv2->type == bool_m)
	result = (avm_tobool(rv1) == avm_tobool(rv2));
			else
			{
	if(rv1->type !=  rv2->type)
		fprintf(stdout,"%s == %s is illegal!", typeStrings[rv1->type], typeStrings[rv1->type]);
	else
	{
			switch(rv1->type)
			 {
				case(string_m):
				{
					result= (strcmp(rv1->data.strVal, rv2->data.strVal) == 0);
					break;
				}
				case(number_m):
				{
			 result=(rv1->data.numVal==rv2->data.numVal);
			 break;
				}
				case(table_m):
				{
					result=(rv1->data.numVal==rv2->data.numVal);
					break;
				}
				default:
				{
			 result=0;
				}
			 }
			}
		}
		}
	}
	
	if(!executionFinished && !result)
		pc =  instr->result->val;
}

void  execute_jle(instruction* instr)
{
	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	if (rv1->type!=rv2->type)
	{
		fprintf(stdout,"%s <= %s is illegal!", typeStrings[rv1->type], typeStrings[rv1->type]);
	}
	else
	{
		switch(rv1->type)
			 {
				case(number_m):
				{
			 result=(rv1->data.numVal<=rv2->data.numVal);
			 break;
				}
				default:
				{
			 result=0;
				}
			 }    

	}
	if(!executionFinished && result)
		pc =  instr->result->val;
}
void  execute_jge(instruction* instr)
{
	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	if (rv1->type!=rv2->type)
	{
		fprintf(stdout,"%s >= %s is illegal!", typeStrings[rv1->type], typeStrings[rv1->type]);
	}
	else
	{
		switch(rv1->type)
			 {
				case(number_m):
				{
			 result=(rv1->data.numVal>=rv2->data.numVal);
			 break;
				}
				default:
				{
			 result=0;
				}
			 }    

	}
	if(!executionFinished && result)
		pc =  instr->result->val;
}

void  execute_jlt(instruction* instr)
{
	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	if (rv1->type!=rv2->type)
	{
		fprintf(stdout,"%s < %s is illegal!", typeStrings[rv1->type], typeStrings[rv1->type]);
	}
	else
	{
		switch(rv1->type)
			 {
				case(number_m):
			 result=(rv1->data.numVal<rv2->data.numVal);
			 break;
				case(string_m):
					result=((strcmp(rv1->data.strVal,rv2->data.strVal))<0);
					break;
				default:
			 result=0;

			 }    
	}
	if(!executionFinished && result)
		pc =  instr->result->val;
}
void  execute_jgt(instruction* instr)
{
	assert(instr->result->type == label_a);
	avm_memcell* rv1 =  avm_translate_operand(instr->arg1, &ax);
	avm_memcell* rv2 =  avm_translate_operand(instr->arg2, &bx);
	
	unsigned char result = 0;
	if (rv1->type!=rv2->type)
	{
		fprintf(stdout,"%s > %s is illegal!", typeStrings[rv1->type], typeStrings[rv1->type]);
	}
	else
	{
		switch(rv1->type)
			 {
				case(number_m):
			 result=(rv1->data.numVal > rv2->data.numVal);
			 break;
				case(string_m):
					result=((strcmp(rv1->data.strVal,rv2->data.strVal))>0);
					break;
				default:
			 result=0;
			 }    
	}
	if(!executionFinished && result)
		pc =  instr->result->val;
}

//ex
void libfunc_totalarguments(void)
{
		unsigned p_topsp =  avm_get_envvalue(topsp + AVM_SAVEDTOPSP_OFFSET);
		avm_memcellclear(&retval);
		
		if(!p_topsp)
		{
			fprintf(stdout,"'totalarguments' called outside a function!!!");
			retval.type =  nil_m;
		}
		else
		{
			retval.type =  number_m;
			retval.data.numVal =  avm_get_envvalue(p_topsp + AVM_NUMACTUALS_OFFSET);
		}
}





void avm_registerlibfunc(char* id, library_func_t addr)
{
	int i =0;
	
	while( libraryFunctions[i] != addr){i++;}
	if(i<0 || i>=3)
	{
	exit(0);
	}
	char* a = strdup(id);
	libNames[i] = a;

	return;
}


library_func_t avm_getlibraryfunc(char* id){
	 int i=0;
	 char* s =  libNames[0];
	 while(strcmp(id,s)!=0)
	 {
		 s= libNames[i++];
	 }
	 
	 
	 assert(i<=3);   
	 return libraryFunctions[i]; 
}


double consts_getnumber(unsigned index)
{

	if(index<TotalNums)
	{
		return Nums[index];
	}
	if(index >= TotalNums)
	{
		printf("ERROR cannot get const number\n");
		executionFinished=1;
	}
}


char* consts_getstring(unsigned index)
{
	if(index >=  TotalStrings)
	{
		printf("ERROR cannot get const string\n");
		executionFinished=1;
	}
	if(index < TotalStrings)
	{
		char* r = Strings[index];
		return strdup(r);
	}
	
}

char * libfuncs_getused(unsigned index){
	
	if(index >= TotalLibs){
		printf("ERROR cannot get used libfuncs \n");
		executionFinished = 1;
	}
	
	if(index<TotalLibs){
		char* r = LibFuncs[index];
		return strdup(r);
	}
}

//tou ntou 
avm_memcell *avm_tablegetelem(avm_table *table, avm_memcell *key){
 
	if (key->type != number_m && key->type != string_m)
		exit(0);
	
	if (key->type == number_m)
		return (avm_memcell *)getelemnum(table, key);
	
	if (key->type == string_m)
		return (avm_memcell *)getelemstr(table, key);

	return NULL;
}


avm_memcell* getelemnum(avm_table *table, avm_memcell *key){
	
	unsigned i;

	if ((int)key->data.numVal>AVM_TABLE_HASHSIZE)
		i = (int)key->data.numVal % AVM_TABLE_HASHSIZE;
	else{
		if ((int)key->data.numVal!=0)
			i = AVM_TABLE_HASHSIZE%(int)key->data.numVal;
		else
			i = 0;
	}  

	avm_table_bucket* tbucket =  table->numIndexed[i];
	for (; tbucket; tbucket = tbucket->next) {
		if (tbucket->key.data.numVal == key->data.numVal)
			return &tbucket->value;
	}
	return NULL;
}

avm_memcell* getelemstr(avm_table *table, avm_memcell *key){

	size_t ui;
	unsigned int uiHash = 0U;
	unsigned i;

	for (ui = 0U; key->data.strVal[ui] != '\0'; ui++)
		uiHash = uiHash*579+key->data.strVal[ui];
	i = uiHash%AVM_TABLE_HASHSIZE;

	avm_table_bucket* tbucket =  table->strIndexed[i];

	for (; tbucket; tbucket = tbucket->next) {
		if (strcmp(tbucket->key.data.strVal,key->data.strVal)==0)
			return &tbucket->value;
	}
	return NULL;
}

 avm_memcell* memcell_value_not_nil(avm_memcell* value){
	avm_memcell* r;

	r = malloc(sizeof(avm_memcell));
	r->type = value->type;
	r->data.numVal = value->data.numVal;
	if(value->data.strVal!=NULL)
		r->data.strVal = strdup(value->data.strVal);
	if(value->data.strVal == NULL)
		r->data.strVal = strdup("NULL");
	r->data.boolVal = value->data.boolVal;
	r->data.tableVal = value->data.tableVal;
	r->data.funcVal = value->data.funcVal;
	if (value->data.libfuncVal!=NULL)
		r->data.libfuncVal = strdup(value->data.libfuncVal);
	if(value->data.libfuncVal == NULL)
		r->data.libfuncVal = strdup("NULL");
	return r;

 }

void memcell_value_number(avm_memcell* key, int IsCleared,avm_memcell* tmp, avm_table* table){
	unsigned i;
	if ((int)key->data.numVal <= AVM_TABLE_HASHSIZE) {
		if ((int)key->data.numVal!=0)
			i=AVM_TABLE_HASHSIZE % (int)key->data.numVal;
		else
			i=0;
	}else{
		i = (int)key->data.numVal%AVM_TABLE_HASHSIZE;
	}

	avm_table_bucket *tbucket,*previous;
	for (tbucket = table->numIndexed[i];tbucket;  previous = tbucket,tbucket = tbucket->next){

		if (tbucket->key.data.numVal == key->data.numVal)
			break;

		if (tbucket == NULL && IsCleared == 1)
			printf("It's already clear\n");

		if (tbucket == NULL && IsCleared != 1) {
			tbucket = malloc(sizeof(avm_table_bucket));
			tbucket->key.data.numVal = key->data.numVal;
			tbucket->value = *tmp;
			table->total++;
			table->numIndexed[i] = tbucket;
			tbucket->next = table->numIndexed[i];
		}

		if (tbucket !=  NULL && IsCleared ==1) {
			avm_memcellclear(&tbucket->value);
			if(previous!=NULL)
				previous->next = tbucket->next;
			avm_memcellclear(&tbucket->key);
			table->total--;
			IsCleared = 0;
			return;
		}

		if (tbucket !=  NULL && IsCleared !=1) {
			tbucket->value=*tmp;
			tbucket->key.data.numVal=key->data.numVal;
		}
	}
}

void memcell_value_string(avm_memcell* key, int IsCleared,avm_memcell* tmp, avm_table* table) {
	
	unsigned i;
	size_t ui;
	unsigned int uiHash = 0U;


	for (ui = 0U; key->data.strVal[ui] != '\0'; ui++)
		uiHash = uiHash * 579 + key->data.strVal[ui];
	i = uiHash%AVM_TABLE_HASHSIZE;
		
	avm_table_bucket *tbucket,*previous;

	for (tbucket = table->strIndexed[i]; tbucket;  previous = tbucket,tbucket = tbucket->next){
		if (strcmp(tbucket->key.data.strVal,key->data.strVal) == 0)
			break;
	}
	
	if (tbucket == NULL && IsCleared == 1)
		printf("It's already clear\n");

	if (tbucket == NULL && IsCleared != 1){
		tbucket =  malloc(sizeof(avm_table_bucket));
		tbucket->value= *tmp;
		tbucket->key.data.strVal=strdup(key->data.strVal);
		tbucket->next=table->strIndexed[i];
		table->strIndexed[i]=tbucket;
		table->total++;
	}

	if (tbucket !=  NULL && IsCleared ==1) {
		avm_memcellclear(&tbucket->value);
		if (previous!=NULL)
			previous->next=tbucket->next;
		avm_memcellclear(&tbucket->key);
		table->total--;
		IsCleared = 0;
		return;
	}

	if(tbucket !=  NULL && IsCleared !=1){
		tbucket->value=*tmp;
		tbucket->key.data.strVal=strdup(key->data.strVal);
	}

}




void avm_tablesetelem(avm_table *table,avm_memcell *key,avm_memcell* value){

	unsigned index;
	if (key->type != number_m && key->type != string_m)
		exit(0);
	
	avm_memcell *tmp;
	int counter = 0;
	int IsCleared = 0;
	if (value->type == nil_m)
		IsCleared = 1;
	else
		tmp = memcell_value_not_nil(value);

	if(key->type == number_m)
		memcell_value_number(key, IsCleared, tmp,table);
	if(key->type ==  string_m)
		memcell_value_string(key, IsCleared,tmp,table);
}


void PrintVmArg(vmarg *tmp, FILE *fout) {

	if(tmp == NULL)
		fprintf(fout,"%10s","nil");
	else{

		if(tmp->type == label_a)
			fprintf(fout,"%10s","label_a");
		else if(tmp->type == global_a)
			fprintf(fout,"%10s","global_a");
		else if(tmp->type == formal_a)
			fprintf(fout,"%10s","formal_a");
		else if(tmp->type == local_a)
			fprintf(fout,"%10s","local_a");
		else if(tmp->type == number_a)
			fprintf(fout,"%10s","number_a");
		else if(tmp->type == string_a)
			fprintf(fout,"%10s","string_a");
		else if(tmp->type == bool_a)
			fprintf(fout,"%10s","bool_a");
		else if(tmp->type == nil_a)
			fprintf(fout,"%10s","nil_a");
		else if(tmp->type == userfunc_a)
			fprintf(fout,"%10s","userfunc_a");
		else if(tmp->type == libfunc_a)
			fprintf(fout,"%10s","libfunc_a");
		else if(tmp->type == retval_a)
			fprintf(fout,"%10s","retval_a");
		else
			exit(0);

		fprintf(fout,"%10u",tmp->val);
	}
}		
		
void PrintInstr(instruction* instr, FILE *fout) {
	
	fprintf(fout,"\n");

	fprintf(fout,"%10d",instr->line);

	if (instr->opcode == assign_v)
		fprintf(fout," %10s","assign_v");
	else if(instr->opcode == add_v)
		fprintf(fout," %10s","add_v");
	else if(instr->opcode == sub_v)
		fprintf(fout," %10s","sub_v");
	else if(instr->opcode == mul_v)
		fprintf(fout," %10s","mul_v");
	else if(instr->opcode == div_v)
		fprintf(fout," %10s","div_v");
	else if(instr->opcode == mod_v)
		fprintf(fout," %10s","mod_v");
	else if(instr->opcode == uminus_v)
		fprintf(fout," %10s","uminus_v");
	else if(instr->opcode == and_v)
		fprintf(fout," %10s","and_v");
	else if(instr->opcode == not_v)
		fprintf(fout," %10s","not_v");
	else if(instr->opcode == jeq_v)
		fprintf(fout," %10s","jeq_v");
	else if(instr->opcode == or_v)
		fprintf(fout," %10s","or_v");
	else if(instr->opcode == jne_v)
		fprintf(fout," %10s","jne_v");
	else if(instr->opcode == jle_v)
		fprintf(fout," %10s","jle_v");
	else if(instr->opcode == jge_v)
		fprintf(fout," %10s","jge_v");
	else if(instr->opcode == jlt_v)
		fprintf(fout," %10s","jlt_v");
	else if(instr->opcode == jgt_v)
		fprintf(fout," %10s","jgt_v");
	else if(instr->opcode == call_v)
		fprintf(fout," %10s","call_v");
	else if(instr->opcode == pusharg_v)
		fprintf(fout," %10s","pusharg_v");
	else if(instr->opcode == funcenter_v)
		fprintf(fout," %10s","funcenter_v");
	else if(instr->opcode == funcexit_v)
		fprintf(fout," %10s","assign_v"); 
	else if(instr->opcode == newtable_v)
		fprintf(fout," %10s","newtable_v"); 
	else if(instr->opcode == tablegetelem_v)
		fprintf(fout," %10s","tablegetelem_v");
	else if(instr->opcode == tablesetelem_v)
		fprintf(fout," %10s","tablesetelem_v"); 
	else if(instr->opcode == nop_v)
		fprintf(fout," %10s","nop_v"); 
	else
		exit(0);

	PrintVmArg(instr->arg1,fout);
	PrintVmArg(instr->arg2,fout);
	PrintVmArg(instr->result,fout);	
}

char* Table2Str(avm_memcell *m, int what) {

	avm_table_bucket *tbucket;
	avm_memcell *temp=malloc(sizeof(avm_memcell));
	avm_table *table=m->data.tableVal;
	char *TableStr = malloc(sizeof(char) * 1000);
	double value;

	bool boolVal;
	char* str;
	int i;

	for(i=0; i<AVM_TABLE_HASHSIZE; i++){

		if (what == 0) { tbucket = table->strIndexed[i]; }

		if (what == 1) { tbucket = table->numIndexed[i]; }

	 	for (tbucket; tbucket;tbucket=tbucket->next) {
			if (&tbucket->value) {
				if (&tbucket->key) {
					if (tbucket->value.type == number_m) {
						str = malloc(sizeof(double));
						sprintf(str,"%5.2f",tbucket->value.data.numVal);
					}

					if (tbucket->value.type == string_m) {
						str = malloc(sizeof(double));
						str = strdup(tbucket->value.data.strVal);
					}
					
					if (tbucket->value.type == bool_m) {
						str = malloc(10*sizeof(char));
						
						if (tbucket->value.data.boolVal !=  0)
							str = strdup("TRUE");
						if (tbucket->value.data.boolVal ==  0)
							str = strdup("FALSE");
					}

					if (tbucket->value.type == table_m) {
						str = malloc(sizeof(double));
						temp->type =  table_m;
						temp->data.tableVal = tbucket->value.data.tableVal;
						str = table_tostring(temp);
					}

					if (tbucket->value.type == userfunc_m) {
						str = malloc(sizeof(double));
						sprintf(str,"%u",tbucket->value.data.funcVal);
					}

					if (tbucket->value.type == libfunc_m) {
						str = malloc(50*sizeof(char));
						str = strdup(tbucket->value.data.libfuncVal);
					}

					if (tbucket->value.type == nil_m) {
						str = malloc(50*sizeof(char));
						str = strdup("NIL");
					}

					if (tbucket->value.type == undef_m) {
						str = malloc(50*sizeof(char));
						str = strdup("UNDEF");
					}
				}
			}
			sprintf(TableStr,"%s[%s]=%s\n",TableStr,tbucket->key.data.strVal,str);
		}
	}
	return TableStr;
}

 
char* table_tostring(avm_memcell* m){ 
	char* str1 = Table2Str(m,0);
	char* str2 = Table2Str(m,1);
	strcat(str1,str2);
	return strdup(str1);
}



int main() {
	fp = fopen("finalCode.abc","rb+");

	avm_initialize();
	if (avmbinaryfile() !=  0){
		
		FILE* fout = fopen("check_binaryFile.txt","w+");

		int i=0;
		for(; i<codeSize; i++){ PrintInstr(&code[i], fout); }
		fclose(fout);

		tp = AVM_STACKSIZE-1-TotalGlobals;

		for(;!executionFinished;){ execute_cycle(); }
	}else{
		
		printf("Something fucked up...!\n");
	}
	return 0;
}