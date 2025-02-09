#include "adce.h"
#include <tuple>
#include "../include/Instruction.h"
#include <unordered_set>
#include <iostream>  // 引入 std::cout

//入口函数
void ADCEPass::Execute() {
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        //调用函数（可以参考mem2reg），目前还没实现，实现了加到这
        //BuildCDG(cfg);
        std::cout << "ADCEPass begin" << std::endl;
        ACDE(cfg);
    }
}

//建立控制依赖图
std::vector<std::vector<LLVMBlock>> ADCEPass::BuildCDG(CFG *C) {
    // 从 DomAnalysis 中获取后支配树
    DominatorTree *ReverseDomTree = domtrees->GetReverseDomTree(C);

    // 初始化数据结构
    std::vector<std::vector<LLVMBlock>> CDG;
    std::vector<int> blockrd;  // 每个基本块的依赖计数
    CDG.resize(C->max_label + 2);
    blockrd.resize(C->max_label + 1, 0);

    // 打印调试信息：显示 CFG 的基本信息
    //std::cout << "CFG has " << C->max_label + 1 << " blocks." << std::endl;

    // 遍历所有基本块，构建控制依赖图
    for (auto &[bbid, block] : *C->block_map) {
        //std::cout << "Processing block " << bbid << std::endl;

        // 获取当前基本块的后支配边界
        auto domFrontier = ReverseDomTree->GetDF(bbid);
        //std::cout << "  Post-Dominance Frontier of block " << bbid << ": { ";
        // for (auto dbbid : domFrontier) {
        //     std::cout << dbbid << " ";
        // }
        // std::cout << "}" << std::endl;

        for (auto dbbid : domFrontier) {
            // 确保 dbbid 存在
            if (C->block_map->find(dbbid) == C->block_map->end()) {
                //std::cout << "  Warning: Block " << dbbid << " does not exist in block_map." << std::endl;
                continue;
            }

            // 添加控制依赖关系
            CDG[bbid].push_back((*C->block_map)[dbbid]);
            if (bbid != dbbid) {
                blockrd[bbid]++;
            }

            // 打印调试信息：显示控制依赖关系
            //std::cout << "  Adding control dependence from block " << bbid << " to block " << dbbid << std::endl;
        }
    }

    // 处理没有依赖的基本块
    for (auto &[bbid, block] : *C->block_map) {
        if (!blockrd[bbid]) {
            CDG[C->max_label + 1].push_back((*C->block_map)[bbid]);
            //std::cout << "  Block " << bbid << " has no dependencies and is added to the exit node." << std::endl;
        } else {
            //std::cout << "  Block " << bbid << " has " << blockrd[bbid] << " dependencies." << std::endl;
        }
    }

    // 打印最终的控制依赖图
    //std::cout << "Final Control Dependence Graph:" << std::endl;
    // for (int i = 0; i <= C->max_label; ++i) {
    //     if (!CDG[i].empty()) {
    //         std::cout << "  Block " << i << " depends on: ";
    //         for (auto &dep : CDG[i]) {
    //             std::cout << dep->block_id << " ";
    //         }
    //         std::cout << std::endl;
    //     }
    // }

    return CDG;
}

