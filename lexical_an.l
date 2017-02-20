%{

#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include<stdlib.h>
#include "parser.h"
//#include "Definitions.h"
#define MAX_LEN 14




/*char* token[]={"CRAP","IF","ELSE","WHILE","FOR","FUNCTION","RETURN","BREAK","CONTINUE","LOCAL","TRUE","FALSE",
"NIL","ASSIGN","PLUS","MINUS","MUL","DIV","MOD","EQUAL","NOTEQUAL","OR","AND","NOT","PLUSPLUS","MINUSMINUS",
"GREATER","LESS","GREATER_EQUAL","LESS_EQUAL","INTEGER","REAL","STRING","COMMENT","LEFTBRACKET","RIGHTBRACKET",
"LEFTSQUARE","RIGHTSQUARE","FULLSTOP","LEFTPAR","RIGHTPAR","SEMICOLON","COMA","DOUBLECOLON","COLON","IDENTIFIER","NEW_LINE_CHECK","COMMENT_LINE"};*/

typedef struct token_t{

    char* type;
    char* Str;
    int line;
    int number;
    int index;
    int textlength;
    int integer;
    double real_v;
}alpha_token_t;

//alpha_token_t *yylval;
int NestedCnt=0;
char *str;

%}

%option noyywrap
%option yylineno

integer_num	[0-9]*
real_num	[0-9]+\.[0-9]+
string		\"([^\"]*|([^\"]*[\\][\"])*[^\"]*)\"
NotValid_str	\"([^\"]*|([^\"]*[\\][\"])*[^\"]*)[[:space:]]
identifier	[[:alpha:]]([[:alnum:]]|[_])*
comment_line		"//".*
  
%%
"if"  { return IF;}
"else" {return    ELSE;}
"while"	{return  WHILE;}
"for"  {return  FOR;}
"function" { return    _FUNCTION_;}
"return"		{ return    RETURN; }
"break"			{ return    BREAK; }
"continue"		{ return    CONTINUE; }
"local"			{ return    _LOCAL_; }
"true"			{ return    _TRUE_; }
"false"			{ return   _FALSE_; }
"nil"			{ return    NIL; }

"="				{return   ASSIGN;}
"+"			   {return    PLUS;}
"-"				{return    MINUS;}
\*				{return    MUL;}
\/				{return    DIV;}
"%"				{return    MOD;}
"=="			{return    EQUAL;}
"!="			{return    NOTEQUAL;}
"||"			{return    OR;}
"or"			{return OR;}
"&&"			{return    AND;}
"and"		{return AND; }
!				{return    NOT;}
"not"				{return    NOT;}
"++"			{return    PLUSPLUS;}
"--"			{return    MINUSMINUS;}
\>				{return    GREATER;}
\<				{return    LESS;}
">="			{return    GREATER_EQUAL;}
"<="			{return    LESS_EQUAL;}

\{				{return   LEFTBRACKET;}
\}				{return    RIGHTBRACKET;}
"\["			{return    LEFTSQUARE;}
"\]"			{return    RIGHTSQUARE;}
\.				{return    FULLSTOP;}
\(				{return    LEFTPAR;}
\)				{return    RIGHTPAR;}
\;				{return    SEMICOLON;}
\,				{return    COMA;}
"::"			{return    DOUBLECOLON;}
\:				{return    COLON;}


{integer_num} { 
				
				
				//alpha_token_t *yylval=(alpha_token_t*)malloc(sizeof(alpha_token_t));
				yylval.intValue= atoi(yytext);
				
				return   INTEGER;
			}

{real_num} { //(alpha_token_t*)yylval;
			
			 // alpha_token_t *yylval=(alpha_token_t*)malloc(sizeof(alpha_token_t));
			   yylval.realValue= atof(yytext);
			  
			  return    REAL;
			}

