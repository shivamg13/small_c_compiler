	%scanner Scanner.h
	%scanner-token-function d_scanner.lex()
	%polymorphic STMT : stmtAst* ; EXPS : expAst* ; EXPLIST : std::list<expAst*> ; STMLIST : std::list<stmtAst*>; ARR : arrayRef*; adi: string;CHAR : char ; ABSNODE : abstract_astnode* ; ABSNODELIST : list<abstract_astnode*>; DEC_STR: decl_struct *; SYMELEM : Symtab_elem *;

	%type <STMLIST>  statement_list
	%type <STMT> statement assignment_statement iteration_statement selection_statement compound_statement
	%type <EXPS> expression logical_and_expression equality_expression relational_expression additive_expression multiplicative_expression unary_expression postfix_expression primary_expression 
	%type <EXPLIST> expression_list
	%type <ARR>  l_expression
	%type<DEC_STR> declarator
	%type <adi> unary_operator FLOAT_CONSTANT INT_CONSTANT STRING_LITERAL IDENTIFIER constant_expression type_specifier
	%type <SYMELEM> function_definition fun_declarator
	%token VOID INT FLOAT  AND_OP OR_OP EQ_OP NE_OP LE_OP GE_OP IF ELSE WHILE FOR RETURN INC_OP

	%%

	translation_unit
	:
	function_definition 
	| translation_unit function_definition 
	;

	function_definition
	: type_specifier fun_declarator
	{



	decl_struct* temp=new decl_struct;
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	temp->type=$1;

	//($2)->type=temp;
	cur->type=temp;

	//$$=$2;

	
	}
	compound_statement 


	;

	type_specifier
	: VOID 
	{$$="VOID";}	
	| INT
	{$$="INT";}
	| FLOAT 
	{$$="FLOAT";}
	;

	fun_declarator
	: IDENTIFIER '(' 
	{

	Symtab_elem * temp = new Symtab_elem;

	temp->name =  $1;
	temp->dec_type="Func";
	temp->offset =4;
	temp->size =0;
	for(int i=0;i<Symbol_table.size();i++)
	{
	if(Symbol_table[i]->name==$1)
	{
	string temp3 = "Redefinition of function "+temp->name;
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	temp->has_error=true;
	is_err=true;
	break;
	}

	}


	Symbol_table.push_back(temp);

	$$=temp;

	}
	parameter_list
	')' 



	| IDENTIFIER '(' ')' 
	{
	Symtab_elem * temp = new Symtab_elem;
	temp->name =  $1;
	temp->dec_type="Func";
	temp->offset =-4;
	temp->size =0;
	for(int i=0;i<Symbol_table.size();i++)
	{
	if(Symbol_table[i]->name==$1)
	{
	string temp3 = "Redefinition of function "+temp->name;
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	temp->has_error=true;
	is_err=true;
	break;
	}
	}
	Symbol_table.push_back(temp);
	$$=temp;
	}

	;

	parameter_list
	: parameter_declaration 
	| parameter_list ',' parameter_declaration 
	;

	parameter_declaration
	: type_specifier
	declarator{
	Symtab_elem * cur=Symbol_table.back();
	Symtab_elem * temp = new Symtab_elem;
	temp->dec_type="Param";
	//($2).type=$1;
	temp->name=($2)->type;
	temp->type = $2;
	temp->type->type=$1;
	if($1=="VOID")
	{
	temp->size = 0;

	string temp3 = "Parameter "+temp->name+" has been declared void";
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;

	}
	else if(($2)->float_ind)
	{
	temp->size = 0;

	string temp3 = "Size of array "+temp->name+" has non integral type";
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	temp->size = 4;
	for(int i=0;i<(($2)->indices).size();i++)
	{
	temp->size=temp->size*(($2)->indices[i]);
	}
	}
	temp->offset=cur->offset;
	cur->offset+=temp->size;
	cur->size=cur->offset;
	string temp_check=cur->dec_check(temp);
	if(temp_check=="1")
	{
	if($1!="VOID" && !(($2)->float_ind) )
	(cur->to_func).push_back(temp);
	}
	else
	{
	error_vec.push_back(temp_check);
	cout<<Scanner::line_num<<":"<<temp_check<<endl;
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}

	} 
	;

	declarator
	: IDENTIFIER
	{
	decl_struct * temp= new decl_struct;
	temp->type = $1;
	$$ = temp;

	} 
	| declarator '[' constant_expression ']' {
	(($1)->indices).push_back(stoi($3));
	$$=$1;
	if(($3).find(".")!=string::npos || ($3).find("e")!=string::npos)
	{
	($1)->float_ind=true;
	}

	}
	;


	constant_expression 
	: INT_CONSTANT
	{$$=$1;
	} 
	| FLOAT_CONSTANT
	{$$=$1;
	} 
	;

	compound_statement
	: '{' '}' 
	{list <stmtAst*> temp;
	stmtAst* t1=new Empty();
	temp.push_back(t1);
	$$=new block_statement(temp);
	ast_vec.push_back($$);} 
	| '{' declaration_list statement_list '}' 
	{$$=new block_statement($3);
	ast_vec.push_back($$);
	//($$)->print();
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	//print statement
	/*cur->print();
	cout<<endl;
	cur->print_child();
	cout<<"\n\n";*/
	}
	;

	statement_list
	: statement
	{
	list <stmtAst *> stmt_list;
	stmt_list.push_back($1);
	$$=stmt_list;
	}		
	| statement_list statement
	{
	$$=$1;
	($$).push_back($2);}	
	;

	statement
	: '{' statement_list '}'
	{
	$$=new block_statement($2);
	}
	| selection_statement
	{$$=$1;} 	
	| iteration_statement
	{$$=$1;} 	
	| assignment_statement	
	{$$=$1;}
	| RETURN expression ';'	
	{
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	//type casting
	if(($2)->type!=cur->type->type && ($2)->type!="error" && ($2)->type!="STRING" && ($2)->type!="VOID" && cur->type->type!="VOID")
	{
	expAst* tempex=new unOp($2,cur->type->type);
	tempex->type=($2)->type;
	$2=tempex;
	}
	//type casting

	if(cur->type->type=="VOID")
	{
	string temp3 = "Return-statement with a value, in function returning VOID.";
	cout<<Scanner::line_num<<":"<<temp3;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else if(($2)->type=="STRING" || ($2)->type=="VOID")
	{
	string temp3 = "Invalid return type. ";
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}


	$$=new returnNode($2);

	}
	| IDENTIFIER '(' ')' ';'
	{
	list <expAst*> var1;
	$$=new funCallst(var1,$1);
	bool check_dec=false;
	string type_temp;
	int comp_param=0;//comp_param=-1 means too few arguments
	for(int i=0;i<Symbol_table.size();i++)
	{
	if((Symbol_table[i])->name==$1)
	{//In this case, function definition should not have any arguments

	check_dec=true;
	type_temp=(Symbol_table[i])->type->type;
	if(((Symbol_table[i])->to_func).size()!=0)
	{
	if((((Symbol_table[i])->to_func)[0])->dec_type=="Param")
	comp_param=-1;
	}
	break;	
	}
	}

	if(!check_dec)
	{
	string fname=$1;
	error_vec.push_back("Function "+fname+" not declared.");
	cout<<Scanner::line_num<<":"<<"Function "+fname+" not declared.\n";
	$$->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	if(comp_param==-1)
	{
	string fname=$1;
	error_vec.push_back("Too few arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too few arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	$$->type=type_temp;
	}
	}

	}

	| IDENTIFIER '(' expression_list ')' ';'
	{

	$$=new funCallst($3,$1);
	if (($1)!= "printf")
	{
	bool check_dec=false;
	string type_temp;
	int comp_param=0;
	int ind=-1;
	for(int i=0;i<Symbol_table.size();i++)
	{
	if((Symbol_table[i])->name==$1)
	{
	ind=i;
	check_dec=true;
	type_temp=(Symbol_table[i])->type->type;
	//Just check the number of parameters with the function call
	//as the type of parameters can only be int or float
	for(int j=0;j<=($3).size();j++)
	{
	//cout<<"i="<<i<<" j="<<j<<endl;
	//cout<<"check1\n";
	if(((Symbol_table[i])->to_func).size()>=j+1)
	{
	//cout<<"check2\n";
	if(j==($3).size())
	{
	//cout<<"check3\n";
	if((((Symbol_table[i])->to_func)[j])->dec_type=="Param")
	comp_param=-1;
	}
	else if((((Symbol_table[i])->to_func)[j])->dec_type=="Local")
	{
	//cout<<"check4\n";
	comp_param=1;
	break;


	}
	}
	else if (j!=($3).size())
	{
	//cout<<"check5\n";
	comp_param=1;
	break;
	}
	}
	break;
	}
	}
	if(!check_dec)
	{
	string fname=$1;
	error_vec.push_back("Function "+fname+" not declared.\n");
	cout<<Scanner::line_num<<":"<<"Function "+fname+" not declared.\n";
	$$->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	if(comp_param==-1)
	{
	string fname=$1;
	error_vec.push_back("Too few arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too few arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else if(comp_param==1)
	{
	string fname=$1;
	error_vec.push_back("Too many arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too many arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{//comp_param=0,casting may be required
	int co=0;
	for(list<expAst*> :: iterator j=($3).begin();j!=($3).end();j++)
	{

	string tempt1=(((Symbol_table[ind])->to_func)[co])->type->type;
	if(tempt1!=(*j)->type && tempt1!="VOID" && tempt1!="STRING" && (*j)->type!="VOID" && (*j)->type!="STRING" && (*j)->type!="error")
	{

	cout<<endl;
	expAst* tempex=new unOp(*j,tempt1);
	tempex->type=(*j)->type;
	(*j)=tempex;

	}
	else if(((*j)->type=="VOID" || (*j)->type=="STRING") && ($$)->type!="error")
	{
	string fname=$1;
	error_vec.push_back("Argument with type "+(*j)->type+" are not allowed");
	cout<<Scanner::line_num<<":"<<"Argument with type "+(*j)->type+" are not allowed.\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	co++;
	}
	if(($$)->type!="error")
	{
	delete $$;
	$$=new funCallst($3,$1);
	$$->type=type_temp;
	}
	}
	}
	}
	};

	assignment_statement
	: ';'
	{$$=new Empty();} 								
	|  l_expression '=' expression ';'
	{
	//Type casting
	if(($1)->type!=($3)->type && ($1)->type!="error" && ($3)->type!="STRING")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	//Type casting
	$$=new ass($1,$3);
	if(!($1)->islexp)
	{
	($$)->type="error";
	handle(Scanner::line_num,"l_val required as left operand of assignment ",($1)->type,($3)->type);
	}
	else	
	{
	
	if(($1)->current_dim!=($1)->correct_dim && ($1)->type!="error") 
	{
	
	handle(Scanner::line_num,":Dimensions of "+($1)->get_name() +" does not match the declaration","INT","INT");
	($$)->type="error";
	}
	else
	{
	$$->type = check(($1)->type,($3)->type,2);
	if(($$)->type=="error")
	handle(Scanner::line_num," :Incompatible types " +($1)->type+" & "+($3)->type,($1)->type,($3)->type);	
	}
	
	}
	}
	;

	expression
	: logical_and_expression
	{$$=$1;

	} 
	| expression OR_OP logical_and_expression

	{
	string copy = ($1)->type;
	$$=new binOp($1,$3,"OR");
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num," :Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);

	}
	;

	logical_and_expression
	: equality_expression
	{
	$$=$1;

	}
	| logical_and_expression AND_OP equality_expression 
	{ 
	string shiv = ($1)->type;
	$$ = new binOp($3,$1,"AND");


	($$)->type = check(shiv,($3)->type,1);



	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +shiv+" & "+($3)->type,shiv,($3)->type);
	}
	;

	equality_expression
	: relational_expression 
	{$$=$1;
	}
	| equality_expression EQ_OP relational_expression
	{
	//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting

	string copy = ($1)->type;
	$$=new binOp($1,$3,"EQ_OP");

	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	| equality_expression NE_OP relational_expression
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"NE_OP");
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);
	}
	;
	relational_expression
	: additive_expression
	{$$=$1;}
	| relational_expression '<' additive_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"LT");
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	| 

	relational_expression '>' additive_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"GT");
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);
	}
	| 

	relational_expression LE_OP additive_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"LE_OP");
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	| 

	relational_expression GE_OP additive_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting

	string copy = ($1)->type;
	$$=new binOp($1,$3,"GE_OP");($$)->type = "INT";
	($$)->type = check(copy,($3)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	;

	additive_expression 
	: multiplicative_expression
	{$$=$1;}
	| 
	additive_expression '+' multiplicative_expression
	{
	//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"PLUS");
	($$)->type = check(copy,($3)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);} 
	| 
	additive_expression '-' multiplicative_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"MINUS");
	($$)->type = check(copy,($3)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	;

	multiplicative_expression
	: unary_expression
	{$$=$1;}
	| multiplicative_expression '*' unary_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"MULT");
	($$)->type = check(copy,($3)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num," :Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);}
	| multiplicative_expression '/' unary_expression 
	{//Type casting
	if(($1)->type=="FLOAT" && ($3)->type=="INT")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	else if(($3)->type=="FLOAT" && ($1)->type=="INT")
	{
	expAst* tempex=new unOp($1,($3)->type);
	tempex->type=($1)->type;
	$1=tempex;
	}
	//Type casting
	string copy = ($1)->type;
	$$=new binOp($1,$3,"DIV");
	($$)->type = check(copy,($3)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num," :Incompatible types " +copy+" & "+($3)->type,copy,($3)->type);
	}
	;
	unary_expression
	: postfix_expression  	
	{$$=$1;}			
	| unary_operator postfix_expression 
	{$$=new unOp($2,$1);
	if($1=="NOT")
	{
	($$)->type = check("INT",($2)->type,1);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible type " +($2)->type,"INT",($2)->type);}
	else
	{
	($$)->type = check("INT",($2)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible type " +($2)->type,"INT",($2)->type);
	}

	}

	;

	postfix_expression
	: primary_expression
	{$$=$1;}

	| IDENTIFIER '(' ')'
	{
	list <expAst*> var1;
	$$=new funCall(var1,$1);
	bool check_dec=false;
	string type_temp;
	int comp_param=0;//comp_param=-1 means too few arguments
	for(int i=0;i<Symbol_table.size();i++)
	{
	if((Symbol_table[i])->name==$1)
	{//In this case, function definition should not have any arguments

	check_dec=true;
	type_temp=(Symbol_table[i])->type->type;
	if(((Symbol_table[i])->to_func).size()!=0)
	{
	if((((Symbol_table[i])->to_func)[0])->dec_type=="Param")
	comp_param=-1;
	}
	break;
	}
	}

	if(!check_dec)
	{
	string fname=$1;
	error_vec.push_back("Function "+fname+" not declared.");
	cout<<Scanner::line_num<<":"<<"Function "+fname+" not declared.\n";
	$$->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	if(comp_param==-1)
	{
	string fname=$1;
	error_vec.push_back("Too few arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too few arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	$$->type=type_temp;
	}
	}

	}

	| IDENTIFIER '(' expression_list ')'
	{

	$$=new funCall($3,$1);
	bool check_dec=false;
	string type_temp;
	int comp_param=0;
	int ind=-1;
	for(int i=0;i<Symbol_table.size();i++)
	{
	if((Symbol_table[i])->name==$1)
	{
	ind=i;
	check_dec=true;
	type_temp=(Symbol_table[i])->type->type;
	//Just check the number of parameters with the function call
	//as the type of parameters can only be int or float
	for(int j=0;j<=($3).size();j++)
	{
	//cout<<"i="<<i<<" j="<<j<<endl;
	//cout<<"check1\n";
	if(((Symbol_table[i])->to_func).size()>=j+1)
	{
	//cout<<"check2\n";
	if(j==($3).size())
	{
	//cout<<"check3\n";
	if((((Symbol_table[i])->to_func)[j])->dec_type=="Param")
	comp_param=-1;
	}
	else if((((Symbol_table[i])->to_func)[j])->dec_type=="Local")
	{
	//cout<<"check4\n";
	comp_param=1;
	break;


	}
	}
	else if (j!=($3).size())
	{
	//cout<<"check5\n";
	comp_param=1;
	break;
	}
	}
	break;
	}
	}
	if(!check_dec)
	{
	string fname=$1;
	error_vec.push_back("Function "+fname+" not declared.\n");
	cout<<Scanner::line_num<<":"<<"Function "+fname+" not declared.\n";
	$$->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	if(comp_param==-1)
	{
	string fname=$1;
	error_vec.push_back("Too few arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too few arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else if(comp_param==1)
	{
	string fname=$1;
	error_vec.push_back("Too many arguments to function "+fname+".");
	cout<<Scanner::line_num<<":"<<"Too many arguments to function "+fname+".\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{//comp_param=0,casting may be required
	int co=0;
	for(list<expAst*> :: iterator j=($3).begin();j!=($3).end();j++)
	{

	string tempt1=(((Symbol_table[ind])->to_func)[co])->type->type;
	if(tempt1!=(*j)->type && tempt1!="VOID" && tempt1!="STRING" && (*j)->type!="VOID" && (*j)->type!="STRING" && (*j)->type!="error")
	{
	expAst* tempex=new unOp(*j,tempt1);
	tempex->type=(*j)->type;
	(*j)=tempex;
	}
	else if(((*j)->type=="VOID" || (*j)->type=="STRING") && ($$)->type!="error")
	{
	string fname=$1;
	error_vec.push_back("Argument with type "+(*j)->type+" are not allowed");
	cout<<Scanner::line_num<<":"<<"Argument with type "+(*j)->type+" are not allowed.\n";
	($$)->type="error";
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	co++;
	}
	if(($$)->type!="error")
	{
	delete $$;
	$$=new funCall($3,$1);
	$$->type=type_temp;
	}
	}
	}
	}

	| l_expression INC_OP
	{
	$$=new unOp($1,"PP");
	//if(($1)->isarray)
	//{
	if(($1)->current_dim!=($1)->correct_dim && ($1)->type!="error")
	{
	//cout<<"yo2\n";
	handle(Scanner::line_num,":Dimensions of "+($1)->get_name() +" does not match the declaration","INT","INT");
	($$)->type="error";
	}
	else
	{

	($$)->type = check("INT",($1)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible type " +($1)->type,"INT",($1)->type);
	}
	//}
	/*else
	{

	($$)->type = check("INT",($1)->type,0);
	if(($$)->type=="error")
	handle(Scanner::line_num,":Incompatible type " +($1)->type,"INT",($1)->type);
	}*/
	}
	;

	primary_expression
	: l_expression
	{$$=$1;

	if(($1)->current_dim!=($1)->correct_dim &&($1)->type!="error")
	{
	//cout<<"yo3\n";
	handle(Scanner::line_num,":Dimensions of "+($1)->get_name() +" does not match the declaration","INT","INT");
	($$)->type="error";
	}

	}
	|

	l_expression '=' expression
	{//Type casting
	if(($1)->type!=($3)->type && ($1)->type!="error" && ($3)->type!="STRING")
	{
	expAst* tempex=new unOp($3,($1)->type);
	tempex->type=($3)->type;
	$3=tempex;
	}
	//Type casting
	$$=new binOp($1,$3,"Assign_exp");
	if(!($1)->islexp)
	{
	($$)->type="error";
	handle(Scanner::line_num,"l_val required as left operand of assignment ",($1)->type,($3)->type);
	}
	else
	{
	($$)->type = check(($1)->type,($3)->type,2);
	if(($$)->type=="error")
	handle(Scanner::line_num," :Incompatible types " +($1)->type+" & "+($3)->type,($1)->type,($3)->type);
	}

	//if(($1)->isarray)
	//{
	if(($1)->current_dim!=($1)->correct_dim && ($1)->type!="error")
	{
	//cout<<"yo\n";
	handle(Scanner::line_num,":Dimensions of "+($1)->get_name() +" does not match the declaration","INT","INT");
	($$)->type="error";
	}
	//}
	}
	| INT_CONSTANT
	{int tempint=1;
	$$=new intConst($1);
	($$)->type="INT";}
	| FLOAT_CONSTANT
	{float tempint=1;
	$$=new floatConst($1);
	($$)->type="FLOAT";}
	| STRING_LITERAL
	{$$=new stringConst($1);
	($$)->type="STRING";}
	| '(' expression ')'
	{$$=$2;} 	
	;

	l_expression
	: IDENTIFIER
	{
	arr_dim=0;
	//$$=new ident($1);
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	bool check_dec=false;
	string type_temp;
	for(int i=0;i<(cur->to_func).size();i++)
	{
	if(((cur->to_func)[i])->name==$1)
	{
	check_dec=true;
	type_temp=((cur->to_func)[i])->type->type;
	cor_dim=(((cur->to_func)[i])->type->indices).size();

	}
	}
	$$=new ident($1,arr_dim,cor_dim);
	if(!check_dec)
	{
	string vname=$1;
	error_vec.push_back("Variable "+vname+" not declared.");
	cout<<Scanner::line_num<<":"<<"Variable "+vname+" not declared.\n";
	($$)->type="error";
	//cout<<"yo6"<<endl;
	($$)->islexp=true;
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else
	{
	($$)->type=type_temp;
	($$)->islexp=true;
	}
	}
	| l_expression '[' expression ']'
	{arr_dim++;
	string cop=($1)->type;
	$$=new index($1,$3,($1)->current_dim+1,($1)->correct_dim);
	//cout<<"yoyo "<<($3)->type<<" "<<cop<<endl;
	if(($3)->type=="INT" && cop!="error")
	{
	($$)->type=cop;
	($$)->islexp=true;
	}
	else if(cop=="error")
	{
	//cout<<"yo7\n";
	($$)->type="error";
	($$)->islexp=true;
	}
	else
	{
	($$)->type="error";
	handle(Scanner::line_num,":Array subscript is not an integer ","INT",($3)->type);
	($$)->islexp=true;
	}
	//cout<<"yo4 "<<($$)->type<<endl;
	} 	
	;


	expression_list
	: expression
	{list <expAst*> l;
	l.push_back($1);
	$$=l;

	}
	| expression_list ',' expression
	{
	$$=$1;
	($$).push_back($3);
	}
	;
	unary_operator
	: '-'
	{
	$$="UMINUS";
	}	
	| '!' 
	{
	$$="NOT";
	}	
	;

	selection_statement
	: IF '(' expression ')' statement ELSE statement 
	{
	$$=new ifNode($3,$5,$7);
	}
	;

	iteration_statement
	: WHILE '(' expression ')' statement
	{$$=new whileNode($3,$5);} 	
	| FOR '(' expression ';' expression ';' expression ')' statement
	{$$=new forNode($3,$5,$7,$9);}  
	;

	declaration_list
	: 
	{
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	cur->offset=-4;
	}
	declaration

	| declaration_list declaration
	;

	declaration
	: type_specifier
	{
	temp_type=$1;
	}
	declarator_list';'
	;

	declarator_list
	: declarator
	{
	//cout<<"yoyo\n";
	if(temp_type  == "VOID"){
	string temp = "Variable "+($1)->type+" has been declared void";
	cout<<Scanner::line_num<<":"<<temp<<endl;
	error_vec.push_back(temp);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else if(($1)->float_ind)
	{

	string temp3 = "Size of array "+($1)->type+" has non integral type";
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else{
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	//cout<<cur->name<<endl;
	Symtab_elem * temp = new Symtab_elem;
	//($2).type=$1;
	temp->name=($1)->type;
	temp->dec_type="Local";
	temp->type = $1;
	temp->type->type=temp_type;
	if(temp_type=="VOID")
	{
	temp->size = 0;
	}
	else
	{
	temp->size = 4;
	for(int i=0;i<(($1)->indices).size();i++)
	{
	temp->size=temp->size*(($1)->indices[i]);
	}
	}
	if(temp->size>4)//array
	{
	temp->offset=cur->offset+4-temp->size;
	cur->offset=temp->offset-4;
	cur->size=-cur->offset;
	}
	else
	{
	temp->offset=cur->offset;
	cur->offset-=temp->size;
	cur->size=-cur->offset;
	}
	//cout<<"size of to_func "<<(cur->to_func).size()<<endl;
	string temp_check=cur->dec_check(temp);
	if(temp_check=="1")
	{
	(cur->to_func).push_back(temp);
	}
	else
	{
	error_vec.push_back(temp_check);
	cout<<Scanner::line_num<<":"<<temp_check<<endl;
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	//cout<<"size of to_func "<<(cur->to_func).size()<<endl;
	}

	}

	| declarator_list ',' declarator 

	{
	//cout<<"yo\n";
	if(temp_type  == "VOID"){
	string temp = "Variable "+($3)->type+" has been declared void";
	cout<<Scanner::line_num<<":"<<temp<<endl;
	error_vec.push_back(temp);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else if(($3)->float_ind)
	{

	string temp3 = "Size of array "+($3)->type+" has non integral type";
	cout<<Scanner::line_num<<":"<<temp3<<endl;
	error_vec.push_back(temp3);
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	else{
	Symtab_elem * cur=Symbol_table[Symbol_table.size()-1];
	Symtab_elem * temp = new Symtab_elem;
	//($2).type=$1;
	temp->name=($3)->type;
	temp->dec_type="Local";
	temp->type = $3;
	temp->type->type=temp_type;
	if(temp_type=="VOID")
	{
	temp->size = 0;
	}
	else
	{
	temp->size = 4;
	for(int i=0;i<(($3)->indices).size();i++)
	{
	temp->size=temp->size*(($3)->indices[i]);
	}
	}
if(temp->size>4)//array
	{
	temp->offset=cur->offset+4-temp->size;
	cur->offset=temp->offset-4;
	cur->size=-cur->offset;
	}
	else
	{
	temp->offset=cur->offset;
	cur->offset-=temp->size;
	cur->size=-cur->offset;
	}
	string temp_check=cur->dec_check(temp);
	if(temp_check=="1")
	{
	(cur->to_func).push_back(temp);
	}
	else
	{
	error_vec.push_back(temp_check);
	cout<<Scanner::line_num<<":"<<temp_check<<endl;
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	}
	}

	}
	;

