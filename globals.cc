vector<Symtab_elem *> Symbol_table;
string temp_type;
vector <string> error_vec;
vector <stmtAst *> ast_vec;
vector <string> asm_code;
int arr_dim;// dimension of array when using
int cor_dim;//correct dimension of declared array
bool is_err=false;//Whether complete code has some error
ofstream mach_code;
int extra_off;
int I = sizeof(int);
int F = sizeof(float);
int temp_vars;
int labels;
Symtab_elem * running_symb;
map<string,int> wrt_ebp;
map<string,char>  code_temp_type;
map<int, set<string> > reg2var; //the register holds which variables?
map<string, varinfo > var2reg;  //this variable is stored in which regiters?
map<int,string> registers;
set<int> freeregs;
int exx;
void handle(int line,string err,string left , string right){
	Symbol_table[Symbol_table.size()-1]->has_error=true;
	is_err=true;
	if(left != "error" && right != "error"){
	 char str[22];
	 sprintf(str, "%d", Scanner::line_num);
	 string yo(str);
	 if(left=="STRING" && right=="STRING")
	 err=":Operands can't be applied to STRING type";
	 string temp = yo+err;
	 error_vec.push_back(temp);

	 cout<<temp<<endl;
	}
	
}

string check(string x,string y,int type){//type=1 for logical,2 for assignment, 0 for everything else
	if(x== "VOID" ||  y=="VOID" || x=="error" || y=="error" || x=="STRING" || y=="STRING"){
		return "error";
	}
	else if((x=="FLOAT" || y== "FLOAT") && type==0) {
		return "FLOAT";
	}
	else if(type==2)
	{
		return x;
	}

	else return "INT";//type 1
} 
int get_score(int a){
	set<string> :: iterator it;
	int score =0;
	for(it = reg2var[a].begin(); it != reg2var[a].end(); it++){//looping through all variables attached to that register
		
				bool varfine = false;
				set<int> :: iterator it3;
				for(it3 = var2reg[*it].regs.begin(); it3 != var2reg[*it].regs.end(); it3++){//looping through all registers of that particular variable
					if(a != *it3){varfine = true;break;} //that variable has anoather location
				}
				if(varfine == false && var2reg[*it].mem_loc ==-1){// there exist a variable which is not in any other register 
					score++;
				}
	}
	return score;
}
int giveReg(){
	
	if(freeregs.size()>0){
		int top = *freeregs.begin();
		freeregs.erase(top);
		return top;
	}
	else{
		map<int, set<string> > :: iterator it;
		map<int,int> score;
		int curReg;
		for(it = reg2var.begin(); it!= reg2var.end(); it++){//looping through all registers
			score[it->first]=get_score(it->first);
		}

		int chosenReg =-2;
		int val=10000;
		map<int,int> :: iterator it4;
		for(it4 = score.begin(); it4 != score.end(); it4++){
			if(val > it4->second){
				val = it4->second;
				chosenReg = it4->first;
			}
		}

		//chosenReg is chosen now perform required stores for reg2var[chosenReg]
		//mach_code<<"giveReg clearing "<<registers[chosenReg]<<endl;
		clear_reg(chosenReg);
		freeregs.erase(chosenReg);
		return chosenReg;
	}
}
string put_temp(int reg, string type){ // register should already be cleaned
	string _name = to_string(temp_vars);
   	temp_vars++;
   	varinfo temp;
   	temp.type = type;
   	temp.regs.insert(reg);
   	reg2var[reg].insert(_name);
   	temp.mem_loc = -1;
   	temp.is_local = false;
   	var2reg[_name] = temp;
   	return _name;
}
gen_ret arr(expAst* var){
	gen_ret yo = var->generate_asm();
				int r1;
				if(var2reg[yo.var_name].regs.size()>0){
					r1 = *var2reg[yo.var_name].regs.begin();
		
				}
				else{
					r1 = giveReg();
					load_it(yo.var_name,r1,false);
				}

				//finished loading register which contains offset
				string type11;
				for(int i=0;i<running_symb->to_func.size();i++){
		          Symtab_elem * curr_row = running_symb->to_func[i];
		           if(var->get_name() == curr_row->name){ //got it
		           	mach_code<<"\taddi("+to_string(curr_row->offset)+","+registers[r1]+");\n";
		           	type11 = curr_row->type->type;
		           	break;
		           }
		        }
		        mach_code<<"\taddi(ebp,"+registers[r1]+");\n";
		        gen_ret temp ;
		        if(var2reg[yo.var_name].is_local == false)clear_temp(yo.var_name);
		        clear_reg(r1);
		        freeregs.erase(r1);
		        if(var->type == "INT")
				{mach_code<<"\tloadi(ind("+registers[r1]+"),"+registers[r1]+");\n";temp.type = "INT";}
			else
				{mach_code<<"\tloadf(ind("+registers[r1]+"),"+registers[r1]+");\n";temp.type = "FLOAT";}
				
				temp.var_name= put_temp(r1,type11);
			
				return temp;
}
gen_ret arr_care(int reg,expAst* var){
	gen_ret yo = var->generate_asm();
				int r1;
				if(var2reg[yo.var_name].regs.size()>0){
					r1 = *var2reg[yo.var_name].regs.begin();
		
				}
				else{
					r1 = giveReg_care(reg);
					load_it(yo.var_name,r1,false);
				}
				//finished loading register which contains offset
				string type11;
				for(int i=0;i<running_symb->to_func.size();i++){
		          Symtab_elem * curr_row = running_symb->to_func[i];
		           if(var->get_name() == curr_row->name){ //got it
		           	mach_code<<"\taddi("+to_string(curr_row->offset)+","+registers[r1]+");\n";
		           	type11 = curr_row->type->type;
		           	break;
		           }
		        }
		        mach_code<<"\taddi(ebp,"+registers[r1]+");\n";
		        gen_ret temp ;
		        if(var2reg[yo.var_name].is_local == false)clear_temp(yo.var_name);

		        clear_reg(r1);
		        freeregs.erase(r1);
		        if(var->type == "INT")
				{mach_code<<"\tloadi(ind("+registers[r1]+"),"+registers[r1]+");\n";temp.type = "INT";}
			else
				{mach_code<<"\tloadf(ind("+registers[r1]+"),"+registers[r1]+");\n";temp.type = "FLOAT";}
				
				temp.var_name= put_temp(r1,type11);
			
				return temp;
}
void load_it(string var, int r2,bool f){
	
	if(var2reg[var].is_local == true){
       	  		if(var2reg[var].type == "INT")
       	  		mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[var].mem_loc)+"),"+registers[r2]+");\n";
       	  		else
       	        mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[var].mem_loc)+"),"+registers[r2]+");\n";

       	  	}
       	  	else{

       	  		
       	  			if(var2reg[var].type == "INT")
       	  		mach_code<<"\tloadi(ind(ebp,"+to_string(-var2reg[var].mem_loc+exx)+"),"+registers[r2]+");\n";
       	  		else
       	        mach_code<<"\tloadf(ind(ebp,"+to_string(-var2reg[var].mem_loc+exx)+"),"+registers[r2]+");\n";
       	  	}
  if(f){
       	var2reg[var].regs.insert(r2);
       	  	reg2var[r2].insert(var);
  }
}
int giveReg_care(int r){
	
	if(freeregs.size()>0){
	
		int top = *freeregs.begin();
		freeregs.erase(top);
		return top;
	}
	else{
		map<int, set<string> > :: iterator it;
		map<int,int> score;
		
		for(it = reg2var.begin(); it!= reg2var.end(); it++){//looping through all registers
			if(it->first != r)
			score[it->first]=get_score(it->first);
		else score[r]=10000;
		}

		int chosenReg =-2;
		int val=10000;
		map<int,int> :: iterator it4;
		for(it4 = score.begin(); it4 != score.end(); it4++){
			if(val > it4->second){
				val = it4->second;
				chosenReg = it4->first;
			}
		}
		if(chosenReg == r)mach_code<<"INFI PAIN\n";
		//chosenReg is chosen now perform required stores for reg2var[chosenReg]
		clear_reg(chosenReg);
		freeregs.erase(chosenReg);
		return chosenReg;
	}
}
stmtAst* fun2ast(string name){
  
  for(int i=0;i<ast_vec.size();i++)
  {  
		  if(Symbol_table[i]->name == name){
		  	return ast_vec[i];
		  }
  }
  
}
string next_label(){
		string lb = "L"+to_string(labels);
		labels++;
		return lb;
}
void init(){
	temp_vars =0;
	registers[0]="eax";
	registers[1]="ebx";
	registers[2]="ecx";
	registers[3]="edx";
	mach_code.open("code.asm");
	mach_code<<std::setprecision(4);
	mach_code<<std::fixed;
	freeregs.insert(3);
	freeregs.insert(2);
	freeregs.insert(1);
	freeregs.insert(0);
}

