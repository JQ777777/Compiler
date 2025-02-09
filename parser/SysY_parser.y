%{
#include <fstream>
#include "SysY_tree.h"
#include "type.h"
Program ast_root;

void yyerror(char *s, ...);
int yylex();
int error_num = 0;
extern int line_number;
extern std::ofstream fout;
extern IdTable id_table;
%}
%union{
    char* error_msg;
    Symbol symbol_token;
    double float_token; // 对于SysY的浮点常量，我们需要先以double类型计算，再在语法树节点创建的时候转为float
    int int_token;
    Program program;  
    CompUnit comp_unit;  std::vector<CompUnit>* comps; 
    Decl decl;
    Def def;  std::vector<Def>* defs;
    FuncDef func_def;
    Expression expression;  std::vector<Expression>* expressions;
    Stmt stmt;
    Block block;
    InitVal initval;  std::vector<InitVal>* initvals;
    FuncFParam formal;   std::vector<FuncFParam>* formals;
    BlockItem block_item;   std::vector<BlockItem>* block_items;
}
//declare the terminals
%token <symbol_token> STR_CONST IDENT
%token <float_token> FLOAT_CONST
%token <int_token> INT_CONST
%token LEQ GEQ EQ NE AQ SQ MQ DQ MODQ// <=   >=   ==   != 
%token AND OR // &&    ||
%token CONST IF ELSE WHILE NONE_TYPE INT FLOAT FOR
%token RETURN BREAK CONTINUE ERROR TODO

//give the type of nonterminals
%type <program> Program
%type <comp_unit> CompUnit     
%type <comps> Comp_list
%type <decl> Decl VarDecl ConstDecl
%type <def> ConstDef VarDef
%type <defs> ConstDef_list VarDef_list 
%type <func_def> FuncDef 
%type <expression> Exp LOrExp AddExp MulExp RelExp EqExp LAndExp UnaryExp PrimaryExp
%type <expression> ConstExp Lval FuncRParams Cond
%type <expression> IntConst FloatConst
%type <expression> Array ConstArray
%type <expressions> Array_list ConstArray_list Exp_list;
%type <stmt> Stmt 
%type <block> Block
%type <block_item> BlockItem
%type <block_items> BlockItem_list
%type <initval> ConstInitVal VarInitVal  
%type <initvals> VarInitVal_list ConstInitVal_list
%type <formal> FuncFParam 
%type <formals> FuncFParams

// THEN和ELSE用于处理if和else的移进-规约冲突
%precedence THEN
%precedence ELSE
%%
Program 
:Comp_list
{
    @$ = @1; 
    ast_root = new __Program($1);
    ast_root->SetLineNumber(line_number);
};

Comp_list
:CompUnit 
{
    $$ = new std::vector<CompUnit>;
    ($$)->push_back($1);
}
|Comp_list CompUnit
{
    ($1)->push_back($2);
    $$ = $1;
};

CompUnit
:Decl{  //声明
    $$ = new CompUnit_Decl($1); 
    $$->SetLineNumber(line_number);
}
|FuncDef{  //函数定义
    $$ = new CompUnit_FuncDef($1); 
    $$->SetLineNumber(line_number);
}
;