void ADCEPass::ACDE(CFG *C)
{
    // 初始化
    std::map<int, Instruction> defMap;   // defMap: 记录每个使用变量的定义位置，regno
    std::map<Instruction,int> workListid;         // workList: 存储所有影响控制流或其他变量的指令,基本块id
    std::set<int> visitedBlocks;            // 用来记录已处理的基本块，避免重复处理
    std::deque<Instruction> workList;

    //std::map<int, std::set<int>> storemap;
    //std::map<int, int> storenum;
    //std::map<int, std::set<int>> loadmap;

    std::set<Instruction> live={};  //所有的活跃指令
    std::set<int> liveBlock; //所有有活跃指令的基本块
    std::set<int> livereg;//活跃的寄存器
    //std::unordered_set<Operand>  workList;         // workList: 存储所有影响控制流或其他变量的指令
    //std::unordered_set<int, std::set<int>> defMap; 
    auto blockmap = *C->block_map;
    auto parentblock = BuildCDG(C);
    std::map<Instruction,int> inst2blockid;//记录所有指令所在的基本块id


    // 遍历所有基本块
    for (auto [blockid, block] : *C->block_map) {
        if (visitedBlocks.find(blockid) != visitedBlocks.end()) {
            continue;  // 如果该块已处理，则跳过
        }
         //std::cout << "Processing block " << blockid << std::endl;

        // 遍历当前基本块中的所有指令
        for (auto I : block->Instruction_list) {
            inst2blockid.insert({I,blockid});
            //std::cout << "Processing instruction: " << I->GetOpcode() << std::endl;
            // 检查当前指令的操作码
            if (I->GetOpcode() == BasicInstruction::STORE) {
                auto StoreI = dynamic_cast<StoreInstruction*>(I);
                //if (StoreI->GetPointer()->GetOperandType() == BasicOperand::GLOBAL) {
                    // 处理修改全局变量的 store 指令
                    Operand op = StoreI->GetPointer();
                    RegOperand* globalOp =  (RegOperand *)(op);
                    int globalVarId = globalOp->GetRegNo();
                    
                    // 将 store 操作记录到 storemap 中
                    //storemap[globalVarId].insert(blockid);
                    //storenum[globalVarId]++;
                    //std::cout << "Store instruction modifying global variable " << globalVarId << " in block " << blockid << std::endl;
                    
                    // 将 store 操作加入 workList
                    workListid.insert({I,blockid});
                    workList.push_back(I);
                    
                //}
            }          
            else if (I->GetOpcode() == BasicInstruction::CALL) {
                // 处理 call 指令（可能涉及全局变量的修改）
                 //std::cout << "Call instruction in block " << blockid << std::endl;
                workListid.insert({I,blockid});
                workList.push_back(I);
            }
            else if (I->GetOpcode() == BasicInstruction::RET) {
                // 处理 ret 指令（可能影响程序控制流）
                //std::cout << "Return instruction in block " << blockid << std::endl;
                workListid.insert({I,blockid});
                workList.push_back(I);
            }
            /*
            // 尝试将其转换为 ArithmeticInstruction 类型
            ArithmeticInstruction* arithmeticInstr = dynamic_cast<ArithmeticInstruction*>(I);
            if (arithmeticInstr != nullptr)
            {
                    Operand op = arithmeticInstr->GetResult();
                    RegOperand* globalOp =  (RegOperand *)(op);
                    int globalVarId = globalOp->GetRegNo();
                    defMap[globalVarId].insert(I); // 记录此指令的定义位置;
            }
            */

            if(I->GetResult())
            {
                Operand op = I->GetResult();
                RegOperand* globalOp =  (RegOperand *)(op);
                int globalVarId = globalOp->GetRegNo();
                defMap[globalVarId]=(I); // 记录此指令的定义位置;
                //std::cout << "Instruction " << globalVarId << " defines variable " << globalVarId << std::endl;
            }
                        
        }
        
        // 标记该基本块已处理
        visitedBlocks.insert(blockid);
    }

    DominatorTree *domTree = domtrees->GetReverseDomTree(C);

      // 活跃指令的处理：根据控制流图和支配图来识别活跃指令
    while (!workList.empty()) {
        // 从 workList 中取出一条指令
        auto inst = workList.front();
        auto liveblockid = workListid[inst];
        workList.pop_front();  // 移除已处理的指令
        //std::cout << "Processing workList instruction: " << inst->GetOpcode() << " in block " << liveblockid << std::endl;
       
        if(live.find(inst)!=live.end())
        {
            continue;
        }

        // 将当前指令标记为活跃
        live.insert(inst);
        //std::cout << "Processing instruction: " << inst << std::endl;
        if (inst->GetResult()) {
            Operand op = inst->GetResult();
            RegOperand* globalOp =  (RegOperand *)(op);
            int globalVarId = globalOp->GetRegNo();

            //std::cout << "Result register: " << globalVarId << std::endl;
        }

        liveBlock.insert(liveblockid);
        //std::cout << "Marking instruction " << inst << " as live" << std::endl;


        // 处理 phi 指令的前驱：对于每个 phi 指令，它的前驱块都应当标记为活跃的
        if (inst->GetOpcode() == BasicInstruction::PHI) {
            auto PhiI = dynamic_cast<PhiInstruction *>(inst);
             // 遍历 phi 指令的前驱基本块
            if(!PhiI->GetPhiList().empty()){ 
                for (auto [Labelreg, Regop] : PhiI->GetPhiList()) {
                    // 获取当前 phi 指令的前驱块
                    auto Label = (LabelOperand *)Labelreg;
                    auto Labelno = Label->GetLabelNo();
                    auto block = (*C->block_map)[Labelno];
                   // std::cout << "Processing PHI instruction, adding predecessor block " << Regop << " to workList" << std::endl;
                    //std::cout << "Processing PHI instruction, adding predecessor block " << Labelno << " to workList" << std::endl;
                    if (block != nullptr && !block->Instruction_list.empty()) {
                    // 获取基本块中最后一条指令
                        BasicInstruction* lastInstruction = block->Instruction_list.back();
                        if(live.find(lastInstruction)==live.end())  
                        {
                            workListid.insert({lastInstruction,Labelno});  
                            workList.push_back(lastInstruction);
                           
                            liveBlock.insert(Labelno);
                        }
                       
                    }
                }
            }
        }

        // 处理控制依赖图（CDG）：加入控制依赖前驱的终止指令
        if(!domTree->df[liveblockid].empty()){
            for (auto cdgPred : domTree->df[liveblockid]) {
                int blockid = cdgPred;
                if(blockid)
                {
                    auto block = (*C->block_map)[blockid];
                    if (block != nullptr && !block->Instruction_list.empty()) {
                        // 获取基本块中最后一条指令
                        BasicInstruction* lastInstruction = block->Instruction_list.back();
                        if(live.find(lastInstruction)==live.end())  
                        {
                           // std::cout<<"add last instruction in"<<blockid<<std::endl;
                            workListid.insert({lastInstruction,blockid});
                            workList.push_back(lastInstruction);
                           
                        }
                       
                    }
                }
            }
        }

        // 处理该指令的每个操作数的定义
        for (auto use : inst->GetRegOperands()) {
            if (use) {
                int regno = use->GetRegNo();
                // 使用 find 查找 regno 对应的定义集合
                auto it = defMap.find(regno);
                if (it == defMap.end()) {  // 找到对应的定义集合
                   continue;
                }
                 auto def = it->second;  // 获取定义集合
                //std::cout << "Processing operand " << use << " with regno " << regno << ", checking definition " << def << std::endl;
                // 遍历定义集合中的每一个定义
                //for (auto def : defSet) {
                    // 如果该定义不在 live 中，则加入工作列表
                    if (live.find(def) == live.end()) {
                        auto blocknewid = inst2blockid[def]; //获取指令的基本块
                        workListid.insert({def, blocknewid});  // 将未标记为活跃的定义加入工作列表
                        workList.push_back(def);
                       // std::cout << "push def " << def << " in block " << blocknewid << " to workList" << std::endl;
                        
                        //liveBlock.insert(blocknewid);  // 将当前基本块也标记为活跃
                    }
                //}
            }
        }
    }
    
     // 死代码消除：移除所有不活跃的指令
    for (auto [blockid, block] : *C->block_map) {
        //std::cout << std::endl;
       // std::cout << "blockid: "<< blockid << std::endl;
        auto tmp_Instruction_list = block->Instruction_list;
        block->Instruction_list.clear();  // 清空当前基本块的指令列表

        for (auto I : tmp_Instruction_list) {
            if (live.find(I) == live.end()) {
                // 如果指令不在活跃指令集合中，则删除它
                // 获取基本块中最后一条指令
                if (I->GetResult()) {
                    Operand op = I->GetResult();
                    RegOperand* globalOp =  (RegOperand *)(op);
                    int globalVarId = globalOp->GetRegNo();
                    //std::cout << "delete instruction "<< I << " with reg: "<< globalVarId << std::endl;
                }
                //std::cout << "delete instruction "<< I << " no resultreg"<< std::endl;

                //if(!block->Instruction_list.empty())
                //{
                    BasicInstruction* lastInstruction = tmp_Instruction_list.back();
                    if(I != nullptr && lastInstruction != nullptr && I == lastInstruction)
                    {   
                        int flag = 0;//没找到
                        //查找反向支配树的下一个
                        auto target = domTree->dom[blockid];
                        //std::cout << "target "<< target << std::endl;
                        if(liveBlock.find(target) != liveBlock.end())//找到
                        {
                          //  std::cout << "target "<< target << " is live" << std::endl;
                            flag = 1;
                        }
                        while(!flag)
                        {
                            target = domTree->dom[target];
                          //  std::cout << "target "<< target << std::endl;
                            if(liveBlock.find(target) != liveBlock.end()) //找到了
                            {
                                flag = 1;
                              //  std::cout << "target "<< target << " is live" << std::endl;
                            }
                        }
                        I = new BrUncondInstruction(GetNewLabelOperand(target));
                    }else
                    {
                        continue;
                    }
                    
                //}    
            }
            block->InsertInstruction(1, I);  // 否则，保留指令
            std::cout << std::endl;
            if (I->GetResult()) {
                    Operand op = I->GetResult();
                    RegOperand* globalOp =  (RegOperand *)(op);
                    int globalVarId = globalOp->GetRegNo();
                   // std::cout << "save instruction "<< I << " with reg: "<< globalVarId << std::endl;
            }
           // std::cout << "save instruction "<< I << " no resultreg"<< std::endl;
        }
    }

    defMap.clear();    // 清空defMap
    workList.clear();  // 清空workList
    workListid.clear();
    visitedBlocks.clear();  // 清空visitedBlocks

    live.clear();      // 清空所有活跃指令
    liveBlock.clear(); // 清空所有有活跃指令的基本块
    livereg.clear();   // 清空所有活跃寄存器

    // 如果 blockmap 和 parentblock 是动态分配的，确保正确处理内存
    // 如果它们是局部对象且没有动态内存分配，则不需要处理
    blockmap.clear();  // 
    parentblock.clear(); 

    inst2blockid.clear(); // 清空指令到基本块ID的映射

}