/*
	Kalaitzakis Ioannis 2288
	Tzanavaras Georgios 2452
	Rigakis Nikolaos 2422
*/


%{
	#include <stdio.h>
	//#include "SymbolTable.h"
	//#include "manage.h"
	#include "final.h"

	int yyerror(char*);
	void print(int, char *, char *);
	int yylex(void);
	int scope=0;
	int funcNameUsed=0;

	int LEGAL=0;
	int functionCounter=0;
	int keepLoopCount = 0;
	quad* quads;
	unsigned programVarOffset;
	unsigned functionLocalOffset;
	unsigned formalArgOffset;
	unsigned scopeSpaceCounter=1;
	unsigned total;

	unsigned int tempCounter;
	struct ArgList* temp_argList = NULL;
	struct expr *temp_expr = NULL;
	struct forvalue *temp_for = NULL;
	struct statement *temp_stmt = NULL;

	bool ERROR =  false;
	bool canBreak = false;
	bool canContinue = false;
	int canReturn = 0;
	
	extern int yylineno;
	extern char* yytext;
	extern FILE* yyin;

	int functionOffset;
	int loopcnt=0;
	char *CurrentFuncName;
	stack_t *scopeoffsetstack;
	const char *customName; 
	int overideflag=1;

	unsigned int QuadLabel[343];
	int QuadLabelIndex=0;
	int totalSize=0;
	
%}

%start program

%union{
	struct SymbolTableEntry* exprNode;
	char* stringValue;
	int 		intValue;
	double 		realValue;

	//enum iopcode iopcodes;
	struct ArgList* arg_list;
	struct indexElement *element;
	struct indexedList *indList;
	struct expr *expression;
	struct statement *statements;
	struct methodCall *call;
	struct forvalue *forVal;
}

%token IF
%token ELSE
%token WHILE
%token FOR
%token _FUNCTION_
%token RETURN
%token BREAK
%token CONTINUE
%token _LOCAL_
%token _TRUE_
%token _FALSE_
%token NIL
%token ASSIGN
%token PLUS
%token MINUS
%token MUL
%token DIV
%token MOD
%token EQUAL
%token NOTEQUAL
%token OR
%token AND
%token NOT
%token PLUSPLUS
%token MINUSMINUS
%token GREATER
%token LESS
%token GREATER_EQUAL
%token LESS_EQUAL
%token LEFTBRACKET
%token RIGHTBRACKET
%token LEFTSQUARE
%token RIGHTSQUARE
%token FULLSTOP
%token LEFTPAR
%token RIGHTPAR
%token SEMICOLON
%token COMA
%token DOUBLECOLON
%token DOUBLEDOT
%token COLON
%token COMMENT_LINE
%token COMMENT
%token INVALID_COMMENT
%token SPACE
%token <intValue> INTEGER
%token <realValue> REAL
%token <stringValue> STRING
%token <stringValue> IDENTIFIER

%type <expression> lvalue funcdef funcprefix expr term primary member call objectdef const  assignexpr tableItem
%type <stringValue> funcname 
%type <intValue>  ifprefix elseprefix  funcbody M N whilestart whilecondition // typous gia ifstmt,elseprefix ....
%type <arg_list> elist elistMore funcargs idlist idlistMore
%type <statements> block ifstmt whilestmt forstmt returnstmt stmt stmts loopstmt
%type <call> callsuffix normcall methodcall 
%type <indList>    	indexed indexes
%type <element>    	indexedelem
%type <forVal>     forprefix


//UPDATED
%right ASSIGN 	//
%left AND  OR 	//
%nonassoc EQUAL  NOTEQUAL //
%nonassoc LESS LESS_EQUAL GREATER GREATER_EQUAL //
%left PLUS MINUS 	//
%left  MUL DIV MOD 	//
%right NOT PLUSPLUS MINUSMINUS UMINUS //
%left FULLSTOP DOUBLEDOT //
%left LEFTSQUARE RIGHTSQUARE 	//
%left LEFTPAR RIGHTPAR //
%left LEFTBRACKET RIGHTBRACKET 	//

%%

program:	stmts
			|/*empty*/{}
			;
			
stmts:		stmts stmt  {

				struct statement* statementEntry = (struct statement* )malloc(sizeof( struct statement ));
				statementEntry->continuelist = New_Stack();
				statementEntry->breaklist = New_Stack();

				

				// if(loopcnt > 0)
				// {

				// 		if($1 != NULL)
				// 	{
				// 		//Stack_Printing(&($1->continuelist));
				// 		statementEntry->continuelist = $1->continuelist;
				// 		statementEntry->breaklist = $1->breaklist;
				// 		Stack_Printing(&($1->continuelist));

				// 	}
				// 	else if($2 != NULL)
				// 	{
				// 		statementEntry->continuelist = $2->continuelist;
				// 		statementEntry->breaklist = $2->breaklist;
				// 	}
				// }
				// else
				// {
				// 		statementEntry->continuelist = Stack_Merging($1->continuelist, $2->continuelist);
				// 		statementEntry->breaklist = Stack_Merging($1->breaklist, $2->breaklist);
				// }
				// //Stack_Printing(&(statementEntry->continuelist));
				$$ = temp_stmt;
				print(yylineno,"stmts stmt" ,"stmts");
			}
			|stmt {
					print(yylineno,"stmt" ,"stmts");
					resetTemp();
					$$ = $1;
			}
			;
			
