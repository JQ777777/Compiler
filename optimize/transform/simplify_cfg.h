#ifndef SIMPLIFY_CFG_H
#define SIMPLIFY_CFG_H
#include "../../include/ir.h"
#include "../pass.h"

//控制流图简化类
class SimplifyCFGPass : public IRPass {
private:
    // TODO():添加更多你需要的成员变量
    // 消除 CFG 中不可达的基本块和指令函数
    void EliminateUnreachedBlocksInsts(CFG *C);

public:
    SimplifyCFGPass(LLVMIR *IR) : IRPass(IR) {}
    void Execute();
};

#endif