#include "semant.h"
#include "../include/SysY_tree.h"
#include "../include/ir.h"
#include "../include/type.h"
#include "../include/Instruction.h"
#include "../include/basic_block.h"
#include "../include/tree.h"
#include <assert.h>
/*
    语义分析阶段需要对语法树节点上的类型和常量等信息进行标注, 即NodeAttribute类
    同时还需要标注每个变量的作用域信息，即部分语法树节点中的scope变量
    你可以在utils/ast_out.cc的输出函数中找到你需要关注哪些语法树节点中的NodeAttribute类及其他变量
    以及对语义错误的代码输出报错信息
*/

/*
    错误检查的基本要求:
    • 检查 main 函数是否存在 (根据SysY定义，如果不存在main函数应当报错)；
    • 检查未声明变量，及在同一作用域下重复声明的变量；
    • 条件判断和运算表达式：int 和 bool 隐式类型转换（例如int a=5，return a+!a）；
    • 数值运算表达式：运算数类型是否正确 (如返回值为 void 的函数调用结果是否参与了其他表达式的计算)；
    • 检查未声明函数，及函数形参是否与实参类型及数目匹配；
    • 检查是否存在整型变量除以整型常量0的情况 (如对于表达式a/(5-4-1)，编译器应当给出警告或者直接报错)；

    错误检查的进阶要求:
    • 对数组维度进行相应的类型检查 (例如维度是否有浮点数，定义维度时是否为常量等)；
    • 对float进行隐式类型转换以及其他float相关的检查 (例如模运算中是否有浮点类型变量等)；
*/
extern LLVMIR llvmIR;

//extern enum LLVMType { I32 = 1, FLOAT32 = 2, PTR = 3, VOID = 4, I8 = 5, I1 = 6, I64 = 7, DOUBLE = 8 };

// BasicInstruction::LLVMType Type2LLvm[6] = {
//     BasicInstruction::LLVMType::VOID, 
//     BasicInstruction::LLVMType::I32, 
//     BasicInstruction::LLVMType::FLOAT32,
//     BasicInstruction::LLVMType::I1,   
//     BasicInstruction::LLVMType::PTR, 
//     BasicInstruction::LLVMType::DOUBLE};

extern BasicInstruction::LLVMType Type2LLvm[6];

SemantTable semant_table;
std::vector<std::string> error_msgs{}; // 将语义错误信息保存到该变量中
static int InWhileCount = 0;
bool havemain = false;

std::map<std::string, VarAttribute> ConstGlobalMap;
std::map<std::string, VarAttribute> StaticGlobalMap;

/*
LLVMType Type2LLvm[6] = {LLVMType::VOID, LLVMType::I32, LLVMType::FLOAT32,
                         LLVMType::I1,   LLVMType::PTR, LLVMType::DOUBLE};
                         */

extern NodeAttribute SemantBinaryNode(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode opcode);
extern NodeAttribute SemantSingleNode(NodeAttribute a, NodeAttribute::opcode opcode);

//类型检查
extern std::vector<std::string> error_msgs;

//类型转换
bool typecheck(Type::ty a, Type::ty b) {
    if (a == b) {
        return true;
    }
    else if (a == Type::INT && b == Type::FLOAT) {
        return true;
    } else if (a == Type::INT && b == Type::BOOL) {
        return true;
    } else if (a == Type::FLOAT && b == Type::INT) {
        return true;
    } else if (a == Type::FLOAT && b == Type::BOOL) {
        return true;
    } else if (a == Type::BOOL && b == Type::INT) {
        return true;
    } else if (a == Type::BOOL && b == Type::FLOAT) {
        return true;
    } 
    else {
        return false;
    }
}

void RecursiveArrayInit(const InitVal &init, VarAttribute &val, int startPosition, int endPosition, int currentDimIndex) {
    int position = startPosition; // 初始化位置
    
    // 计算当前维度之后所有维度的乘积
    int blockSize = 1;
    for (int i = currentDimIndex + 1; i < val.dims.size(); ++i) {
        blockSize *= val.dims[i];
    }

    // 遍历初始化值列表
    for (const auto &initValue : *(init->GetList())) {
        if (initValue->IsExp()) {
            // 处理表达式初始化值
            if (initValue->attribute.T.type == Type::VOID) {
                error_msgs.push_back("初始化值中的表达式不能为void类型，位于行：" + std::to_string(init->GetLineNumber()) + "\n");
                continue;
            }

            if (val.type == Type::INT) {
                if (initValue->attribute.T.type == Type::INT) {
                    val.IntInitVals[position] = initValue->attribute.V.val.IntVal;
                } else if (initValue->attribute.T.type == Type::FLOAT) {
                    val.IntInitVals[position] = initValue->attribute.V.val.FloatVal;
                }
            }
            if (val.type == Type::FLOAT) {
                if (initValue->attribute.T.type == Type::INT) {
                    val.FloatInitVals[position] = initValue->attribute.V.val.IntVal;
                } else if (initValue->attribute.T.type == Type::FLOAT) {
                    val.FloatInitVals[position] = initValue->attribute.V.val.FloatVal;
                }
            }

            position++;
        } else {
            // 处理嵌套的多维数组初始化值
            // 递归初始化嵌套的多维数组
            int subBlockSize = blockSize;
            RecursiveArrayInit(initValue, val, position, position + subBlockSize - 1, currentDimIndex + 1);
            position += subBlockSize;
        }
    }
}

//获取指定索引位置的整数值
int GetArrayIntVal(VarAttribute &val, std::vector<int> &indexs) {
    int idx = 0;
    for (int curIndex = 0; curIndex < indexs.size(); curIndex++) {
        idx *= val.dims[curIndex];
        idx += indexs[curIndex];
    }
    return val.IntInitVals[idx];
}