void init1(){
	var2reg.clear();
	reg2var.clear();
	freeregs.clear();
	freeregs.insert(3);
	freeregs.insert(2);
	freeregs.insert(1);
	freeregs.insert(0);
	labels =0;
	for(int i=0;i<running_symb->to_func.size();i++){
		  Symtab_elem * curr_row = running_symb->to_func[i];
		  varinfo temp;
		  temp.type = curr_row->type->type;
		  temp.is_local = true;
		  
		  temp.mem_loc = -curr_row->offset;
		  temp.offset = -curr_row->offset;
		  var2reg[curr_row->name]=temp;

		
	   
	}
}
int lin_num =0;
void gen_code(string s){
    
	mach_code<<"\nvoid "+s+"()\n";
	mach_code<<"{\n";
	mach_code<<"\tpushi(ebp);\n"; // Setting dynamic link
	mach_code<<"\tmove(esp,ebp);\n";

	//Get the offset of topmost local variable
	int tempof=((running_symb->to_func).back())->offset;
	mach_code<<"\taddi("<<tempof<<",esp);\n";	
	//use ast for the function
	stmtAst * t =fun2ast(s);
	t->generate_asm();
	mach_code<<"\tmove(ebp,esp);\n";
	mach_code<<"\tloadi(ind(ebp), ebp);\n"; // restoring dynamic link
	mach_code<<"\tpopi(1);\n"; //pop stack
	mach_code<<"\treturn;\n"; //return
	mach_code<<"}\n";
}