Decl
:VarDecl{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
|ConstDecl{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
;

VarDecl
:INT VarDef_list ';'{
    $$ = new VarDecl(Type::INT,$2); 
    $$->SetLineNumber(line_number);
}
|FLOAT VarDef_list ';'{   //增加float类型
    $$ = new VarDecl(Type::FLOAT,$2); 
    $$->SetLineNumber(line_number);
}
;

//增加数组定义
Array   
:'[' Exp ']'
{
   $$=$2;
   $$->SetLineNumber(line_number);
}
;

ConstArray
:
'[' ConstExp ']'
{
    $$ = $2;
    $$->SetLineNumber(line_number);
}
;

// TODO(): 考虑变量定义更多情况  

ConstDecl
:CONST INT ConstDef_list ';'{   //增加float类型
    $$ = new ConstDecl(Type::INT,$3); 
    $$->SetLineNumber(line_number);
}
|CONST FLOAT ConstDef_list ';'{
    $$ = new ConstDecl(Type::FLOAT,$3); 
    $$->SetLineNumber(line_number);
}
;

// TODO(): 考虑变量定义更多情况  

VarDef_list
:VarDef  
{  
    // 创建一个新的向量来存储变量定义  
    $$ = new std::vector<Def>;  
    ($$)->push_back($1); 
}
|  VarDef_list ',' VarDef  
{  
    // 将新的 VarDef 添加到现有的列表中  
    ($1)->push_back($3);   
    $$ = $1;  
}  
;

ConstDef_list
:ConstDef  
{  
    // 创建一个新的向量来存储常量定义  
    $$ = new std::vector<Def>; 
    ($$)->push_back($1); 
}  
| ConstDef_list ',' ConstDef  
{  
    // 将新的 ConstDef 添加到现有的列表中  
    ($1)->push_back($3);    
    $$ = $1;  
}  
;

//增加数组list定义
Array_list
:Array
{
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|Array_list Array
{
    ($1)->push_back($2);
    $$ = $1;
}
;

ConstArray_list
:ConstArray
{
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|ConstArray_list ConstArray
{
    ($1)->push_back($2);
    $$ = $1;
}
;

FuncDef
:INT IDENT '(' FuncFParams ')' Block
{
    $$ = new __FuncDef(Type::INT,$2,$4,$6);
    $$->SetLineNumber(line_number);
}
|INT IDENT '(' ')' Block
{
    $$ = new __FuncDef(Type::INT,$2,new std::vector<FuncFParam>(),$5); 
    $$->SetLineNumber(line_number);
}
|FLOAT IDENT '(' FuncFParams ')' Block   //增加float类型
{
    $$ = new __FuncDef(Type::FLOAT,$2,$4,$6);
    $$->SetLineNumber(line_number);
}
|FLOAT IDENT '(' ')' Block
{
    $$ = new __FuncDef(Type::FLOAT,$2,new std::vector<FuncFParam>(),$5); 
    $$->SetLineNumber(line_number);
}
|NONE_TYPE IDENT '(' FuncFParams ')' Block   //增加返回类型为“无”
{
    $$ = new __FuncDef(Type::VOID, $2, $4, $6);
    $$->SetLineNumber(line_number);
}
|NONE_TYPE IDENT '(' ')' Block
{
    $$ = new __FuncDef(Type::VOID, $2, new std::vector<FuncFParam>(), $5);
    $$->SetLineNumber(line_number);
}
;
// TODO(): 考虑函数定义更多情况    

VarDef
:IDENT '=' VarInitVal
{
    $$ = new VarDef($1,nullptr,$3); 
    $$->SetLineNumber(line_number);
    }
|IDENT
{
    $$ = new VarDef_no_init($1,nullptr); 
    $$->SetLineNumber(line_number);
    }
|IDENT Array_list '=' VarInitVal
{
    $$ =  new VarDef($1,$2,$4); 
    $$->SetLineNumber(line_number);
}
|IDENT Array_list
{
    $$ = new VarDef_no_init($1,$2); 
    $$->SetLineNumber(line_number);
}
; 
// TODO(): 考虑变量定义更多情况


ConstDef
:  IDENT '=' ConstInitVal
    {
        $$ = new ConstDef($1,nullptr,$3);
        $$->SetLineNumber(line_number);
    }
    |
    IDENT ConstArray_list '=' ConstInitVal
    {
        $$ = new ConstDef($1,$2,$4);
        $$->SetLineNumber(line_number);
    }
   
    
;

ConstInitVal
:ConstExp{   //常量表达式
    $$ = new ConstInitVal_exp($1); 
    $$->SetLineNumber(line_number);
    }
|'{' ConstInitVal_list '}'{
    $$ = new ConstInitVal($2); 
    $$->SetLineNumber(line_number);
    }
|'{' '}'{   //空列表
    $$ = new ConstInitVal(new std::vector<InitVal>());
     $$->SetLineNumber(line_number);
     }
;

VarInitVal
:Exp{
    $$ = new VarInitVal_exp($1); 
    $$->SetLineNumber(line_number);
    }
|'{' VarInitVal_list '}'{
    $$ = new VarInitVal($2); 
    $$->SetLineNumber(line_number);
    }
|'{' '}'{
    $$ = new VarInitVal(new std::vector<InitVal>()); 
    $$->SetLineNumber(line_number);
    }
;

ConstInitVal_list
:ConstInitVal{   //单个常量初始化值
    $$ = new std::vector<InitVal>;
    ($$)->push_back($1);
}
|ConstInitVal_list ',' ConstInitVal{   //多个常量初始化值
    ($1)->push_back($3);
    $$ = $1;
}
;

VarInitVal_list
:VarInitVal{
    $$ = new std::vector<InitVal>;
    ($$)->push_back($1);
}
|VarInitVal_list ',' VarInitVal{
    ($1)->push_back($3);
    $$ = $1;
}
;


FuncFParams
:FuncFParam{
    $$ = new std::vector<FuncFParam>;
    ($$)->push_back($1);
}
|FuncFParams ',' FuncFParam{
    ($1)->push_back($3);
    $$ = $1;
}
;

FuncFParam
:INT IDENT{
    $$ = new __FuncFParam(Type::INT,$2,nullptr);
    $$->SetLineNumber(line_number);
}
|FLOAT IDENT{
    $$ = new __FuncFParam(Type::FLOAT,$2,nullptr);
    $$->SetLineNumber(line_number);
}
|INT IDENT '[' ']' Array_list{    // 二维数组
    $5->insert($5->begin(),nullptr);
    $$ = new __FuncFParam(Type::INT,$2,$5);
    $$->SetLineNumber(line_number);
}
|FLOAT IDENT '[' ']' Array_list{
    $5->insert($5->begin(),nullptr);
    $$ = new __FuncFParam(Type::FLOAT,$2,$5);
    $$->SetLineNumber(line_number);
}
|INT IDENT '[' ']'{    // 一维数组
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    $$ = new __FuncFParam(Type::INT,$2,temp);
    $$->SetLineNumber(line_number);
}
| INT IDENT '[' Exp ']'
{
    std::vector<Expression>* temp = new std::vector<Expression>; 
    temp->push_back(new Exp($4));
    $$ = new __FuncFParam(Type::INT,$2,temp);
    $$->SetLineNumber(line_number); 

}
| FLOAT IDENT '[' Exp ']'
{
    std::vector<Expression>* temp = new std::vector<Expression>; 
    temp->push_back(new Exp($4));
    $$ = new __FuncFParam(Type::FLOAT,$2,temp);
    $$->SetLineNumber(line_number); 

}
|FLOAT IDENT '[' ']'{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    $$ = new __FuncFParam(Type::FLOAT,$2,temp);
    $$->SetLineNumber(line_number);
}
|INT IDENT '[' ']' '[' ']'{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(nullptr);
    //std::vector<Expression>* temp1 = new std::vector<Expression>;
    //temp1->push_back(nullptr);
    $$ = new __FuncFParam(Type::FLOAT,$2,temp);
    $$->SetLineNumber(line_number);
}|INT IDENT '[' Exp ']' '[' Exp ']'
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(new Exp($4));
    temp->push_back(new Exp($7));
    $$ = new __FuncFParam(Type::INT,$2,temp);
    $$->SetLineNumber(line_number);  
}
|FLOAT IDENT '[' Exp ']' '[' Exp ']'
{
    std::vector<Expression>* temp = new std::vector<Expression>;
    temp->push_back(new Exp($4));
    temp->push_back(new Exp($7));
    $$ = new __FuncFParam(Type::FLOAT,$2,temp);
    $$->SetLineNumber(line_number);  
}
;
// TODO(): 考虑函数形参更多情况

//函数体，可以是代码块列表项，可以为空
Block 
:'{' BlockItem_list '}'{ 
    $$ = new __Block($2);
    $$->SetLineNumber(line_number);
}
|'{' '}'{ 
    $$ = new __Block(new std::vector<BlockItem>);
    $$->SetLineNumber(line_number);
}
;

//代码块列表项，可以为单个，可以为多个
BlockItem_list 
:BlockItem{ 
    $$ = new std::vector<BlockItem>;
    ($$)->push_back($1);
}
|BlockItem_list BlockItem{
    ($1)->push_back($2);
    $$ = $1;
}
;

//函数体内代码可以是声明Decl或语句Stmt,创建子类对象
BlockItem
:Decl{
    $$ = new BlockItem_Decl($1);
    $$->SetLineNumber(line_number);
}
|Stmt{
    $$ = new BlockItem_Stmt($1);
    $$->SetLineNumber(line_number);
}
;

//语句规则
Stmt
:';'{ //空
    $$ = new null_stmt();
    $$->SetLineNumber(line_number);
}
|Exp ';'{ //赋值语句
    $$ = new expr_stmt($1);
    $$->SetLineNumber(line_number);
}
|Lval '=' Exp ';'{ //左值赋值
    $$ = new assign_stmt($1,$3);
    $$->SetLineNumber(line_number);
}
|Lval AQ Exp ';'{
    auto add = new AddExp_plus($1,$3); 
    $$ = new assign_stmt($1,add);
    $$->SetLineNumber(line_number);
}
|Lval SQ Exp ';'{
    auto sub = new AddExp_sub($1,$3); 
    $$ = new assign_stmt($1,sub);
    $$->SetLineNumber(line_number);
}
|Lval MQ Exp ';'{
    auto mul = new MulExp_mul($1,$3);
    $$ = new assign_stmt($1,mul);
    $$->SetLineNumber(line_number);
}
|Lval DQ Exp ';'{
    auto div = new MulExp_div($1,$3);
    $$ = new assign_stmt($1,div);
    $$->SetLineNumber(line_number);
}
|Lval MODQ Exp ';'{
    auto mod = new MulExp_mod($1,$3);
    $$ = new assign_stmt($1,mod);
    $$->SetLineNumber(line_number);
}

|Block{ //函数体
    $$ = new block_stmt($1);
    $$->SetLineNumber(line_number);
}

|IF '(' Cond ')' Stmt %prec THEN{ //if语句，没有else语句所以返回空指针
    $$ = new if_stmt($3,$5);
    $$->SetLineNumber(line_number);
} 
|IF '(' Cond ')' Stmt ELSE Stmt{ //if-else语句
    $$ = new ifelse_stmt($3,$5,$7);
    $$->SetLineNumber(line_number);
}
|WHILE '(' Cond ')' Stmt{ //while语句
    $$ = new while_stmt($3,$5);
    $$->SetLineNumber(line_number);
}
/*|FOR '(' VarDecl Cond ';' Stmt ')' Stmt{ //for语句
    $$ = new for_stmt($3, $4, $6, $8);
    $$->SetLineNumber(line_number);
}*/
|FOR '(' VarDecl Cond ';' Lval '=' Exp ')' Stmt {
    auto n = new assign_stmt($6,$8);
    $$ = new for_stmt($3,$4,n,$10);
    $$->SetLineNumber(line_number);
}
|BREAK ';'{
    $$ = new break_stmt();
    $$->SetLineNumber(line_number);
}
|CONTINUE ';'{
    $$ = new continue_stmt();
    $$->SetLineNumber(line_number);
}
|RETURN Exp ';'{
    $$ = new return_stmt($2);
    $$->SetLineNumber(line_number);
}
|RETURN ';'{
    $$ = new return_stmt_void();
    $$->SetLineNumber(line_number);
}
;

//表达式被定义为加法表达式
Exp
:AddExp{$$ = $1; $$->SetLineNumber(line_number);}
;

//条件表达式被定义为逻辑或表达式
Cond
:LOrExp{$$ = $1; $$->SetLineNumber(line_number);}
;

//左值表达式（变量或数组元素等）
Lval
:IDENT{ //标识符
    $$ = new Lval($1,nullptr);
    $$->SetLineNumber(line_number);
}
| IDENT Array_list{
    $$ = new Lval($1,$2);
    $$->SetLineNumber(line_number);
}
;

//基本表达式
PrimaryExp
:'(' Exp ')'{
    $$ = new PrimaryExp_branch($2);
    $$->SetLineNumber(line_number);
}
|Lval{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
|IntConst{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
|FloatConst{
    $$ = $1; 
    $$->SetLineNumber(line_number);
}
;

IntConst
:INT_CONST{
    $$ = new IntConst($1);
    $$->SetLineNumber(line_number);
}
;

FloatConst
:FLOAT_CONST{
    $$ = new FloatConst($1);
    $$->SetLineNumber(line_number);
}
;

//条目表达式
UnaryExp
:PrimaryExp{$$ = $1;}
|IDENT '(' FuncRParams ')'{
    $$ = new Func_call($1,$3);
    $$->SetLineNumber(line_number);
}
|IDENT '(' ')'{
    // 在sylib.h这个文件中,starttime()是一个宏定义
    // #define starttime() _sysy_starttime(__LINE__)
    // 我们在语法分析中将其替换为_sysy_starttime(line_number)
    // stoptime同理
    if($1->get_string() == "starttime"){
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        $$ = new Func_call(id_table.add_id("_sysy_starttime"),temp);
        $$->SetLineNumber(line_number);
    }
    else if($1->get_string() == "stoptime"){
        auto params = new std::vector<Expression>;
        params->push_back(new IntConst(line_number));
        Expression temp = new FuncRParams(params);
        $$ = new Func_call(id_table.add_id("_sysy_stoptime"),temp);
        $$->SetLineNumber(line_number);
    }
    else{
        $$ = new Func_call($1,nullptr);
        $$->SetLineNumber(line_number);
    }
}
|'+' UnaryExp{
    $$ = new UnaryExp_plus($2);
    $$->SetLineNumber(line_number);
}
|'-' UnaryExp{
    $$ = new UnaryExp_neg($2);
    $$->SetLineNumber(line_number);
}
|'!' UnaryExp{
    $$ = new UnaryExp_not($2);
    $$->SetLineNumber(line_number);
}
;

//函数调用实参列表
FuncRParams
:Exp_list{ //为空
    $$ = new FuncRParams($1);
    $$->SetLineNumber(line_number);
}
;

//表达式列表
Exp_list
:Exp{
    $$ = new std::vector<Expression>;
    ($$)->push_back($1);
}
|Exp_list ',' Exp{
    ($1)->push_back($3);
    $$ = $1;
}
;

//乘法表达式
MulExp
:UnaryExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|MulExp '*' UnaryExp{
    $$ = new MulExp_mul($1, $3);
    $$->SetLineNumber(line_number);
}
|MulExp '/' UnaryExp{
    $$ = new MulExp_div($1, $3);
    $$->SetLineNumber(line_number);
}
|MulExp '%' UnaryExp{
    $$ = new MulExp_mod($1, $3);
    $$->SetLineNumber(line_number);
}
;

//加法表达式
AddExp
:MulExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|AddExp '+' MulExp{
    $$ = new AddExp_plus($1,$3); 
    $$->SetLineNumber(line_number);
}
|AddExp '-' MulExp{
    $$ = new AddExp_sub($1,$3); 
    $$->SetLineNumber(line_number);
}
;

//关系表达式
RelExp
:AddExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|RelExp '<' AddExp{
    $$ = new RelExp_lt($1, $3);
    $$->SetLineNumber(line_number);
}
|RelExp '>' AddExp{
    $$ = new RelExp_gt($1, $3);
    $$->SetLineNumber(line_number);
}
|RelExp LEQ AddExp{
    $$ = new RelExp_leq($1, $3);
    $$->SetLineNumber(line_number);
}
|RelExp GEQ AddExp{
    $$ = new RelExp_geq($1, $3);
    $$->SetLineNumber(line_number);
}
;

//等价表达式
EqExp
:RelExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|EqExp EQ RelExp{
    $$ = new EqExp_eq($1, $3);
    $$->SetLineNumber(line_number);
}
|EqExp NE RelExp{
    $$ = new EqExp_neq($1, $3);
    $$->SetLineNumber(line_number);
}
;

//逻辑与表达式
LAndExp
:EqExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|LAndExp AND EqExp{
    $$ = new LAndExp_and($1, $3);
    $$->SetLineNumber(line_number);
}
;

//逻辑或表达式
LOrExp
:LAndExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
|LOrExp OR LAndExp{
    $$ = new LOrExp_or($1, $3);
    $$->SetLineNumber(line_number);
}
;

//常量表达式
ConstExp
:AddExp{
    $$ = $1;
    $$->SetLineNumber(line_number);
}
;

// TODO: 也许你需要添加更多的非终结符


%% 

void yyerror(char* s, ...)
{
    ++error_num;
    fout<<"parser error in line "<<line_number<<"\n";
}