float GetArrayFloatVal(VarAttribute &val, std::vector<int> &indexs) {
    int idx = 0;
    for (int curIndex = 0; curIndex < indexs.size(); curIndex++) {
        idx *= val.dims[curIndex];
        idx += indexs[curIndex];
    }
    return val.FloatInitVals[idx];
}


//二元运算符 INT
NodeAttribute BinaryCalculateInt(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode op) {
    NodeAttribute result;
    result.V.ConstTag = a.V.ConstTag & b.V.ConstTag;//如果a、b都是常量则进行计算

    if(op == NodeAttribute::ADD||op == NodeAttribute::SUB||op == NodeAttribute::MUL||op == NodeAttribute::DIV||op == NodeAttribute::MOD)
    {
            result.T.type = Type::INT;
    }else
    {
            result.T.type = Type::BOOL;
    }

    if (!result.V.ConstTag) {
        // 如果任意一个不是常量，则不进行计算
        return result;
    }

    switch (op) {
        case NodeAttribute::ADD:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal + b.V.val.IntVal;
            break;
        case NodeAttribute::SUB:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal - b.V.val.IntVal;
            break;
        case NodeAttribute::MUL:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal * b.V.val.IntVal;
            break;
        case NodeAttribute::DIV:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal / b.V.val.IntVal;
            // if (b.V.val.IntVal == 0) {
            //     // 抛出错误或记录错误信息
            //     error_msgs.push_back("Division by zero");
            // }
            break;
        case NodeAttribute::MOD:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal % b.V.val.IntVal;
            break;
        case NodeAttribute::GEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal >= b.V.val.IntVal;
            break;
        case NodeAttribute::GT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal > b.V.val.IntVal;
            break;
        case NodeAttribute::LEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal <= b.V.val.IntVal;
            break;
        case NodeAttribute::LT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal < b.V.val.IntVal;
            break;
        case NodeAttribute::EQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal == b.V.val.IntVal;
            break;
        case NodeAttribute::NEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal != b.V.val.IntVal;
            break;
        case NodeAttribute::OR:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal || b.V.val.IntVal;
            break;
        case NodeAttribute::AND:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.IntVal && b.V.val.IntVal;
            break;
        default:
            // 不支持的操作符
            break;
    }

    return result;
}

//二元运算符 FLOAT
NodeAttribute BinaryCalculateFloat(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode op) {
    NodeAttribute result;
    result.V.ConstTag = a.V.ConstTag & b.V.ConstTag;//如果a、b都是常量则进行计算
     if(op == NodeAttribute::ADD||op == NodeAttribute::SUB||op == NodeAttribute::MUL||op == NodeAttribute::DIV||op == NodeAttribute::MOD)
    {
            result.T.type = Type::FLOAT;
    }else
    {
            result.T.type = Type::BOOL;
    }

    if (!result.V.ConstTag) {
        // 如果任意一个不是常量，则不进行计算
        return result;
    }

    switch (op) {
        case NodeAttribute::ADD:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = a.V.val.FloatVal + b.V.val.FloatVal;
            break;
        case NodeAttribute::SUB:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = a.V.val.FloatVal - b.V.val.FloatVal;
            break;
        case NodeAttribute::MUL:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = a.V.val.FloatVal * b.V.val.FloatVal;
            break;
        case NodeAttribute::DIV:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = a.V.val.FloatVal / b.V.val.FloatVal;
            // if (b.V.val.FloatVal == 0.0) {
            //     // 抛出错误或记录错误信息
            //     error_msgs.push_back("Division by zero");
            // }
            break;
        case NodeAttribute::MOD:
            result.T.type = Type::VOID;
            result.V.ConstTag = 0;
            error_msgs.push_back("mod on float type in line " + std::to_string(a.line_number) + "\n");
            break;
        case NodeAttribute::GEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal >= b.V.val.FloatVal;
            break;
        case NodeAttribute::GT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal > b.V.val.FloatVal;
            break;
        case NodeAttribute::LEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal <= b.V.val.FloatVal;
            break;
        case NodeAttribute::LT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal < b.V.val.FloatVal;
            break;
        case NodeAttribute::EQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal == b.V.val.FloatVal;
            break;
        case NodeAttribute::NEQ:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal != b.V.val.FloatVal;
            break;
        case NodeAttribute::OR:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal || b.V.val.FloatVal;
            break;
        case NodeAttribute::AND:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = a.V.val.FloatVal && b.V.val.FloatVal;
            break;
        default:
            // 不支持的操作符
            break;
    }

    return result;
}

NodeAttribute tmp_a, tmp_b;

