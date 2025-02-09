%option noyywrap 
%{
#include <cmath>
#include <vector>
#include <cstring>
#include "../include/symtab.h"
#include "../parser/SysY_parser.tab.h"

extern int line_number; /*记录行号*/
extern int col_number; /*记录列号*/
extern int cur_col_number; /*记录当前token起始列号*/
extern IdTable id_table; /*用于管理标识符的表*/
void yyerror(char* s, ...);
%}
%x COMMENT
%%
    /*TODO():增加处理注释的代码*/
"//"[^\n]* { cur_col_number = col_number;col_number += strlen(yytext);}/* 忽略单行注释 */
"/*" {cur_col_number = col_number;col_number += strlen(yytext);BEGIN(COMMENT); }
<COMMENT>. {cur_col_number = col_number;col_number += strlen(yytext);}// 处理每个字符，包括换行符以外的其他字符
<COMMENT>\n {++line_number;col_number = 0;}
<COMMENT>"*/" { cur_col_number = col_number;col_number += strlen(yytext);BEGIN(INITIAL);}  

    /*TODO():增加处理列号的代码(cur_col_number表示当前token开始位置, col_number表示当前token结束位置)*/
"<=" {cur_col_number = col_number;col_number += strlen(yytext);return LEQ;}         
">=" {cur_col_number = col_number;col_number += strlen(yytext);return GEQ;}         
"==" {cur_col_number = col_number;col_number += strlen(yytext);return EQ;}        
"!=" {cur_col_number = col_number;col_number += strlen(yytext);return NE;}
"+=" {cur_col_number = col_number;col_number += strlen(yytext);return AQ;}
"-=" {cur_col_number = col_number;col_number += strlen(yytext);return SQ;}
"*=" {cur_col_number = col_number;col_number += strlen(yytext);return MQ;}
"/=" {cur_col_number = col_number;col_number += strlen(yytext);return DQ;}
"%=" {cur_col_number = col_number;col_number += strlen(yytext);return MODQ;}
"&&" {cur_col_number = col_number;col_number += strlen(yytext);return AND;}         
"||" {cur_col_number = col_number;col_number += strlen(yytext);return OR;}               
"const" {cur_col_number = col_number;col_number += strlen(yytext);return CONST;}     
"if" {cur_col_number = col_number;col_number += strlen(yytext);return IF;}       
"else" {cur_col_number = col_number;col_number += strlen(yytext);return ELSE;}  
"while" {cur_col_number = col_number;col_number += strlen(yytext);return WHILE;}
"for" {cur_col_number = col_number;col_number += strlen(yytext);return FOR;}
"void" {cur_col_number = col_number;col_number += strlen(yytext);return NONE_TYPE;}    
"int" {cur_col_number = col_number;col_number += strlen(yytext);return INT;}    
"float" {cur_col_number = col_number;col_number += strlen(yytext);return FLOAT;}      
"return" {cur_col_number = col_number;col_number += strlen(yytext);return RETURN;}    
"break" {cur_col_number = col_number;col_number += strlen(yytext);return BREAK;}     
"continue" {cur_col_number = col_number;col_number += strlen(yytext);return CONTINUE;}

\n {++line_number;col_number = 0;} //换行符

[ \t\f\r\v] {col_number += strlen(yytext);} //空格、制表符、换页符、回车符和垂直制表符

[\.\+\-\*\/\=\<\!\%\>] {cur_col_number = col_number;col_number += strlen(yytext);return yytext[0];} 

[\{\}\;\(\)\,\[\]] {cur_col_number = col_number;col_number += strlen(yytext);return yytext[0];} 

