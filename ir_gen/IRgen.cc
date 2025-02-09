#include "IRgen.h"
#include "../include/ir.h"
#include "semant.h"
#include "../include/Instruction.h"
#include "../include/basic_block.h"
#include <algorithm>

extern SemantTable semant_table;    // 也许你会需要一些语义分析的信息

IRgenTable irgen_table;    // 中间代码生成的辅助变量
LLVMIR llvmIR;             // 我们需要在这个变量中生成中间代码

int max_reg = -1;
int max_label = -1; //已分配的最大标签值

static int now_label = 0;
static FuncDefInstruction function_now; //当前正在生成的函数
static Type::ty function_returntype = Type::VOID;

//跟踪当前循环的起始和结束位置的标签
static int loop_start_label = -1;    // continue;
static int loop_end_label = -1;      // break;

std::map<FuncDefInstruction, int> max_label_map{};
std::map<FuncDefInstruction, int> max_reg_map{};


//BasicInstruction::LLVMType Type2LLvm[6];
BasicInstruction::LLVMType Type2LLvm[6] = {
    BasicInstruction::LLVMType::VOID, 
    BasicInstruction::LLVMType::I32, 
    BasicInstruction::LLVMType::FLOAT32,
    BasicInstruction::LLVMType::I1,   
    BasicInstruction::LLVMType::PTR, 
    BasicInstruction::LLVMType::DOUBLE};

void AddLibFunctionDeclare();

// 在基本块B末尾生成一条新指令
void IRgenArithmeticI32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg);
void IRgenArithmeticF32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg);
void IRgenArithmeticI32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int reg2, int result_reg);
void IRgenArithmeticF32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, int reg2,
                               int result_reg);
void IRgenArithmeticI32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int val2, int result_reg);
void IRgenArithmeticF32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, float val2,
                              int result_reg);

void IRgenIcmp(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int reg2, int result_reg);
void IRgenFcmp(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, int reg2, int result_reg);
void IRgenIcmpImmRight(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int val2, int result_reg);
void IRgenFcmpImmRight(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, float val2, int result_reg);

void IRgenFptosi(LLVMBlock B, int src, int dst);//浮点数到整数的类型转换
void IRgenSitofp(LLVMBlock B, int src, int dst);//整数到浮点数的类型转换
void IRgenZextI1toI32(LLVMBlock B, int src, int dst);//从1位整数到32位整数的零扩展指令
//void IRgenFpext(LLVMBlock B, int src, int dst);//从32位浮点数到64位浮点数的扩展指令

//获取32位整数数组元素指针
void IRgenGetElementptrIndexI32(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs);

//获取64位整数数组元素指针
void IRgenGetElementptrIndexI64(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs);

void IRgenLoad(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr);
void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, int value_reg, Operand ptr);
void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, Operand value, Operand ptr);

void IRgenCall(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg,
               std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name);

//无返回值的函数调用
void IRgenCallVoid(LLVMBlock B, BasicInstruction::LLVMType type,
                   std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name);

//无参数的函数调用
void IRgenCallNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, std::string name);
//无参数且无返回值的函数调用
void IRgenCallVoidNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, std::string name);
//从寄存器返回值的返回指令
void IRgenRetReg(LLVMBlock B, BasicInstruction::LLVMType type, int reg);
//从立即数返回整数值的返回指令
void IRgenRetImmInt(LLVMBlock B, BasicInstruction::LLVMType type, int val);
//从立即数返回浮点值的返回指令
void IRgenRetImmFloat(LLVMBlock B, BasicInstruction::LLVMType type, float val);
//无返回值的返回指令
void IRgenRetVoid(LLVMBlock B);
//无条件分支指令
void IRgenBRUnCond(LLVMBlock B, int dst_label);
//生成条件分支指令
void IRgenBrCond(LLVMBlock B, int cond_reg, int true_label, int false_label);
//分配指令
void IRgenAlloca(LLVMBlock B, BasicInstruction::LLVMType type, int reg);
//分配数组指令
void IRgenAllocaArray(LLVMBlock B, BasicInstruction::LLVMType type, int reg, std::vector<int> dims);
//创建一个新的寄存器操作数对象
RegOperand *GetNewRegOperand(int RegNo);

// generate TypeConverse Instructions from type_src to type_dst
// eg. you can use fptosi instruction to converse float to int
// eg. you can use zext instruction to converse bool to int

