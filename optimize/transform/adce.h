#ifndef ADCE_H
#define ADCE_H
#include "../../include/ir.h"
#include "../pass.h"

#include "../analysis/dominator_tree.h"

class ADCEPass : public IRPass {
private:
    DomAnalysis *domtrees;
    // TODO():添加更多你需要的成员变量
    std::vector<std::vector<LLVMBlock>> BuildCDG(CFG *C);
    

public:
    ADCEPass(LLVMIR *IR, DomAnalysis *dom) : IRPass(IR) { domtrees = dom; }
    void Execute();
    void ACDE(CFG *C);
};

#endif