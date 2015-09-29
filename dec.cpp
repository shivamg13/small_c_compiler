#include <iostream>
#include <vector>
#include <list>
#include <string>
#include <sstream>
#include <map>
#include <set>
#include <stack>
#include <iomanip>
#include <fstream>
using namespace std;

extern ofstream mach_code;
extern map<int, set<string> > reg2var;

extern map<int,string> registers;
extern set<int> freeregs;
extern int temp_vars;
extern void load_it(string name, int r,bool j);
extern int exx;
extern int giveReg();
extern int extra_off;
extern int get_score(int a);
extern void clear_reg(int r);
extern void clear_temp(string);
extern int giveReg_care(int r);
extern string put_temp(int , string);
extern int labels;
extern void compar(string op, string notop, int r);
extern string next_label();
extern void clear_all_reg();
struct gen_ret{
	bool is_const;
	string str;
	float val;
	
	string type;
	string var_name;
	int offset;
	
	gen_ret(){
		is_const = false;
	}	
};

struct varinfo{
	bool is_local;
	int mem_loc;
	set <int> regs;
	string type;
	int offset;
	varinfo(){mem_loc =-1;}
};
extern map<string, varinfo > var2reg;

struct decl_struct{

	vector<int>  indices;
bool float_ind;//whether any index is float-for error detection purpose
string type;//this field temporarily stores the id(name)
decl_struct()
{
	float_ind=false;
}
void print()
{
	
	cout<<type;
	
	for(int i=0;i<indices.size();i++)
	{
		cout<<"["<<indices[i]<<"]";
	}
}

};

struct Symtab_elem {

	int size;
	string name;

	decl_struct* type;
string dec_type;//local,param,func
int offset;
bool has_error;
vector<Symtab_elem *>  to_func;

Symtab_elem()
{
	has_error=false;
}

void print()
{
	if(dec_type=="Func")
		cout<<name<<"\t\t"<<dec_type<<"\t\t";
	else
		cout<<size<<"\t\t"<<name<<"\t\t"<<offset<<"\t\t"<<dec_type<<"\t\t";
	
	type->print();	
	cout<<endl;
}
void print_child()
{
	cout<<"Symbol table for function "<<name<<endl;
	cout<<"size"<<"\t"<<"name"<<"\t"<<"offset"<<"\t"<<"declared_as\t"<<"type"<<endl;
	//cout<<"size of to_func is "<<to_func.size()<<endl;
	for(int i=0;i<to_func.size();i++)
	{
		//cout<<"entered\n";
		(to_func[i])->print();
	}
}

string dec_check(Symtab_elem* in)
{//Checks whether declaration follows all rules or not
	string ret_val="1";

//Redeclaration check	
	for(int i=0;i<to_func.size();i++)
	{
		if((to_func[i])->name==in->name)
		{

			if(in->dec_type=="Func")
		{//Incomplete
		//Check if all the parameter types match or not for	conflict
			ret_val="Redefinition of "+in->name;
			break;
		}
		else
		{
			ret_val="Redeclaration of "+in->name;
			break;
		}
		
	}
}



return ret_val;	
}

};

extern vector<Symtab_elem *> Symbol_table;
extern Symtab_elem * running_symb;
class abstract_astnode
{
public:
	virtual void print ()  = 0;
	virtual gen_ret generate_asm () = 0;
	string type,typeop;
	bool islexp;
	abstract_astnode()
	{
		islexp=false;
	}

};

class stmtAst : public abstract_astnode
{
public:

	virtual void print ()  = 0;
	virtual gen_ret generate_asm () = 0;
};

class expAst : public abstract_astnode
{
public:
	bool isarray;
	bool isconst;
	virtual void print ()  = 0;
	virtual gen_ret generate_asm (){};
	virtual string get_name(){};

	expAst()
	{
		isarray=false;
		isconst=false;
	}
};
extern gen_ret arr(expAst* var);
extern gen_ret arr_care(int reg,expAst* var);

class arrayRef : public expAst
{

public:
	int current_dim;
	int correct_dim;
	virtual void print()=0;
	virtual string get_name(){};
	gen_ret generate_asm(){}
	arrayRef()
	{
		current_dim=0;
		correct_dim=0;
	}
	arrayRef(int arg1,int arg2)
	{
		current_dim=arg1;
		correct_dim=arg2;
	}	
};

/*class seq : public stmtAst
{
	public:
	stmtAst* var1;
	stmtAst* var2;
	
	
	seq(stmtAst* arg1,stmtAst* arg2)
	{
		var1=arg1;
		var2=arg2;
	}
	void print()
	{
		cout<<"(";
		var1->print();
		cout<<")(";
		var2->print();
		cout<<")";
	}
	
};*/

	class block_statement : public stmtAst
	{
	public:
		list <stmtAst *> stmt_list;
		block_statement(list <stmtAst *> l)
		{
			stmt_list=l;
		}
		gen_ret generate_asm(){
			
			for(list <stmtAst*> :: iterator i=stmt_list.begin();i!=stmt_list.end();i++)
			{

				(*i)->generate_asm();
				


			}


			gen_ret t;

			return t;
		}
		void print()
		{
			cout<<"(Block[";
				list <stmtAst*> :: iterator temp=stmt_list.end();
				temp--;
				for(list <stmtAst*> :: iterator i=stmt_list.begin();i!=stmt_list.end();i++)
				{
					(*i)->print();
					if (i!=temp)
						cout<<endl;

				}
				cout<<"])";
}
};

class ass : public stmtAst
{
public:
	expAst* var1;
	expAst* var2;
	
	gen_ret generate_asm(){
		gen_ret left,right;
		if(var2->isarray == true){

			right = arr(var2);
		}
		if(var1->isarray == true){
			
			gen_ret  left = var1->generate_asm();
			cerr<<freeregs.size()<<endl;
			
			int r1;

			if(var2reg[left.var_name].regs.size()>0){
				r1 = *var2reg[left.var_name].regs.begin();

				
			}
			else{
				r1 = giveReg();
				
				load_it(left.var_name,r1,false);
			}

			for(int i=0;i<running_symb->to_func.size();i++){
				Symtab_elem * curr_row = running_symb->to_func[i];
		           if(var1->get_name() == curr_row->name){ //got it
		           	mach_code<<"\taddi("+to_string(curr_row->offset)+","+registers[r1]+");\n";
		           	break;
		           }
		       }

		       mach_code<<"\taddi(ebp,"+registers[r1]+");\n";
		       gen_ret right;
		       int r2;
		       if(var2->isarray == false)
		       	right = var2->generate_asm();
		       else{
		       	r2 = *var2reg[right.var_name].regs.begin();
		       }

		       
		       if(right.is_const == true){
		        //	cerr<<registers[r1]<<" "<<registers[r2]<<endl;
		       	r2 = giveReg_care(r1);
		       	cerr<<registers[r1]<<" "<<registers[r2]<<endl;
		       	if(right.type == "INT")
		       		mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r2]+");\n";
		       	else
		       		mach_code<<"\tmove("+to_string(right.val)+","+registers[r2]+");\n";	


		       }
		       else if(var2reg[right.var_name].regs.size()>0){

		       	r2 = *var2reg[right.var_name].regs.begin();


		       }
		       else if(var2->isarray == false){
		       	r2 = giveReg_care(r1);

					load_it(right.var_name,r2,true); //r2 has register to be stored
				}
				if(var2reg[left.var_name].regs.size() == 0){
					r1 = giveReg_care(r2);
					load_it(left.var_name,r1,false);
				}
				
				mach_code<<"\tstorei("+registers[r2]+",ind("+registers[r1]+"));\n";
				if(var2reg[left.var_name].is_local == false){clear_temp(left.var_name);}
				if(var2reg[right.var_name].is_local == false)clear_temp(right.var_name);
				gen_ret t;
				return t;

			}
			
			left = var1->generate_asm();
			if(var2->isarray == false)
				right = var2->generate_asm();
			gen_ret t;
			int r1;
			if(right.is_const == true){

				if(var2reg[left.var_name].regs.size() >0){
					int r1 = *var2reg[left.var_name].regs.begin();
					clear_reg(r1);
					freeregs.erase(r1);	
					if(right.type == "FLOAT")
						mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
					else
						mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
					var2reg[left.var_name].regs.insert(r1);
					var2reg[left.var_name].mem_loc =-1;
					reg2var[r1].insert(left.var_name);

				}
				else{
					int r1 = giveReg();
					if(right.type == "FLOAT")
						mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
					else
						mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
					
					var2reg[left.var_name].regs.insert(r1);
					var2reg[left.var_name].mem_loc =-1;
					reg2var[r1].insert(left.var_name);
				}
				t.var_name = left.var_name;
				return t;
			}