//生成32位整数的二元运算
void BinaryIRgenInt(LLVMBlock B, int reg1, int reg2, NodeAttribute::opcode opType) {
    int newReg = ++max_reg;//分配一个新的寄存器编号
    switch (opType) {
        case NodeAttribute::ADD:
            IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::ADD, reg1, reg2, newReg);
            break;
        case NodeAttribute::SUB:
            IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::SUB, reg1, reg2, newReg);
            break;
        case NodeAttribute::MUL:
            IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MUL, reg1, reg2, newReg);
            break;
        case NodeAttribute::DIV:
            IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::DIV, reg1, reg2, newReg);
            break;
        case NodeAttribute::MOD:
            IRgenArithmeticI32(B, BasicInstruction::LLVMIROpcode::MOD, reg1, reg2, newReg);
            break;
        case NodeAttribute::GEQ:
            IRgenIcmp(B, BasicInstruction::IcmpCond::sge, reg1, reg2, newReg);
            break;
        case NodeAttribute::GT:
            IRgenIcmp(B, BasicInstruction::IcmpCond::sgt, reg1, reg2, newReg);
            break;
        case NodeAttribute::LEQ:
            IRgenIcmp(B, BasicInstruction::IcmpCond::sle, reg1, reg2, newReg);
            break;
        case NodeAttribute::LT:
            IRgenIcmp(B, BasicInstruction::IcmpCond::slt, reg1, reg2, newReg);
            break;
        case NodeAttribute::EQ:
            IRgenIcmp(B, BasicInstruction::IcmpCond::eq, reg1, reg2, newReg);
            break;
        case NodeAttribute::NEQ:
            IRgenIcmp(B, BasicInstruction::IcmpCond::ne, reg1, reg2, newReg);
            break;
        default:
            assert(false); // 不支持的操作
            break;
        
    }
}
//生成浮点数的二元运算
void BinaryIRgenFloat(LLVMBlock B, int reg1, int reg2, NodeAttribute::opcode op) {
    switch (op) {
        case NodeAttribute::ADD:
            IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FADD, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::SUB:
            IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FSUB, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::MUL:
            IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FMUL, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::DIV:
            IRgenArithmeticF32(B, BasicInstruction::LLVMIROpcode::FDIV, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::MOD:
            assert(false); // MOD 操作对于浮点数未定义
            break;
        case NodeAttribute::GEQ:
            IRgenFcmp(B, BasicInstruction::FcmpCond::OGE, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::GT:
            IRgenFcmp(B, BasicInstruction::FcmpCond::OGT, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::LEQ:
            IRgenFcmp(B, BasicInstruction::FcmpCond::OLE, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::LT:
            IRgenFcmp(B, BasicInstruction::FcmpCond::OLT, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::EQ:
            IRgenFcmp(B, BasicInstruction::FcmpCond::OEQ, reg1, reg2, ++max_reg);
            break;
        case NodeAttribute::NEQ:
            IRgenFcmp(B, BasicInstruction::FcmpCond::ONE, reg1, reg2, ++max_reg);
            break;
        default:
            assert(false); // 不支持的操作
            break;
    }
}

//生成二元操作节点
void IRgenBinaryNode(tree_node *a, tree_node *b, NodeAttribute::opcode opcode, LLVMBlock B) {
    Type::ty typeA = a->attribute.T.type; 
    Type::ty typeB = b->attribute.T.type;
    int reg1;
    int reg2;
    switch (typeA) {
        case Type::INT:
            switch (typeB) {
                case Type::INT:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    BinaryIRgenInt(B, reg1, reg2,opcode);
                    break;
                case Type::FLOAT:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenSitofp(B, reg1, ++max_reg);    // a int->float
                    reg1 = max_reg;

                    BinaryIRgenFloat(B, reg1, reg2,opcode);
                    break;
                case Type::BOOL:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
                    reg2 = max_reg;

                    BinaryIRgenInt(B, reg1, reg2,opcode);
                    break;
                // 其他类型组合可以添加在这里
                default:
                    assert(false); // 不支持的类型组合
                    break;
            }
            break;

        case Type::FLOAT:
            switch (typeB) {
                case Type::INT:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenSitofp(B, reg2, ++max_reg);    // b int->float
                    reg2 = max_reg;

                    BinaryIRgenFloat(B, reg1, reg2,opcode);
                    break;
                case Type::FLOAT:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    BinaryIRgenFloat(B, reg1, reg2,opcode);
                    break;
                case Type::BOOL:
                     a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
                    reg2 = max_reg;

                    IRgenSitofp(B, reg2, ++max_reg);    // int -> float
                    reg2 = max_reg;

                    BinaryIRgenFloat(B, reg1, reg2,opcode);
                    break;
                // 其他类型组合
                default:
                    assert(false);
                    break;
            }
            break;

        case Type::BOOL:
            switch (typeB) {
                case Type::INT:
                     a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
                    reg1 = max_reg;

                    BinaryIRgenInt(B, reg1, reg2,opcode);
                    break;
                case Type::FLOAT:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
                    reg1 = max_reg;

                    IRgenSitofp(B, reg1, ++max_reg);    // int -> float
                    reg1 = max_reg;

                    BinaryIRgenFloat(B, reg1, reg2,opcode);
                    break;
                case Type::BOOL:
                    a->codeIR();
                    reg1 = max_reg;

                    b->codeIR();
                    reg2 = max_reg;

                    IRgenZextI1toI32(B, reg1, ++max_reg);    // bool -> int
                    reg1 = max_reg;

                    IRgenZextI1toI32(B, reg2, ++max_reg);    // bool -> int
                    reg2 = max_reg;

                    BinaryIRgenInt(B, reg1, reg2,opcode);
                    break;
                // 其他类型组合
                default:
                    assert(false);
                    break;
            }
            break;

        // 其他类型可以作为单独的 case 添加
        default:
            assert(false); // 不支持的类型作为操作数
            break;
    }
}

void SingleIRgenInt(LLVMBlock B, NodeAttribute::opcode op, int reg1) {
    switch (op) {
        case NodeAttribute::ADD:
            
            break;
        case NodeAttribute::SUB:
            IRgenArithmeticI32ImmLeft(B, BasicInstruction::LLVMIROpcode::SUB, 0, reg1, ++max_reg);
            break;
        case NodeAttribute::NOT:
            IRgenIcmpImmRight(B, BasicInstruction::IcmpCond::eq, reg1, 0, ++max_reg);
            break;
        default:
            std::cerr << "Unsupported NodeAttribute\n";
            break;
    }
}

void SingleIRgenFloat(LLVMBlock B, NodeAttribute::opcode op, int reg1) {
    switch (op) {
        case NodeAttribute::ADD:
            
            break;
        case NodeAttribute::SUB:
            IRgenArithmeticF32ImmLeft(B, BasicInstruction::LLVMIROpcode::FSUB, 0, reg1, ++max_reg);
            break;
        case NodeAttribute::NOT:
            IRgenFcmpImmRight(B, BasicInstruction::FcmpCond::OEQ, reg1, 0, ++max_reg);
            break;
        default:
            std::cerr << "Unsupported NodeAttribute for float operations\n";
            break;
    }
}

//生成一元操作节点
void IRgenSingleNode(tree_node *a, NodeAttribute::opcode opcode, LLVMBlock B) {
    int reg1;
    switch (a->attribute.T.type) {
        case Type::INT:
            a->codeIR();
            reg1 = max_reg;
            SingleIRgenInt(B,opcode, reg1);
            break;
        case Type::FLOAT:
            a->codeIR();
            reg1 = max_reg;
            SingleIRgenFloat(B, opcode,reg1);
            break;
        case Type::BOOL:
            a->codeIR();
            reg1 = max_reg;
            IRgenZextI1toI32(B, reg1, ++max_reg);
            reg1 = max_reg;

            SingleIRgenInt(B,opcode, reg1);
            break;
        case Type::PTR:
        break;
        case Type::VOID:
        break;
        default:
            assert(false);
            break;
    }
}

//类型转换
void IRgenTypeConverse(LLVMBlock B, Type::ty type_src, Type::ty type_dst, int src) {
    if (type_src == type_dst) {
        return;
    }
    if (type_src == Type::INT && type_dst == Type::FLOAT) {
        IRgenSitofp(B, src, ++max_reg);
    } else if (type_src == Type::INT && type_dst == Type::BOOL) {
        IRgenIcmpImmRight(B, BasicInstruction::IcmpCond::ne, src, 0, ++max_reg);
    } else if (type_src == Type::FLOAT && type_dst == Type::INT) {
        IRgenFptosi(B, src, ++max_reg);
    } else if (type_src == Type::FLOAT && type_dst == Type::BOOL) {
        IRgenFcmpImmRight(B, BasicInstruction::FcmpCond::ONE, src, 0, ++max_reg);
    } else if (type_src == Type::BOOL && type_dst == Type::INT) {
        IRgenZextI1toI32(B, src, ++max_reg);
    } else if (type_src == Type::BOOL && type_dst == Type::FLOAT) {
        IRgenZextI1toI32(B, src, ++max_reg);
        src = max_reg;
        IRgenSitofp(B, src, ++max_reg);
    } 
    else {
        //assert(false);
    }
}

std::vector<int> GetIndexes(const std::vector<int> dims, int absoluteIndex) {
    std::vector<int> ret;
    int currentDimProduct = 1;
 
    // 从最后一个维度开始计算索引
    for (int i = dims.size() - 1; i >= 0; --i) {
        int dim = dims[i];
        int indexInDim = absoluteIndex / currentDimProduct;
        ret.push_back(indexInDim);
        absoluteIndex %= currentDimProduct * dim; // 更新绝对索引
        currentDimProduct *= dim; // 更新当前维度的乘积
    }
 
    // 反转结果
    std::reverse(ret.begin(), ret.end());
    return ret;
}



void RecursiveArrayInitIR(LLVMBlock block, const std::vector<int> dims, int arrayaddr_reg_no,
                          InitVal init, int beginPos, int endPos, int dimsIdx, Type::ty ArrayType) {
    int pos = beginPos;
     // 计算当前维度之后所有维度的乘积
    int blockSize = 1;
    for (int i = dimsIdx + 1; i < dims.size(); ++i) {
        blockSize *= dims[i];
    }
    auto initList = init->GetList();
    for (InitVal iv : *(init->GetList())) {
        if (iv->IsExp()) {
            // Generate IR for the initialization expression
            iv->codeIR();
            int init_val_reg = max_reg;
            
            // Optionally convert the type if necessary
            
            IRgenTypeConverse(block, iv->attribute.T.type, ArrayType, init_val_reg);
            init_val_reg = max_reg;
            // Calculate the element address
            int addr_reg = ++max_reg;
            auto gep = new GetElementptrInstruction(Type2LLvm[ArrayType], GetNewRegOperand(addr_reg),
                                                    GetNewRegOperand(arrayaddr_reg_no), dims,BasicInstruction::LLVMType::I32);
            gep->push_idx_imm32(0);
            std::vector<int> indexes = GetIndexes(dims, pos);
            for (int idx : indexes) {
                gep->push_idx_imm32(idx);
            }
            block->InsertInstruction(1,gep);
            
            // Store the initialized value at the calculated address
            IRgenStore(block, Type2LLvm[ArrayType], GetNewRegOperand(init_val_reg), GetNewRegOperand(addr_reg));
            
            pos++;
        } else {       
            int subBlockSize = blockSize;
            RecursiveArrayInitIR(block, dims, arrayaddr_reg_no, iv, pos, pos + subBlockSize - 1,
                                 dimsIdx + 1, ArrayType);
            pos += subBlockSize;
        }
    }
}

void IRgenTypeConverse(LLVMBlock B, Type::ty type_src, Type::ty type_dst, int src, int dst) {
    TODO("IRgenTypeConverse. Implement it if you need it");
}

//在基本块中插入一条指令，Instruction_list 是一个存储指令的列表
void BasicBlock::InsertInstruction(int pos, Instruction Ins) {
    assert(pos == 0 || pos == 1);
    if (pos == 0) {
        Instruction_list.push_front(Ins);//将指令 Ins 插入到列表的前端
    } else if (pos == 1) {
        Instruction_list.push_back(Ins);//将指令 Ins 插入到列表的末尾
    }
}

bool IsBr(Instruction ins) {
    int opcode = ins->GetOpcode();
    return opcode == BasicInstruction::BR_COND || opcode == BasicInstruction::BR_UNCOND;
}

bool IsRet(Instruction ins) {
    int opcode = ins->GetOpcode();
    return opcode == BasicInstruction::RET;
}

void AddNoReturnBlock() {
    for (auto block : llvmIR.function_block_map[function_now]) {
        LLVMBlock B = block.second;
        if (B->Instruction_list.empty() || (!IsRet(B->Instruction_list.back()) && !IsBr(B->Instruction_list.back()))) {
            if (function_returntype == Type::VOID) {
                IRgenRetVoid(B);
            } else if (function_returntype == Type::INT) {
                IRgenRetImmInt(B, BasicInstruction::LLVMType::I32, 0);
            } else if (function_returntype == Type::FLOAT) {
                IRgenRetImmFloat(B, BasicInstruction::LLVMType::FLOAT32, 0);
            }
        }
    }
}

/*
二元运算指令生成的伪代码：
    假设现在的语法树节点是：AddExp_plus
    该语法树表示 addexp + mulexp

    addexp->codeIR()
    mulexp->codeIR()
    假设mulexp生成完后，我们应该在基本块B0继续插入指令。
    addexp的结果存储在r0寄存器中，mulexp的结果存储在r1寄存器中
    生成一条指令r2 = r0 + r1，并将该指令插入基本块B0末尾。
    标注后续应该在基本块B0插入指令，当前节点的结果寄存器为r2。
    (如果考虑支持浮点数，需要查看语法树节点的类型来判断此时是否需要隐式类型转换)
*/

/*
while语句指令生成的伪代码：
    while的语法树节点为while(cond)stmt

    假设当前我们应该在B0基本块开始插入指令
    新建三个基本块Bcond，Bbody，Bend
    在B0基本块末尾插入一条无条件跳转指令，跳转到Bcond

    设置当前我们应该在Bcond开始插入指令
    cond->codeIR()    //在调用该函数前你可能需要设置真假值出口
    假设cond生成完后，我们应该在B1基本块继续插入指令，Bcond的结果为r0
    如果r0的类型不为bool，在B1末尾生成一条比较语句，比较r0是否为真。
    在B1末尾生成一条条件跳转语句，如果为真，跳转到Bbody，如果为假，跳转到Bend

    设置当前我们应该在Bbody开始插入指令
    stmt->codeIR()
    假设当stmt生成完后，我们应该在B2基本块继续插入指令
    在B2末尾生成一条无条件跳转语句，跳转到Bcond

    设置当前我们应该在Bend开始插入指令
*/

void __Program::codeIR() {
    AddLibFunctionDeclare();//添加库函数声明
    auto comp_vector = *comp_list;//获取编译单元列表
    for (auto comp : comp_vector) {
        comp->codeIR();
    }
}

void Exp::codeIR() { addexp->codeIR(); }

void AddExp_plus::codeIR() { 
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(addexp, mulexp, NodeAttribute::ADD, B);
}

void AddExp_sub::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(addexp, mulexp, NodeAttribute::SUB, B);
}

void MulExp_mul::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(mulexp, unary_exp, NodeAttribute::MUL, B);
}

void MulExp_div::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(mulexp, unary_exp, NodeAttribute::DIV, B);
}

void MulExp_mod::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(mulexp, unary_exp, NodeAttribute::MOD, B);
}

void RelExp_leq::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(relexp, addexp, NodeAttribute::LEQ, B);
}