stmt:		expr SEMICOLON{ 

				print(yylineno,"expr;" ,"stmt");
				struct statement *st=malloc(sizeof(struct statement));
				st->breaklist=New_Stack();
				st->continuelist=New_Stack();
				$$=st;

			}

			|ifstmt	{ 
				
				print(yylineno,"ifstmt" ,"stmt");
				$$=$1;LEGAL--;
			}
			
			|whilestmt{ 
				print(yylineno,"whilestmt" ,"stmt");
				$$=$1;
				canBreak = false;
				canContinue = false;
			}

			|forstmt { 

				
				print(yylineno,"forstmt" ,"stmt"); 
				$$=$1;
				canBreak = false;
				canContinue = false;
			}

			|returnstmt {

				print(yylineno,"returnstmt" ,"stmt");
				$$=NULL;
			} 
			
			|BREAK SEMICOLON {

				if(loopcnt > 0 && keepLoopCount<loopcnt){

					if( loopcnt != 0)
					{

						push(nextquadlabel(), &(temp_stmt->breaklist));
					}
					else{
						 
						 temp_stmt->breaklist =  New_Stack();
					 	 temp_stmt->continuelist = New_Stack();
					

					}
					emit(jump, NULL, NULL, NULL, 0, yylineno);
					$$ = temp_stmt;

				}
				else if(keepLoopCount>=loopcnt){
					fprintf(stdout, "\n\t[Error]: line %d: Use of 'break' while in function.\n\n",  yylineno);
					ERROR = true;
					$$=NULL;
				}else{
					fprintf(stdout, "\n\t[Error]: line %d: Use of 'break' while not in a loop.\n\n",  yylineno);
					ERROR = true;
					$$=NULL;

				}
				print(yylineno,"break;" ,"stmt");
			}
			
			|CONTINUE SEMICOLON {

				if(loopcnt >0 && keepLoopCount<loopcnt){
					

					if( loopcnt != 0)
					{

						push(nextquadlabel(), &(temp_stmt->continuelist));
					}
					else{
						
						temp_stmt->breaklist =  New_Stack();
					 	temp_stmt->continuelist = New_Stack();
					

					}
					
					
					emit(jump, NULL, NULL, NULL, 0, yylineno);
					$$ = temp_stmt;
					overideflag=0;

				}else if(keepLoopCount>=loopcnt){
					fprintf(stdout, "\n\t[Error]: line %d: Use of 'continue' while in function.\n\n",  yylineno);
					ERROR = true;
					$$=NULL;
				}else{
					fprintf(stdout, "\n\t[Error]: line %d: Use of 'continue' while not in a loop\n\n",  yylineno);
					ERROR = true;
					$$=NULL;
				}
				print(yylineno,"continue;" ,"stmt");

			}
			
			|block	{
				print(yylineno,"block" ,"stmt");
				$$ = $1;
			}
					
			|funcdef {

				print(yylineno,"funcdef" ,"stmt");
				$$ = NULL;
			}
			
			|SEMICOLON { $$ = NULL;}

			|COMMENT {;}
			;
			
expr:		assignexpr {

				print(yylineno,"assignexpr" ,"expr");
				$$ = $1;
			}
			|expr PLUS expr {

				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($1)?$1->sym:istempexpr($3)?$3->sym:newTemp();
				emit(add, $1 , $3, $$, 0, yylineno);
				print(yylineno,"expr + expr" ,"expr");

			}
			|expr MINUS expr {

				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($1)?$1->sym:istempexpr($3)?$3->sym:newTemp();
				emit(sub, $1 , $3, $$, 0, yylineno);
				print(yylineno,"expr - expr" ,"expr");

			}
			|expr MUL expr {

				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($1)?$1->sym:istempexpr($3)?$3->sym:newTemp();
				emit(mul, $1 , $3, $$, 0, yylineno);
				print(yylineno,"expr * expr" ,"expr");
			}
			|expr DIV expr {

				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($1)?$1->sym:istempexpr($3)?$3->sym:newTemp();
				emit(div_, $1 , $3, $$, 0, yylineno);
				print(yylineno,"expr / expr" ,"expr");

			}
			|expr MOD expr {

				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($1)?$1->sym:istempexpr($3)?$3->sym:newTemp();
				emit(mod, $1 , $3, $$, 0, yylineno);
				print(yylineno,"expr \% expr" ,"expr");
			}
			|expr GREATER expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_greater, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr > expr" ,"expr");

			}
			|expr GREATER_EQUAL expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_greatereq, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr >= expr" ,"expr");

			}
			|expr LESS expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_less, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr < expr" ,"expr");

			}
			|expr LESS_EQUAL expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_lesseq, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr <= expr" ,"expr");

			}
			|expr EQUAL expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_eq, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr == expr" ,"expr");

			}
			|expr NOTEQUAL expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(if_noteq, $1, $3, $$, nextquadlabel()+3, yylineno);
				emit(assign, newexpr_constbool(0), NULL, $$, 0, yylineno);
				emit(jump, NULL, NULL, NULL, nextquadlabel()+2, yylineno);
				emit(assign, newexpr_constbool(1), NULL, $$, 0, yylineno);
				print(yylineno,"expr != expr" ,"expr");

			}
			|expr AND expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(and, $1, $3, $$,0, yylineno);
				print(yylineno,"expr AND expr" ,"expr");

			}
			|expr OR expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(or, $1, $3, $$,0, yylineno);
				print(yylineno,"expr OR expr" ,"expr");

			}
			|term {

				print(yylineno,"term" ,"expr");
				$$ = $1;

			}
			|error ';'{yyerrok;}
			;