			if(var2reg[right.var_name].regs.size() == 0){
				r1 = giveReg();
				load_it(right.var_name,r1,true);
			}
			var2reg[left.var_name].regs.clear();
			var2reg[left.var_name].regs = var2reg[right.var_name].regs;
			var2reg[left.var_name].mem_loc =-1;
			
			
			reg2var[*var2reg[right.var_name].regs.begin()].insert(left.var_name);

			if(var2reg[right.var_name].is_local == false)
				clear_temp(right.var_name);

			t.var_name = right.var_name;
			return t;
		}
		ass(expAst* arg1,expAst* arg2)
		{

			var1=arg1;
			var2=arg2;

		}
		virtual void print()
		{
			cout<<"(Ass";
				var1->print();
				cout<<"";
				var2->print();
				cout<<")";
}

};

class Empty : public stmtAst
{
public:	
	gen_ret generate_asm(){

		gen_ret t;
		return t;
	}
	void print()
	{
		cout<<"(Empty)";
	}	
};

class returnNode : public stmtAst
{
public:
	expAst* var1;
	gen_ret generate_asm(){
		gen_ret tempgen;
		if(!var1->isarray)
		tempgen=var1->generate_asm();
	else tempgen = arr(var1);
		mach_code<<"\tmove(ebp,esp);\n";

		mach_code<<"\taddi(4,esp);\n";
		int numparam=0;
		for(int i=0;i<(running_symb->to_func).size();i++)
		{
			if(((running_symb->to_func)[i])->dec_type=="Param")
				numparam++;
			else
				break;
		}
			//TODO: Store the return value from tempgen to ind(esp,(numparam)*4)
		if(tempgen.is_const == true){
			if(tempgen.type == "INT")
				mach_code<<"\tstorei("+to_string(((int)tempgen.val))+",ind(esp,"<<(numparam)*4<<"));\n";
			else
				mach_code<<"\tstoref("+to_string(tempgen.val)+",ind(esp,"<<(numparam)*4<<"));\n";
		}
		else if(var2reg[tempgen.var_name].regs.size() >0){
			if(var2reg[tempgen.var_name].type == "INT")
				mach_code<<"\tstorei("+registers[*var2reg[tempgen.var_name].regs.begin()]+",ind(esp,"<<(numparam)*4<<"));\n";	
			else
				mach_code<<"\tstoref("+registers[*var2reg[tempgen.var_name].regs.begin()]+",ind(esp,"<<(numparam)*4<<"));\n";	

			if(var2reg[tempgen.var_name].is_local == false)clear_temp(tempgen.var_name);
		}
		else{
			int r =0;
			load_it(tempgen.var_name,0,false);
			if(var2reg[tempgen.var_name].type == "INT")
				mach_code<<"\tstorei(eax,ind(esp,"<<(numparam)*4<<"));\n";
			else
				mach_code<<"\tstoref(eax,ind(esp,"<<(numparam)*4<<"));\n";
		}
		mach_code<<"\tloadi(ind(ebp),ebp);\n";


		mach_code<<"\treturn;\n";
		gen_ret temppp;
		return temppp;

	}
	returnNode(expAst* arg1)
	{
		var1=arg1;
	}
	void print()
	{
		cout<<"(RETURN";
			var1->print();
			cout<<")";
}

};

class ifNode : public stmtAst
{
public:
	expAst* var1;
	stmtAst* var2;
	stmtAst* var3;
	gen_ret generate_asm(){

		string lab3 = next_label(),lab2 = next_label();
		
		gen_ret cond;

		if(var1->isarray == false)
			cond = var1->generate_asm();
		else{
			cond = arr(var1);
		}

		int r1 = *var2reg[cond.var_name].regs.begin();
		if(var2reg[cond.var_name].is_local == false)clear_temp(cond.var_name);
		clear_all_reg();
		mach_code<<"\tpushi(esp);\n";
		int tempextra=extra_off;
		//mach_code<<"//start if extraoff1 is"<<extra_off<<"\n";
		extra_off+=4;
		mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
		mach_code<<"\tje("+lab2+");\n";

		var2->generate_asm();

		clear_all_reg();
		mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
		//mach_code<<"\taddi(4,esp);\n";
		extra_off=tempextra+4;
		//mach_code<<"//end if extraoff1 is"<<extra_off<<"\n";
		mach_code<<"\tj("+lab3+");\n";

		mach_code<<lab2+" :\n";
		
		var3->generate_asm();
		clear_all_reg();
		mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
		//mach_code<<"\taddi(4,esp);\n";
		extra_off=tempextra+4;
		//mach_code<<"//end else extraoff1 is"<<extra_off<<"\n";
		mach_code<<lab3+" :\n";
		gen_ret t;
		return t;

	}
	ifNode(expAst* arg1,stmtAst* arg2,stmtAst* arg3)
	{
		var1=arg1;
		var2=arg2;
		var3=arg3;
	}
	void print()
	{
		cout<<"(IF";
			var1->print();
			cout<<"\n";
			var2->print();
			cout<<"\nELSE\n";
			var3->print();
			cout<<")";
}

};

class whileNode : public stmtAst
{
public:
	expAst* var1;
	stmtAst* var2;
	gen_ret generate_asm(){

		string lab1 = next_label();
		string lab2 = next_label();
		clear_all_reg();
		mach_code<<lab1+" :\n";
		gen_ret cond;
		if(!var1->isarray)
			cond = var1->generate_asm();
		else cond= arr(var1);
		int r1 = *var2reg[cond.var_name].regs.begin();
		if(var2reg[cond.var_name].is_local == false)clear_temp(cond.var_name);
		clear_all_reg();
		mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
		mach_code<<"\tje("+lab2+");\n";
		mach_code<<"\tpushi(esp);\n";
		int tempextra=extra_off;
		extra_off+=4;
		var2->generate_asm();

		clear_all_reg();
		mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
		mach_code<<"\taddi(4,esp);\n";
		extra_off=tempextra;
		mach_code<<"\tj("+lab1+");\n";
		mach_code<<lab2+" :\n";
		gen_ret t;
		return t;

	}
	whileNode(expAst* arg1,stmtAst* arg2)
	{
		var1=arg1;
		var2=arg2;
	}
	void print()
	{
		cout<<"(WHILE";
			var1->print();
			cout<<"\n";
			var2->print();
			cout<<")";

}

};

class forNode : public stmtAst
{
public:
	expAst* var1;
	expAst* var2;
	expAst* var3;
	stmtAst* var4;
	gen_ret generate_asm(){

		gen_ret t;
		if(!var1->isarray)
			var1->generate_asm();
		else arr(var1);
		string lab = next_label();
		clear_all_reg();
		mach_code<<lab+": \n";
		string lab1 = next_label();
		gen_ret cond;
		if(!var2->isarray)
			cond = var2->generate_asm();
		else cond = arr(var2);
		int r1 = *var2reg[cond.var_name].regs.begin();
		if(var2reg[cond.var_name].is_local == false)clear_temp(cond.var_name);
		clear_all_reg();
		mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
		mach_code<<"\tje("+lab1+");\n";
		mach_code<<"\tpushi(esp);\n";
		int tempextra=extra_off;
		extra_off+=4;
		var4->generate_asm();

		if(!var3->isarray)
			var3->generate_asm();
		else arr(var3);
		clear_all_reg();
		mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
		mach_code<<"\taddi(4,esp);\n";
		extra_off=tempextra;
		mach_code<<"\tj("+lab+");\n";

		mach_code<<lab1+" :\n";
		return t;

	}
	forNode(expAst* arg1,expAst* arg2,expAst* arg3,stmtAst* arg4)
	{
		var1=arg1;
		var2=arg2;
		var3=arg3;
		var4=arg4;
	}
	void print()
	{
		cout<<"(FOR";
			var1->print();
			cout<<"\n";
			var2->print();
			cout<<"\n";
			var3->print();
			cout<<"\n";
			var4->print();
			cout<<")";
}

};

class funCallst : public stmtAst
{
public:
	list <expAst*> var1;
	string var2;