{string} {//(alpha_token_t*) yylval;
			//alpha_token_t *yylval=(alpha_token_t*)malloc(sizeof(alpha_token_t));
	char buffer[10204];
	int i=0, j=0;
	while ((yytext[i] != '\n') && (yytext[i]!=EOF)) {

		if(yytext[i] == '\\') {

			if ((yytext[i+1] == 'n') && (yytext[i+1]!=EOF))
				buffer[j++]='\n';

			else if ((yytext[i+1] == 't') && (yytext[i+1]!=EOF))
				buffer[j++] = '\t';

			else if ((yytext[i+1] == 'r') && (yytext[i+1]!=EOF))
				buffer[j++] = '\r';

			else if ((yytext[i+1] == 'f') && (yytext[i+1]!=EOF))
				buffer[j++] = '\f';

			else if ((yytext[i+1] == '\\') && (yytext[i+1]!=EOF))
				buffer[j++] = '\\';

			else if ((yytext[i+1] == '"') && (yytext[i+1]!=EOF))
				buffer[j++] = '"';
			
			else{
				if (j <= strlen(yytext)){
					buffer[j++] = yytext[i++];
					buffer[j++] = yytext[i++];
					continue;
				}
			}
			i++;
		}
		else{

			if (j <= strlen(yytext))
				buffer[j++] = yytext[i];
			else
				break;
		}
		i++;
	}
	yylval.stringValue = (char*)malloc(sizeof(sizeof(char)*strlen(buffer)));
	yylval.stringValue = buffer;
	return STRING;

}

{NotValid_str} { 
			int i=0,n_Line= yylineno;
			while(yytext[i] != '\n')
			{
				if(yytext[i] == '\n')
				{
					n_Line--;
					break;
				}
				i++;	
			}			
			fprintf(stderr, "\n At line %d: quote  not found, String not valid! \n",n_Line);
			}


{identifier}	{
				// alpha_token_t *yylval=(alpha_token_t*)malloc(sizeof(alpha_token_t));
				yylval.stringValue = strdup(yytext);
			     return    IDENTIFIER;
			}

{comment_line} {;}

 
"/*" {
		int	left=1,right=0,countercheck=1, flag=0, line=yylineno;unput(*yytext);
		char c;
		while((c=input())!=EOF){
			if( c== '/')
			{
			    if(c =input() =='*')
			    {
					left++;
					//unput(*yytext);
			    }
		  	}
			    
			if( c == '*')
			{
			      if (c =input() ==  '/')
			      {
			          right++;
			         // unput(*yytext);
				  
				}  
			}
	      if( left == right)
	      { 
		      if( left >1)
		      {
		         NestedCnt= left-1;
		      }
		      return COMMENT;
	      
	      }
			
		}
		
		//fprintf(stderr,"\tError: line: %d unclosed Comment" , line );
		//return -1;
	}

[^ \t\n] 		{ //fprintf(stderr,"line %d: Error unmatched character: %s\n",yylineno, yytext);
	//return -1;
			  ;
			}
[ \t\n]			{;}

