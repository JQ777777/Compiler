#ifndef DOMINATOR_TREE_H
#define DOMINATOR_TREE_H
#include "../../include/ir.h"
#include "../pass.h"
#include <set>
#include <vector>

class DominatorTree {
public:
    CFG *C;

    std::vector<std::vector<LLVMBlock>> dom_tree{};
    std::vector<LLVMBlock> idom{};
    std::map<int, int> dom;
    std::map<int, std::unordered_set<int>> domset;

    //std::vector<std::vector<LLVMBlock>> dom_reverse_tree{};//反向支配树

    void BuildDominatorTree(bool reverse = false);    // build the dominator tree of CFG* C
    std::set<int> GetDF(std::set<int> S);             // return DF(S)  S = {id1,id2,id3,...}获取支配边界集合
    std::set<int> GetDF(int id);                      // return DF(id)获取单个节点的支配边界
    //检查一个节点是否支配另一个节点
    bool IsDominate(int id1, int id2);                // if blockid1 dominate blockid2, return true, else return false
    bool IsDominateHelper(int id1, int id2,  std::map<int, std::unordered_set<int>> domset);
    void UpdateDomSet(int blockid, int new_idom);
    int CommonDominator(int a, int b, std::vector<std::vector<LLVMBlock>>& invG);

    // TODO(): add or modify functions and members if you need
    std::map<int, std::set<int>> df;
};

class DomAnalysis : public IRPass {
private:
    std::map<CFG *, DominatorTree> DomInfo;
    std::map<CFG *, DominatorTree> DomInfoReverse;

public:
    DomAnalysis(LLVMIR *IR) : IRPass(IR) {}
    void Execute();
    DominatorTree *GetDomTree(CFG *C) { return &DomInfo[C]; }
    // TODO(): add more functions and members if you need
    DominatorTree *GetReverseDomTree(CFG *C) { return &DomInfoReverse[C]; }
};
#endif