	funCallst(list <expAst*> arg1,string arg2)
	{
		var2=arg2;
		for(list <expAst*> :: iterator i=arg1.begin();i!=arg1.end();i++)
		{
			var1.push_back(*i);
		}
	}
	gen_ret generate_asm()
	{   
		
		
		list<expAst*> :: iterator it;
		
		if(var2 == "printf"){

			for(it = var1.begin(); it != var1.end(); it++){

				if((*it)->isarray == true){
					//int r1 = giveReg();
					//cerr<<"save me\n";
					gen_ret yo = arr(*it);
					int r1 = *var2reg[yo.var_name].regs.begin();
					if(yo.type == "INT"){
						mach_code<<"\tprint_int("+registers[r1]+");\n";
					}
					else if(yo.type == "FLOAT"){
						mach_code<<"\tprint_float("+registers[r1]+");\n";
					}

					
					continue;
				}
				gen_ret yo = (*it)->generate_asm();
				
				if(yo.is_const == true){
					if((*it)->type == "INT"){
						mach_code<<"\tprint_int("+to_string(((int)yo.val))+");\n";
					}
					else if((*it)->type == "FLOAT"){
						mach_code<<"\tprint_float("+to_string(yo.val)+");\n";
					}
					else{
						

						mach_code<<"\tprint_string("+yo.str+");\n";
					}
				}
				else if(var2reg[yo.var_name].regs.size() ==0){
					int r1 = giveReg();
					load_it(yo.var_name,r1,false);
					if(var2reg[yo.var_name].type  == "INT"){
						mach_code<<"\tprint_int("+registers[r1]+");\n";
					}
					else{
						mach_code<<"\tprint_float("+registers[r1]+");\n";
					}
					
				}
				else {
					if((*it)->type == "INT"){
						mach_code<<"\tprint_int("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
					else if((*it)->type == "FLOAT"){
						mach_code<<"\tprint_float("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
					else{
						mach_code<<"\tprint_string("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
				}
				
				// Special print methods

			}

			gen_ret t;
			return t;
		}

		vector<gen_ret> all_rets;
		for(it = var1.begin(); it!= var1.end(); it++){
			int r1; 

			if((*it)->isarray == true){
				//r1 = giveReg();

				gen_ret t =arr(*it);
				all_rets.push_back(t);
				
			}
			else {

				gen_ret temp1 = (*it)->generate_asm();
				all_rets.push_back(temp1);
				
			}
			
		}

		



		vector<int> stored_regs;
		clear_reg(3);
		for(int i=0; i<4; i++){
			if(reg2var[i].size()>0){
				stored_regs.push_back(i);
				if(var2reg[*reg2var[i].begin()].type == "INT")
					mach_code<<"\tpushi("+registers[i]+");\n";
				else
					mach_code<<"\tpushf("+registers[i]+");\n";	
			}
			//extra_off += 4;
		}

		mach_code<<"\taddi(-4,esp);\n";//Space for return value

		for(int i=all_rets.size()-1; i>=0; i--){
			if(all_rets[i].is_const == true){
				
				if(all_rets[i].type == "INT")
					mach_code<<"\tpushi("+to_string(((int)all_rets[i].val))+");\n";
				else
					mach_code<<"\tpushf("+to_string(all_rets[i].val)+");\n";
				//extra_off += 4;
			}
			else if(var2reg[all_rets[i].var_name].regs.size() >0){
				if(var2reg[all_rets[i].var_name].type == "INT")
					mach_code<<"\tpushi("+registers[*var2reg[all_rets[i].var_name].regs.begin()]+");\n";
				else
					mach_code<<"\tpushf("+registers[*var2reg[all_rets[i].var_name].regs.begin()]+");\n";
				//extra_off += 4;
				if(var2reg[all_rets[i].var_name].is_local == false)clear_temp(all_rets[i].var_name);
			}
			else{
				int r =3;
				load_it(all_rets[i].var_name,3,false);
				mach_code<<"\taddi(-4,esp);\n";
				if(var2reg[all_rets[i].var_name].type == "INT")
					mach_code<<"\tstorei(edx,ind(esp));\n";
				else
					mach_code<<"\tstoref(edx,ind(esp));\n";
			}

		}

		mach_code<<"\t"<<var2<<"();\n";

		//Restore stack and registers
		int decsize=var1.size()*4;
		mach_code<<"\taddi("<<decsize<<",esp);\n";
		for(int i=stored_regs.size()-1;i>=0;i--)
		{   
			if(reg2var.find(stored_regs[i]) == reg2var.end() || reg2var[stored_regs[i]].size() ==0)
				continue;
			if(var2reg[*reg2var[stored_regs[i]].begin()].type == "INT")
			{
				mach_code<<"\tloadi(ind(esp,"+to_string(((int)stored_regs.size()-i)*4)<<"),"<<registers[stored_regs[i]]<<");\n";
			}
			else
			{
				mach_code<<"\tloadf(ind(esp,"+to_string((stored_regs.size()-i)*4)<<"),"<<registers[stored_regs[i]]<<");\n";
			}		
		}
		//int tempextra=extra_off;
		//extra_off += ((stored_regs.size()+1)*4);

		gen_ret ttemp;

		int r3 = 3;
		
		string ttype1;
		for(int i=0;i<Symbol_table.size();i++)
		{
			if(Symbol_table[i]->name==var2)
			{
				ttype1=Symbol_table[i]->type->type;
			}
		}
		//mach_code<<"\t///////exx="<<exx<<endl;
		//mach_code<<"\t///////tempextra="<<tempextra<<endl;
		//mach_code<<"\t///////stored="<<(int)((stored_regs.size())*4)<<endl;
		//mach_code<<"\t///////final="<<(exx-(int)((stored_regs.size())*4)-tempextra )<<endl;
		if(ttype1=="INT")
		{

			mach_code<<"\tloadi(ind(esp),"+registers[r3]+");\n";	
			ttemp.var_name = put_temp(r3,"INT");
		}
		else
		{
			mach_code<<"\tloadf(ind(esp),"+registers[r3]+");\n";	
			ttemp.var_name = put_temp(r3,"FLOAT");
		}
		 //dont know type
		//TODO:store the value at esp in a register and set ttemp accordingly

		decsize=(stored_regs.size()+1)*4;
		mach_code<<"\taddi("<<decsize<<",esp);\n";
		
		return ttemp;
	}
	void print()
	{
		//cout<<var2<<"(";//check what to print
		cout<<"(FUNCALL(Id \""<<var2<<"\")(Arguments(";
			for(list <expAst*> :: iterator i=var1.begin();i!=var1.end();i++)
			{

				(*i)->print();

			}
			cout<<")))";
}

};

class binOp : public expAst
{
public:
	expAst* var1;
	expAst* var2;
	string var3;
	//string typeop;

	gen_ret generate_asm(){

		gen_ret left;
		if(var1->isarray == false)
			left = var1->generate_asm();
		gen_ret right;
     // if( !(var3 == "AND" || var3 == "OR") && var1->isarray == false && var2->isarray == false )
     //  right = var2->generate_asm();

		if(var3 != "Assign_exp"){
			int r5=-1;
			if(var1->isarray){
	   	   	 //  r5 = giveReg();

				left = arr(var1);
				r5 = *var2reg[left.var_name].regs.begin();
			}

			if(var2->isarray){

				if(r5 == -1)
					right = arr(var2);
				else right = arr_care(r5,var2);


			}else{
				bool u =true;
	   	   if(!(var3 == "AND" || var3 == "OR")){u = false;right = var2->generate_asm();}///////important##$$##
	   	   if(var2->isconst == true && u)right = var2->generate_asm();
	   	}     
	   }
	   else if(var3 == "Assign_exp" && var2->isarray == true){
     			//int r3 = giveReg();
	   	right = arr(var2);
	   }
	   else if(var3 == "Assign_exp" && var2->isarray == false)right = var2->generate_asm();
	   bool ftype = true;
	   if(var3.size() >3 && var3.substr(var3.size()-3) == "INT") ftype = false;
	   if(var3 == "Assign_exp"){
	   	int r1;
	   	gen_ret t;
   	   	 //mach_code<<"fdds\n";
	   	if(var1->isarray == true){

	   		gen_ret  left = var1->generate_asm();
	   		int r1;
	   		if(var2reg[left.var_name].regs.size()>0){
	   			r1 = *var2reg[left.var_name].regs.begin();				
	   		}
	   		else{
	   			r1 = giveReg();

	   			load_it(left.var_name,r1,false);
	   		}
	   		for(int i=0;i<running_symb->to_func.size();i++){
	   			Symtab_elem * curr_row = running_symb->to_func[i];
		           if(var1->get_name() == curr_row->name){ //got it
		           	mach_code<<"\taddi("+to_string(((int)curr_row->offset))+","+registers[r1]+");\n";
		           	break;
		           }
		       }
		       mach_code<<"\taddi(ebp,"+registers[r1]+");\n";
		       gen_ret right = var2->generate_asm();
		       int r2;
		       if(right.is_const == true){
		       	r2 = giveReg_care(r1);
		       	if(right.type == "INT")
		       		mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r2]+");\n";
		       	else
		       		mach_code<<"\tmove("+to_string(right.val)+","+registers[r2]+");\n";	
		       }
		       else if(var2reg[right.var_name].regs.size()>0){
		       	r2 = *var2reg[right.var_name].regs.begin();


		       }
		       else if(var2->isarray == false){
		       	r2 = giveReg_care(r1);
					load_it(right.var_name,r2,true); //r2 has register to be stored
				}
				if(var2reg[left.var_name].regs.size() == 0){
					r1 = giveReg_care(r2);
					load_it(left.var_name,r1,false);
				}
				mach_code<<"\tstorei("+registers[r2]+",ind("+registers[r1]+"));\n";
				if(var2reg[left.var_name].is_local == false){clear_temp(left.var_name);}
				if(var2reg[right.var_name].is_local == false)clear_temp(right.var_name);
				gen_ret t;
				return t;

			}
			if(right.is_const == true){

				if(var2reg[left.var_name].regs.size() >0){
					int r1 = *var2reg[left.var_name].regs.begin();
					clear_reg(r1);
					freeregs.erase(r1);
					if(right.type == "FLOAT")
						mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
					else
						mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
					var2reg[left.var_name].regs.insert(r1);
					var2reg[left.var_name].mem_loc =-1;
					reg2var[r1].insert(left.var_name);

				}
				else{
					int r1 = giveReg();
					if(right.type == "FLOAT")
						mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
					else
						mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
					var2reg[left.var_name].regs.insert(r1);
					var2reg[left.var_name].mem_loc =-1;
					reg2var[r1].insert(left.var_name);
				}
				t.var_name = left.var_name;
				return t;
			}


			if(var2reg[right.var_name].regs.size() == 0){
				r1 = giveReg();
				load_it(right.var_name,r1,true);
			}
			var2reg[left.var_name].regs.clear();
			var2reg[left.var_name].regs = var2reg[right.var_name].regs;
			var2reg[left.var_name].mem_loc =-1;
			
			
			reg2var[*var2reg[right.var_name].regs.begin()].insert(left.var_name);

			if(var2reg[right.var_name].is_local == false)
				clear_temp(right.var_name);

			t.var_name = right.var_name;
			return t;
		}

		if(left.is_const == true && right.is_const == true){
			int r1 = giveReg();
			if(var3 == "AND"){
				if(left.type == "FLOAT")
					mach_code<<"\tmove("+to_string(left.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)left.val))+","+registers[r1]+");\n";
				mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
				string lab1 = next_label();
				mach_code<<"\tje("+lab1+");\n";
				gen_ret right = var2->generate_asm();
				if(right.type == "FLOAT")
					mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
				mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
				string lab2 = next_label();
				mach_code<<"\tje("+lab1+");\n";
				mach_code<<"\tmove(1,"+registers[r1]+");\n";
				mach_code<<"\tjne("+lab2+");\n";
				mach_code<<lab1+":\n";
				mach_code<<"\tmove(0,"+registers[r1]+");\n";
				mach_code<<lab2<<+": \n";
				gen_ret t1;
				t1.var_name  = put_temp(r1,"INT");
				return t1;

			}
			if(var3 == "OR"){

				if(left.type == "FLOAT")
					mach_code<<"\tmove("+to_string(left.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)left.val))+","+registers[r1]+");\n";
				mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
				string lab1 = next_label();
				mach_code<<"\tje("+lab1+");\n";
				gen_ret right = var2->generate_asm();
				if(right.type == "FLOAT")
					mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
				mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
				string lab2 = next_label();
				mach_code<<"\tje("+lab1+");\n";
				mach_code<<"\tmove(0,"+registers[r1]+");\n";
				mach_code<<"\tjne("+lab2+");\n";
				mach_code<<lab1+":\n";
				mach_code<<"\tmove(1,"+registers[r1]+");\n";
				mach_code<<lab2<<+": \n";
				gen_ret t1;
				t1.var_name  = put_temp(r1,"INT");
				return t1;
			}
			if(right.type == "FLOAT")
				mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
			else
				mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";