term:		LEFTPAR expr RIGHTPAR {

				print(yylineno,"( expr )" ,"term");
				$$ = $2;
			}
			|MINUS expr %prec UMINUS{

				checkuminus($2);
				$$ = newexpr(arithexpr_e);
				$$->sym = istempexpr($2)? $2->sym:newTemp();
				emit(uminus, $2, NULL, $$, 0, yylineno);
				print(yylineno,"-expr" ,"term");
			}
			|NOT expr {

				$$ = newexpr(boolexpr_e);
				$$->sym = newTemp();
				emit(not, $2, NULL, $$, 0, yylineno);
				print(yylineno,"NOT  expr" ,"term");
			}
			|PLUSPLUS lvalue {

				if($2){
					if($2->type == programfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using ProgramFunc '%s' as an lvalue\n\n", yylineno, $2->sym->name);
						ERROR = true;

					}else if($2->type == libraryfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using LibFunc '%s' as an lvalue\n\n", yylineno, $2->sym->name);
						ERROR = true;

					}else if($2->type == tableitem_e){

						$$ = emit_iftableitem($2);
						emit(add, $$, newexpr_constnum(1), $$, 0, yylineno);
						emit(tablesetelem, $2, $2->index, $$, 0, yylineno);

					}else{

						$$ = newexpr(arithexpr_e);
						$$->sym = istempexpr($2)? $2->sym:newTemp();
						emit(add, $2, newexpr_constnum(1), $2, 0, yylineno);
						emit(assign, $2, NULL, $$, 0, yylineno);
					}			
				}
				print(yylineno,"++lvalue", "term");
			}
			|lvalue PLUSPLUS {

				if($1){
					if($1->type == programfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using ProgramFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else if($1->type == libraryfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using LibFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else{

						expr* value = NULL;
						$$ = newexpr(var_e);
						$$->sym = istempexpr($1)? $1->sym:newTemp();

						if($1->type == tableitem_e){
							value = emit_iftableitem($1);
							emit(assign, $1, NULL, $$, 0, yylineno);
							emit(add, $1, newexpr_constnum(1), $1, 0, yylineno);
							emit(tablesetelem, $1, $1->index, $1, 0, yylineno);

						}else{
							emit(assign, $1, NULL, $$, 0, yylineno);
							emit(add, $1, newexpr_constnum(1), $1, 0, yylineno);
						}
					}				
				}
				print(yylineno,"lvalue++" ,"term");

			}
			|MINUSMINUS lvalue {

				if($2){
					if($2->type == programfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using ProgramFunc '%s' as an lvalue\n\n", yylineno, $2->sym->name);
						ERROR = true;

					}else if($2->type == libraryfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using LibFunc '%s' as an lvalue\n\n", yylineno, $2->sym->name);
						ERROR = true;

					}else if($2->type == tableitem_e){
						$$ = emit_iftableitem($2);
						emit(sub, $$, newexpr_constnum(1), $$, 0, yylineno);
						emit(tablesetelem, $2, $2->index, $$, 0, yylineno);

					}else{
						emit(sub, $2, newexpr_constnum(1), $2, 0, yylineno);
						$$ = newexpr(arithexpr_e);
						$$->sym = istempexpr($2)? $2->sym:newTemp();
						emit(assign, $2, NULL, $$, 0, yylineno);	
					}
				}
				print(yylineno,"--lvalue" ,"term");

			}
			|lvalue MINUSMINUS {

				if($1){
					if($1->type == programfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using ProgramFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else if($1->type == libraryfunc_e){
						fprintf(stdout, "\n\t[Error]: line %d: Using LibFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else{

						expr* value = NULL;
						$$ = newexpr(var_e);
						$$->sym = istempexpr($1)? $1->sym:newTemp();

						if($1->type == tableitem_e){
							value = emit_iftableitem($1);
							emit(assign, $1, NULL, $$, 0, yylineno);
							emit(sub, $1, newexpr_constnum(1), $1, 0, yylineno);
							emit(tablesetelem, $1, $1->index, $1, 0, yylineno);

						}else{
							emit(assign, $1, NULL, $$, 0, yylineno);
							emit(sub, $1, newexpr_constnum(1), $1, 0, yylineno);
						}
					}				
				}
				print(yylineno,"lvalue--" ,"term");

			}
			|primary {
				print(yylineno,"primary" ,"term");
				$$ = $1;
			}
			;
			
assignexpr:	lvalue ASSIGN expr{

				if($1){

					SymbolTableEntry* persistCheck= (SymbolTableEntry*)malloc (sizeof(SymbolTableEntry));
					persistCheck= $1->sym;
					int ok = 0;
					SymbolTableEntry* HashTableCheck=(SymbolTableEntry*)malloc (sizeof(SymbolTableEntry));
					int i;
					int index;
					HashTableCheck=HT[hash( persistCheck->name)];
					if (HashTableCheck != NULL) {	
						while (HashTableCheck != NULL) {
							
							index= hash(HashTableCheck->name);
							if (strcmp(persistCheck->name,HashTableCheck->name)==0) {
									if (persistCheck->isActive)
										ok = 1;
							}
							HashTableCheck = HashTableCheck->next;
						}
					}

					if($1->type == programfunc_e && ok== 0){
						fprintf(stdout, "\n\t[Error]: line %d: Using ProgramFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else if($1->type == libraryfunc_e && ok== 0){
						fprintf(stdout, "\n\t[Error]: line %d: Using LibFunc '%s' as an lvalue\n\n", yylineno, $1->sym->name);
						ERROR = true;

					}else if($1->type == tableitem_e && ok== 0){
							
						emit(tablesetelem, $1, $1->index, $3, 0, yylineno);	
						$$ = emit_iftableitem($1);
						$$->type = assignexpr_e;
						
					}else{
						emit(assign, $3, NULL, $1, 0, yylineno);
						$$ = newexpr(assignexpr_e);
						$$->sym = newTemp();
						emit(assign, $1, NULL, $$, 0, yylineno);
					}
					print(yylineno,"lvalue = expr" ,"assignexpr");
				}
			}
			;
			
			
primary:	lvalue {

				$$ = emit_iftableitem($1);
				print(yylineno,"lvalue", "primary");
			}
			|call {
			
				$$ = $1;
				print(yylineno,"call", "primary");
			}
			|objectdef {

				$$ = $1;
				print(yylineno,"objectdef", "primary");
			}
			|LEFTPAR funcdef RIGHTPAR {

				$$ = newexpr(programfunc_e);
				$$->sym = $2->sym;
				print(yylineno,"( funcdef )", "primary");
			}
			|const {
				
				$$ = $1;
				print(yylineno,"const", "primary");
			}
			;
			
lvalue:		IDENTIFIER {

				SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));

				// den yparxei sto hash table - kane INSERT
				if( (symbol = LookUp($1)) == NULL ){

					symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
					symbol->test = functionCounter;
					symbol->isActive = true;
					symbol->name = strdup($1);
					symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
					symbol->value.varVal->scope = scope;
					symbol->value.varVal->line = yylineno;
					symbol->ValType = VARIABLE;
					symbol->type = var_s;
					symbol->offset = currscopeoffset();
					symbol->space = currscopespace();
					inccurrscopeoffset();

					if(symbol->space != formalarg){
						symbol->SymType = (scope>0)?LOCAL:GLOBAL;
					}else{
						symbol->SymType = FORMAL;
					}
				
					Insert(symbol, HT);

				}else{
					//check accessibility
					symbol=(SymbolTableEntry*)isVisible(symbol, scope);
					if(symbol == NULL){
																																																																																																																																																																											
						symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						symbol->test = functionCounter;
						symbol->isActive = true;
						symbol->name = strdup($1);
						symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
						symbol->value.varVal->scope = scope;
						symbol->value.varVal->line = yylineno;
						symbol->ValType = VARIABLE;
						symbol->type = var_s;
						symbol->offset = currscopeoffset();
						symbol->space = currscopespace();
						inccurrscopeoffset();
					
						Insert(symbol, HT);
						fprintf(stdout, "\n\t[Error]: line %d: Cannot access variable '%s' at line: %d.\n\n", yylineno, symbol->name, symbol->value.varVal->line);
					}
				}
				
				$$=lvalue_expr(symbol);
				print(yylineno,"IDENTIFIER" ,"lvalue");

			}
			|_LOCAL_ IDENTIFIER {

				//check for lib func collision
				if(isLibFunc($2)){
					fprintf(stdout, "\n\t[Error]: line %d: Collision with library function '%s'.\n\n", yylineno, $2);
					ERROR = true;


				}else{

					SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
					//look up for symbols in same scope
					if( (symbol = LookUpScope($2, scope)) == NULL ){

						symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						symbol->isActive = true;
						symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
						symbol->name = strdup($2);
						symbol->value.varVal->scope = scope;
						symbol->value.varVal->line = yylineno;
						symbol->ValType = VARIABLE;
						symbol->SymType = (scope==0) ? GLOBAL:LOCAL;
						symbol->type = var_s;
						symbol->offset = currscopeoffset();
						symbol->space = currscopespace();
						inccurrscopeoffset();
						
						Insert(symbol, HT);

					}else{
						//anaferomaste sth func/var pou vrikame
						if(symbol->ValType == FUNCTION){
							fprintf(stdout, "[Warning]: line %d: '%s' is a function.\n", yylineno, $2);
						}
					}
					$$ = lvalue_expr(symbol);
				}

				print(yylineno,"LOCAL IDENTIFIER" ,"lvalue");
								
			}
			|DOUBLECOLON IDENTIFIER {

				SymbolTableEntry *symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
				
				if((symbol = LookUpScope($2, 0)) == NULL){
					fprintf(stdout, "\tError: line %d: no GLOBAL %s declared.\n\n", yylineno, $2);
					$$=lvalue_expr(newTemp());
					ERROR = true;

				}else{
					//anaferomaste sth func/var pou vrikame
					$$=lvalue_expr(symbol);
				}
				print(yylineno,"::IDENTIFIER" ,"lvalue");
			}
			|member{

				print(yylineno,"member" ,"lvalue");
				$$ = $1;
			}
			;

tableItem:  lvalue FULLSTOP IDENTIFIER {

				$$ = member_item($1,(char*)$3);
				print(yylineno, "lvalue.IDENTIFIER", "tableItem");
			}

			|lvalue LEFTSQUARE expr RIGHTSQUARE {

				$1 = emit_iftableitem($1);
				$$ = newexpr(tableitem_e);
				$$->sym = $1->sym;
				$$->index = $3;

				print(yylineno, "lvalue[expr]", "tableItem");
			}
	        ;
			
member:		tableItem {
				
				print(yylineno,"tableItem" ,"member");
				$$ =$1;
			}
			|call FULLSTOP IDENTIFIER {
				
				print(yylineno,"call.IDENTIFIER" ,"member");
				$$ = $1;
			}
			|call LEFTSQUARE expr RIGHTSQUARE {
				print(yylineno,"call[expr]" ,"member");
				$$ = $1;
			}
			;
			
call:		call LEFTPAR elist RIGHTPAR { 
				
				print(yylineno,"call(elist)" ,"call");
				$$ = make_call($1, $3); //ch
				temp_argList = NULL;
			}
			|lvalue callsuffix {

				if($1->sym->type == programfunc_s || $1->sym->type == libraryfunc_s){

					SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
					symbol = LookUp(($1)->sym->name);
						
					if(symbol == NULL){
						
						symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						symbol->test = functionCounter;
						symbol->isActive = true;
						symbol->name = strdup(($1)->sym->name);
						symbol->value.varVal=(Variable*)malloc(sizeof(Variable));
						symbol->value.varVal->scope = scope;
						symbol->value.varVal->line = yylineno;
						symbol->ValType = VARIABLE;
						symbol->type = var_s;
						symbol->offset = currscopeoffset();
						symbol->space = currscopespace();
						inccurrscopeoffset();

						if(symbol->space != formalarg){
							symbol->SymType = (scope>0)?LOCAL:GLOBAL;
						}else{
							symbol->SymType = FORMAL;
						}
						
						Insert(symbol, HT);
							
					}else{

						if(symbol->ValType == FUNCTION){
							// check accecibilities
							if(isVisible(symbol, scope) != NULL){
								if($2->method){
									struct expr* self = $1;
									$1 = emit_iftableitem(member_item(self, $2->name));
									$2->elist = InsertArgument($2->elist, self->sym);
								}
							}
						}
					}
				}

				print(yylineno,"lvalue callsuffix" ,"call");
				$$ = make_call($1, $2->elist);
				

			}
			|LEFTPAR funcdef RIGHTPAR LEFTPAR elist RIGHTPAR {

				struct expr* func = newexpr(programfunc_e);
				func->sym = $2->sym;
				$$ = make_call(func, $5);
				temp_argList = NULL;
				print(yylineno,"(funcdef)(elist)" ,"call");
			}
			;
			
callsuffix:	normcall {

				print(yylineno,"normcall", "callsuffix");
				$$ = $1;
			}
			|methodcall {

				print(yylineno,"methodcall", "callsuffix");
				$$ = $1;
			}
			;
			
normcall:	LEFTPAR elist RIGHTPAR {

				print(yylineno,"(elist)" ,"normcall");
				//if ($$ != NULL)
				//{
				$$ = malloc(sizeof(method));
				$$->elist = $2;
				$$->method = false;
				$$->name = NULL;
				temp_argList = NULL;
				//}
			}
			;
			
methodcall:	FULLSTOP FULLSTOP IDENTIFIER LEFTPAR elist RIGHTPAR {

				print(yylineno,"..IDENTIFIER(elist)" ,"methodcall");
				$$ = malloc(sizeof(method));
				$$->elist = $5;
				$$->method = true;
				$$->name = strdup($3);
				temp_argList = NULL;
			}
			;
			
elist:		expr elistMore{
				//search for sibling param symbol - CONFLICT
				if(SearchArgList($2, $1->sym)){
					temp_argList = InsertArgument($2, makeSymbolCopy($1->sym));
				}else{
					temp_argList = InsertArgument($2, $1->sym);
				}
				$$ = temp_argList;

				print(yylineno,"expr elistMore", "elist");
			}

			|/* empty */{

				temp_argList = (ArgumentList*)malloc(sizeof(ArgumentList));
				temp_argList->head = NULL;
				$$ = temp_argList;
			}
			;

elistMore:	COMA expr elistMore{
				//search for sibling param symbol - CONFLICT
				if(SearchArgList($3, $2->sym)){
					temp_argList = InsertArgument($3, makeSymbolCopy($2->sym));
				}else{
					temp_argList = InsertArgument($3, $2->sym);
				}
				$$ = temp_argList;

				print(yylineno,",expr elistMore", "elist");

			}
			|/* empty */{

				$$ = temp_argList;
			}
			;
			
objectdef:	LEFTSQUARE elist RIGHTSQUARE { 

				expr* t = newexpr(newtable_e);
				t->sym = newTemp();
				emit(tablecreate, NULL, NULL, t, 0, yylineno);

				SymbolTableEntry* tmp = $2->head;
				double i = 0.0;

				for(tmp; tmp != NULL; tmp = tmp->next){

					if(tmp->ValType == VARIABLE){

						expr* k = newexpr(var_e);
						k->sym = tmp;
						emit(tablesetelem, newexpr_constnum(i++), k, t, 0, yylineno);
					}
				
					if(tmp->ValType == FUNCTION){

						expr* k = newexpr(programfunc_e);
						k->sym = tmp;
						emit(tablesetelem, newexpr_constnum(i++), k, t, 0, yylineno);
					}
				}
				$$ = t;
				temp_argList = NULL;
				print(yylineno,"[elist]", "objectdef");
			}
			|LEFTSQUARE indexed  RIGHTSQUARE{

				expr* t= newexpr(newtable_e);
				t->sym = newTemp();
				emit(tablecreate, NULL, NULL, t, 0 ,yylineno);
				
				indexElement *element= $2 ->head;
				for(element; element != NULL; element = element->next){
					emit(tablesetelem, element->index, element->object, t, 0, yylineno);
				}
				$$ = t;
				print(yylineno,"[indexed]" ,"objectdef");
			}
			;


indexed:	indexedelem indexes{ 
				//print(yylineno,"indexedelem indexes" ,"indexed");

				print(yylineno, "indexedelem indexes", "indexed");

				if($1 == NULL)
				{ 
					$$=malloc(sizeof(indexedList));
				}

				if($1 != NULL)
				{
					if($2->head == NULL){

						$1->next= NULL;
						$2->head= $1;
						$2->listSize ++;
					}
					else
					{
						$1->next = $2->head;
						$2->head = $1;
					}
				}

				$$ = $2;
			}
			;
			
indexes:	COMA indexedelem indexes{

				if($3 == NULL)
				{
					$$ = malloc(sizeof(indexedList));
				}

				if($2 != NULL)
				{
					if($3->head == NULL)
					{
						$3->head = $2;
						$3->listSize++;
					}
					else
					{	
						$2->next = $3->head;
						$3->head = $2;
					}
				}
				print(yylineno,",indexedelem indexes" ,"indexes");
				$$ = $3;
		}
		|/* empty */
		{ 
			indexedList* tmp;
			tmp = (indexedList*)malloc(sizeof(indexedList));
			$$ = tmp;
		}
		;

indexedelem:LEFTBRACKET expr COLON expr RIGHTBRACKET{

				indexElement *elem = (indexElement*)malloc(sizeof(indexElement));

				elem->index = $2;
				elem->object = $4;
				elem->next = NULL;

				print(yylineno, "{expr;expr}", "indexedelem");
				$$ = elem;
			}
			;

funcblockstart:	{ }//push(loopcounterstack, loopcnt); loopcnt=0; };

funcblockend:	{ }//loopcnt = pop(loopcounterstack); };

funcdef:	 funcprefix funcargs funcblockstart funcbody funcblockend{
				
				print(yylineno,"FUNCTION IDENTIFIER(idlist)block" ,"funcdef");
				
				$1->totallocals = $4;
				$$ = $1;
				
				expr * exp = newexpr(programfunc_e);
				exp=$1;
				emit(funcend, NULL, NULL, exp, 0, yylineno);
				
				exitscopespace();
				functionCounter--;
				countFuncs--;
				canReturn--;
				isFunction = false;
			}
			;

funcname: IDENTIFIER {
				
				SymbolTableEntry* func = NULL;

				// check for lib func collision
				if(isLibFunc($1)){
				
					//lib func collision
					fprintf(stdout, "\n\t[Error]: line %d: Collision with library function '%s'.\n\n", yylineno, $1);
					ERROR = true;

				}else{

					// den yparxei sto idio scope
					if((func = LookUpScope($1, scope)) == NULL){

						func = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						func->name = strdup($1);
						func->isActive = true;
						func->space = currscopespace();
						func->type = programfunc_s;
						func->ValType = FUNCTION;
						func->SymType = USERFUNC;
						func->value.funcVal=(Function*)malloc(sizeof(Function));
						func->value.funcVal->scope = scope;
						func->value.funcVal->line = yylineno;
						
						Insert(func, HT);

					}else{

						//found a function in current scope
						if(func->ValType == FUNCTION){

							if(func->SymType == LIBFUNC){
								fprintf(stdout, "\n\t[Error]: line %d: Collision with library function '%s'.\n\n", yylineno, $1);

							}else{
								fprintf(stdout, "\n\t[Error]: line %d: Function '%s' already exists at line: %d.\n\n", yylineno, $1, func->value.funcVal->line);
							}

						//found a variable in current scope
						}else{
							fprintf(stdout, "\n\t[Error]: line %d: Cannot define function '%s'. Variable '%s' already defined at line: %d.\n\n", yylineno, $1, $1, func->value.varVal->line);
						}
						ERROR = true;

					}
				}
				$$ = func?func->name:$1;
			}
 			| /*empty */{
 				customName = (const char *)malloc(7*sizeof(char));
				sprintf((char*)customName, "_f%d", funcNameUsed++);

				SymbolTableEntry* func = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
				func->name = strdup(customName);
				func->isActive = true;
				func->space = currscopespace();
				func->ValType = FUNCTION;
				func->SymType = USERFUNC;
				func->type = programfunc_s;
				func->value.funcVal = (Function*)malloc(sizeof(Function));
				func->value.funcVal->scope = scope;
				func->value.funcVal->line = yylineno;

				Insert(func, HT);
				$$ = func->name;
 			}
		;
			
		

funcprefix: _FUNCTION_ funcname {

				CurrentFuncName = strdup($2);
				if(!isLibFunc($2)){
					CurrentFuncName = strdup($2);
					expr *r =  newexpr(programfunc_e);
					r->sym = LookUp($2);
					r->sym->iaddress = nextquadlabel();
					
					$$ = r;
					emit(funcstart, NULL, NULL, r, 0,yylineno);
				}
				

				if(scopeoffsetstack == NULL)
				{scopeoffsetstack = New_Stack();}

				push(currscopeoffset(), &(scopeoffsetstack)); 

				functionOffset = 0;
				enterscopespace(); 
				resetformalargsoffset();
				canReturn++;
				keepLoopCount = loopcnt;
				

				print(yylineno,"function funcname", "funprefix");
			};

funcargs: LEFTPAR idlist {

			/// 8ELOUN SVISIMOOO
			SymbolTableEntry *tmp2;
			SymbolTableEntry *tmp=LookUp(CurrentFuncName);
			if (tmp != NULL){
				tmp->value.funcVal->args=$2;
			}
			tmp2=LookUpScope(CurrentFuncName, scope);
			if( tmp2 != NULL){	
				tmp2->value.funcVal->args=$2;
			}

			print(yylineno,"function arguments " ,"Left Parenthesis Id lists");
		}
			RIGHTPAR {
			enterscopespace(); // Now entering function locals space.
			resetfunctionlocaloffset(); // Start locals from zero.
			$$ = $2;
		}
		;

funcbody: {isFunction = true; countFuncs++;} block {

			print(yylineno,"function body " ,"block");
			$$ = currscopeoffset(); // Extract #total locals.
			exitscopespace(); // Exiting function locals space
		}	
		;


block:		LEFTBRACKET{
				scope++;
				LEGAL++;
			}
			stmts RIGHTBRACKET
			{
				LEGAL--;

				Hide(scope--); //hides the scope
				print(yylineno,"{stmts}","block");
				//print(yylineno,"{stmts}" ,"block");
				$$=$3;
			}
			;
			|LEFTBRACKET{scope++;} RIGHTBRACKET{
				Hide(scope--);
				print(yylineno,"{}","block");
			}
			;
			
const:		INTEGER	{
				$$ = newexpr_constnum($1);
				print(yylineno,"INTEGER" ,"const");
			}
			|REAL	{
				$$ = newexpr_constnum($1);
				print(yylineno,"REAL" ,"const");
			}
			|STRING	{
				$$ = newexpr_conststring((char*)$1);
				print(yylineno,"STRING" ,"const");
			}
			//|COMMENT{ print(yylineno,"COMMENT" ,"const");}
			//|INVALID_COMMENT	{ print(yylineno,"INVALID_COMMENT" ,"const");}
			|NIL	{
				$$ = newexpr(nil_e);
				print(yylineno,"NIL" ,"const");
			}
			|_TRUE_	{
				$$ = newexpr_constbool(1);
				print(yylineno,"TRUE" ,"const");
			}
			|_FALSE_{
				$$ = newexpr_constbool(1);
				print(yylineno,"FALSE" ,"const");
			}
			;
			 
idlist:		IDENTIFIER idlistMore{

				// check for lib func collision
				if(isLibFunc($1)){
					//lib func collision
					fprintf(stdout, "\n\t[Error]: line %d: Formal '%s' shadows libfunc.\n\n", yylineno, $1);
					ERROR = true;

				}else{

					SymbolTableEntry* symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));			
					//lookup for symbols in same scope
					if((symbol = LookUpScope($1, scope+1)) == NULL){

						symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						symbol->name = strdup($1);
						symbol->isActive = true;
						symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
						symbol->value.varVal->scope = scope+1;
						symbol->value.varVal->line = yylineno;
						symbol->space = currscopespace();
						symbol->ValType = VARIABLE;
						symbol->SymType = FORMAL;
						symbol->nextArgument = NULL;
						
						Insert(symbol, HT);
						temp_argList = InsertArgument(temp_argList, symbol);

					}else{

						//found a function in same scope
						if(symbol->ValType == FUNCTION){
								fprintf(stdout, "\n\t[Error]: line %d: Redeclaration of function '%s' as formal argument.\n\n", yylineno, $1);
								ERROR = true;


						//found variable in same scope
						}else{

							//check if it is active
							if(symbol->isActive){
								fprintf(stdout, "\n\t[Error]: line %d: Redeclaration of variable '%s' as formal argument.\n\n", yylineno, $1);
								ERROR = true;


							//it's inactive - insert new formal arg in ST
							}else{

								symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
								symbol->name = strdup($1);
								symbol->isActive = true;
								symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
								symbol->value.varVal->scope = scope+1;
								symbol->value.varVal->line = yylineno;
								symbol->space = currscopespace();
								symbol->ValType = VARIABLE;
								symbol->SymType = FORMAL;
								symbol->nextArgument = NULL;
								
								Insert(symbol, HT);
								temp_argList = InsertArgument(temp_argList, symbol);

							}
						}
					}
				}

				$$ = temp_argList ? temp_argList:NULL;/////////////////////////????
				temp_argList = NULL;

				print(yylineno, "IDENTIFIER idlistMore", "idlist");
			}
			|/*empty*/{

				ArgumentList* newArgList = (ArgumentList*)malloc(sizeof(ArgumentList));
				newArgList->head = NULL;
				$$ = newArgList;
			}
			;

idlistMore:	COMA IDENTIFIER idlistMore{

				// check for lib func collision
				if(isLibFunc($2)){
					//lib func collision
					fprintf(stdout, "\n\t[Error]: line %d: Formal '%s' shadows libfunc.\n\n", yylineno, $2);
					ERROR = true;


				}else{

					SymbolTableEntry* lookupSymbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
					SymbolTableEntry* symbol;//= (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
					
					//lookup for symbols in same scope
					if( (lookupSymbol = LookUpScope($2, scope+1)) == NULL ){

						symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
						symbol->name = strdup($2);
						symbol->isActive = true;
						symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
						symbol->value.varVal->scope = scope+1;
						symbol->value.varVal->line = yylineno;
						symbol->space = currscopespace();
						symbol->ValType = VARIABLE;
						symbol->SymType = FORMAL;
						symbol->nextArgument = NULL;
						
						Insert(symbol, HT);
						temp_argList = InsertArgument(temp_argList, symbol);

					}else{
						//found a function in same scope
						if(lookupSymbol->ValType == FUNCTION){
							fprintf(stdout, "\n\t[Error]: line %d: Redeclaration of function '%s' as formal argument.\n\n", yylineno, $2);
							ERROR = true;


						//found variable in same scope
						}else{

							//check if it is active
							if(lookupSymbol->isActive){
								fprintf(stdout, "\n\t[Error]: line %d: Redeclaration of variable '%s' as formal argument.\n\n", yylineno, $2);
								ERROR = true;


							//it's inactive - insert new formal arg in ST
							}else{

								symbol = (SymbolTableEntry*)malloc(sizeof(SymbolTableEntry));
								symbol->name = strdup($2);
								symbol->isActive = true;
								symbol->value.varVal = (Variable*)malloc(sizeof(Variable));
								symbol->value.varVal->scope = scope+1;
								symbol->value.varVal->line = yylineno;
								symbol->space = currscopespace();
								symbol->ValType = VARIABLE;
								symbol->SymType = FORMAL;
								symbol->nextArgument = NULL;
								
								Insert(symbol, HT);
								temp_argList = InsertArgument(temp_argList, symbol);

							}
						}
					}
				}

				$$ = temp_argList ? temp_argList:NULL;/////////////////////////????

				print(yylineno, ",IDENTIFIER idlistMore", "idlist");
			}
			|/*empty*/{

				ArgumentList* newArgList = (ArgumentList*)malloc(sizeof(ArgumentList));
				newArgList->head = NULL;
				$$ = newArgList;
			}
			;





ifprefix:	IF LEFTPAR expr RIGHTPAR{

				emit(if_eq, $3, newexpr_constbool(1), NULL, nextquadlabel()+3, yylineno);
				
				emit(jump, NULL, NULL, NULL, 0, yylineno);

				QuadLabel[QuadLabelIndex]=nextquadlabel()-1;
				printf("$$$$$$$$$$$$$$$$$%d\n", QuadLabel[QuadLabelIndex]);
				$$ = QuadLabel[QuadLabelIndex];
				QuadLabelIndex++;
				totalSize++;
				LEGAL++;
				//$$ = nextquadlabel();
			}
			;

			//TO DO : H sunthikh aplh else lish me flag wste NA KANOUME thn poustia na to vriskou

ifstmt:	ifprefix stmt{
			
			patchlabel($1, nextquadlabel()+1);
			//QuadLabel[totalSize-1] = 0;
			totalSize--;
			$$ = $2;
			print(yylineno,"IF(expr)stmt", "ifstmt");
			}
		|ifprefix stmt elseprefix stmt {
			//QuadLabel[totalSize-1] = 0;

			patchlabel($1, QuadLabel[totalSize]);
			
			patchlabel($3, QuadLabel[totalSize]);

			//QuadLabel[totalSize-1] = 0;
			totalSize--;
			
			statements* st = (statements*)malloc(sizeof(statements));
			st->breaklist = (stack_t*)malloc(sizeof(stack_t));

			if(	overideflag == 1 )
			{	
				if( ( $2->breaklist != NULL) && ( $4->breaklist != NULL) )
				{	
					st->breaklist=Stack_Merging($2->breaklist, $4->breaklist);
				}


				st->continuelist=(stack_t*)malloc(sizeof(stack_t));
			
				if(( $2 != NULL) && ( $4 != NULL))
				{
					st->continuelist = Stack_Merging($2->continuelist, $4->continuelist);
				}
			}
				$$ = st;
			overideflag=0;

			print(yylineno,"IF(expr)stmt ELSE stmt", "ifstmt");
		}
		;

elseprefix:	ELSE{
				//$$ = nextquadlabel();

				QuadLabel[QuadLabelIndex]=nextquadlabel();
				$$ = QuadLabel[QuadLabelIndex];
				emit(jump, NULL, NULL, NULL, 0, yylineno);
			}
			;

N:	{
		$$ = nextquadlabel();
		emit(jump, NULL, NULL, NULL, 0, yylineno); 
	}
	;

M:	{
		$$ = nextquadlabel();
	}
	;	
	
loopstart:	{ ++loopcnt; };

loopend:	{ --loopcnt; };

loopstmt: loopstart stmt loopend { $$ = $2; };

whilestart:	WHILE {

	            $$ = nextquadlabel();
				canBreak = true;
				canContinue = true;
				if(loopcnt == 0){
					temp_stmt = (statements*)malloc(sizeof(statements));
					temp_stmt->continuelist = New_Stack();
					temp_stmt->breaklist = New_Stack();
				}
	        }
	        ;

whilestmt:	whilestart  whilecondition loopstmt  {


				emit(jump, NULL, NULL, NULL, $1, yylineno);
				patchlabel($2, nextquadlabel());
				if( $3 != NULL)
				{

					while(!isEmpty(&(temp_stmt->breaklist)))
					{
						patchlabel(pop(&(temp_stmt->breaklist)), nextquadlabel());
					}
				
					while(!isEmpty(&(temp_stmt->continuelist)))
					{
						patchlabel(pop(&(temp_stmt->continuelist)), $1);
					}
				}

				print(yylineno, "WHILE(expr)stmt", "whilestmt");
				$$ = temp_stmt;
			}				
			;

whilecondition:	LEFTPAR expr RIGHTPAR {

					emit(if_eq, $2, newexpr_constbool(1), NULL, nextquadlabel()+2, yylineno);
					$$ = nextquadlabel();
					emit(jump, NULL, NULL, NULL, 0, yylineno);
					LEGAL++;
                }                    
                ;		


forstmt:	forprefix N elist RIGHTPAR N loopstmt N {

				patchlabel($1->enter, ($5)+1);    
				patchlabel($2, nextquadlabel()); 
				patchlabel($5, $1->test); 
				patchlabel($7, ($2)+1); 
				if( $6 != NULL)
				{
					while(!(isEmpty(&(temp_stmt->breaklist)))){
						patchlabel(pop(&(temp_stmt->breaklist)), nextquadlabel());
					}
				
					while(!(isEmpty(&(temp_stmt->continuelist)))){
						patchlabel(pop(&(temp_stmt->continuelist)), ($2)+1);
					}
				}

				print(yylineno, "FOR(elist;expr;elist)stmt", "forstmt");
				forStatement= false;
				temp_argList = NULL;
				$$=temp_stmt;
			}
			;

forprefix : FOR LEFTPAR elist SEMICOLON M expr SEMICOLON {
				
				$$ = malloc(sizeof(struct forvalue));
				$$->test = $5;
				$$->enter = nextquadlabel();
				emit(if_eq, $6, newexpr_constbool(1), NULL, 0, yylineno);
				LEGAL++;

				canBreak = true;
				canContinue = true;
				forStatement= true;
				temp_argList = NULL;
				if(loopcnt == 0){
					temp_stmt = (statements*)malloc(sizeof(statements));
					temp_stmt->continuelist = New_Stack();
					temp_stmt->breaklist = New_Stack();
				}
				print(yylineno, "forprefix","FOR LEFTPAR elist SEMICOLON M expr SEMICOLON");
			}
			;
			
returnstmt:	RETURN expr SEMICOLON{ 
				//print(yylineno,"RETURN expr" ,"returnstmt");
				if(canReturn > 0){
					print(yylineno, "RETURN expr", "returnstmt");
					emit(ret, NULL, NULL, $2, 0, yylineno);
				}else{
					fprintf(stdout,"\n\t[Error]: line %d: Use of 'return' while not in a function\n\n",yylineno);
					ERROR = true;

				}

			}
			|RETURN SEMICOLON {

				if(canReturn > 0){
					print(yylineno,"RETURN expr","returnstmt");
					emit(ret, NULL, NULL, NULL, 0, yylineno);
				}else{
					fprintf(stdout,"\n\t[Error]: line %d: Use of 'return' while not in a function\n\n",yylineno);
					ERROR = true;
				}
				print(yylineno, "RETURN;", "returnstmt");
			}
			;
			
%%

void print(int line, char *rule1, char *rule2){
	fprintf(stdout, "line %d: %s -> %s\n", line, rule1, rule2);
}


int yyerror( char* yaccProvidedMessage)
{
	fprintf(stderr, "\n%s: line %d, before token: %s \n" , yaccProvidedMessage, yylineno, yytext);
	fprintf(stderr, "INPUT NOT VALID \n");
	return 0;
}


int main( int argc, char ** argv)
{
	if (argc >1 ){
		if(!(yyin = fopen(argv[1], "r"))){
			fprintf(stderr,"Cannot read file: %s \n", argv[1]);
			return 1;
		}
	}
    else{
		yyin = stdin;
	}
	
	CreateHashTable();
	fprintf(stdout, "\n");
	yyparse();
	printSymbols();
	
	if(ERROR == false){ 
		print_quads();
		generateALL();
		patch_incomplete_jumps();
		createBinaryFile();
		printfReadbleInstructions();
	}
	return 0;
}
