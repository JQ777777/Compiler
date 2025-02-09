#include "simplify_cfg.h"
#include <unordered_set>
#include <functional>

//传递的入口点，负责遍历所有函数的 CFG，并调用 EliminateUnreachedBlocksInsts 来简化每个 CFG
void SimplifyCFGPass::Execute() {
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        EliminateUnreachedBlocksInsts(cfg);
    }
}

void RemoveInstructionsAfter(BasicBlock* block, int instr_index) {
    if (instr_index >= 0 && instr_index < block->Instruction_list.size()) {
        // 删除从 instr_index 之后的所有指令
        //block->Instruction_list.erase(block->Instruction_list.begin() + instr_index + 1, block->Instruction_list.end());
        while (block->Instruction_list.size()>instr_index+1)
        {
            block->Instruction_list.pop_back();
        }
    }
}

// 删除从函数入口开始到达不了的基本块和指令
// 不需要考虑控制流为常量的情况，你只需要从函数入口基本块(0号基本块)开始dfs，将没有被访问到的基本块和指令删去即可
void SimplifyCFGPass::EliminateUnreachedBlocksInsts(CFG *C) { 
    //TODO("EliminateUnreachedBlocksInsts"); 

    std::stack<int> stack;
    stack.push(0);
    // 创建一个集合，用于存储已访问的基本块编号，避免重复访问
    std::map<int, int> visited;

    // 从 0 号基本块开始进行 DFS
    while(!stack.empty())
    {
        int current = stack.top();
        stack.pop();
        visited[current] = 1;
        
        BasicBlock* block = (*C->block_map)[current];
        // 遍历当前基本块的指令列表
        for (size_t i = 0; i < block->Instruction_list.size(); ++i) {
            const Instruction& instr = block->Instruction_list[i];
            switch (instr->GetOpcode()) {
                case BasicInstruction::BR_UNCOND:
                case BasicInstruction::RET: {
                    // 如果遇到无条件跳转或返回指令，删除该指令之后的所有指令
                    RemoveInstructionsAfter(block, i);
                    break;
                }
                default:
                    // 继续处理其他指令
                    break;
            }
        }
        Instruction lastins;
        if (!block->Instruction_list.empty()) {
            lastins = block->Instruction_list.back();
        }
        if (lastins->GetOpcode() == BasicInstruction::BR_UNCOND)
        {
            BrUncondInstruction *br_uncond = (BrUncondInstruction *)lastins;
            Operand label = br_uncond->GetDestLabel();
            LabelOperand* labelop = (LabelOperand*)label;
            int labelno = labelop->GetLabelNo();
            if(visited[labelno] != 1)
            {
                visited[labelno] = 1;
                stack.push(labelno);
            }
        }
        if (lastins->GetOpcode() == BasicInstruction::BR_COND)
        {
            BrCondInstruction *br_cond = (BrCondInstruction *)lastins;
            Operand labeltrue = br_cond->GetTrueLabel();
            Operand labelfalse = br_cond->GetFalseLabel();
            LabelOperand* labeloptrue = (LabelOperand*)labeltrue;
            LabelOperand* labelopfalse = (LabelOperand*)labelfalse;
            int labelnotrue = labeloptrue->GetLabelNo();
            int labelnofalse = labelopfalse->GetLabelNo();
            if(visited[labelnotrue] != 1)
            {
                visited[labelnotrue] = 1;
                stack.push(labelnotrue);
            }
            if(visited[labelnofalse] != 1)
            {
                visited[labelnofalse] = 1;
                stack.push(labelnofalse);
            }
        }
    }

    // 遍历整个 CFG，删除不可达的基本块
    std::vector<int> unreachable_blocks;
    for (auto& [block_id, block] : *C->block_map) {
        if (visited[block_id] != 1) {
            // 删除不可达的基本块及其包含的指令
            unreachable_blocks.push_back(block_id);
        }
    }
    // 一次性删除所有不可达的基本块
    for (int block_id : unreachable_blocks) {
        C->block_map->erase(block_id);
    }

    // 更新 CFG 结构，确保删除后的 CFG 仍然正确
    //C->UpdateEdges();
}