			if(var3 == "PLUS_INT" )
				mach_code<<"\taddi("+to_string(((int)left.val))+","+registers[r1]+");\n";

			if(var3 == "PLUS_FLOAT" )
				mach_code<<"\taddf("+to_string(left.val)+","+registers[r1]+");\n";

			if(var3 == "MULT_INT")
				{mach_code<<"\tmuli("+to_string(((int)left.val))+","+registers[r1]+");\n";}

			if(var3 == "MULT_FLOAT")
				mach_code<<"\tmulf("+to_string(left.val)+","+registers[r1]+");\n";	

			if(var3 == "DIV_INT")
				{mach_code<<"\tdivi("+to_string(((int)left.val))+","+registers[r1]+");\n";}

			if(var3 == "DIV_FLOAT")
				mach_code<<"\tdivf("+to_string(left.val)+","+registers[r1]+");\n";

			if(var3 == "MINUS_INT"){
				mach_code<<"\tmuli(-1,"+registers[r1]+");\n";
				mach_code<<"\taddi("+to_string(left.val)+","+registers[r1]+");\n";

			}

			if(var3 == "MINUS_FLOAT"){ 
				mach_code<<"\tmulf(-1,"+registers[r1]+");\n";	
				mach_code<<"\taddf("+to_string(left.val)+","+registers[r1]+");\n";
			}

			if(var3 == "LT_INT" ){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("jl","jge",r1);

			}

			if(var3 == "LE_OP_INT"){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("jle","jg",r1);
			}

			if(var3 == "GT_INT"){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("jg","jle",r1);
			}

			if(var3 == "GE_OP_INT"){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("jge","jl",r1);
			}

			if(var3 == "LT_FLOAT" ){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("jl","jge",r1);
			}

			if(var3 == "LE_OP_FLOAT"){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("jle","jg",r1);
			}

			if(var3 == "GT_FLOAT"){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("jg","jle",r1);
			}

			if(var3 == "GE_OP_FLOAT"){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("jge","jl",r1);
			}

			if(var3 == "EQ_OP_FLOAT"){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("je","jne",r1);
			}
			if(var3 == "EQ_OP_INT"){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("je","jne",r1);
			}

			if(var3 == "NE_OP_FLOAT"){
				mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
				compar("jne","je",r1);
			}
			if(var3 == "NE_OP_INT"){
				mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
				compar("jne","je",r1);
			}

			gen_ret  temp;

			if(ftype == true)
				temp.var_name= put_temp(r1,"FLOAT");
			else
				temp.var_name = put_temp(r1,"INT");

