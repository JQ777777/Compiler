#ifndef CFG_H
#define CFG_H

#include "SysY_tree.h"
#include "basic_block.h"
#include <bitset>
#include <iostream>
#include <map>
#include <queue>
#include <set>
#include <vector>
#include <unordered_map>
#include <memory>

class CFG {
public:
    

    int max_label = 0;
    int max_reg = 0;
    FuncDefInstruction function_def;//存储函数定义的信息

    /*this is the pointer to the value of LLVMIR.function_block_map
      you can see it in the LLVMIR::CFGInit()*/
    //存储基本块的标识符到LLVMBlock对象的映射关系
    std::map<int, LLVMBlock> *block_map;
    std::map<int, std::vector<int>> edges;  // 存储边关系
    LLVMBlock return_block; //return的基本块
    //std::vector<std::unique_ptr<BasicBlock>> blocks;  // 存储所有基本块
    std::vector<BasicBlock*> retBlocks;

    // 使用邻接表存图
    std::vector<std::vector<LLVMBlock>> G{};       // 存储控制流图
    std::vector<std::vector<LLVMBlock>> invG{};    // 逆向控制流图，存储了每个基本块的所有前驱基本块

    void BuildCFG();//构建控制流图

    // 获取某个基本块节点的前驱/后继
    std::vector<LLVMBlock> GetPredecessor(LLVMBlock B);//前驱
    std::vector<LLVMBlock> GetPredecessor(int bbid);
    std::vector<LLVMBlock> GetSuccessor(LLVMBlock B);//后继
    std::vector<LLVMBlock> GetSuccessor(int bbid);

    std::pair<std::vector<int>, bool> GetBranchTargets(BasicBlock* bb);
    // 创建一个新的基本块并插入 PHI 指令
    BasicBlock* CreateReturnMergeBlock(const std::vector<BasicBlock*>& retBlocks);

    void RemoveBlock(int block_id);
    void UpdateEdges();
    bool IsBlockExist(int block_id);
    void HandleIsolatedBlocks();
    BasicBlock* FindNewReturnBlock();
    void RemoveInstructionsFromBlock(int block_id);
    void RebuildBlockMap();
    void InitializeCFGMembers(CFG *cfg);
    void RebuildG();
    void RebuildInvG(); 
};

#endif