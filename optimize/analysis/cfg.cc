#include "../../include/Instruction.h"
#include "../../include/ir.h"
#include "../../include/basic_block.h"
#include <assert.h>
#include <algorithm>
#include <unordered_set>


extern std::map<FuncDefInstruction, int> max_label_map;
extern std::map<FuncDefInstruction, int> max_reg_map;

//判断是否为return基本块
bool IsReturnBlock(const LLVMBlock& block) {
    // 遍历基本块的指令列表，检查是否存在返回指令
    for (const auto& instr : block->Instruction_list) {
        if (instr->GetOpcode() == BasicInstruction::RET) {
            return true;
        }
    }
    return false;
}

//初始化控制流图
void LLVMIR::CFGInit() {
    for (auto &[defI, bb_map] : function_block_map) {
        CFG *cfg = new CFG();
        cfg->block_map = &bb_map;
        cfg->function_def = defI;
        cfg->max_reg = max_reg_map[defI];
        cfg->max_label = max_label_map[defI];
        
        //cfg->BuildCFG();
        //TODO("init your members in class CFG if you need");
        llvm_cfg[defI] = cfg;
        // 初始化 edges
        cfg->edges.clear();  // 确保 edges 是空的
        cfg->max_label = max_label_map[defI];
        // 初始化 blocks
        //cfg->blocks.clear();  // 确保 blocks 是空的
        
        // 构建 CFG
        cfg->BuildCFG();
        
        // 初始化 return_block
        // 你可以在这里根据需要查找并设置 return_block
        // 例如，找到最后一个基本块并将其设置为 return_block
        // for (const auto &[block_id, block] : bb_map) {
        //     if (IsReturnBlock(block)) {  // 假设有一个 IsReturnBlock 函数来判断是否是返回块
        //         cfg->return_block = block;
        //         break;
        //     }
        // }
    }
}

//初始化blocks
// void CFG::InitializeCFGMembers(CFG *cfg) {
//     // 假设 bb_map 是 std::map<int, BasicBlock*>
//     for (const auto &[block_id, block_ptr] : *cfg->block_map) {
//         // 将每个基本块的指针包装为 unique_ptr 并添加到 blocks 向量中
//         blocks.push_back(std::unique_ptr<BasicBlock>(block_ptr));
//     }
// }

//构建所有函数的控制流图
void LLVMIR::BuildCFG() {
    for (auto [defI, cfg] : llvm_cfg) {
        cfg->BuildCFG();
    }
}

//构建控制流图判断指令函数（自行编写的）
std::pair<std::vector<int>, bool> CFG::GetBranchTargets(BasicBlock* bb) {
    if (bb->Instruction_list.empty()) {
        return {{}, false};  // 空指令列表，返回空向量和 false
    }

    std::vector<int> targets;
    bool isRet = false;

    // 遍历所有指令，查找 BR_UNCOND, BR_COND 和 RET 指令
    for (const auto& instr : bb->Instruction_list) {
        switch (instr->GetOpcode()) {
            case BasicInstruction::BR_UNCOND: {
                auto* bruncond = dynamic_cast<BrUncondInstruction*>(instr);
                if (!bruncond) {
                    throw std::runtime_error("Invalid cast to BrUncondInstruction");
                }
                auto* labelOp = dynamic_cast<LabelOperand*>(bruncond->GetDestLabel());
                if (!labelOp) {
                    throw std::runtime_error("Invalid cast to LabelOperand");
                }
                int blockno = labelOp->GetLabelNo();
                targets.push_back(blockno);
                break;
            }
            case BasicInstruction::BR_COND: {
                auto* brcond = dynamic_cast<BrCondInstruction*>(instr);
                if (!brcond) {
                    throw std::runtime_error("Invalid cast to BrCondInstruction");
                }
                auto* labelOptrue = dynamic_cast<LabelOperand*>(brcond->GetTrueLabel());
                auto* labelOpfalse = dynamic_cast<LabelOperand*>(brcond->GetFalseLabel());
                if (!labelOptrue || !labelOpfalse) {
                    throw std::runtime_error("Invalid cast to LabelOperand");
                }
                int blocktrueno = labelOptrue->GetLabelNo();
                targets.push_back(blocktrueno);
                int blockfalseno = labelOpfalse->GetLabelNo();
                targets.push_back(blockfalseno);
                break;
            }
            case BasicInstruction::RET: {
                isRet = true;
                // 如果遇到 RET 指令，可以提前结束遍历，因为后面的指令不会被执行
                break;
            }
            default:
                // 忽略其他类型的指令
                continue;
        }

        // 如果遇到 RET 指令，可以提前结束遍历，因为后面的指令不会被执行
        if (isRet) {
            break;
        }
    }

    return {targets, isRet};
}