[_a-zA-Z][_a-zA-Z0-9]* { 
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.symbol_token = id_table.add_id(std::string(yytext));
    return IDENT;
}
([1-9][0-9]*)|0 { /*处理整数常量，并计算其数值*/
    cur_col_number = col_number;
    col_number += strlen(yytext);
    // yylval.int_token = stoi(std::string(yytext));
    yylval.int_token = 0;
    for(int i = 0;yytext[i];i++){
        yylval.int_token *= 10;
        yylval.int_token += yytext[i] - '0';
    }
    return INT_CONST;
}
    /*
      在词法分析中，你只需要考虑yylval的三种类型，分别为int_token, float_token, symbol_token
      之所以要定义这三种类型，是因为我们只返回这个token的类型是不够的，我们还需要知道这个token的属性
      例如我们需要知道INT_CONST具体的值是多少
    */

    /*TODO():参考SysY2022定义, 处理更多的词法, 如浮点数，十六进制数等*/
    
    /*处理浮点数*/
(([0-9]*\.[0-9]+)|([0-9]+\.))([eE][\+\-]?[0-9]+)? {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.float_token = 0.0;
    int index = 0;

    /*处理整数部分*/
    int integer_part = 0;
    while(yytext[index] != '.'){
        integer_part *= 10;
        integer_part += yytext[index]-'0';
        index++;
    }

    index++; //跳过'.'

    /*处理小数部分*/
    double decimal_part = 0.0;
    double decimal_place = 0.1;
    while(yytext[index] != 'e' && yytext[index] != 'E'&& yytext[index] != '\0'){
        decimal_part += ((yytext[index]-'0') * decimal_place);
        decimal_place /= 10;
        index++;
    }

    /*处理科学计数法部分*/
    double exponent = 0.0;
    int sign = 1;
    if (yytext[index] == 'e' || yytext[index] == 'E'){
        index++;
        if (yytext[index] == '+' || yytext[index] == '-'){
            sign = (yytext[index] == '+') ? 1 : -1;
            index++;
        }
        while(isdigit(yytext[index])){
            exponent *= 10;
            exponent += yytext[index] - '0';
            index++;
        }
    }

    /*整合整数和小数部分*/
    yylval.float_token = integer_part + decimal_part;
    exponent *= sign;
    yylval.float_token *= pow(10,exponent);

    return FLOAT_CONST;
}

[0-9]+[eE][\+\-]?[0-9]+ {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.float_token = 0.0;
    int index = 0;

    /*处理整数部分*/
    int integer_part = 0;
    while(yytext[index] != 'e' && yytext[index] != 'E'){
        integer_part *= 10;
        integer_part += yytext[index]-'0';
        index++;
    }

    /*处理科学计数法部分*/
    double exponent = 0.0;
    int sign = 1;
    if (yytext[index] == 'e' || yytext[index] == 'E'){
        index++;
        if (yytext[index] == '+' || yytext[index] == '-'){
            sign = (yytext[index] == '+') ? 1 : -1;
            index++;
        }
        while(isdigit(yytext[index])){
            exponent *= 10;
            exponent += yytext[index] - '0';
            index++;
        }
    }
    yylval.float_token = integer_part;

    if(sign == 1){
        yylval.float_token *= pow(10,exponent);
    }
    if(sign == -1){
        yylval.float_token /= pow(10,exponent);
    }

    return FLOAT_CONST;
}

    /*处理十六进制数*/