%%
/*int alpha_yylex(void* yylval)
{
	int index;
	
	if(( index= yylex() )!= EOF)
	{	
		//printf("%d index",index);
		if( index <= 0 )
		 {
			
			return 0;
		 }
		else if( index == 100)
		{
		      ((alpha_token_t *)yylval)->type=NULL;
					((alpha_token_t *)yylval)->Str=NULL;
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number;
					((alpha_token_t *)yylval)->index= NEW_LINE_CHECK;
		}
		else{
			if( (index >= 1) && (index <=12) )
				{
					((alpha_token_t *)yylval)->type=strdup("KEYWORD");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			
		   if( (index >= 13) && (index <=29) )
				{
					
					((alpha_token_t *)yylval)->type=strdup("OPERATOR");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}


			if( (index == 30))
				{
					((alpha_token_t *)yylval)->type=strdup("INTEGER");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			if( (index == 31))
				{
					((alpha_token_t *)yylval)->type=strdup("REAL");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			if( (index == 32))
				{
					((alpha_token_t *)yylval)->type=strdup("STRING");
					((alpha_token_t *)yylval)->Str=strdup(str);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			if( (index == 33))
				{
					((alpha_token_t *)yylval)->type=strdup("COMMENT");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index; 
					
				}
			if( (index == 47))
				{
					((alpha_token_t *)yylval)->type=strdup("COMMENT_LINE");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index; 
					
				}
			if( (index >= 34) && (index <=44) )
				{
					((alpha_token_t *)yylval)->type=strdup(" PUNCTUATION ");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			 if( (index == 45))
				{
					((alpha_token_t *)yylval)->type=strdup("IDENTIFIER");
					((alpha_token_t *)yylval)->Str=strdup(yytext);
					((alpha_token_t *)yylval)->line=yylineno;
					((alpha_token_t *)yylval)->number++;
					((alpha_token_t *)yylval)->index= index;
					
				}
			}
			
			return 1;	
	}
	else{
		
		return 0;
	}	


}



void print_YYClark(alpha_token_t* yylval,FILE * fp)
{

	if( (yylval->index <100))
	{
			if((yylval->type) )
			{
				if(strcmp(yylval->type,"KEYWORD")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s %s \n",yylval->line,yylval->number,yylval->Str,yylval->type,token[yylval->index]);
				}
				if(strcmp(yylval->type,"OPERATOR")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s %s \n",yylval->line,yylval->number,yylval->Str,yylval->type,token[yylval->index]);
				}
				if(strcmp(yylval->type,"INTEGER")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s \"%s\" \n",yylval->line,yylval->number,yylval->Str,yylval->type, yylval->Str);
				}
				if(strcmp(yylval->type,"REAL")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s \"%s\" \n",yylval->line,yylval->number,yylval->Str,yylval->type, yylval->Str);
				}
				if(strcmp(yylval->type,"STRING")==0)
				{
				//printf("!!!!!!!!!! %s",yylval->Str);
			fprintf(stdout,"\n%d:  #%d %s %s %s \n",yylval->line,yylval->number,yylval->Str,yylval->type, yylval->Str);
				}
				if(strcmp(yylval->type,"COMMENT")==0)
				{
					if( NestedCnt >0 )
					{
					  while (NestedCnt != 0){
					  
					    fprintf(stdout,"\n%d:  #%d   NESTED COMMENT \n",yylval->line,yylval->number);
					    NestedCnt--;
					    }
				    
					}
					NestedCnt=0;
					fprintf(stdout,"\n%d:  #%d  %s \n",yylval->line,yylval->number,yylval->type,token[yylval->index]);
				}
				if(strcmp(yylval->type,"COMMENT_LINE")==0)
				{
				
					   
					fprintf(stdout,"\n%d:  #%d  %s  \n",yylval->line,yylval->number,yylval->type);
				}
				
				
				if(strcmp(yylval->type," PUNCTUATION ")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s %s \n",yylval->line,yylval->number,yylval->Str,yylval->type,token[yylval->index]);
				}

				if(strcmp(yylval->type,"IDENTIFIER")==0)
				{
			fprintf(stdout,"\n%d:  #%d \"%s\" %s \"%s\" \n",yylval->line,yylval->number,yylval->Str,yylval->type, yylval->Str);
				}
				
			}
	}
			
	


}



/*int main(int argc,char** argv)
{
	FILE *fp;
    alpha_token_t *yylval=(alpha_token_t*)malloc(sizeof(alpha_token_t));
	yylval->number=0;
	int No_of_tokens=0;
	int cnt=0;
	if( argc > 1)
		{
		
			if( argc == 2)
			{
				yyin=fopen(argv[1],"r");
				if( yyin == 0 )
				{
					printf("ERRROR:FAILED TO READ FILE");
					return 0;
				}
				fprintf(stdout," Attempt to read file: %s \n",argv[1]);
				
				
				while(alpha_yylex(yylval))
				{
					
					
							
							print_YYClark(yylval, stdout);
							
					
					
						
				}

		}
	
	}
	
	return 1;

}

*/