void RelExp_lt::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(relexp, addexp, NodeAttribute::LT, B);
}

void RelExp_geq::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(relexp, addexp, NodeAttribute::GEQ, B);
}

void RelExp_gt::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(relexp, addexp, NodeAttribute::GT, B);
}

void EqExp_eq::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(eqexp, relexp, NodeAttribute::EQ, B);
}

void EqExp_neq::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBinaryNode(eqexp, relexp, NodeAttribute::NEQ, B);
}

// short circuit &&
// Reference:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc line174-line192
void LAndExp_and::codeIR() {
    
    int start_label = now_label;
    //创建一个新的基本快，在左表达式为真时跳转到此
    int lefttrue_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    landexp->true_label = lefttrue_label;
    //如果左表达式为假直接跳转
    landexp->false_label = this->false_label;
    //生成左表达式中间代码
    landexp->codeIR();
    //获取当前基本快
    LLVMBlock B1 = llvmIR.GetBlock(function_now, now_label);
    //将左表达式计算结果转成布尔类型并存储在寄存器中
    IRgenTypeConverse(B1, landexp->attribute.T.type, Type::BOOL, max_reg);
    //生成一个条件分支指令，根据寄存器的值决定跳转位置
    IRgenBrCond(B1, max_reg, lefttrue_label, this->false_label);
    //更新当前标签为左表达式为真
    now_label = lefttrue_label;
    //设置右表达式
    eqexp->true_label = this->true_label;
    eqexp->false_label = this->false_label;
    eqexp->codeIR();
    LLVMBlock B2 = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B2, eqexp->attribute.T.type, Type::BOOL, max_reg);
}