// 创建一个新的基本块并插入 PHI 指令(自行编写的)
BasicBlock* CFG::CreateReturnMergeBlock(const std::vector<BasicBlock*>& retBlocks) {
    // 创建一个新的基本块
    int newLabel = max_label + 1;
    max_label = newLabel;
    BasicBlock* mergeBlock = new BasicBlock(newLabel);
    (*block_map)[newLabel] = mergeBlock;
    // 新合并块的标签
    Operand mergeLabelOp = new LabelOperand(mergeBlock->block_id);

    for (auto* block : retBlocks) {
        if (block_map->find(block->block_id) == block_map->end()){
            continue;
        }
        for (auto& instr : block->Instruction_list) {
            if (instr->GetOpcode()==BasicInstruction::RET){
                auto* ret = dynamic_cast<RetInstruction*>(instr);
                enum BasicInstruction::LLVMType phiType = ret->GetType();
                if (phiType == BasicInstruction::VOID){
                    //修改该return指令，将return指令改成跳转到新标签
                    // 替换 RET 指令为 BR_UNCOND 指令
                    BrUncondInstruction* brUncond = new BrUncondInstruction(mergeLabelOp);
                    instr = brUncond;  // 替换原有指令

                    // 创建 RET 指令，返回 PHI 指令的结果
                    RetInstruction* ret = new RetInstruction(phiType, nullptr);
                    mergeBlock->Instruction_list.push_back(ret);

                    return mergeBlock;
                }
                else break;
            }
        }
    }

    // 插入 PHI 指令
    std::vector<std::pair<Operand, Operand>> phiOperands;
    // 确定 PHI 指令的类型（假设返回值是整数类型）
    enum BasicInstruction::LLVMType phiType;
    for (auto* block : retBlocks) {
        if (block_map->find(block->block_id) == block_map->end()){
            continue;
        }
        for (auto& instr : block->Instruction_list) {
            if (instr->GetOpcode()==BasicInstruction::RET){
                // 假设每个 RET 指令有一个返回值，获取该返回值
                auto* ret = dynamic_cast<RetInstruction*>(instr);
                if (!ret) {
                    throw std::runtime_error("Invalid cast to RetInstruction");
                }
                Operand retValue = ret->GetRetVal();  // 获取返回值
                Operand labelOp = new LabelOperand(block->block_id);
                phiType = ret->GetType() ;
                phiOperands.emplace_back(labelOp, retValue);

                //修改该return指令，将return指令改成跳转到新标签
                // 替换 RET 指令为 BR_UNCOND 指令
                BrUncondInstruction* brUncond = new BrUncondInstruction(mergeLabelOp);
                instr = brUncond;  // 替换原有指令
                break;
            }
        }
    }

    // 创建一个新的虚拟寄存器作为 PHI 指令的结果操作数
    Operand phiResult = GetNewRegOperand(++max_reg);

    // 创建 PHI 指令并插入到新的基本块中
    PhiInstruction* phi = new PhiInstruction(phiType, phiResult, phiOperands);
    mergeBlock->Instruction_list.push_back(phi);

    // 创建 RET 指令，返回 PHI 指令的结果
    RetInstruction* ret = new RetInstruction(phiType, phiResult);
    mergeBlock->Instruction_list.push_back(ret);

    return mergeBlock;
}

//构建控制流图
void CFG::BuildCFG() { 
    //TODO("BuildCFG"); 

    // 初始化控制流图和逆向控制流图
    G.clear();
    invG.clear();
    
    G.resize(max_label+2);
    invG.resize(max_label+2);

    // 记录所有包含 RET 语句的基本块
    //std::vector<BasicBlock*> retBlocks;

    // 遍历所有基本块
    for (auto &[bbid, block] : *block_map) {
        auto [targets, isRet] = GetBranchTargets(block);

        if (isRet) {
            return_block = block;
            retBlocks.push_back(block);
        } else {
            for (int target_id : targets) {
                G[bbid].push_back((*block_map)[target_id]);
                invG[target_id].push_back(block);
            }
        }
    }

    // 如果有多个 RET 语句，创建一个新的基本块并更新控制流图
    // if (retBlocks.size() > 1) {
    //     BasicBlock* mergeBlock = CreateReturnMergeBlock(retBlocks);

    //     // 更新控制流图，将所有 RET 语句所在的块指向新的合并块
    //     for (auto* block : retBlocks) {
    //         G[block->block_id].push_back(mergeBlock);
    //         invG[mergeBlock->block_id].push_back(block);
    //     }

    //     // 将新的合并块作为返回块
    //     return_block = mergeBlock;
    // } else if (retBlocks.size() == 1) {
    //     // 如果只有一个 RET 语句，直接将其作为返回块
    //     return_block = retBlocks[0];
    // }
}

