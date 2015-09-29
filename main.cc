#include <iostream>
#include "Scanner.h"
#include "Parser.h"

using namespace std;
extern vector<Symtab_elem *> Symbol_table;
extern vector <stmtAst *> ast_vec;
extern bool is_err;
extern void gen_code(string );
extern void init();
extern Symtab_elem * running_symb;
extern int extra_off;
extern int  exx;
extern void init1();
int main (int argc, char** arg)
{
  Parser parser;
  parser.parse();
  bool mainex=false;
  for(int i=0;i<Symbol_table.size();i++)
  {
	  if(Symbol_table[i]->name=="main")
	  {
		  mainex=true;
		  break;
	  }
  }
  if(!mainex)
  {
	  cout<<"  :function main is not declared.\n";
	  is_err=true;
  }
  cout<<"\nGlobal symbol table"<<endl;
  //cout<<"size"<<"\t"<<"name"<<"\t"<<"offset"<<"\t"<<"declared_as\t"<<"type"<<endl;
  cout<<"name"<<"\t"<<"declared_as\t"<<"type"<<endl;
  for(int i=0;i<Symbol_table.size();i++)
  {
	  Symbol_table[i]->print();
  }
  cout<<"\n\n";
  for(int i=0;i<Symbol_table.size();i++)
  {
	  Symbol_table[i]->print_child();
	  cout<<"\n\n";
  }
  
  //cout<<"\n\n AST\n";
  for(int i=0;i<ast_vec.size();i++)
  {
	  if(Symbol_table[i]->has_error==true)
	  {
		  cout<<"Error(s) in function "<<Symbol_table[i]->name<<endl;
	  }
	  else
	  {
		  cout<<"AST for function "<<Symbol_table[i]->name<<endl;
		  ast_vec[i]->print();
		  cout<<endl;
	  }
	  cout<<endl;
  }

  init();
  if(is_err == false){
    for(int i=0;i<Symbol_table.size();i++)
  {
    running_symb = Symbol_table[i];
    extra_off =0;
    exx = ((running_symb->to_func).back())->offset  -4;
    init1();
     gen_code(Symbol_table[i]->name);
     
  }
  }
}