// short circuit ||
// Reference:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc line197-line216
void LOrExp_or::codeIR() {
    int start_label = now_label;
    int leftfalse_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;

    lorexp->true_label = this->true_label;
    lorexp->false_label = leftfalse_label;
    lorexp->codeIR();
    LLVMBlock B1 = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B1, lorexp->attribute.T.type, Type::BOOL, max_reg);
    IRgenBrCond(B1, max_reg, this->true_label, leftfalse_label);

    now_label = leftfalse_label;
    landexp->true_label = this->true_label;
    landexp->false_label = this->false_label;
    landexp->codeIR();
    LLVMBlock B2 = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B2, landexp->attribute.T.type, Type::BOOL, max_reg);
}

void ConstExp::codeIR() { addexp->codeIR(); }

void Lval::codeIR() {
    // 获取当前基本块
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);

    // 初始化索引向量
    std::vector<Operand> arrayindexs;

    // 处理数组维度
    if (dims != nullptr) {
        for (auto d : *dims) {
            d->codeIR();  // 生成维度表达式的 IR 代码
            IRgenTypeConverse(B, d->attribute.T.type, Type::INT, max_reg);  // 类型转换为整型
            arrayindexs.push_back(GetNewRegOperand(max_reg));  // 将结果添加到索引向量中
        }
    }

    Operand ptr_operand;//存储指针操作数
    VarAttribute lval_attribute;//存储变量属性
    bool formal_array_tag = false;//标记是否是形式数组

    // 查找name变量对应的寄存器编号
    int alloca_reg = irgen_table.symbol_table.lookup(name);
    if (alloca_reg != -1) {  // 局部变量
        ptr_operand = GetNewRegOperand(alloca_reg);//创建一个新寄存器操作数对象
        lval_attribute = irgen_table.RegTable[alloca_reg];
        formal_array_tag = irgen_table.FormalArrayTable[alloca_reg];
    } else {  // 全局变量
        ptr_operand = GetNewGlobalOperand(name->get_string());
        lval_attribute = semant_table.GlobalTable[name];
    }

    // 获取变量的 LLVM 类型
    auto lltype = Type2LLvm[lval_attribute.type];

    // 辅助函数：生成 getelementptr 指令
    auto generateGetElementPtr = [&](bool use_initial_index) {
        if (use_initial_index) {
            arrayindexs.insert(arrayindexs.begin(), new ImmI32Operand(0));
        }
        IRgenGetElementptrIndexI64(B, lltype, ++max_reg, ptr_operand, lval_attribute.dims, arrayindexs);
        ptr_operand = GetNewRegOperand(max_reg);  // 最终地址的指针操作数
    };

    // 如果是数组或指针，生成 getelementptr 指令
    if (!arrayindexs.empty() || attribute.T.type == Type::PTR) {
        generateGetElementPtr(!formal_array_tag);  // 根据形式数组标记决定是否使用初始索引 0
    }

    // 存储指针操作数
    ptr = ptr_operand;

    // 如果是右值，生成 load 指令
    if (!is_left) {
        if (attribute.T.type != Type::PTR) {  // 如果不是指针类型
            IRgenLoad(B, lltype, ++max_reg, ptr_operand);  // 生成 load 指令
        }
    }
}