			return temp;

		}
		if(left.is_const == true){
			//mach_code<<"LEFT \n";
   
			int r1,r2;
			if(var3 == "AND"){
				int r1 = giveReg();
				if(left.type == "FLOAT")
					mach_code<<"\tmove("+to_string(left.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)left.val))+","+registers[r1]+");\n";
				mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
				string lab1 = next_label();
				mach_code<<"\tje("+lab1+");\n";
				gen_ret right = var2->generate_asm();
				if(var2reg[right.var_name].regs.size() == 0)
					load_it(right.var_name,r1,false);
       		   	 else if(var2reg[right.var_name].is_local == false){ // is temporary

       		   	 	r1 = *var2reg[right.var_name].regs.begin();
       		   	 	clear_temp(right.var_name);
       		   	 	clear_reg(r1);
       		   	 	freeregs.erase(r1);


       		   	 }
		       	  else{// is local
		       	  	r1 = *var2reg[right.var_name].regs.begin();
		       	  	clear_reg(r1);
		       	  	freeregs.erase(r1);
		       	  }
		       	  mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
		       	  string lab2 = next_label();
		       	  mach_code<<"\tje("+lab1+");\n";
		       	  mach_code<<"\tmove(1,"+registers[r1]+");\n";
		       	  mach_code<<"\tjne("+lab2+");\n";
		       	  mach_code<<lab1+":\n";
		       	  mach_code<<"\tmove(0,"+registers[r1]+");\n";
		       	  mach_code<<lab2<<+": \n";
		       	  gen_ret t1;
		       	  t1.var_name  = put_temp(r1,"INT");
		       	  return t1;

		       	}
		       	if(var3 == "OR"){
		       		int r1 = giveReg();
		       		if(left.type == "FLOAT")
		       			mach_code<<"\tmove("+to_string(left.val)+","+registers[r1]+");\n";
		       		else
		       			mach_code<<"\tmove("+to_string(((int)left.val))+","+registers[r1]+");\n";
		       		mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
		       		string lab1 = next_label();
		       		mach_code<<"\tje("+lab1+");\n";
		       		gen_ret right = var2->generate_asm();
		       		if(var2reg[right.var_name].regs.size() == 0)
		       			load_it(right.var_name,r1,false);
       		   	 else if(var2reg[right.var_name].is_local == false){ // is temporary

       		   	 	r1 = *var2reg[right.var_name].regs.begin();
       		   	 	clear_temp(right.var_name);
       		   	 	clear_reg(r1);
       		   	 	freeregs.erase(r1);


       		   	 }
		       	  else{// is local
		       	  	r1 = *var2reg[right.var_name].regs.begin();
		       	  	clear_reg(r1);
		       	  	freeregs.erase(r1);


		       	  }
		       	 /* else{
		       	  	if(right.type == "FLOAT")
       		   	  mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n"; //not sure **##**
       		   	else
       		   		 mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n"; //not sure **##**

       		   	}*/
       		   		mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
       		   		string lab2 = next_label();
       		   		mach_code<<"\tje("+lab1+");\n";
       		   		mach_code<<"\tmove(0,"+registers[r1]+");\n";
       		   		mach_code<<"\tjne("+lab2+");\n";
       		   		mach_code<<lab1+":\n";
       		   		mach_code<<"\tmove(1,"+registers[r1]+");\n";
       		   		mach_code<<lab2<<+": \n";
       		   		gen_ret t1;
       		   		t1.var_name  = put_temp(r1,"INT");
       		   		return t1;
       		   	}
       	  if(var2reg[right.var_name].regs.size() == 0){//not in a register
       	  	r1 = giveReg();
       	  	if(var2reg[right.var_name].is_local == true){
       	  		if(var2reg[right.var_name].type == "INT")
       	  			mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc)+"),"+registers[r1]+");\n";
       	  		else
       	  			mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc)+"),"+registers[r1]+");\n";

       	  	}
       	  	else{
       	  		if(var2reg[right.var_name].type == "INT")
       	  			mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";
       	  		else
       	  			mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";
       	  	}
       	  	
       	  }
       	  else if(var2reg[right.var_name].is_local == false){ // is temporary
       	  	
       	  	r1 = *var2reg[right.var_name].regs.begin();
       	  	clear_temp(right.var_name);
       	  	clear_reg(r1);
       	  	freeregs.erase(r1);
       	  	
       	  	
       	  }
       	  else{// is local
       	  	r1 = *var2reg[right.var_name].regs.begin();
       	  	clear_reg(r1);
       	  	freeregs.erase(r1);


       	  }
       	  


       	  if(var3 == "PLUS_INT")
       	  	mach_code<<"\taddi("+to_string(((int)left.val))+","+registers[r1]+");\n";

       	  if(var3 == "PLUS_FLOAT")
       	  	mach_code<<"\taddf("+to_string(left.val)+","+registers[r1]+");\n";

       	  if(var3 == "MULT_INT")
       	  	mach_code<<"\tmuli("+to_string(((int)left.val))+","+registers[r1]+");\n";

       	  if(var3 == "MULT_FLOAT")
       	  	mach_code<<"\tmulf("+to_string(left.val)+","+registers[r1]+");\n";

       	  if(var3 == "DIV_INT")
       	  	mach_code<<"\tdivi("+to_string(((int)left.val))+","+registers[r1]+");\n";

       	  if(var3 == "DIV_FLOAT")
       	  	mach_code<<"\tdivf("+to_string(left.val)+","+registers[r1]+");\n";

       	  if(var3 == "MINUS_INT"){
       	  	mach_code<<"\tmuli(-1,"+registers[r1]+");\n";
       	  	mach_code<<"\taddi("+to_string(((int)left.val))+","+registers[r1]+");\n";

       	  }

       	  if(var3 == "MINUS_FLOAT"){

       	  	mach_code<<"\tmulf(-1,"+registers[r1]+");\n";
       	  	mach_code<<"\taddf("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  }

       	  if(var3 == "LT_INT" ){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("jl","jge",r1);

       	  }

       	  if(var3 == "LE_OP_INT"){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("jle","jg",r1);
       	  }

       	  if(var3 == "GT_INT"){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("jg","jle",r1);
       	  }

       	  if(var3 == "GE_OP_INT"){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("jge","jl",r1);
       	  }

       	  if(var3 == "LT_FLOAT" ){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("jl","jge",r1);
       	  }

       	  if(var3 == "LE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("jle","jg",r1);
       	  }

       	  if(var3 == "GT_FLOAT"){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("jg","jle",r1);
       	  }

       	  if(var3 == "GE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("jge","jl",r1);
       	  }

       	  if( var3 == "EQ_OP_INT"){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("je","jne",r1);
       	  }

       	  if( var3 == "EQ_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("je","jne",r1);
       	  }

       	   if( var3 == "NE_OP_INT"){
       	  	mach_code<<"\tcmpi("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	compar("jne","je",r1);
       	  }

       	  if( var3 == "NE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+to_string(left.val)+","+registers[r1]+");\n";
       	  	compar("jne","je",r1);
       	  }
       	   /*	if(var2reg[right.var_name].regs.size() != 0 && var2reg[right.var_name].is_local == false ){
       	   		mach_code<<"move("+registers[r1]+","+registers[r2]+");\n";
       	   		r1 = r2;
       	   	}
			*/
       	   	if(var2reg[right.var_name].is_local == false)
       	   		clear_temp(right.var_name);

       	   	gen_ret  temp;
       	   	if(ftype == true)
       	   		temp.var_name= put_temp(r1,"FLOAT");
       	   	else
       	   		temp.var_name = put_temp(r1,"INT");
       	   	return temp;


       	   }
       	   if(right.is_const == true){
       	   	int r1,r2;
       	   	//mach_code<<"RIGHT \n";

       	if(var2reg[left.var_name].regs.size() == 0){//not in a register
       		r1 = giveReg();
       		if(var2reg[left.var_name].is_local == true){
       			if(var2reg[left.var_name].type == "INT")
       				mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc)+"),"+registers[r1]+");\n";
       			else
       				mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc)+"),"+registers[r1]+");\n";

       		}
       		else{
       			if(var2reg[left.var_name].type == "INT")
       				mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";
       			else
       				mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";
       		}

       	}
       	  else if(var2reg[left.var_name].is_local == false){ // is temporary
       	  	
       	  	r1 = *var2reg[left.var_name].regs.begin();
       	  	clear_temp(left.var_name);
       	  	clear_reg(r1);
       	  	freeregs.erase(r1);
       	  	
       	  }
       	  else{// is local

       	  	r1 = *var2reg[left.var_name].regs.begin();
       	  	
       	  	clear_reg(r1);
       	  	freeregs.erase(r1);


       	  }

       	  if(var3 == "AND"){


       	  	mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
       	  	string lab1 = next_label();
       	  	mach_code<<"\tje("+lab1+");\n";
       	  	gen_ret right = var2->generate_asm();
       	  	if(right.type == "FLOAT")
       	  		mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
       	  	else
       	  		mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  	mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
       	  	string lab2 = next_label();
       	  	mach_code<<"\tje("+lab1+");\n";
       	  	mach_code<<"\tmove(1,"+registers[r1]+");\n";
       	  	mach_code<<"\tjne("+lab2+");\n";
       	  	mach_code<<lab1+":\n";
       	  	mach_code<<"\tmove(0,"+registers[r1]+");\n";
       	  	mach_code<<lab2<<+": \n";
       	  	gen_ret t1;
       	  	t1.var_name  = put_temp(r1,"INT");
       	  	return t1;

       	  }
       	  if(var3 == "OR"){
       	  	int r1 = giveReg();
       	  	if(left.type =="FLOAT")
       	  		mach_code<<"\tmove("+to_string(left.val)+","+registers[r1]+");\n";
       	  	else
       	  		mach_code<<"\tmove("+to_string(((int)left.val))+","+registers[r1]+");\n";
       	  	mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
       	  	string lab1 = next_label();
       	  	mach_code<<"\tje("+lab1+");\n";
       	  	gen_ret right = var2->generate_asm();       		   	
       	  	if(right.type == "FLOAT")
       	  		mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
       	  	else
       	  		mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  	mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
       	  	string lab2 = next_label();
       	  	mach_code<<"\tje("+lab1+");\n";
       	  	mach_code<<"\tmove(0,"+registers[r1]+");\n";
       	  	mach_code<<"\tjne("+lab2+");\n";
       	  	mach_code<<lab1+":\n";
       	  	mach_code<<"\tmove(1,"+registers[r1]+");\n";
       	  	mach_code<<lab2<<+": \n";
       	  	gen_ret t1;
       	  	t1.var_name  = put_temp(r1,"INT");
       	  	return t1;
       	  }
       	  if(var3 == "PLUS_INT")
       	  	mach_code<<"\taddi("+to_string(((int)right.val))+","+registers[r1]+");\n";

       	  if(var3 == "PLUS_FLOAT")
       	  	mach_code<<"\taddf("+to_string(right.val)+","+registers[r1]+");\n";

       	  if(var3 == "MULT_INT")
       	  	mach_code<<"\tmuli("+to_string(((int)right.val))+","+registers[r1]+");\n";

       	  if(var3 == "MULT_FLOAT")
       	  	mach_code<<"\tmulf("+to_string(right.val)+","+registers[r1]+");\n";

       	  if(var3 == "DIV_INT")
       	  	{   int r3 = giveReg_care(r1);
       	  		mach_code<<"move("+registers[r1]+","+registers[r3]+");\n";
       	  		if(right.type == "FLOAT")
       	  			mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
       	  		else
       	  			mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		mach_code<<"\tdivi("+registers[r3]+","+registers[r1]+");\n";

       	  	}

       	  	if(var3 == "DIV_FLOAT")
       	  	{
       	  		int r3 = giveReg_care(r1);
       	  		mach_code<<"move("+registers[r1]+","+registers[r3]+");\n";
       	  		
       	  		if(right.type == "FLOAT")
       	  			mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
       	  		else
       	  			mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		mach_code<<"\tdivf("+registers[r3]+","+registers[r1]+");\n";
       	  	}

       	  	if(var3 == "MINUS_INT"){
       	   	   // mach_code<<"\tmuli(-1,"+to_string(right.val)+");\n";
       	  		mach_code<<"\taddi(-"+to_string((int)right.val)+","+registers[r1]+");\n";

       	  	}

       	  	if(var3 == "MINUS_FLOAT"){

       		   // mach_code<<"\tmulf(-1,"+to_string(right.val)+");\n";
       	  		mach_code<<"\taddf(-"+to_string(right.val)+","+registers[r1]+");\n";
       	  	}
       	  	if(var3 == "LT_INT" ){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("jg","jle",r1);

       	  	}

       	  	if(var3 == "LE_OP_INT"){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("jge","jl",r1);
       	  	}

       	  	if(var3 == "GT_INT"){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("jl","jge",r1);
       	  	}

       	  	if(var3 == "GE_OP_INT"){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("jle","jg",r1);
       	  	}

       	  	if(var3 == "LT_FLOAT" ){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("jg","jle",r1);
       	  	}

       	  	if(var3 == "LE_OP_FLOAT"){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("jge","jl",r1);
       	  	}

       	  	if(var3 == "GT_FLOAT"){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("jl","jge",r1);
       	  	}

       	  	if(var3 == "GE_OP_FLOAT"){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("jle","jg",r1);
       	  	}

       	  	if(var3 == "EQ_OP_FLOAT"){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("je","jne",r1);
       	  	}

       	  	if(var3 == "EQ_OP_INT"){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("je","jne",r1);
       	  	}

       	  	if(var3 == "NE_OP_FLOAT"){
       	  		mach_code<<"\tcmpf("+to_string(right.val)+","+registers[r1]+");\n";
       	  		compar("jne","je",r1);
       	  	}

       	  	if(var3 == "NE_OP_INT"){
       	  		mach_code<<"\tcmpi("+to_string(((int)right.val))+","+registers[r1]+");\n";
       	  		compar("jne","je",r1);
       	  	}
       	  	if(var2reg[left.var_name].is_local == false)
       	  		clear_temp(left.var_name);

       	  	gen_ret  temp;
       	  	if(ftype == true)
       	  		temp.var_name= put_temp(r1,"FLOAT");
       	  	else
       	  		temp.var_name = put_temp(r1,"INT");
       	  	return temp; 
       	  }
       else{ //  both are not constants !
       	int r1,r2;
//mach_code<<"LEFT RIGHT NOT\n";
       		if(var2reg[left.var_name].regs.size() == 0){//not in a register
       			r1 = giveReg();

       			if(var2reg[left.var_name].is_local == true){

       				if(var2reg[left.var_name].type == "INT")
       					mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc)+"),"+registers[r1]+");\n";
       				else
       					mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc)+"),"+registers[r1]+");\n";

       			}
       			else{

       				if(var2reg[left.var_name].type == "INT")
       					mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";
       				else
       					mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[left.var_name].mem_loc+exx)+"),"+registers[r1]+");\n";

       			}
       			var2reg[left.var_name].regs.insert(r1);
       			reg2var[r1].insert(left.var_name);
       		}
       		else{

       			r1 = *var2reg[left.var_name].regs.begin();


       		}

       		if(var3 == "AND"){
       			int r2;

       			mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
       			string lab1 = next_label();
       			mach_code<<"\tpushi(esp);\n";
				int tempextra=extra_off;
				extra_off+=4;
       			mach_code<<"\tje("+lab1+");\n";
       			gen_ret right = var2->generate_asm();

       			if(var2reg[right.var_name].regs.size() == 0){
       				r2 = giveReg();
       				load_it(right.var_name,r2,false);
       			}
       			else{       		   	 	
       				r2 = *var2reg[right.var_name].regs.begin();
       				if(var2reg[right.var_name].is_local == false)
       					clear_temp(right.var_name);
       				clear_reg(r2);
       				freeregs.erase(r2);

       			}

       			mach_code<<"\tcmpi(0,"+registers[r2]+");\n";
       			string lab2 = next_label();

       			
       			mach_code<<"\tje("+lab1+");\n";


       			mach_code<<"\tmove(1,"+registers[r2]+");\n";
       			mach_code<<"\tjne("+lab2+");\n";
       			mach_code<<lab1+":\n";
       			mach_code<<"\tmove(0,"+registers[r2]+");\n";
       			mach_code<<lab2<<+": \n";
       			gen_ret t1;
       			clear_all_reg();

       			mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
				mach_code<<"\taddi(4,esp);\n";
				extra_off=tempextra;
				freeregs.erase(r2);
       			t1.var_name  = put_temp(r2,"INT");
       			return t1;

       		}
       		if(var3 == "OR"){


       			mach_code<<"\tcmpi(1,"+registers[r1]+");\n";
       			string lab1 = next_label();
       			mach_code<<"\tpushi(esp);\n";
				int tempextra=extra_off;
				extra_off+=4;
       			mach_code<<"\tje("+lab1+");\n";

       			gen_ret right = var2->generate_asm();   
       			if(var2reg[right.var_name].regs.size() == 0){
       				r2 = giveReg();
       				load_it(right.var_name,r2,false);
       			}

       			else{

       				r2 = *var2reg[right.var_name].regs.begin();
       				if(var2reg[right.var_name].is_local == false)
       					clear_temp(right.var_name);
       				clear_reg(r2);
       				freeregs.erase(r2);

       			}    		   	

       			mach_code<<"\tcmpi(1,"+registers[r2]+");\n";
       			string lab2 = next_label();
       			mach_code<<"\tje("+lab1+");\n";
       			mach_code<<"\tmove(0,"+registers[r2]+");\n";
       			mach_code<<"\tjne("+lab2+");\n";
       			mach_code<<lab1+":\n";
       			mach_code<<"\tmove(1,"+registers[r2]+");\n";
       			mach_code<<lab2<<+": \n";
       			gen_ret t1;
       			clear_all_reg();
       			mach_code<<"\tloadi(ind(ebp,"+to_string(exx-tempextra)+"),esp);\n";
				mach_code<<"\taddi(4,esp);\n";
				extra_off=tempextra;
				freeregs.erase(r2);
       			t1.var_name  = put_temp(r2,"INT");
       			return t1;
       		}

       	  if(var2reg[right.var_name].regs.size() == 0){//not in a register
       	  	r2 = giveReg_care(r1);
       	  	
       	  	if(var2reg[right.var_name].is_local == true){
       	  		if(var2reg[right.var_name].type == "INT")
       	  			mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc)+"),"+registers[r2]+");\n";
       	  		else
       	  			mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc)+"),"+registers[r2]+");\n";

       	  	}
       	  	else{
       	  		if(var2reg[right.var_name].type == "INT")
       	  			mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc+exx)+"),"+registers[r2]+");\n";
       	  		else
       	  			mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[right.var_name].mem_loc+exx)+"),"+registers[r2]+");\n";
       	  	}
       	  	
       	  }
       	  else{

       	  	r2 = *var2reg[right.var_name].regs.begin();
       	  	
       	  	if(var2reg[right.var_name].is_local == false)
       	  		clear_temp(right.var_name);

       	  	clear_reg(r2);
       	  	freeregs.erase(r2);
       	  }
       	  
       	  

       	  if(var3 == "PLUS_INT")
       	  	mach_code<<"\taddi("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "PLUS_FLOAT")
       	  	mach_code<<"\taddf("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "MULT_INT")
       	  	mach_code<<"\tmuli("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "MULT_FLOAT")
       	  	mach_code<<"\tmulf("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "DIV_INT")
       	  	mach_code<<"\tdivi("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "DIV_FLOAT")
       	  	mach_code<<"\tdivf("+registers[r1]+","+registers[r2]+");\n";

       	  if(var3 == "MINUS_INT"){
       	  	mach_code<<"\tmuli(-1,"+registers[r2]+");\n";
       	  	mach_code<<"\taddi("+registers[r1]+","+registers[r2]+");\n";

       	  }
       	  if(var3 == "LT_INT" ){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jl","jge",r2);

       	  }

       	  if(var3 == "LE_OP_INT"){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jle","jg",r2);
       	  }

       	  if(var3 == "GT_INT"){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jg","jle",r2);
       	  }

       	  if(var3 == "GE_OP_INT"){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jge","jl",r2);
       	  }

       	  if(var3 == "LT_FLOAT" ){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jl","jge",r2);
       	  }

       	  if(var3 == "LE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jle","jg",r2);
       	  }

       	  if(var3 == "GT_FLOAT"){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jg","jle",r2);
       	  }

       	  if(var3 == "GE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jge","jl",r2);
       	  }
       	  if(var3 == "EQ_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("je","jne",r2);
       	  }

       	  if(var3 == "EQ_OP_INT"){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("je","jne",r2);
       	  }
       	   if(var3 == "NE_OP_FLOAT"){
       	  	mach_code<<"\tcmpf("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jne","je",r2);
       	  }

       	  if(var3 == "NE_OP_INT"){
       	  	mach_code<<"\tcmpi("+registers[r1]+","+registers[r2]+");\n";
       	  	compar("jne","je",r2);
       	  }
       	  if(var3 == "MINUS_FLOAT"){

       	  	mach_code<<"\tmulf(-1,"+registers[r2]+");\n";
       	  	mach_code<<"\taddf("+registers[r1]+","+registers[r2]+");\n";
       	  }
       	  if(var2reg[left.var_name].is_local == false)
       	  	clear_temp(left.var_name);

       	  gen_ret  temp;
       	  if(ftype == true)
       	  	temp.var_name= put_temp(r2,"FLOAT");
       	  else
       	  	temp.var_name = put_temp(r2,"INT");
       	  return temp;

       	}

       }
       binOp(expAst* arg1,expAst* arg2,string arg3)
       {

       	var1=arg1;
       	var2=arg2;
       	var3=arg3;
		/*if(var3=="PLUS" || var3=="DIV" || var3=="MULT" || var3=="MINUS")
		{
			typeop=type;
		}
		else
		{
			if(var1->type=="FLOAT" || var2->type=="FLOAT")
				typeop="FLOAT";
			else
				typeop="INT";
		}*/

		}

		void print()
		{
			if(var3=="PLUS" || var3=="DIV" || var3=="MULT" || var3=="MINUS")
			{
				typeop=type;
				var3=var3+"_"+typeop;
			}
			else if(var3!="AND" && var3!="OR" && var3!="Assign_exp")
			{
				if(var1->type=="FLOAT" || var2->type=="FLOAT")
					typeop="FLOAT";
				else
					typeop="INT";
				var3=var3+"_"+typeop;
			}

			cout<<"("<<var3;
			/*if(var3!="OR" && var3!="AND" && var3!="Assign_exp")
			{
				cout<<"_"<<typeop;
			}*/
				var1->print();
				cout<<"";
				var2->print();
				cout<<")";
}

};