//类型转换
NodeAttribute SemantBinaryNode(NodeAttribute a, NodeAttribute b, NodeAttribute::opcode opcode) {
    switch (a.T.type) {
        case Type::INT:
            switch (b.T.type) {
                case Type::INT:   
                    return BinaryCalculateInt(a, b, opcode);
                    break;
                case Type::FLOAT: 
                    tmp_a = a;
                    tmp_a.T.type = Type::FLOAT;
                    tmp_a.V.val.FloatVal = (float)tmp_a.V.val.IntVal;
                    return BinaryCalculateFloat(tmp_a, b, opcode);
                    break;
                case Type::BOOL:  
                    tmp_b = b;
                    tmp_b.T.type = Type::INT;
                    tmp_b.V.val.IntVal = tmp_b.V.val.BoolVal;
                    return BinaryCalculateInt(a, tmp_b, opcode);
                    break;
                default:          
                    //error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
                    NodeAttribute result;
                    result.T.type = Type::VOID;
                    result.V.ConstTag = 0;
                    return result;// PTR or VOID
                    break;
            }
            break;
        case Type::FLOAT:
            switch (b.T.type) {
                case Type::INT:  
                    tmp_b = b;
                    tmp_b.T.type = Type::FLOAT;
                    tmp_b.V.val.FloatVal = (float)tmp_b.V.val.IntVal; 
                    return BinaryCalculateFloat(a, tmp_b, opcode);
                    break;
                case Type::FLOAT: 
                    return BinaryCalculateFloat(a, b, opcode);
                    break;
                case Type::BOOL:  
                    tmp_b = b;
                    tmp_b.T.type = Type::FLOAT;
                    tmp_b.V.val.FloatVal = tmp_b.V.val.BoolVal;
                    return BinaryCalculateFloat(a, tmp_b, opcode);
                    break;
                default:          
                    //error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
                    NodeAttribute result;
                    result.T.type = Type::VOID;
                    result.V.ConstTag = 0;
                    return result; // PTR or VOID
                    break;
            }
            break;
        case Type::BOOL:
            switch (b.T.type) {
                case Type::INT:
                    tmp_a = a;
                    tmp_a.T.type = Type::INT;
                    tmp_a.V.val.IntVal = tmp_a.V.val.BoolVal;   
                    return BinaryCalculateInt(tmp_a, b, opcode);
                    break;
                case Type::FLOAT:
                    tmp_a = a;
                    tmp_a.T.type = Type::FLOAT;
                    tmp_a.V.val.FloatVal = tmp_a.V.val.BoolVal; 
                    return BinaryCalculateFloat(tmp_a, b, opcode);
                    break;
                case Type::BOOL:
                    tmp_a = a, tmp_b = b;
                    tmp_a.T.type = Type::INT;
                    tmp_b.T.type = Type::INT;
                    tmp_a.V.val.IntVal = tmp_a.V.val.BoolVal;
                    tmp_b.V.val.IntVal = tmp_b.V.val.BoolVal;  
                    return BinaryCalculateInt(tmp_a, tmp_b, opcode);
                    break;
                default:          
                    //error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
                    NodeAttribute result;
                    result.T.type = Type::VOID;
                    result.V.ConstTag = 0;
                    return result; // PTR or VOID
                    break;
            }
            break;
        default: // a.T.type is PTR or VOID
            //error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
            NodeAttribute result;
            result.T.type = Type::VOID;
            result.V.ConstTag = 0;
            return result; // PTR or VOID
            break;
    }
}

//单目运算 INT
NodeAttribute SingleCalculateInt(NodeAttribute a, NodeAttribute::opcode op) {
    NodeAttribute result;
    result.V.ConstTag = a.V.ConstTag;
    if(op == NodeAttribute::NOT)
    {
            result.T.type = Type::BOOL;
    }else
    {
            result.T.type = Type::INT;
    }

    if (!result.V.ConstTag) {
        // 如果任意一个不是常量，则不进行计算
        return result;
    }

    switch (op) {
        case NodeAttribute::ADD:
            result.T.type = Type::INT;
            result.V.val.IntVal = a.V.val.IntVal;
            break;
        case NodeAttribute::SUB:
            result.T.type = Type::INT;
            result.V.val.IntVal = -a.V.val.IntVal;
            break;
        case NodeAttribute::NOT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = !a.V.val.IntVal;
            break;
        default:
            // 不支持的操作符
            break;
    }

    return result;
}

//单目运算 FLOAT
NodeAttribute SingleCalculateFloat(NodeAttribute a, NodeAttribute::opcode op) {
    NodeAttribute result;
    result.V.ConstTag = a.V.ConstTag;

    if(op == NodeAttribute::NOT)
    {
            result.T.type = Type::BOOL;
    }else
    {
            result.T.type = Type::FLOAT;
    }

    if (!result.V.ConstTag) {
        // 如果任意一个不是常量，则不进行计算
        return result;
    }

    switch (op) {
        case NodeAttribute::ADD:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = a.V.val.FloatVal;
            break;
        case NodeAttribute::SUB:
            result.T.type = Type::FLOAT;
            result.V.val.FloatVal = -a.V.val.FloatVal;
            break;
        case NodeAttribute::NOT:
            result.T.type = Type::BOOL;
            result.V.val.BoolVal = !a.V.val.FloatVal;
            break;
        default:
            // 不支持的操作符
            break;
    }

    return result;
}

//类型转换
NodeAttribute SemantSingleNode(NodeAttribute a, NodeAttribute::opcode opcode) {
    switch (a.T.type) {
        case Type::INT:   
            return SingleCalculateInt(a, opcode);
            break;
        case Type::FLOAT: 
            return SingleCalculateFloat(a, opcode);
            break;
        case Type::BOOL:  
            tmp_a = a;
            tmp_a.T.type = Type::INT;
            tmp_a.V.val.IntVal = a.V.val.BoolVal;
            return SingleCalculateInt(tmp_a, opcode);
            break;
        default:          
            //error_msgs.push_back("invalid operators in line " + std::to_string(a.line_number) + "\n");
            NodeAttribute result;
            result.T.type = Type::VOID;
            result.V.ConstTag = 0;
            return result;
            break;
    }
}

void __Program::TypeCheck() {
    semant_table.symbol_table.enter_scope();
    auto comp_vector = *comp_list;
    for (auto comp : comp_vector) {
        comp->TypeCheck();
    }
    if (havemain==false){
        error_msgs.push_back("Do not have main.\n");
    }
}

void Exp::TypeCheck() {
    addexp->TypeCheck();

    attribute = addexp->attribute;
}