void FuncRParams::codeIR() {}

void Func_call::codeIR() {
    // 获取当前基本块
    LLVMBlock current_block = llvmIR.GetBlock(function_now, now_label);

    // 获取函数的返回类型
    Type::ty function_return_type = semant_table.FunctionTable[name]->return_type;
    GlobalVarDefineInstruction::LLVMType return_type = Type2LLvm[function_return_type];

    if(funcr_params == nullptr){
        if (function_return_type == Type::VOID) {
            IRgenCallVoidNoArgs(current_block, return_type, name->get_string());
        } else {
            IRgenCallNoArgs(current_block, return_type, ++max_reg, name->get_string());
        }
    }
    else{
        // 初始化参数列表
        std::vector<std::pair<BasicInstruction::LLVMType, Operand>> parameters;
        auto actual_params = ((FuncRParams *)funcr_params)->params;
        auto formal_params = semant_table.FunctionTable[name]->formals;

        for (int i = 0 ; i < (*formal_params).size(); i++) {
            auto a_params = (*actual_params)[i];
            auto f_params = (*formal_params)[i];
            a_params->codeIR();
            IRgenTypeConverse(current_block, a_params->attribute.T.type, f_params->attribute.T.type, max_reg);
            parameters.push_back({Type2LLvm[f_params->attribute.T.type], GetNewRegOperand(max_reg)});
        }

        // 生成调用指令
        if (function_return_type == Type::VOID) {
            IRgenCallVoid(current_block, return_type, parameters, name->get_string());
        } else {
            IRgenCall(current_block, return_type, ++max_reg, parameters, name->get_string());
        }
    }
}

void UnaryExp_plus::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenSingleNode(unary_exp, NodeAttribute::ADD, B);
}

void UnaryExp_neg::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenSingleNode(unary_exp, NodeAttribute::SUB, B);
}

void UnaryExp_not::codeIR() {
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenSingleNode(unary_exp, NodeAttribute::NOT, B);
}

void IntConst::codeIR()
{  
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);//获取当前基本块
    ++max_reg;  //为新生成的指令分配一个唯一的寄存器编号
    IRgenArithmeticI32ImmAll(B, BasicInstruction::LLVMIROpcode::ADD, val, 0, max_reg);//生成加法指令
}

void FloatConst::codeIR() { 
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    ++max_reg;
    IRgenArithmeticF32ImmAll(B, BasicInstruction::LLVMIROpcode::FADD, val, 0, max_reg);
}

void StringConst::codeIR() { TODO("StringConst CodeIR"); }

void PrimaryExp_branch::codeIR() { exp->codeIR(); }

void assign_stmt::codeIR() { 
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    lval->codeIR();//调用左值的codeIR
    exp->codeIR(); //生成表达式计算结果的IR代码
    IRgenTypeConverse(B, exp->attribute.T.type, lval->attribute.T.type, max_reg);//类型转换
    IRgenStore(B, Type2LLvm[lval->attribute.T.type], GetNewRegOperand(max_reg), ((Lval *)lval)->ptr);//将表达式计算的结果（存储在由GetNewRegOperand(max_reg)返回的寄存器中）存储到左值指定的位置
}

void expr_stmt::codeIR() { exp->codeIR(); 
   attribute = exp->attribute;
}

void block_stmt::codeIR() { 
    irgen_table.symbol_table.enter_scope();  //进入一个新作用域
    b->codeIR();
    irgen_table.symbol_table.exit_scope();  //退出当前作用域
}

// Reference:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc line372-line394
void ifelse_stmt::codeIR() { 

    //创建新的LLVM基本块，并获取标签
    int if_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int else_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int end_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;

    //条件表达式（Cond）的真分支和假分支的标签
    Cond->true_label = if_label;
    Cond->false_label = else_label;
    Cond->codeIR();

    //获取当前标签对应的基本块
    LLVMBlock B1 = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B1, Cond->attribute.T.type, Type::BOOL, max_reg);//类型转换
    IRgenBrCond(B1, max_reg, if_label, else_label);//生成一个条件跳转指令

    //处理if分支
    now_label = if_label;
    ifstmt->codeIR();
    LLVMBlock B2 = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B2, end_label);//无条件跳转指令，跳转到if-else语句的结束位置

    //处理else分支
    now_label = else_label;
    elsestmt->codeIR();
    LLVMBlock B3 = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B3, end_label);//无条件跳转指令，跳转到if-else语句的结束位置

    now_label = end_label;//表示if-else语句完成
}

void if_stmt::codeIR() {  
     // 为if语句的真分支创建一个新的基本块，并获取其标签
    int if_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int end_label = llvmIR.NewBlock(function_now, ++max_label)->block_id; // 创建一个结束标签
 
    // 设置条件表达式的分支标签
    Cond->true_label = if_label;
    Cond->false_label = end_label;//条件为假时跳转到end_label
    // 调用条件表达式的codeIR方法来生成其IR
    Cond->codeIR();
 
    // 获取当前的基本块
    LLVMBlock B1 = llvmIR.GetBlock(function_now, now_label);
    // 类型转换
    IRgenTypeConverse(B1, Cond->attribute.T.type, Type::BOOL, max_reg);
    // 生成条件跳转指令，根据条件表达式的值跳转到if分支或跳过if分支
    IRgenBrCond(B1, max_reg, if_label, end_label);
 
    // 更新当前标签为if分支的标签
    now_label = if_label;
    // 调用if分支语句的codeIR方法来生成其IR
    ifstmt->codeIR();

    LLVMBlock B2 = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B2, end_label);
 
    // 更新当前标签为结束标签，表示if语句的结束
    now_label = end_label;
}