class unOp : public expAst
{
public:
	expAst* var1;
	string var2;	
	unOp(expAst* arg1,string arg2)
	{
		var1=arg1;
		var2=arg2;
	}
	gen_ret generate_asm(){
		gen_ret right;
		if(var1->isarray == false)
			right = var1->generate_asm();
		gen_ret t;
		
		if(right.is_const == true){
			int r1 = giveReg();

			if(var2 == "INT"){
				if(right.type == "FLOAT")
					mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
				mach_code<<"\tfloatToint("+registers[r1]+");\n";
				t.var_name = put_temp(r1,"INT");
			}
			else if(var2 == "FLOAT"){
				if(right.type == "FLOAT")
					mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
				else
					mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
				mach_code<<"\tintTofloat("+registers[r1]+");\n";	
				t.var_name = put_temp(r1,"FLOAT");
			}
			else if(var2 == "UMINUS"){

				right.val = -right.val;
				return right;

			}

			return t;
		}
		int r1;
		if(var1->isarray == true){

			right =arr(var1);
			r1 = *var2reg[right.var_name].regs.begin();
		}
		else if(var2reg[right.var_name].regs.size()>0)
			r1 = *var2reg[right.var_name].regs.begin();
		else
		{
			r1 = giveReg();
			load_it(right.var_name,r1,true);

		}


		if(var2reg[right.var_name].is_local == false)clear_temp(right.var_name);
		if(var2 == "PP"){ 
			string type1 = "FLOAT"; 
			if(var2reg[right.var_name].type == "INT")
				type1 = "INT"; 
			clear_reg(r1); 
			freeregs.erase(r1); 
			var2reg[right.var_name].regs.insert(r1); 
			reg2var[r1].insert(right.var_name); 
			mach_code<<"\taddi(1,"+registers[r1]+");\n"; 
			t.var_name = right.var_name; 
			t.type = type1; 
			var2reg[t.var_name].mem_loc =-1; }
			if(var2 == "INT"){
				clear_reg(r1);
				freeregs.erase(r1);

				mach_code<<"\tfloatToint("+registers[r1]+");\n";
				t.var_name = put_temp(r1,"INT");
			}
			else if(var2 == "FLOAT"){

				clear_reg(r1);
				freeregs.erase(r1);

				mach_code<<"\tintTofloat("+registers[r1]+");\n";			
				t.var_name = put_temp(r1,"FLOAT");
			}
			else if(var2 == "UMINUS"){
				clear_reg(r1);
				freeregs.erase(r1);

				if(var2reg[right.var_name].type== "INT")
					{mach_code<<"\tmuli(-1,"+registers[r1]+");\n";t.var_name = put_temp(r1,"INT");}
				else
					{mach_code<<"\tmulf(-1,"+registers[r1]+");\n";t.var_name = put_temp(r1,"FLOAT");}

			}
			else if(var2 == "NOT"){
				clear_reg(r1);
				freeregs.erase(r1);
				string type1 = "FLOAT";
				if(var2reg[right.var_name].type == "INT")
					mach_code<<"\tcmpi(0,"+registers[r1]+");\n";
				else
					mach_code<<"\tcmpf(0,"+registers[r1]+");\n";	
				compar("je","jne",r1);
				t.var_name = put_temp(r1,"INT");
			}

			return t;


		}
		void print()
		{
			cout<<"("<<var2<<"";
				var1->print();
				cout<<")";
}

};