("0"[xX]([0-9a-fA-F]*\.[0-9a-fA-F]+)|([0-9a-fA-F]+\.))([pP][\+\-]?[0-9]+)? {  
    cur_col_number = col_number;  
    col_number += strlen(yytext);  
    std::string str = yytext;  
      
    /* 处理整数部分 */  
    size_t integer_end_index = str.find('.');  
    int integer_num = 0;  
    unsigned int integer = 0;  
    for (size_t i = 2; i < integer_end_index; ++i) {  
        char digit = str[i];  
        integer += (isdigit(digit) ? digit - '0' : tolower(digit) - 'a' + 10) * pow(16, integer_end_index - i - 1);  
    }  
  
    /* 处理小数部分 */  
    double decimal = 0.0;  
    int decimal_place = -1;  
    for (size_t i = integer_end_index + 1; i < str.size(); ++i) {  
        if (str[i] == 'p' || str[i] == 'P' || str[i] == '0') break;  
        char digit = str[i];  
        decimal += (isdigit(digit) ? digit - '0' : tolower(digit) - 'a' + 10) * pow(16, decimal_place--);  
    }  
  
    /* 处理二进制计数法部分 */  
    double exponent = 0.0;  
    int sign = 1;  
    size_t exp_start_index = str.find_first_of("pP");  
    if (exp_start_index != std::string::npos) {  
        ++exp_start_index; // 跳过'p'或'P'  
        if (str[exp_start_index] == '+' || str[exp_start_index] == '-') {  
            sign = (str[exp_start_index] == '+') ? 1 : -1;  
            ++exp_start_index;  
        }  
        while (isdigit(str[exp_start_index])) {  
            exponent = exponent * 10 + (str[exp_start_index] - '0');  
            ++exp_start_index;  
        }  
    }  
  
    /* 整合整数和小数部分 */  
    double value = integer + decimal;  
    if (sign == 1) {  
        value *= pow(2, exponent);  
    } else {  
        value /= pow(2, exponent);  
    }  
  
    yylval.float_token = value;  
    return FLOAT_CONST;  
}

"0"[xX][0-9a-fA-F]+[pP][\+\-]?[0-9]+ {  
    cur_col_number = col_number;  
    col_number += strlen(yytext);  
    std::string str = yytext;  
      
    /* 处理整数部分 */  
    int integer_end_index = 0; 
    while (str[integer_end_index] != 'p' && str[integer_end_index] != 'P') {
        integer_end_index++;
    }
    int integer_num = 0;  
    unsigned int integer = 0;  
    for (size_t i = 2; i < integer_end_index; ++i) {  
        char digit = str[i];  
        integer += (isdigit(digit) ? digit - '0' : tolower(digit) - 'a' + 10) * pow(16, integer_end_index - i - 1);  
    }  
  
  
    /* 处理二进制计数法部分 */  
    double exponent = 0.0;  
    int sign = 1;  
    size_t exp_start_index = integer_end_index;  
    if (exp_start_index != std::string::npos) {  
        ++exp_start_index; // 跳过'p'或'P'  
        if (str[exp_start_index] == '+' || str[exp_start_index] == '-') {  
            sign = (str[exp_start_index] == '+') ? 1 : -1;  
            ++exp_start_index;  
        }  
        while (isdigit(str[exp_start_index])) {  
            exponent = exponent * 10 + (str[exp_start_index] - '0');  
            ++exp_start_index;  
        }  
    } 
  
    /* 计算最终值 */  
    double value = integer;  
    if (sign == 1) {  
        value *= pow(2, exponent);  
    } else {  
        value /= pow(2, exponent);  
    }  
  
    yylval.float_token = value;  
    return FLOAT_CONST;  
}

"0"[xX][0-9a-fA-F]+ {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    int end = strlen(yytext);
    int integer_num = 0;  
    unsigned int integer = 0;  
    for (size_t i = 2; i < end; ++i) {  
        char digit = yytext[i];  
        integer += (isdigit(digit) ? digit - '0' : tolower(digit) - 'a' + 10) * pow(16, end - i - 1);  
    }  
    yylval.int_token = integer;
    return INT_CONST;
}

  /*处理八进制*/
"0"[0-7]* {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.int_token = 0;
    int k_8=1;
    for(int i=strlen(yytext)-1;i>0;i--)
    {
       yylval.int_token+=k_8*(yytext[i]-'0');
       k_8*=8;
    }
   return INT_CONST;
}

    /*unknown tokens, return ERROR*/
. {
    cur_col_number = col_number;
    col_number += strlen(yytext);
    yylval.error_msg = yytext;
    return ERROR;
}
%%