void while_stmt::codeIR() {
    // 创建新基本块，并获取其标签
    int cond_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int body_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int end_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;

    // 保存当前的循环起始和结束标签
    int saved_loop_start_label = loop_start_label;
    int saved_loop_end_label = loop_end_label;
 
    // 设置循环的起始和结束标签
    loop_start_label = cond_label;
    loop_end_label = end_label;

    //获取当前的代码块，并跳转到条件判断
    LLVMBlock B1 = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B1, cond_label);

    // 更新当前标签为条件标签，表示循环的开始,生成条件表达式的代码
    now_label = cond_label;

    Cond->true_label = body_label;
    Cond->false_label = end_label;
    Cond->codeIR();
    
    //类型转换和条件跳转
    LLVMBlock B2 = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B2, Cond->attribute.T.type, Type::BOOL, max_reg);
    IRgenBrCond(B2, max_reg, body_label, end_label); //根据条件判断是否进入循环体


    // 更新当前标签为循环体标签
    now_label = body_label;
    body->codeIR();

    // 生成一个无条件跳转指令，从循环体跳回到条件判断处
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B, cond_label);
 
    // 更新当前标签为循环结束标签
    now_label = end_label;
 
    // 恢复循环起始和结束标签
    loop_start_label = saved_loop_start_label;
    loop_end_label = saved_loop_end_label;
}

void for_stmt::codeIR() { 
      // 创建新基本块，并获取其标签
    int init_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int cond_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int body_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int update_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
    int end_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
 
    // 保存当前的循环起始和结束标签
    int saved_loop_start_label = loop_start_label;
    int saved_loop_end_label = loop_end_label;
 
    // 设置循环的起始和结束标签
    loop_start_label = cond_label;
    loop_end_label = end_label;
 
    // 初始化部分
    now_label = init_label;
    decl->codeIR();
 
    // 跳转到条件判断
    LLVMBlock B_init = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B_init, cond_label);
 
    // 条件判断部分
    now_label = cond_label;
    Cond->true_label = body_label;
    Cond->false_label = end_label;
    Cond->codeIR();
 
    // 类型转换和条件跳转
    LLVMBlock B_cond = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B_cond, Cond->attribute.T.type, Type::BOOL, max_reg);
    IRgenBrCond(B_cond, max_reg, body_label, end_label);
 
    // 循环体部分
    now_label = body_label;
    body->codeIR();
 
    // 更新部分
    now_label = update_label;
    latch->codeIR();
 
    // 生成一个无条件跳转指令，从更新部分跳回到条件判断处
    LLVMBlock B_update = llvmIR.GetBlock(function_now, now_label);
    IRgenBRUnCond(B_update, cond_label);
 
    // 循环结束标签
    now_label = end_label;
 
    // 恢复循环起始和结束标签
    loop_start_label = saved_loop_start_label;
    loop_end_label = saved_loop_end_label;
}

void continue_stmt::codeIR() { 
    // 获取当前的代码块
    LLVMBlock B = llvmIR.function_block_map[function_now][now_label];
 
    // 生成无条件跳转指令到循环的起始处
    IRgenBRUnCond(B, loop_start_label);

    // 创建新的代码块
    now_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
}

void break_stmt::codeIR() {  

    // 获取当前的代码块
    LLVMBlock B = llvmIR.function_block_map[function_now][now_label];
 
    // 生成无条件跳转指令到循环结束后的位置
    IRgenBRUnCond(B, loop_end_label);

    // 创建新的代码块
    now_label = llvmIR.NewBlock(function_now, ++max_label)->block_id;
}

void return_stmt::codeIR() {
    return_exp->codeIR();
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenTypeConverse(B, return_exp->attribute.T.type, function_returntype, max_reg);//类型转换
    IRgenRetReg(B, Type2LLvm[function_returntype], max_reg);//返回值存储在指定的寄存器中
}

void return_stmt_void::codeIR() { 
    LLVMBlock B = llvmIR.GetBlock(function_now, now_label);
    IRgenRetVoid(B);
}

void ConstInitVal::codeIR() {  }

void ConstInitVal_exp::codeIR() {  exp->codeIR();}

void VarInitVal::codeIR() { }

void VarInitVal_exp::codeIR() {  exp->codeIR(); }

void VarDef_no_init::codeIR() {  }

void VarDef::codeIR() { }

void ConstDef::codeIR() {  }

// Reference:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc line593-line635
void VarDecl::codeIR() {
    // 获取当前基本块
    LLVMBlock B = llvmIR.GetBlock(function_now, 0);
    LLVMBlock InitB = llvmIR.GetBlock(function_now, now_label);

    // 获取变量定义列表
    auto def_vector = *var_def_list;

    // 遍历变量定义列表
    for (auto def : def_vector) {
        // 创建变量属性对象
        VarAttribute val;
        val.type = type_decl;    // 初始化变量类型

        // 添加符号到符号表
        irgen_table.symbol_table.add_Symbol(def->get_name(), ++max_reg);
        int alloca_reg = max_reg;

        // 检查是否为数组变量
        if (def->GetDims() != nullptr) {
            // 获取数组维度
            auto dim_vector = *def->GetDims();
            for (auto d : dim_vector) {
                val.dims.push_back(d->attribute.V.val.IntVal);
            }

            // 生成数组分配指令
            IRgenAllocaArray(B, Type2LLvm[type_decl], alloca_reg, val.dims);
            irgen_table.RegTable[alloca_reg] = val;

            // 获取初始化值
            InitVal init = def->get_init();
            if (init != nullptr) {
                // 计算数组大小
                int array_sz = 1;
                for (auto d : val.dims) {
                    array_sz *= d;
                }

                // 生成 memset 指令将数组初始化为零
                CallInstruction *memsetCall = new CallInstruction(CallInstruction::LLVMType::VOID, Operand(nullptr), std::string("llvm.memset.p0.i32"));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::PTR, GetNewRegOperand(alloca_reg));    // 数组地址
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I8, new ImmI32Operand(0));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I32, new ImmI32Operand(array_sz * sizeof(int)));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I1, new ImmI32Operand(0));
                llvmIR.function_block_map[function_now][now_label]->InsertInstruction(1, memsetCall);

                // 递归初始化数组
                RecursiveArrayInitIR(InitB, val.dims, alloca_reg, init, 0, array_sz - 1, 0, type_decl);
            }
        } else {    // 普通变量
            // 生成普通变量分配指令
            IRgenAlloca(B, Type2LLvm[type_decl], alloca_reg);
            irgen_table.RegTable[alloca_reg] = val;

            // 获取初始化值
            InitVal init = def->get_init();
            if (init != nullptr) {
                // 生成初始化表达式的 IR 代码
                Expression initExp = init->GetExp();
                initExp->codeIR();
                //转换为目标类型
                IRgenTypeConverse(InitB, initExp->attribute.T.type, type_decl, max_reg);
                Operand val_operand = GetNewRegOperand(max_reg);//记录操作数

                // 存储初始化值
                IRgenStore(InitB, Type2LLvm[type_decl], val_operand, GetNewRegOperand(alloca_reg));
            } else {    // 默认初始化为零
                Operand val_operand;
                if (type_decl == Type::INT) {
                    //生成默认值0
                    IRgenArithmeticI32ImmAll(InitB, BasicInstruction::LLVMIROpcode::ADD, 0, 0, ++max_reg);
                    val_operand = GetNewRegOperand(max_reg);//存储值到新寄存器
                } else if (type_decl == Type::FLOAT) {
                    IRgenArithmeticF32ImmAll(InitB, BasicInstruction::LLVMIROpcode::FADD, 0, 0, ++max_reg);
                    val_operand = GetNewRegOperand(max_reg);
                }

                // 存储默认值
                IRgenStore(InitB, Type2LLvm[type_decl], val_operand, GetNewRegOperand(alloca_reg));
            }
        }
    }
}