class funCall : public expAst
{
public:
	list <expAst*> var1;
	string var2;
	funCall(list <expAst*> arg1,string arg2)
	{
		var2=arg2;
		
		for(list <expAst*> :: iterator i=arg1.begin();i!=arg1.end();i++)
		{
			var1.push_back(*i);

		}

	}
	gen_ret generate_asm()
	{   
		
		
		list<expAst*> :: iterator it;
		
		if(var2 == "printf"){

			for(it = var1.begin(); it != var1.end(); it++){

				if((*it)->isarray == true){
					//int r1 = giveReg();
					//cerr<<"save me\n";
					gen_ret yo = arr(*it);
					int r1 = *var2reg[yo.var_name].regs.begin();
					if(yo.type == "INT"){
						mach_code<<"\tprint_int("+registers[r1]+");\n";
					}
					else if(yo.type == "FLOAT"){
						mach_code<<"\tprint_float("+registers[r1]+");\n";
					}

					
					continue;
				}
				gen_ret yo = (*it)->generate_asm();
				
				if(yo.is_const == true){
					if((*it)->type == "INT"){
						mach_code<<"\tprint_int("+to_string(((int)yo.val))+");\n";
					}
					else if((*it)->type == "FLOAT"){
						mach_code<<"\tprint_float("+to_string(yo.val)+");\n";
					}
					else{
						

						mach_code<<"\tprint_string("+yo.str+");\n";
					}
				}
				else if(var2reg[yo.var_name].regs.size() ==0){
					int r1 = giveReg();
					load_it(yo.var_name,r1,false);
					if(var2reg[yo.var_name].type  == "INT"){
						mach_code<<"\tprint_int("+registers[r1]+");\n";
					}
					else{
						mach_code<<"\tprint_float("+registers[r1]+");\n";
					}
					
				}
				else {
					if((*it)->type == "INT"){
						mach_code<<"\tprint_int("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
					else if((*it)->type == "FLOAT"){
						mach_code<<"\tprint_float("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
					else{
						mach_code<<"\tprint_string("+registers[*var2reg[yo.var_name].regs.begin()]+");\n";
					}
				}
				
				// Special print methods

			}

			gen_ret t;
			return t;
		}

		vector<gen_ret> all_rets;
		for(it = var1.begin(); it!= var1.end(); it++){
			int r1; 

			if((*it)->isarray == true){
				//r1 = giveReg();

				gen_ret t =arr(*it);
				all_rets.push_back(t);
				
			}
			else {

				gen_ret temp1 = (*it)->generate_asm();
				all_rets.push_back(temp1);
				
			}
			
		}

		



		vector<int> stored_regs;
		//mach_code<<"//Saving edx\n";
		clear_reg(3);
		//mach_code<<"//Saving registers and extraoff="<<extra_off<<"\n";
		for(int i=0; i<4; i++){
			if(reg2var[i].size()>0){
				stored_regs.push_back(i);
				if(var2reg[*reg2var[i].begin()].type == "INT")
					mach_code<<"\tpushi("+registers[i]+");\n";
				else
					mach_code<<"\tpushf("+registers[i]+");\n";	
			}
			//extra_off += 4;
		}
		//mach_code<<"//Saving registers done. make space for return value\n";
		mach_code<<"\taddi(-4,esp);\n";//Space for return value
		//mach_code<<"//pushing parameter\n";
		for(int i=all_rets.size()-1; i>=0; i--){
			if(all_rets[i].is_const == true){
				
				if(all_rets[i].type == "INT")
					mach_code<<"\tpushi("+to_string(((int)all_rets[i].val))+");\n";
				else
					mach_code<<"\tpushf("+to_string(all_rets[i].val)+");\n";
				//extra_off += 4;
			}
			else if(var2reg[all_rets[i].var_name].regs.size() >0){
				if(var2reg[all_rets[i].var_name].type == "INT")
					mach_code<<"\tpushi("+registers[*var2reg[all_rets[i].var_name].regs.begin()]+");\n";
				else
					mach_code<<"\tpushf("+registers[*var2reg[all_rets[i].var_name].regs.begin()]+");\n";
				//extra_off += 4;
				if(var2reg[all_rets[i].var_name].is_local == false)clear_temp(all_rets[i].var_name);
			}
			else{
				int r =3;
				load_it(all_rets[i].var_name,3,false);
				mach_code<<"\taddi(-4,esp);\n";
				if(var2reg[all_rets[i].var_name].type == "INT")
					mach_code<<"\tstorei(edx,ind(esp));\n";
				else
					mach_code<<"\tstoref(edx,ind(esp));\n";
			}

		}

		mach_code<<"\t"<<var2<<"();\n";

		//Restore stack and registers

		int decsize=var1.size()*4;
		mach_code<<"\taddi("<<decsize<<",esp);\n";
		for(int i=stored_regs.size()-1;i>=0;i--)
		{   
			if(reg2var.find(stored_regs[i]) == reg2var.end() || reg2var[stored_regs[i]].size() ==0)
				continue;
			if(var2reg[*reg2var[stored_regs[i]].begin()].type == "INT")
			{
				mach_code<<"\tloadi(ind(esp,"+to_string(((int)stored_regs.size()-i)*4)<<"),"<<registers[stored_regs[i]]<<");\n";
			}
			else
			{
				mach_code<<"\tloadf(ind(esp,"+to_string((stored_regs.size()-i)*4)<<"),"<<registers[stored_regs[i]]<<");\n";
			}		
		}
		//int tempextra=extra_off;
		//extra_off += ((stored_regs.size()+1)*4);

		gen_ret ttemp;

		int r3 = 3;
		
		string ttype1;
		for(int i=0;i<Symbol_table.size();i++)
		{
			if(Symbol_table[i]->name==var2)
			{
				ttype1=Symbol_table[i]->type->type;
			}
		}
		//mach_code<<"\t///////exx="<<exx<<endl;
		//mach_code<<"\t///////tempextra="<<tempextra<<endl;
		//mach_code<<"\t///////stored="<<(int)((stored_regs.size())*4)<<endl;
		//mach_code<<"\t///////final="<<(exx-(int)((stored_regs.size())*4)-tempextra )<<endl;
		if(ttype1=="INT")
		{

			mach_code<<"\tloadi(ind(esp),"+registers[r3]+");\n";	
			ttemp.var_name = put_temp(r3,"INT");
		}
		else
		{
			mach_code<<"\tloadf(ind(esp),"+registers[r3]+");\n";	
			ttemp.var_name = put_temp(r3,"FLOAT");
		}
		 //dont know type
		//TODO:store the value at esp in a register and set ttemp accordingly

		decsize=(stored_regs.size()+1)*4;
		mach_code<<"\taddi("<<decsize<<",esp);\n";
		
		return ttemp;
	}
	void print()
	{
		//cout<<var2<<"(";//check what to print
		cout<<"(FUNCALL(Id \""<<var2<<"\")(Arguments(";
			for(list <expAst*> :: iterator i=var1.begin();i!=var1.end();i++)
			{

				(*i)->print();

			}
			cout<<")))";
}

};

class floatConst : public expAst
{
public:
	float var1;
	gen_ret generate_asm(){
		gen_ret t;
		t.val = var1;
		t.is_const = true;
		t.type = "FLOAT";
		return t;
	}
	floatConst(string arg1)
	{
		stringstream ss;
		ss<<arg1;
		ss>>var1;
		isconst=true;
	}
	void print()
	{
		cout<<"(FloatConst "<<var1<<")";
	}
	
};

class intConst : public expAst
{
public:
	int var1;
	gen_ret generate_asm(){
		gen_ret t;
		t.val = var1;
		t.type = "INT";
		t.is_const = true;
		return t;
	}
	intConst(string arg1)
	{
		isconst=true;
		var1=stoi(arg1);
	}
	void print()
	{
		cout<<"(IntConst "<<var1<<")";
	}
	
};

class stringConst : public expAst
{
public:
	string var1;
	stringConst(string arg1)
	{
		isconst = true;
		var1=arg1;
	}
	gen_ret generate_asm()
	{
		gen_ret t;
		t.str=var1;
		t.is_const=true;
		return t;
	}
	void print()
	{
		cout<<"(StrConst "<<var1<<")";
	}
	
};


class ident : public arrayRef
{
public:
	string var1;
	ident(string arg1)
	{
		var1=arg1;
	}
	gen_ret generate_asm(){
		
		gen_ret  temp;
		temp.var_name  = var1;
		temp.offset =1;
		
		return temp;

	}
	ident(string arg1,int arg2,int arg3):arrayRef(arg2,arg3)
	{
		var1=arg1;
	}
	void print()
	{
		cout<<"(Id \""<<var1<<"\")";
	}
	string get_name()
	{
		return var1;
	}
};

class index : public arrayRef
{
public:
	arrayRef *var1;
	expAst *var2;//check if this is expAst

	gen_ret generate_asm(){

		gen_ret left = var1->generate_asm();
		int t1=4;
		for(int i=0; i<running_symb->to_func.size(); i++){
			Symtab_elem * curr_row = running_symb->to_func[i];
			if(curr_row->name == var1->get_name()){ //got it
				for(int j=left.offset; j<curr_row->type->indices.size(); j++){
					t1=t1*curr_row->type->indices[j];
					break;
				}
			}
		}
		
		gen_ret temp,right;
		if(var2->isarray == false)
			right = var2->generate_asm();
		int r1 ; // right ke register ko add karo muliply 120*
		if(var2->isarray == true){
			gen_ret ty =arr(var2);
			r1 = *var2reg[ty.var_name].regs.begin();
		}
		else if(right.is_const == true){
			r1 = giveReg();
			if(right.type == "FLOAT")
				mach_code<<"\tmove("+to_string(right.val)+","+registers[r1]+");\n";
			else
				mach_code<<"\tmove("+to_string(((int)right.val))+","+registers[r1]+");\n";
		}else{
			if(var2reg[right.var_name].regs.size()>0)
				{r1 = *var2reg[right.var_name].regs.begin();

					clear_reg(r1);
					freeregs.erase(r1);
				}
				else
				{
					r1 = giveReg();
				//mach_code<<"12\n";
					load_it(right.var_name,r1,false);

				}
			}		
			int r2;
			bool rt =false;
			if(left.offset != 1){
				rt = true;
				if(var2reg[left.var_name].regs.size()>0)
					{r2 = *var2reg[left.var_name].regs.begin();

					}
					else
					{
						r2 = giveReg_care(r1);
			//mach_code<<"123\n";
						load_it(left.var_name,r2,true);

					}
				}

				mach_code<<"\tmuli("+to_string(t1)+","+registers[r1]+");\n";
				if(rt == true)
					mach_code<<"\taddi("+registers[r2]+","+registers[r1]+");\n";
				if(var2reg[right.var_name].is_local == false)clear_temp(right.var_name);
				if(var2reg[left.var_name].is_local == false)clear_temp(left.var_name);	
				temp.offset = left.offset+1;

				temp.var_name = put_temp(r1,"INT");



				return temp;


			}
			index(arrayRef* arg1,expAst* arg2,int arg3,int arg4)
			{
				var1=arg1;
				var2=arg2;
				current_dim=arg3;
				correct_dim=arg4;
				isarray=true;
			}
			void print()
			{
				cout<<"(Index";
					var1->print();
					cout<<"";
					var2->print();
					cout<<")";
}
string get_name()
{
	return var1->get_name();
}
};

/*int main()
{
	Bop a=AND_OP;
	cout<<"yo"<<a;
}*/

	