void AddExp_plus::TypeCheck() {
    addexp->TypeCheck();//对左侧的加法表达式进行类型检查
    mulexp->TypeCheck();//对右侧的乘法表达式进行类型检查

    //实现二元表达式的语义分析逻辑
    attribute = SemantBinaryNode(addexp->attribute, mulexp->attribute, NodeAttribute::ADD);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void AddExp_sub::TypeCheck() {
    addexp->TypeCheck();
    mulexp->TypeCheck();

    attribute = SemantBinaryNode(addexp->attribute, mulexp->attribute, NodeAttribute::SUB);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void MulExp_mul::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    attribute = SemantBinaryNode(mulexp->attribute, unary_exp->attribute, NodeAttribute::MUL);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void MulExp_div::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    // 检查除数是否为零
    if (unary_exp->attribute.V.val.IntVal == 0 && unary_exp->attribute.V.ConstTag && unary_exp->attribute.T.type == Type::INT) {
        // 抛出错误或记录错误信息
        error_msgs.push_back("Division by zero in line " + std::to_string(unary_exp->GetLineNumber()) + "\n");
        attribute.T.type = Type::VOID;//将当前节点设置为无效类型
        attribute.V.ConstTag = false;
        return;
    }

    attribute = SemantBinaryNode(mulexp->attribute, unary_exp->attribute, NodeAttribute::DIV);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void MulExp_mod::TypeCheck() {
    mulexp->TypeCheck();
    unary_exp->TypeCheck();

    attribute = SemantBinaryNode(mulexp->attribute, unary_exp->attribute, NodeAttribute::MOD);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void RelExp_leq::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = SemantBinaryNode(relexp->attribute, addexp->attribute, NodeAttribute::LEQ);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void RelExp_lt::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = SemantBinaryNode(relexp->attribute, addexp->attribute, NodeAttribute::LT);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void RelExp_geq::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = SemantBinaryNode(relexp->attribute, addexp->attribute, NodeAttribute::GEQ);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void RelExp_gt::TypeCheck() {
    relexp->TypeCheck();
    addexp->TypeCheck();

    attribute = SemantBinaryNode(relexp->attribute, addexp->attribute, NodeAttribute::GT);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void EqExp_eq::TypeCheck() {
    eqexp->TypeCheck();
    relexp->TypeCheck();

    attribute = SemantBinaryNode(eqexp->attribute, relexp->attribute, NodeAttribute::EQ);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void EqExp_neq::TypeCheck() {
    eqexp->TypeCheck();
    relexp->TypeCheck();

    attribute = SemantBinaryNode(eqexp->attribute, relexp->attribute, NodeAttribute::NEQ);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void LAndExp_and::TypeCheck() {
    landexp->TypeCheck();
    eqexp->TypeCheck();

    attribute = SemantBinaryNode(landexp->attribute, eqexp->attribute, NodeAttribute::AND);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void LOrExp_or::TypeCheck() {
    lorexp->TypeCheck();
    landexp->TypeCheck();
    
    attribute = SemantBinaryNode(lorexp->attribute, landexp->attribute, NodeAttribute::OR);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

//检查常量表达式的类型正确性
void ConstExp::TypeCheck() {
    addexp->TypeCheck();
    attribute = addexp->attribute;
    if (!attribute.V.ConstTag) {    // addexp is not const
        error_msgs.push_back("Expression is not const " + std::to_string(line_number) + "\n");
    }
}

void Lval::TypeCheck() { 
    is_left = false;

    std::vector<int> arrayDimensions;
    bool allDimensionsConstant = true;
 
    if (dims != nullptr) {
        for (auto dim : *dims) {
            dim->TypeCheck();
    
            if (dim->attribute.T.type == Type::VOID) {
                error_msgs.push_back("Array dimension cannot be void at line " + std::to_string(line_number) + "\n");
            } else if (dim->attribute.T.type == Type::FLOAT) {
                error_msgs.push_back("Array dimension cannot be float at line " + std::to_string(line_number) + "\n");
            }
                arrayDimensions.push_back(dim->attribute.V.val.IntVal); 
                allDimensionsConstant &= dim->attribute.V.ConstTag;

            
        }
    }

    // 查找变量的属性信息
    VarAttribute val = semant_table.symbol_table.lookup_val(name);

    // 检查变量是否在当前作用域中定义
    if (val.type != Type::VOID) {    // local var
        scope = semant_table.symbol_table.lookup_scope(name);
    } else if (semant_table.GlobalTable.find(name) != semant_table.GlobalTable.end()) {    // global var
        val = semant_table.GlobalTable[name];
        scope = 0;
    } else {
        error_msgs.push_back("Undefined var in line " + std::to_string(line_number) + "\n");
        return;
    }

    
/*
    // 如果变量是常量，获取其值
    if (attribute.V.ConstTag) {
        if (attribute.T.type == Type::INT) {
            attribute.V.val.IntVal = val.getIntValue();
        } else if (attribute.T.type == Type::FLOAT) {
            attribute.V.val.FloatVal = val.getFloatValue();
        }
    }
    */

    if (arrayDimensions.size() == val.dims.size()) {
        //不是数组
        attribute.V.ConstTag = val.ConstTag & allDimensionsConstant; 
        attribute.T.type = val.type;
    
        if (attribute.V.ConstTag) {
            // 如果 val 是常量，计算其值
            if (attribute.T.type == Type::INT) {
                attribute.V.val.IntVal = GetArrayIntVal(val, arrayDimensions);
            } else if (attribute.T.type == Type::FLOAT) {
                attribute.V.val.FloatVal = GetArrayFloatVal(val, arrayDimensions);
            }
        }
    } else if (arrayDimensions.size() < val.dims.size()) {
        // lval 是一个数组（提供的索引少于 val 的维度）
        attribute.V.ConstTag = false;
        attribute.T.type = Type::PTR; // 指向数组的指针
    } else {
        // 提供的索引多于 val 的维度，错误
        error_msgs.push_back("Array index count exceeds value dimensions at line " + std::to_string(line_number) + "\n");
    }
 }

void FuncRParams::TypeCheck() { 
    for (auto &param : *params) {
        param->TypeCheck();  // 对每个参数进行类型检查
        if (param->attribute.T.type == Type::VOID) {
            error_msgs.push_back("FuncRParam is void in line " + std::to_string(line_number) + "\n");
        }
    }
}

void Func_call::TypeCheck() { 
    int funcr_params_len = 0;
    if (funcr_params != nullptr) {
        funcr_params_len = ((FuncRParams *)funcr_params)->params->size();
        funcr_params->TypeCheck();  // 对参数列表进行类型检查
    }

    //在符号表中查找函数名，如果未找到，记录错误信息并返回。
    auto it = semant_table.FunctionTable.find(name);
    //find方法返回一个迭代器，指向找到的元素；如果未找到，则返回end()迭代器
    if (it == semant_table.FunctionTable.end()) {
        error_msgs.push_back("Function '" + name->get_string() + "' is undefined in line " +
                             std::to_string(line_number) + "\n");
        return;
    }

    //获取函数的形式参数类型列表和返回类型
    FuncDef funcdef = it->second;
    //检查实际参数的数量是否与形式参数的数量相等。如果不相等，记录错误信息。
    if ((funcdef->formals)->size() != funcr_params_len) {
        error_msgs.push_back("Function '" + name->get_string() + "' expects " +
                             std::to_string((funcdef->formals)->size()) + " arguments, but " +
                             std::to_string(funcr_params_len) + " were provided in line " +
                             std::to_string(line_number) + "\n");
    }
    bool flag = true;
    for (int i = 0 ; i < funcr_params_len; i++) {
            auto a_params = (*((FuncRParams *)funcr_params)->params)[i];
            auto f_params = (*(funcdef->formals))[i];
            flag = typecheck(a_params->attribute.T.type, f_params->attribute.T.type);
            if (flag == false){
                error_msgs.push_back("Formal and actual parameters have incompatible types");
                break;
            }
        }
    
    //设置当前函数调用的返回类型和常量属性
    attribute.T.type = funcdef->return_type;
    attribute.V.ConstTag = false;

}

void UnaryExp_plus::TypeCheck() { 
    unary_exp->TypeCheck();
    attribute = SemantSingleNode(unary_exp->attribute, NodeAttribute::ADD);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
 }

void UnaryExp_neg::TypeCheck() {
    unary_exp->TypeCheck();
    attribute = SemantSingleNode(unary_exp->attribute, NodeAttribute::SUB);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}


void UnaryExp_not::TypeCheck() { 
    unary_exp->TypeCheck();
    attribute = SemantSingleNode(unary_exp->attribute, NodeAttribute::NOT);
    if (attribute.T.type == Type::VOID)
    {
        error_msgs.push_back("invalid operators in line " + std::to_string(line_number) + "\n");
    }
}

void IntConst::TypeCheck() {
    attribute.T.type = Type::INT;
    attribute.V.ConstTag = true;
    attribute.V.val.IntVal = val;
}

void FloatConst::TypeCheck() {
    attribute.T.type = Type::FLOAT;
    attribute.V.ConstTag = true;
    attribute.V.val.FloatVal = val;
}

void StringConst::TypeCheck() { TODO("StringConst Semant"); }

void PrimaryExp_branch::TypeCheck() {
    exp->TypeCheck();
    attribute = exp->attribute;
}

void assign_stmt::TypeCheck() { 
    lval->TypeCheck();
    exp->TypeCheck();
    ((Lval *)lval)->is_left = true;//标记左值为true，((Lval *)lval)将lval强制转换为Lval类型的指针
    if (exp->attribute.T.type == Type::VOID) { //判断右值表达式类型
        error_msgs.push_back("void type can not be assign_stmt's expression " + std::to_string(line_number) + "\n");
    }
}

void expr_stmt::TypeCheck() {
    exp->TypeCheck();
    attribute = exp->attribute;
}

void block_stmt::TypeCheck() { b->TypeCheck(); }

void ifelse_stmt::TypeCheck() {
    Cond->TypeCheck();
    if (Cond->attribute.T.type == Type::VOID) {
        error_msgs.push_back("if cond type is invalid " + std::to_string(line_number) + "\n");
    }
    ifstmt->TypeCheck();
    elsestmt->TypeCheck();
}

void if_stmt::TypeCheck() {
    Cond->TypeCheck();
    if (Cond->attribute.T.type == Type::VOID) {
        error_msgs.push_back("if cond type is invalid " + std::to_string(line_number) + "\n");
    }
    ifstmt->TypeCheck();
}

void while_stmt::TypeCheck() { 
    Cond->TypeCheck();

    if (Cond->attribute.T.type == Type::VOID) {
        error_msgs.push_back("while cond type is invalid " + std::to_string(line_number) + "\n");
    }

    //处理循环嵌套情况
    InWhileCount++;
    body->TypeCheck();
    InWhileCount--;
 }

void for_stmt::TypeCheck() { 
    //for循环的初始化声明部分可能会引入新的变量，这些变量的作用域仅限于for循环内部
    semant_table.symbol_table.enter_scope();
    decl->TypeCheck();
    Cond->TypeCheck();

    InWhileCount++;
    body->TypeCheck();
    InWhileCount--;
    latch->TypeCheck();
    semant_table.symbol_table.exit_scope();
 }

void continue_stmt::TypeCheck() { 
    if (!InWhileCount) {
        error_msgs.push_back("continue is not in while stmt in line " + std::to_string(line_number) + "\n");
    }
 }

void break_stmt::TypeCheck() { 
    if (!InWhileCount) {
        error_msgs.push_back("break is not in while stmt in line " + std::to_string(line_number) + "\n");
    }
 }

void return_stmt::TypeCheck() { 
    return_exp->TypeCheck();

    if (return_exp->attribute.T.type == Type::VOID) {
        error_msgs.push_back("return exp type is invalid " + std::to_string(line_number) + "\n");
    }
 }

void return_stmt_void::TypeCheck() {}

void ConstInitVal::TypeCheck() { 
    //if (!initval) return; 

    for (auto init : *initval) {
        // 对初始化表达式进行类型检查
        init->TypeCheck();
    }
 }

void ConstInitVal_exp::TypeCheck() { 
    if (!exp) return;

    exp->TypeCheck(); //exp是一个指向对象的指针，而TypeCheck()是该对象所属父类中的一个成员函数。
    attribute = exp->attribute;//语法树节点的属性,tree_node定义

    if (attribute.T.type == Type::VOID) {
        error_msgs.push_back("Initval expression can not be void in line " + std::to_string(line_number) + "\n");
    }
    if (!attribute.V.ConstTag) {    //是否是常量表达式
        error_msgs.push_back("Expression is not const " + std::to_string(line_number) + "\n");
    }
 }

void VarInitVal::TypeCheck() { 
    if (!initval) return;

    for (auto init : *initval) {
        // 对初始化表达式进行类型检查
        init->TypeCheck();
    }
 }

void VarInitVal_exp::TypeCheck() { 
    if (!exp) return;

    exp->TypeCheck(); //exp是一个指向对象的指针，而TypeCheck()是该对象所属父类中的一个成员函数。
    attribute = exp->attribute;//语法树节点的属性,tree_node定义

    if (attribute.T.type == Type::VOID) {
        error_msgs.push_back("Initval expression can not be void in line " + std::to_string(line_number) + "\n");
    }
 }

void VarDef_no_init::TypeCheck() { }

void VarDef::TypeCheck() { }

void ConstDef::TypeCheck() { }

void VarDecl::TypeCheck() { 
    // 检查 var_def_list 是否为空
    if (!var_def_list) {
        std::cerr << "Error: Variable definition list is null in VarDecl at line " << GetLineNumber() << std::endl;
        return;
    }

    auto defvector = *var_def_list;
    for (auto def : defvector)
    {
        // 检查多重定义
        if (semant_table.symbol_table.lookup_scope(def->get_name()) == semant_table.symbol_table.get_current_scope()) {
            error_msgs.push_back("Multiple definition of variable '" + def->get_name()->get_string() +
                                   "' in line " + std::to_string(line_number) + ".\n");
            continue; // 跳过当前循环迭代
        }

        VarAttribute val;
        val.ConstTag = false;
        val.type = type_decl; 

        def->scope = semant_table.symbol_table.get_current_scope();//设置当前作用域

        if (def->GetDims() != nullptr)  //是否为数组
        {
            auto dimVector = *def->GetDims();
            for (auto dim : dimVector) {
                dim->TypeCheck();  //维度进行类型检查
                if (!dim->attribute.V.ConstTag) {  //维度是否为常量表达式
                    error_msgs.push_back("Array dimension must be a constant expression in line " +
                                           std::to_string(line_number) + " for variable '" + def->get_name()->get_string() + "'.\n");
                }
                if (dim->attribute.T.type == Type::FLOAT) {  //维度的类型是否为浮点型
                    error_msgs.push_back("Array dimension cannot be of type float in line " +
                                           std::to_string(line_number) + " for variable '" + def->get_name()->get_string() + "'.\n");
                }
                val.dims.push_back(dim->attribute.V.val.IntVal);  //维度的整数值添加到 val 的 dims 成员
            }
        }

         // 检查初始化值
        InitVal init = def->get_init();
        if (init != nullptr) {
            init->TypeCheck();
        }
 
        // 将变量添加到符号表
        semant_table.symbol_table.add_Symbol(def->get_name(), val);

    }

}

void ConstDecl::TypeCheck() { 
       // 检查 const_def_list 是否为空
    if (!var_def_list) {
        std::cerr << "Error: Constant definition list is null in ConstDecl at line " << GetLineNumber() << std::endl;
        return;
    }
 
    auto defvector = *var_def_list;
    for (auto def : defvector) {
        // 检查多重定义
        if (semant_table.symbol_table.lookup_scope(def->get_name()) == semant_table.symbol_table.get_current_scope()) {
            error_msgs.push_back("Multiple definition of constant '" + def->get_name()->get_string() +
                                 "' in line " + std::to_string(line_number) + ".\n");
            continue; // 跳过当前循环迭代
        }
 
        VarAttribute val;
        val.ConstTag = true; // 标记为常量
        val.type = type_decl;
 
        def->scope = semant_table.symbol_table.get_current_scope(); // 设置当前作用域

         if (def->GetDims() != nullptr)  //是否为数组
        {
            auto dimVector = *def->GetDims();
            for (auto dim : dimVector) {
                dim->TypeCheck();  //维度进行类型检查
                if (!dim->attribute.V.ConstTag) {  //维度是否为常量表达式
                    error_msgs.push_back("Array dimension must be a constant expression in line " +
                                           std::to_string(line_number) + " for variable '" + def->get_name()->get_string() + "'.\n");
                }
                if (dim->attribute.T.type == Type::FLOAT) {  //维度的类型是否为浮点型
                    error_msgs.push_back("Array dimension cannot be of type float in line " +
                                           std::to_string(line_number) + " for variable '" + def->get_name()->get_string() + "'.\n");
                }
                
            }
            for (auto dim : dimVector){
            val.dims.push_back(dim->attribute.V.val.IntVal);  //维度的整数值添加到 val 的 dims 成员
            }
        }

 
        // 检查初始化值
        InitVal init = def->get_init();
        if (init==nullptr) {
            error_msgs.push_back("Constant '" + def->get_name()->get_string() +
                                 "' must be initialized in line " + std::to_string(line_number) + ".\n");
            continue;
        }
 
        init->TypeCheck();
        
        // 检查初始化值的类型是否与声明的类型匹配
        // if (init->attribute.T.type != val.type) {
        //     error_msgs.push_back("Initialization type mismatch for constant '" + def->get_name()->get_string() +
        //                          "' in line " + std::to_string(line_number) + ".\n");
        // }
 
        // // 检查初始化值是否是一个常量表达式
        // if (!init->attribute.V.ConstTag) {
        //     error_msgs.push_back("Constant '" + def->get_name()->get_string() +
        //                          "' must be initialized with a constant expression in line " +
        //                          std::to_string(line_number) + ".\n");
        // }

        if (type_decl == Type::INT)
        {
            int arraySz = 1;
            for (auto d : val.dims) {
                arraySz *= d;
            }
            val.IntInitVals.resize(arraySz, 0);
 
            if (val.dims.empty()) {
                // 非数组常量
                if (init->GetExp() != nullptr) {
                    if (init->GetExp()->attribute.T.type == Type::INT) {
                        val.IntInitVals[0] = init->GetExp()->attribute.V.val.IntVal;
                    } else if (init->GetExp()->attribute.T.type == Type::FLOAT) {
                        // 如果初始化值是浮点型，需要转换为整型
                        val.IntInitVals[0] = static_cast<int>(init->GetExp()->attribute.V.val.FloatVal);
                        error_msgs.push_back("Implicit conversion from float to int in initialization of constant '" +
                                             def->get_name()->get_string() + "' in line " + std::to_string(line_number) + ".\n");
                    } else if (init->GetExp()->attribute.T.type == Type::VOID) {
                        error_msgs.push_back("Expression can not be void in initialization of constant '" +
                                             def->get_name()->get_string() + "' in line " + std::to_string(line_number) + ".\n");
                    }
                }
            } else {
                // 数组常量
                if (init->IsExp()) {
                    error_msgs.push_back("Initialization of array constant '" + def->get_name()->get_string() +
                                         "' must use an initializer list in line " + std::to_string(line_number) + ".\n");
                } else {
                    // 递归地初始化数组
                    RecursiveArrayInit(init, val, 0, arraySz - 1, 0);
                }
            }

        }
        else if(type_decl == Type::FLOAT)
        {
            int arraySz = 1;
            for (auto d : val.dims) {
                arraySz *= d;
            }
            val.FloatInitVals.resize(arraySz, 0);
 
            if (val.dims.empty()) {
                // 非数组常量
                if (init->GetExp() != nullptr) {
                    if (init->GetExp()->attribute.T.type == Type::INT) {
                        val.FloatInitVals[0] = init->GetExp()->attribute.V.val.IntVal;
                    } else if (init->GetExp()->attribute.T.type == Type::FLOAT) {
                        // 如果初始化值是浮点型，需要转换为整型
                       val.FloatInitVals[0] = init->GetExp()->attribute.V.val.FloatVal;
                    } else if (init->GetExp()->attribute.T.type == Type::VOID) {
                        error_msgs.push_back("Expression can not be void in initialization of constant '" +
                                             def->get_name()->get_string() + "' in line " + std::to_string(line_number) + ".\n");
                    }
                }
            } else {
                // 数组常量
                if (init->IsExp()) {
                    error_msgs.push_back("Initialization of array constant '" + def->get_name()->get_string() +
                                         "' must use an initializer list in line " + std::to_string(line_number) + ".\n");
                } else {
                    // 递归地初始化数组
                    RecursiveArrayInit(init, val, 0, arraySz - 1,0);
                }
            }


        }
 
        // 将常量添加到符号表
        semant_table.symbol_table.add_Symbol(def->get_name(), val);
    }
 }

void BlockItem_Decl::TypeCheck() { decl->TypeCheck(); }

void BlockItem_Stmt::TypeCheck() { stmt->TypeCheck(); }

void __Block::TypeCheck() {
    semant_table.symbol_table.enter_scope();
    auto item_vector = *item_list;
    for (auto item : item_vector) {
        item->TypeCheck();
    }
    semant_table.symbol_table.exit_scope();
}

void __FuncFParam::TypeCheck() {
    VarAttribute val;
    val.ConstTag = false;
    val.type = type_decl;
    scope = 1;

    // 如果dims为nullptr, 表示该变量不含数组下标, 如果你在语法分析中采用了其他方式处理，这里也需要更改
    if (dims != nullptr) {    
        auto dim_vector = *dims;

        // the fisrt dim of FuncFParam is empty
        // eg. int f(int A[][30][40])
        val.dims.push_back(-1);    // 这里用-1表示empty，你也可以使用其他方式
        for (int i = 1; i < dim_vector.size(); ++i) {
            auto d = dim_vector[i];
            d->TypeCheck();
            if (d->attribute.V.ConstTag == false) {
                error_msgs.push_back("Array Dim must be const expression in line " + std::to_string(line_number) +
                                     "\n");
            }
            if (d->attribute.T.type == Type::FLOAT) {
                error_msgs.push_back("Array Dim can not be float in line " + std::to_string(line_number) + "\n");
            }
            val.dims.push_back(d->attribute.V.val.IntVal);
        }
        attribute.T.type = Type::PTR;
    } else {
        attribute.T.type = type_decl;
    }

    if (name != nullptr) {
        if (semant_table.symbol_table.lookup_scope(name) != -1) {
            error_msgs.push_back("multiple difinitions of formals in function " + name->get_string() + " in line " +
                                 std::to_string(line_number) + "\n");
        }
        semant_table.symbol_table.add_Symbol(name, val);
    }
}

void __FuncDef::TypeCheck() {
    semant_table.symbol_table.enter_scope();

    semant_table.FunctionTable[name] = this;

    if(name->get_string() == "main"){
        havemain = true;
    }

    auto formal_vector = *formals;
    for (auto formal : formal_vector) {
        formal->TypeCheck();
    }

    // block TypeCheck
    if (block != nullptr) {
        auto item_vector = *(block->item_list);
        for (auto item : item_vector) {
            item->TypeCheck();
        }
    }

    semant_table.symbol_table.exit_scope();
}

void CompUnit_Decl::TypeCheck() { 
    //获取声明类型
    Type::ty type_decl = decl->GetTypedecl();
    //获取定义列表
    auto def_vector = *decl->GetDefs();
    for (auto def : def_vector) {
        //如果变量名已存在则输出错误
        if (semant_table.GlobalTable.find(def->get_name()) != semant_table.GlobalTable.end()) {
            error_msgs.push_back("multilpe difinitions of vars in line " + std::to_string(line_number) + "\n");
            //continue;
        }

        // 初始化变量属性
        VarAttribute val;
        val.ConstTag = def->IsConst();
        val.type = (Type::ty)type_decl;
        def->scope = 0;// 全局作用域

        if(def->GetDims() != nullptr)  //数组
            {
                auto dim_vector = *def->GetDims();
                for (auto d : dim_vector) {
                    d->TypeCheck();
                    if (d->attribute.V.ConstTag == false) {
                        error_msgs.push_back("Array Dim must be const expression " + std::to_string(line_number) + "\n");
                    }
                    if (d->attribute.T.type == Type::FLOAT) {
                        error_msgs.push_back("Array Dim can not be float in line " + std::to_string(line_number) + "\n");
                    }
                }
                for (auto d : dim_vector) {
                    val.dims.push_back(d->attribute.V.val.IntVal);
                }
            }


        // 处理初始化值
        InitVal init = def->get_init();
        if (init != nullptr) {
            init->TypeCheck();
            if (type_decl == Type::INT) {
                
                //调用初始化处理函数
                val.type = Type::INT;
                int arraySz = 1;
                for (auto d : val.dims) {
                    arraySz *= d;
                }
                val.IntInitVals.resize(arraySz, 0);
                
                if (val.dims.empty()) {
                    if (init->GetExp() != nullptr) {
                        if (init->GetExp()->attribute.T.type == Type::VOID) {
                            error_msgs.push_back("exp can not be void in initval in line " + std::to_string(init->GetLineNumber()) +
                                                "\n");
                        } else if (init->GetExp()->attribute.T.type == Type::FLOAT) {
                            val.IntInitVals[0] = init->GetExp()->attribute.V.val.FloatVal;
                        } else if (init->GetExp()->attribute.T.type == Type::INT) {
                            val.IntInitVals[0] = init->GetExp()->attribute.V.val.IntVal;
                        }
                    }
                    
                } else {
                    if (init->IsExp()) {
                        if ((init)->GetExp() != nullptr) {
                            error_msgs.push_back("InitVal can not be exp in line " + std::to_string(init->GetLineNumber()) + "\n");
                        }
                        return;
                    } else {
                        RecursiveArrayInit(init, val, 0, arraySz - 1,0);
                    }
                }
            } else if (type_decl == Type::FLOAT) {
                //调用初始化处理函数
                val.type = Type::FLOAT;
                int arraySz = 1;
                for (auto d : val.dims) {
                    arraySz *= d;
                }
                val.FloatInitVals.resize(arraySz, 0);
                if (val.dims.empty()) {
                    if (init->GetExp() != nullptr) {
                        if (init->GetExp()->attribute.T.type == Type::VOID) {
                            error_msgs.push_back("exp can not be void in initval in line " + std::to_string(init->GetLineNumber()) +
                                                "\n");
                        } else if (init->GetExp()->attribute.T.type == Type::FLOAT) {
                            val.FloatInitVals[0] = init->GetExp()->attribute.V.val.FloatVal;
                        } else if (init->GetExp()->attribute.T.type == Type::INT) {
                            val.FloatInitVals[0] = init->GetExp()->attribute.V.val.IntVal;
                        }
                    }
                    
                } else {
                    if (init->IsExp()) {
                        if ((init)->GetExp() != nullptr) {
                            error_msgs.push_back("InitVal can not be exp in line " + std::to_string(init->GetLineNumber()) + "\n");
                        }
                        return;
                    } else {
                        RecursiveArrayInit(init, val, 0, arraySz - 1,0);
                    }
                }
            }
        }

        // 处理常量变量
        if (def->IsConst()) {
            ConstGlobalMap[def->get_name()->get_string()] = val;
        }

        // 添加到静态全局映射和全局符号表
        StaticGlobalMap[def->get_name()->get_string()] = val;
        semant_table.GlobalTable[def->get_name()] = val;

        BasicInstruction::LLVMType lltype = Type2LLvm[type_decl];

        Instruction globalDecl;
        if (def->GetDims() != nullptr) {
            globalDecl = new GlobalVarDefineInstruction(def->get_name()->get_string(), lltype, val);
        } else if (init == nullptr) {
            globalDecl = new GlobalVarDefineInstruction(def->get_name()->get_string(), lltype, nullptr);
        } else if (lltype == GlobalVarDefineInstruction::I32) {
            globalDecl =
            new GlobalVarDefineInstruction(def->get_name()->get_string(), lltype, new ImmI32Operand(val.IntInitVals[0]));
        } else if (lltype == GlobalVarDefineInstruction::FLOAT32) {
            globalDecl = new GlobalVarDefineInstruction(def->get_name()->get_string(), lltype,
                                                        new ImmF32Operand(val.FloatInitVals[0]));
        }
        llvmIR.global_def.push_back(globalDecl);

    }
}

void CompUnit_FuncDef::TypeCheck() { func_def->TypeCheck(); }