void ConstDecl::codeIR() {
    //获取当前函数和标签块
    LLVMBlock B = llvmIR.GetBlock(function_now, 0);
    LLVMBlock InitB = llvmIR.GetBlock(function_now, now_label);

    //遍历变量定义列表
    auto def_vector = *var_def_list;
    for (auto def : def_vector) {
        VarAttribute val;
        val.type = type_decl;    //初始化变量属性
        //添加符号到符号表
        irgen_table.symbol_table.add_Symbol(def->get_name(), ++max_reg);
        int alloca_reg = max_reg;

        //处理数组变量
        if (def->GetDims() != nullptr) {    
            auto dim_vector = *def->GetDims();

            //遍历数组的维度，初始化 val.dims
            for (auto d : dim_vector) {   
                val.dims.push_back(d->attribute.V.val.IntVal);
            }
            IRgenAllocaArray(B, Type2LLvm[type_decl], alloca_reg, val.dims);//分配空间
            irgen_table.RegTable[alloca_reg] = val;

            InitVal init = def->get_init();


            //数组有初始值
            if (init != nullptr) {
                int array_sz = 1;
                for (auto d : val.dims) {
                    array_sz *= d;
                }

                CallInstruction *memsetCall = new CallInstruction(CallInstruction::LLVMType::VOID, nullptr, std::string("llvm.memset.p0.i32"));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::PTR, GetNewRegOperand(alloca_reg));    // array address
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I8, new ImmI32Operand(0));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I32, new ImmI32Operand(array_sz * sizeof(int)));
                memsetCall->push_back_Parameter(CallInstruction::LLVMType::I1, new ImmI32Operand(0));
                llvmIR.function_block_map[function_now][now_label]->InsertInstruction(1, memsetCall);
                // 递归地初始化数组元素
                RecursiveArrayInitIR(InitB, val.dims, alloca_reg, init, 0, array_sz - 1, 0, type_decl);
            }
        } else {    // 不是数组
            IRgenAlloca(B, Type2LLvm[type_decl], alloca_reg); //分配单个空间
            irgen_table.RegTable[alloca_reg] = val;
            Operand val_operand;
            InitVal init = def->get_init();
            //判断一定有初始值
            assert(init != nullptr);
            Expression initExp = init->GetExp(); //获取初始值表达式
            initExp->codeIR();
            IRgenTypeConverse(InitB, initExp->attribute.T.type, type_decl, max_reg);//类型转换
            val_operand = GetNewRegOperand(max_reg);
            
            //生成存储指令并插入到指定的基本块中
            IRgenStore(InitB, Type2LLvm[type_decl], val_operand, GetNewRegOperand(alloca_reg));
        }
    }
}

void BlockItem_Decl::codeIR() { decl->codeIR(); }

void BlockItem_Stmt::codeIR() { stmt->codeIR();}

void __Block::codeIR() {
    irgen_table.symbol_table.enter_scope();

    auto item_vector = *item_list;
    for (auto item : item_vector) {
        item->codeIR();
    }

    irgen_table.symbol_table.exit_scope();
}

void __FuncFParam::codeIR() {}