void compar(string op, string notop, int r1){
	string lab1 = next_label(),lab2 = next_label(), lab3 = next_label();
       		    mach_code<<"\t"+op+"("+lab1+");\n";
       		    mach_code<<"\t"+notop+"("+lab3+");\n";
       		    mach_code<<lab1+":\n";
       		    mach_code<<"\tmove(1,"+registers[r1]+");\n";
       		    mach_code<<"\t"+op+"("+lab2+");\n";
       		    mach_code<<lab3+":\n";
       		    mach_code<<"\tmove(0,"+registers[r1]+");\n";
       		    mach_code<<lab2+":\n";
}

void clear_reg(int reg) //all variables associated with that register are stored if thier copies dont exist
{
	set<string> tp =reg2var[reg];
	set<string> :: iterator it;

	if(reg2var.find(reg) == reg2var.end() || reg2var[reg].size() == 0){freeregs.insert(reg);return ;}
	for(it = reg2var[reg].begin(); it!= reg2var[reg].end(); it++){ // loop through all variables
		string var1 = *it;
		var2reg[var1].regs.erase(reg);
		if(var2reg[var1].mem_loc == -1 && var2reg[var1].regs.size() == 0){//no other copy exist
		//mach_code<<"////"<<" "<<var1<<" "<<registers[reg]<<endl;
			if(var2reg[var1].is_local == true){ // put in its ebp location
						     
			      	if(var2reg[var1].type == "INT")
			      	{mach_code<<"\tstorei("+registers[reg]+",ind(ebp,"+to_string(-var2reg[var1].offset)+"));\n";}
			       else
			       {mach_code<<"\tstoref("+registers[reg]+",ind(ebp,"+to_string(-var2reg[var1].offset)+"));\n";}

			   var2reg[var1].mem_loc = var2reg[var1].offset;
	
			}
			else{
				 mach_code<<"\taddi(-4,esp);\n";
				if(var2reg[var1].type == "INT")
				{mach_code<<"\tstorei("+registers[reg]+",ind(esp));\n";}
				else
		        {mach_code<<"\tstoref("+registers[reg]+",ind(esp));\n";}
		        var2reg[var1].mem_loc = extra_off;
		        extra_off += 4;

		      

		       
			}
		}
	}

	reg2var[reg].clear();

	freeregs.insert(reg);
}
void clear_all_reg(){
for(int i=0; i<3; i++){
	clear_reg(i);
}
}
void clear_temp(string temp){ 
	if(var2reg[temp].is_local == true)cout<<"WRONG CODE GLOBALS 209\n";
	set<int> :: iterator it;
	for(it = var2reg[temp].regs.begin(); it!=var2reg[temp].regs.end(); it++){
		reg2var[*it].erase(temp);
		if(reg2var[*it].size() == 0){freeregs.insert(*it);}
	}
	var2reg.erase(temp);
}


int good_reg(int a,int b){
	int score_a = get_score(a),score_b=get_score(b);
	if(score_a> score_b)return b;
	else return a;
		
}