//返回其所有前驱基本块
std::vector<LLVMBlock> CFG::GetPredecessor(LLVMBlock B) { return invG[B->block_id]; }

std::vector<LLVMBlock> CFG::GetPredecessor(int bbid) { return invG[bbid]; }

//返回其所有后驱基本块
std::vector<LLVMBlock> CFG::GetSuccessor(LLVMBlock B) { return G[B->block_id]; }

std::vector<LLVMBlock> CFG::GetSuccessor(int bbid) { return G[bbid]; }

//删除基本块内的指令
void BasicBlock::RemoveInstructions(){
    Instruction_list.clear();
}

// 删除指定基本块中的所有指令(自行编写)
    void CFG::RemoveInstructionsFromBlock(int block_id) {
        LLVMBlock &block = (*block_map)[block_id];
        block->RemoveInstructions();
    }

// 辅助函数：重新构建 block_map
// void CFG::RebuildBlockMap() {
//     if (block_map != nullptr) {
//         block_map->clear();
//         for (const auto& block_ptr : blocks) {
//             // 使用 get() 方法获取 std::unique_ptr 中的裸指针
//             (*block_map)[block_ptr->block_id] = block_ptr.get();
//         }
//     }
// }

// 辅助函数：重新构建 G 和 invG
void CFG::RebuildG() {
    // 清空现有的 G
    G.clear();
    G.resize(max_label+1);

    // 重新构建 G
    for (const auto& [block_id, block] : *block_map) {
        std::vector<LLVMBlock> successors;
        for (const auto& target_block : GetSuccessor(block_id)) {
            successors.push_back(target_block);
        }
        if (block_id < static_cast<int>(G.size())) {
            G[block_id] = successors;
        } else {
            G.push_back(successors);
        }
    }
}

void CFG::RebuildInvG() {
    // 清空现有的 invG
    invG.clear();
    invG.resize(G.size());
    // 重新构建 invG
    for (const auto& [block_id, block] : *block_map) {
        for (const auto& target_block : GetSuccessor(block_id)) {
            int target_id = target_block->block_id;
            if (target_id < static_cast<int>(invG.size())) {
                invG[target_id].push_back(target_block);
            }
        }
    }
}


//移除基本块（自行编写的）
void CFG::RemoveBlock(int block_id) {
    // 检查 block_id 是否有效
    if (block_id < 0 || block_id >= G.size()) {
        throw std::out_of_range("Block index out of range");
    }
    // 从正向图 G 中移除对该基本块的引用
    for (int i = 0; i < G.size(); ++i) {
        auto& successors = G[i];
        successors.erase(std::remove_if(successors.begin(), successors.end(),
                                        [block_id, this](LLVMBlock b) { return b->block_id == block_id; }),
                         successors.end());
    }
    // 从逆向图 invG 中移除对该基本块的引用
    for (int i = 0; i < invG.size(); ++i) {
        auto& predecessors = invG[i];
        predecessors.erase(std::remove_if(predecessors.begin(), predecessors.end(),
                                          [block_id, this](LLVMBlock b) { return b->block_id == block_id; }),
                           predecessors.end());
    }
    // 删除基本块中的所有指令
    RemoveInstructionsFromBlock(block_id);

    // 从 blocks 向量中移除该基本块
    // if (block_id < blocks.size()) {
    //     blocks.erase(blocks.begin() + block_id);
    // }

    // 从 block_map 中移除该基本块的条目
    // if (block_map != nullptr && block_map->find(block_id) != block_map->end()) {
    //     block_map->erase(block_id);
    // }

    // 更新返回基本块（如果被删除的是返回基本块）
    if (return_block != nullptr && return_block->block_id == block_id) {
        return_block = nullptr;
    }

    // 重新构建 block_map 以确保一致性
    // if (block_map != nullptr) {
    //     RebuildBlockMap();
    // }

    // 从 G 和 invG 中移除该基本块的条目
    if (block_id < static_cast<int>(G.size())) {
        G.erase(G.begin() + block_id);
    }
    if (block_id < static_cast<int>(invG.size())) {
        invG.erase(invG.begin() + block_id);
    }
}

// 更新 CFG 中的边关系(自行编写的)
void CFG::UpdateEdges(){
    // 重新构建 G 和 invG 以确保一致性
    RebuildG();
    RebuildInvG();
}