//// Reference:https://github.com/yuhuifishash/SysY/blob/master/ir_gen/IRgen.cc line723-line780
void __FuncDef::codeIR() {
    // 添加函数定义的 LLVM 指令
    irgen_table.symbol_table.enter_scope();

    // 获取函数返回类型的 LLVM 类型
    BasicInstruction::LLVMType funcRetType = Type2LLvm[return_type];
    FuncDefInstruction funcDefIns = new FunctionDefineInstruction(funcRetType, name->get_string());

    // 初始化寄存器和标签计数器
    max_reg = -1;
    irgen_table.RegTable.clear();
    irgen_table.FormalArrayTable.clear();

    // 初始化标签计数器
    now_label = 0;
    max_label = 0;
    function_now = funcDefIns;
    function_returntype = return_type;

    // 创建新的函数对象
    llvmIR.NewFunction(function_now);

    // 创建入口基本块
    LLVMBlock entryBlock = llvmIR.NewBlock(function_now, max_label);

    // 处理形式参数
    auto formalsVec = *formals;
    max_reg = formalsVec.size() - 1;
    for (int i = 0; i < formalsVec.size(); ++i) {
        auto formal = formalsVec[i];
        VarAttribute attr;
        attr.type = formal->type_decl;
        GlobalVarDefineInstruction::LLVMType llvmType = Type2LLvm[formal->type_decl];

        if (formal->dims != nullptr) {  // 形式参数是数组
            // 在 SysY 中，我们假设不能修改数组地址，因此不需要 alloca
            funcDefIns->InsertFormal(BasicInstruction::LLVMType::PTR);

            for (int j = 1; j < formal->dims->size(); ++j) {  // 忽略第一个维度
                auto dim = formal->dims->at(j);
                attr.dims.push_back(dim->attribute.V.val.IntVal);
            }

            irgen_table.FormalArrayTable[i] = 1;
            irgen_table.symbol_table.add_Symbol(formal->name, i);
            irgen_table.RegTable[i] = attr;
        } else {  // 形式参数不是数组
            funcDefIns->InsertFormal(llvmType);
            IRgenAlloca(entryBlock, llvmType, ++max_reg);
            IRgenStore(entryBlock, llvmType, GetNewRegOperand(i), GetNewRegOperand(max_reg));
            irgen_table.symbol_table.add_Symbol(formal->name, max_reg);
            irgen_table.RegTable[max_reg] = attr;
        }
    }

    // 生成无条件跳转指令到下一个基本块
    IRgenBRUnCond(entryBlock, 1);

    // 创建函数体基本块
    LLVMBlock bodyBlock = llvmIR.NewBlock(function_now, ++max_label);
    now_label = max_label;

    // 生成函数体的 IR 代码
    block->codeIR();

    // 添加没有返回值的基本块
    AddNoReturnBlock();

    // 记录最大寄存器和标签编号
    max_reg_map[funcDefIns] = max_reg;
    max_label_map[funcDefIns] = max_label;

    // 退出当前作用域
    irgen_table.symbol_table.exit_scope();
}

void CompUnit_Decl::codeIR() {}

void CompUnit_FuncDef::codeIR() { func_def->codeIR(); }

void AddLibFunctionDeclare() {
    FunctionDeclareInstruction *getint = new FunctionDeclareInstruction(BasicInstruction::I32, "getint");
    llvmIR.function_declare.push_back(getint);

    FunctionDeclareInstruction *getchar = new FunctionDeclareInstruction(BasicInstruction::I32, "getch");
    llvmIR.function_declare.push_back(getchar);

    FunctionDeclareInstruction *getfloat = new FunctionDeclareInstruction(BasicInstruction::FLOAT32, "getfloat");
    llvmIR.function_declare.push_back(getfloat);

    FunctionDeclareInstruction *getarray = new FunctionDeclareInstruction(BasicInstruction::I32, "getarray");
    getarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(getarray);

    FunctionDeclareInstruction *getfloatarray = new FunctionDeclareInstruction(BasicInstruction::I32, "getfarray");
    getfloatarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(getfloatarray);

    FunctionDeclareInstruction *putint = new FunctionDeclareInstruction(BasicInstruction::VOID, "putint");
    putint->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(putint);

    FunctionDeclareInstruction *putch = new FunctionDeclareInstruction(BasicInstruction::VOID, "putch");
    putch->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(putch);

    FunctionDeclareInstruction *putfloat = new FunctionDeclareInstruction(BasicInstruction::VOID, "putfloat");
    putfloat->InsertFormal(BasicInstruction::FLOAT32);
    llvmIR.function_declare.push_back(putfloat);

    FunctionDeclareInstruction *putarray = new FunctionDeclareInstruction(BasicInstruction::VOID, "putarray");
    putarray->InsertFormal(BasicInstruction::I32);
    putarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(putarray);

    FunctionDeclareInstruction *putfarray = new FunctionDeclareInstruction(BasicInstruction::VOID, "putfarray");
    putfarray->InsertFormal(BasicInstruction::I32);
    putfarray->InsertFormal(BasicInstruction::PTR);
    llvmIR.function_declare.push_back(putfarray);

    FunctionDeclareInstruction *starttime = new FunctionDeclareInstruction(BasicInstruction::VOID, "_sysy_starttime");
    starttime->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(starttime);

    FunctionDeclareInstruction *stoptime = new FunctionDeclareInstruction(BasicInstruction::VOID, "_sysy_stoptime");
    stoptime->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(stoptime);

    // 一些llvm自带的函数，也许会为你的优化提供帮助
    FunctionDeclareInstruction *llvm_memset =
    new FunctionDeclareInstruction(BasicInstruction::VOID, "llvm.memset.p0.i32");
    llvm_memset->InsertFormal(BasicInstruction::PTR);
    llvm_memset->InsertFormal(BasicInstruction::I8);
    llvm_memset->InsertFormal(BasicInstruction::I32);
    llvm_memset->InsertFormal(BasicInstruction::I1);
    llvmIR.function_declare.push_back(llvm_memset);

    FunctionDeclareInstruction *llvm_umax = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.umax.i32");
    llvm_umax->InsertFormal(BasicInstruction::I32);
    llvm_umax->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_umax);

    FunctionDeclareInstruction *llvm_umin = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.umin.i32");
    llvm_umin->InsertFormal(BasicInstruction::I32);
    llvm_umin->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_umin);

    FunctionDeclareInstruction *llvm_smax = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.smax.i32");
    llvm_smax->InsertFormal(BasicInstruction::I32);
    llvm_smax->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_smax);

    FunctionDeclareInstruction *llvm_smin = new FunctionDeclareInstruction(BasicInstruction::I32, "llvm.smin.i32");
    llvm_smin->InsertFormal(BasicInstruction::I32);
    llvm_smin->InsertFormal(BasicInstruction::I32);
    llvmIR.function_declare.push_back(llvm_smin);

    FunctionDeclareInstruction *llvm_fmin = new FunctionDeclareInstruction(BasicInstruction::FLOAT32, "llvm.fmin.f32");
    llvm_fmin->InsertFormal(BasicInstruction::FLOAT32);
    llvm_fmin->InsertFormal(BasicInstruction::FLOAT32);
    llvmIR.function_declare.push_back(llvm_fmin);

    FunctionDeclareInstruction *llvm_fmax = new FunctionDeclareInstruction(BasicInstruction::FLOAT32, "llvm.fmax.f32");
    llvm_fmax->InsertFormal(BasicInstruction::FLOAT32);
    llvm_fmax->InsertFormal(BasicInstruction::FLOAT32);
    llvmIR.function_declare.push_back(llvm_fmax);
}
