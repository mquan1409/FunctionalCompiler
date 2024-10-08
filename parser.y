%{
    #include <stdio.h>
    #include "y.tab.h"
	#include "ast.h"
    #include "stack.h"

	extern char *yytext;
    extern struct stack st;

    int yylex(void);
    void yyerror(char *msg);

	int extra = 0;
	void pt(int);				// simple printing func for tracing execution
%}

%start program

%union {int val; char* str;}

%token<str> FUNCDEF GETINT GETBOOL BOP IF NOT LET TYPE PEP ID BOOL CONST AOP MAOP COMP

%type<val> program decllist decllist_base expr exprlist_base const id funcid bool rettype type

%%

id : ID { $$ = insert_node($1,1);}

funcid : ID { $$ = insert_node($1,0);}

const : CONST { $$ = insert_node($1,0);}

bool : BOOL { $$ = insert_node($1,0);}

rettype : TYPE { char str[20] = "ret ";$$ = insert_node(strcat(str,$1),0);}

type: TYPE { $$ = insert_node($1,0); }

program: '(' PEP expr ')' 									{ insert_child($3);
																	  insert_node("PEP", 0);
																	  pt(1); }
	| '(' FUNCDEF funcid decllist_base rettype expr ')' program { insert_child($3);
                                                             pop(&st);
                                                             while (st.top != NULL && st.top->val != (-2)) insert_child(pop(&st));
																	  insert_child($5);
																	  insert_child($6);
																	  insert_node("funcdef", 0);
																	  pt(2); }
decllist_base: decllist       { push(&st,(-2)); }

decllist:							{ pt(3); }
	| '(' id type ')' decllist { push(&st,$2);
                                push(&st,$3);
										  pt(19); }

expr: const 									{ $$ = $1;
													  pt(4); }
	| id 											{ $$ = $1;
	       	  									  pt(5); }
	| bool 										{ $$ = $1;
              									  pt(6); }
	| '(' GETBOOL ')' 						{ $$ = insert_node("getbool",0);
													  pt(7); }
	| '(' GETINT ')' 							{ $$ = insert_node("getint",0);
													  pt(8); }
	| '(' NOT expr ')'						{ insert_child($3);
													  $$ = insert_node("not",7);
													  pt(9); }
	| '(' BOP expr expr exprlist_base ')'		{ insert_child($3);
													  insert_child($4);
                                         pop(&st);
                                         while(st.top != NULL && st.top->val != (-2)) {
                                           insert_child(pop(&st));
                                         };
													  $$ = insert_node($2,5);
													  pt(10); }
	| '(' MAOP expr expr exprlist_base ')'	{ insert_child($3);
													  insert_child($4);
                                         pop(&st);
                                         while(st.top != NULL && st.top->val != (-2)) {
                                           insert_child(pop(&st));
                                         };
													  $$ = insert_node($2,4);
													  pt(11); }
	| '(' AOP expr expr ')' 				{ insert_child($3);
													  insert_child($4);
													  $$ = insert_node($2,4);
													  pt(12); }
	| '(' COMP expr expr ')' 				{ insert_child($3);
													  insert_child($4);
													  $$ = insert_node($2,0);
													  pt(13); }
	| '(' IF expr expr expr ')' 			{ insert_child($3);
													  insert_child($4);
													  insert_child($5);
													  $$ = insert_node("if",6);
													  pt(14); }
	| '(' ID exprlist_base ')' 			{ pop(&st);
                                         while (st.top != NULL && st.top->val != (-2)) insert_child(pop(&st));
                                         $$ = insert_node($2,2);
													  pt(15); }
	| '(' LET '(' id expr ')' expr ')' 	{ insert_child($4);
	                                      insert_child($5);
													  insert_child($7);
													  $$ = insert_node("let",3);
													  pt(16); }

exprlist_base: exprlist { push(&st,(-2)); }

exprlist:				{ pt(17); }
	| expr exprlist	{ push(&st,$1);
							  pt(18); }

%%

int yywrap() {
    return 1;
}

void yyerror(char *msg){
    fprintf(stderr, "Error: %s\n", msg);
}

// comment out the print if unnecessary
void pt(int i) {
	//printf("%i\n", i);
}
