#include "mem2reg.h"
#include <tuple>
#include "../include/Instruction.h"

static std::set<Instruction> DeleteSet;
static std::map<int, int> UpdateReg;
static std::set<int> usual_alloc;//追踪alloca
static std::map<PhiInstruction *, int> phi_map;//记录指令由那个额变量插入
// 检查该条alloca指令是否可以被mem2reg
// eg. 数组不可以mem2reg
// eg. 如果该指针直接被使用不可以mem2reg(在SysY一般不可能发生,SysY不支持指针语法)
void Mem2RegPass::IsPromotable(CFG *C, Instruction AllocaInst) 
{ 
    //TODO("IsPromotable"); 
    // 确保传入的是 alloca 指令
    if (!AllocaInst->isAlloca()) {
        // 如果不是 alloca 指令，则不能进行 mem2reg 转换
        return;
    }
    // 检查是否是标量类型（非数组、非结构体）
    if (AllocaInst->GetType() != LLVMType::I32 && AllocaInst->GetType() != LLVMType::FLOAT32) {
        // 如果不是 I32 或 FLOAT32 类型，则不能进行 mem2reg 转换
        return;
    }
    // 获取所有使用该 alloca 指令的地方
    for (auto user : AllocaInst->uses()) {
        int userOpcode = user->GetOpcode();

        // 检查是否有取地址操作 (&)
        if (userOpcode == BasicInstruction::GETELEMENTPTR || userOpcode == BasicInstruction::LOAD) {
            // 如果有取地址操作或通过 load 使用，则不能进行 mem2reg 转换
            return;
        }
    }
}
/*
    int a1 = 5,a2 = 3,a3 = 11,b = 4
    return b // a1,a2,a3 is useless
-----------------------------------------------
pseudo IR is:
    %r0 = alloca i32 ;a1
    %r1 = alloca i32 ;a2
    %r2 = alloca i32 ;a3
    %r3 = alloca i32 ;b
    store 5 -> %r0 ;a1 = 5
    store 3 -> %r1 ;a2 = 3
    store 11 -> %r2 ;a3 = 11
    store 4 -> %r3 ;b = 4
    %r4 = load i32 %r3
    ret i32 %r4
--------------------------------------------------
%r0,%r1,%r2只有store, 但没有load,所以可以删去
优化后的IR(pseudo)为:
    %r3 = alloca i32
    store 4 -> %r3
    %r4 - load i32 %r3
    ret i32 %r4
*/

// vset is the set of alloca regno that only store but not load
// 该函数对你的时间复杂度有一定要求, 你需要保证你的时间复杂度小于等于O(nlognlogn), n为该函数的指令数
// 提示:deque直接在中间删除是O(n)的, 可以先标记要删除的指令, 最后想一个快速的方法统一删除
void Mem2RegPass::Mem2RegNoUseAlloca(CFG *C, std::set<int> &vset) {
    // this function is used in InsertPhi
    //TODO("Mem2RegNoUseAlloca");

    // 用于记录 alloca 指令到寄存器编号的映射
    std::unordered_map<Instruction, int> allocaToReg;
    // 用于记录哪些 alloca 有 store 操作
    std::unordered_set<int> hasStore;
    // 用于记录哪些 alloca 有 load 操作
    std::unordered_set<int> hasLoad;

    // 第一次遍历：记录所有 alloca 和 store 操作
    for (auto [blockid, block] : *C->block_map) {
        //BasicBlock *block = block_ptr.get();
        for (auto I : block->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::STORE) {
                // 检查 store 指令的目标地址是否是一个 alloca 指令
                auto StoreI = dynamic_cast<StoreInstruction*>(I);
                if (StoreI->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    Operand op = StoreI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();
                    allocaToReg[I] = regno;
                    hasStore.insert(regno);  // 记录有 store 操作的寄存器编号
                }
            }
        }
    }

    // 第二次遍历：检查 load 操作
    for (auto [blockid, block] : *C->block_map) {
        //BasicBlock *block = block_ptr.get();
        for (auto I : block->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::LOAD) {
                // 检查 load 指令的源是否是一个 alloca 指令
                auto LoadI = dynamic_cast<LoadInstruction*>(I);
                if (LoadI->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    Operand op = LoadI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();
                    allocaToReg[I] = regno;
                    hasLoad.insert(regno);  // 记录有 store 操作的寄存器编号
                }
            }
        }
    }

    // 将只 store 但没有 load 的 alloca 寄存器编号添加到 vset，并标记要删除的 alloca 指令
    for (auto entry : allocaToReg) {
        Instruction inst = entry.first;
        int regno = entry.second;
        if (hasStore.find(regno) != hasStore.end() && hasLoad.find(regno) == hasLoad.end()) {
            if (vset.find(regno) != vset.end())  // 插入寄存器编号
            {
                DeleteSet.insert(inst);  // 标记要删除的 store 指令
            }
        }
    }
}

/*
    int b = getint();
    b = b + 10
    return b // def and use of b are in same block
-----------------------------------------------
pseudo IR is:
    %r0 = alloca i32 ;b
    %r1 = call getint()
    store %r1 -> %r0
    %r2 = load i32 %r0
    %r3 = %r2 + 10
    store %r3 -> %r0
    %r4 = load i32 %r0
    ret i32 %r4
--------------------------------------------------
%r0的所有load和store都在同一个基本块内
优化后的IR(pseudo)为:
    %r1 = call getint()
    %r3 = %r1 + 10
    ret %r3

对于每一个load，我们只需要找到最近的store,然后用store的值替换之后load的结果即可
*/

// 
// 该函数对你的时间复杂度有一定要求，你需要保证你的时间复杂度小于等于O(nlognlogn), n为该函数的指令数
void Mem2RegPass::Mem2RegUseDefInSameBlock(CFG *C, std::set<int> &vset, int block_id) {
    // this function is used in InsertPhi
    //TODO("Mem2RegUseDefInSameBlock");
    // 获取基本块对象
    BasicBlock *bb = (*C->block_map)[block_id]; 

    // 存储每个 alloca 操作符的映射，alloca 操作符 -> 寄存器编号
    std::map<Operand, Operand> alloca_to_val_map;

     // 存储所有已经被 store 的值，映射为寄存器
    std::map<Operand, int> store_to_reg_map;

    Operand alloca_operand ;

      // 遍历基本块中的指令
    for (auto it = bb->Instruction_list.begin(); it != bb->Instruction_list.end();) {
        Instruction instr = *it;
        // 如果当前指令是 alloc 指令，标记并继续处理
        /*
        if (instr->GetOpcode() == BasicInstruction::LLVMIROpcode::ALLOCA) {
            alloca_operand = dynamic_cast<AllocaInstruction*>(instr)->GetResult();
            // 初始化 val 为 undef
            alloca_to_val_map[alloca_operand] = nullptr;  
        }
        // 处理 store 指令
        else */
        
        if (instr->GetOpcode() == BasicInstruction::LLVMIROpcode::STORE) {
           // 如果是 store 指令，检查是否涉及到当前的 alloca
            auto store = dynamic_cast<StoreInstruction*>(instr);
            //auto StoreI = dynamic_cast<StoreInstruction*>(I);
           
            if (store) {
                Operand alloca_operand = store->GetPointer();
                Operand store_value = store->GetValue();

                    int regno ;
                if (store->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    Operand op = store->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    regno = regOp->GetRegNo();
                    
                    // 获取新旧寄存器映射
                    int old_reg_no = regno ;

                    if(vset.find(old_reg_no) != vset.end())
                    {

                        // 将 store 指令的值映射到一个新的寄存器
                        
                        //int new_reg_no = ((RegOperand *)(store->GetValue()))->GetRegNo();
                        store_to_reg_map[alloca_operand] = ((RegOperand *)(store->GetValue()))->GetRegNo();
                        // 删除 store 指令，因为它已经被替换为寄存器
                        //it = bb->Instruction_list.erase(it);
                        DeleteSet.insert(*it);
                        //it = bb->Instruction_list.erase(it);
                        ++it;
                        continue;
                    }
                }
                
            }
            
        }
        
        else if (instr->GetOpcode() == BasicInstruction::LLVMIROpcode::LOAD) {
            // 如果是 load 指令，检查是否涉及到当前的 alloca
            auto load = dynamic_cast<LoadInstruction*>(instr);
            if (load) {
                Operand load_pointer = load->GetPointer();

                // 如果 store_to_reg_map 中包含了 load 指令所访问的 alloca 操作符
                if (store_to_reg_map.find(load_pointer) != store_to_reg_map.end()) {
                    int new_reg = store_to_reg_map[load_pointer];
                    Operand result = load->GetResult();

                    // 获取新旧寄存器映射
                    int old_reg_no ;
                    if (load->GetPointer()->GetOperandType() == BasicOperand::REG) {
                        Operand op = load->GetPointer();
                        RegOperand* regOp = (RegOperand *)(op);
                        old_reg_no = regOp->GetRegNo();
                    

                        if(vset.find(old_reg_no) != vset.end())
                        {

                            int old_rereg_no = load->GetResultRegNo();
                            
                            
                            //int new_reg_no = ((RegOperand *)(store_to_reg_map[load_pointer]))->GetRegNo();

                            // 将新的寄存器号和旧的寄存器号存入 static std::map<int, int> UpdateReg
                            UpdateReg[old_rereg_no] = new_reg;

                            // 删除 load 指令，因为它已经被替换为寄存器
                            //it = bb->Instruction_list.erase(it);
                            DeleteSet.insert(*it);
                            //it = bb->Instruction_list.erase(it);
                            ++it;
                            continue;
                        }
                    }
                }
                
            }

        }

        // 移动到下一条指令
        ++it;
    }

}

// vset is the set of alloca regno that one store dominators all load instructions
// 该函数对你的时间复杂度有一定要求，你需要保证你的时间复杂度小于等于O(nlognlogn)
void Mem2RegPass::Mem2RegOneDefDomAllUses(CFG *C, std::set<int> &vset) {
    // this function is used in InsertPhi
    //TODO("Mem2RegOneDefDomAllUses");

    // 用于记录每个 alloca 变量的 store 指令及其对应的寄存器编号
    std::unordered_map<Operand, int> storeMap;
    // 用于记录每个 alloca 变量的所有 load 指令
    std::unordered_map<int, std::vector<LoadInstruction*>> loadMap;
   
    //遍历：记录所有 store 和 load 指令
    for (auto [blockid, block] : *C->block_map) {
        for (auto I : block->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::STORE) {
                auto StoreI = dynamic_cast<StoreInstruction*>(I);
                if (StoreI->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    Operand op = StoreI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();
                    
                    // Operand opval = StoreI->GetValue();
                    // RegOperand* regOpval = (RegOperand *)(opval);
                    // int regnoval = regOpval->GetRegNo();

                    if (vset.find(regno) != vset.end()) {
                        storeMap[op] = ((RegOperand *)(StoreI->GetValue()))->GetRegNo();
                        DeleteSet.insert(I);
                    }
                }
                //DeleteSet.insert(I);
            } 
        }
    }
    for (auto [blockid, block] : *C->block_map) {
            for (auto I : block->Instruction_list) {
                if (I->GetOpcode() == BasicInstruction::LOAD) {
                auto LoadI = dynamic_cast<LoadInstruction*>(I);
                if (LoadI->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    Operand op = LoadI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();

                    // Operand opresult = LoadI->GetResult();
                    // RegOperand* regOpresult = (RegOperand *)(opresult);
                    // int regnoresult = regOpresult->GetRegNo();

                    int regnoresult = LoadI->GetResultRegNo();

                    if (vset.find(regno) != vset.end()) {
                        UpdateReg[regnoresult] = storeMap[op];
                        DeleteSet.insert(I);
                    }
                }
                //DeleteSet.insert(I);
            }  
        }
    }
}

void Mem2RegPass::InsertPhi(CFG *C) { 
    //TODO("InsertPhi"); 

    //记录每个alloca的store和load寄存器编号及基本块id，记录其被store次数
    std::map<int, std::set<int>> storemap;
    std::map<int, int> storenum;
    std::map<int, std::set<int>> loadmap;
    for(auto[blockid,block]:*C->block_map)
    {
        for(auto I : block->Instruction_list)
        {
            if (I->GetOpcode() == BasicInstruction::STORE)
            {
                auto StoreI = dynamic_cast<StoreInstruction*>(I);
                if (StoreI->GetPointer()->GetOperandType() != BasicOperand::GLOBAL)
                {
                    Operand op = StoreI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();
                    storemap[regno].insert(blockid);
                    storenum[regno]++;
                }
            }
            else if (I->GetOpcode() == BasicInstruction::LOAD)
            {
                auto LoadI = dynamic_cast<LoadInstruction*>(I);
                if (LoadI->GetPointer()->GetOperandType() != BasicOperand::GLOBAL)
                {
                    Operand op = LoadI->GetPointer();
                    RegOperand* regOp = (RegOperand *)(op);
                    int regno = regOp->GetRegNo();
                    loadmap[regno].insert(blockid);
                }
            }
        }
    }

    std::set<int> noload_vset;//只store没load过的集合
    std::map<int, std::set<int>> sameblock_vset;//store和load在同一个block
    std::set<int> onestore_vset;//store对应所有load

    //从开始块遍历
    for (auto I : (*C->block_map)[0]->Instruction_list)
    {
        if (I->GetOpcode() == BasicInstruction::ALLOCA)
        {
            auto AllocaI = dynamic_cast<AllocaInstruction*>(I);
            //数组不可以进行此操作
            if (AllocaI->GetDims().empty())
            {
                Operand allocaop = AllocaI->GetResult();
                RegOperand* allocaregOp = (RegOperand *)(allocaop);
                int allocaregno = allocaregOp->GetRegNo();
                //寻找该alloca被store和load的位置
                auto alloca_store = storemap[allocaregno];
                auto alloca_load = loadmap[allocaregno];
                int store_num = storenum[allocaregno];
                if (alloca_load.size() == 0)//没被load过
                {
                    DeleteSet.insert(I);//删除对应的alloca语句
                    noload_vset.insert(allocaregno);//加到set中
                    continue;
                }
                if (store_num == 1)//只store了一次
                {
                   // 判断是否在其他基本块没有被load过
                    bool has_load_in_other_block = false;
                    for (auto blockid : alloca_load) {
                        if (blockid != *alloca_store.begin()) {  // 只要在其他块load过
                            has_load_in_other_block = true;
                            break;
                        }
                    }

                    if (!has_load_in_other_block) {
                        DeleteSet.insert(I);  // 删除对应的alloca语句
                        onestore_vset.insert(allocaregno);  // 加到set中
                        continue;
                    }
                }
                if (alloca_store.size() == 1)
                {
                    int blockid = *(alloca_store.begin());
                    if (alloca_load.size() == 1 && *(alloca_load.begin()) == blockid)
                    {
                        DeleteSet.insert(I);//删除对应的alloca语句
                        sameblock_vset[blockid].insert(allocaregno);//加到set中
                        continue;
                    }
                }

                usual_alloc.insert(allocaregno);//跟踪当前正在处理的alloca变量
                DeleteSet.insert(I);
                std::set<int> phi_insert{};//已插入phi指令的基本块,记录已经处理过的基本快
                std::set<int> def_block = storemap[allocaregno];//包含变量的基本块

                while (!def_block.empty())
                {
                    int BB_X = *def_block.begin();
                    DominatorTree *domTree = domtrees->GetDomTree(C);
                    def_block.erase(BB_X);
                    // 获取该基本块的后继基本块，控制流合并点
                    for (auto BB_Y : domTree->GetDF(BB_X)) 
                    {
                         // 打印当前基本块和后继块的ID
                        //std::cout << "Prblock_map[max_label + 1] = mergeBlock;ocessing BB_X: " << BB_X << ", BB_Y: " << BB_Y << std::endl;
                          
                        
                        // 如果在该块尚未插入phi指令，则插入
                        if (phi_insert.find(BB_Y) == phi_insert.end())
                        {
                            //打印插入phi指令的块ID
                            //std::cout << "Inserting phi instruction into BB_Y: " << BB_Y << std::endl;

                            // 检查 block_map 中是否包含 BB_Y
                            if (C->block_map->find(BB_Y) == C->block_map->end()) {
                                //std::cerr << "Error: BB_Y: " << BB_Y << " not found in block_map." << std::endl;
                                return;  // 或者抛出异常
                            }
                            
                            PhiInstruction *phi = new PhiInstruction(AllocaI->GetDataType(), GetNewRegOperand(++C->max_reg));
                            
                            (*C->block_map)[BB_Y]->InsertInstruction(0, phi); // 将phi指令插入基本块
                            phi_map[phi]=allocaregno;
                            phi_insert.insert(BB_Y);//避免重复插入
                            //std::cout << "Inserted phi instruction for BB_Y: " << BB_Y << std::endl;

                            // 如果BB_Y没有定义allocatedregno，则不插入phi指令
                            if (storemap[allocaregno].find(BB_Y) == storemap[allocaregno].end()) {
                                //std::cout << "BB_Y: " << BB_Y << " does not store allocatedregno, skipping phi insertion." << std::endl;
                                 // 如果BB_Y没有对alloca变量进行store操作,等待后续的遍历
                                 def_block.insert(BB_Y);
                            
                                
                            }
                            
                        }else{
                            //如果phi已经插入过，打印该块信息
                            //std::cout << "Phi instruction already inserted for BB_Y: " << BB_Y << std::endl;
                        }
                    }
                }
            }
        }
    }
    Mem2RegNoUseAlloca(C, noload_vset);
    Mem2RegOneDefDomAllUses(C, onestore_vset);
    for (auto [blockid, vset] : sameblock_vset) {
        Mem2RegUseDefInSameBlock(C, vset, blockid);
    }
}

void Mem2RegPass::VarRename(CFG *C) //{ TODO("VarRename"); }
{
    std::map<int, std::map<int, int>> blockWorkList;    //< BB, <alloca_reg, val_reg> >
    blockWorkList.insert({0, std::map<int, int>{}});     // 初始块的工作列表为空映射
    std::vector<int> blockVisitedFlags(C->max_label + 1, 0);    // 标记每个基本块是否已经遍历过

    while (!blockWorkList.empty()) {
        int currentBlockId = (*blockWorkList.begin()).first;               // 当前处理的基本块
        auto incomingValues = (*blockWorkList.begin()).second;   // 当前基本块的变量映射
        blockWorkList.erase(currentBlockId);                               // 移除已处理的基本块

        if (blockVisitedFlags[currentBlockId]) {
            continue;  // 如果该基本块已经处理过，跳过
        }
        blockVisitedFlags[currentBlockId] = 1;   // 标记该基本块已经遍历

        // 遍历基本块中的所有指令
        for (auto &I : (*C->block_map)[currentBlockId]->Instruction_list) {
            // 处理 LOAD 指令
            if (I->GetOpcode() == BasicInstruction::LOAD) {
                auto *LoadI = dynamic_cast<LoadInstruction *>(I);
                if (LoadI) {
                    int regno = -1;
                    // 如果指针操作数不是寄存器类型，则跳过
                    if (LoadI->GetPointer()->GetOperandType() != BasicOperand::REG) {
                        regno = -1;
                    } else {
                        // 获取指针寄存器的编号
                        Operand op = LoadI->GetPointer();
                        RegOperand* regOp = (RegOperand *)(op);
                        int pointer = regOp->GetRegNo();
                        // 检查指针是否在 usual_alloc 中
                        if (usual_alloc.find(pointer) != usual_alloc.end()) {
                            regno = pointer;
                        }
                    }

                    if (regno >= 0) {
                        // 如果当前是 LOAD 指令，替换所有使用该 LOAD 结果的地方
                        DeleteSet.insert(LoadI);  // 删除该 LOAD 指令
                        UpdateReg[LoadI->GetResultRegNo()] = incomingValues[regno];
                    }
                }
            }

            // 处理 STORE 指令
            if (I->GetOpcode() == BasicInstruction::STORE) {
                auto *StoreI = dynamic_cast<StoreInstruction *>(I);
                if (StoreI) {
                    int regno = -1;
                    // 如果指针操作数不是寄存器类型，则跳过
                    if (StoreI->GetPointer()->GetOperandType() != BasicOperand::REG) {
                        regno = -1;
                    } else {
                        Operand op = StoreI->GetPointer();
                        RegOperand* regOp = (RegOperand *)(op);
                        int pointer = regOp->GetRegNo();
                        // 检查指针是否在 usual_alloc 中
                        if (usual_alloc.find(pointer) != usual_alloc.end()) {
                            regno = pointer;
                        }
                    }

                    if (regno >= 0) {
                        // 如果当前是 STORE 指令，更新变量的最新值
                        DeleteSet.insert(StoreI);  // 删除该 STORE 指令
                        incomingValues[regno] = dynamic_cast<RegOperand *>(StoreI->GetValue())->GetRegNo();
                    }
                }
            }

            // 处理 PHI 指令
            if (I->GetOpcode() == BasicInstruction::PHI) {
                auto *PhiI = dynamic_cast<PhiInstruction *>(I);
                if (PhiI) {
                    // 如果当前 PHI 指令已经在 DeleteSet 中，跳过
                    if (DeleteSet.find(PhiI) != DeleteSet.end()) {
                        continue;
                    }

                    auto it = phi_map.find(PhiI);
                    if (it != phi_map.end()) {   // 如果 PHI 对应的 alloca 存在
                        // 更新 incomingValues 映射，确保每个 PHI 使用最新的值
                        Operand op = PhiI->GetResult();
                        RegOperand* regOp = (RegOperand *)(op);
                        int regno = regOp->GetRegNo();
                        incomingValues[it->second] = regno;
                    }
                }
            }
        }

        // 将当前基本块的后继基本块加入工作列表
        for (auto succ : C->G[currentBlockId]) {
            int successorBlockId = succ->block_id;
            // 检查 successorBlockId 是否有效
            if (C->block_map->find(successorBlockId) == C->block_map->end()) {
                std::cerr << "错误：找不到 successorBlockId " << successorBlockId << "！" << std::endl;
                continue;
            }
            blockWorkList.insert({successorBlockId, incomingValues});   // 更新后继块的变量映射

            // 遍历后继基本块中的 PHI 指令，更新变量映射
            for (auto I : (*C->block_map)[successorBlockId]->Instruction_list) {
                if (I->GetOpcode() != BasicInstruction::PHI) {
                    break;   // PHI 指令是块的入口，所以遇到非 PHI 指令直接跳出
                }
                auto *PhiI = dynamic_cast<PhiInstruction *>(I);
                // 找到 PHI 指令对应的 alloca
                auto it = phi_map.find(PhiI);
                if (it != phi_map.end()) {
                    int v = it->second;
                    if (incomingValues.find(v) == incomingValues.end()) {
                        DeleteSet.insert(PhiI);   // 如果没有找到对应值，删除 PHI
                        continue;
                    }
                    // 向 PHI 指令中插入来自前驱块的映射值
                    PhiI->InsertPhi(GetNewRegOperand(incomingValues[v]), GetNewLabelOperand(currentBlockId));
                }
            }
        }
    }


    //  handle UpdateReg and DeleteSet
    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            if (I->GetOpcode() == BasicInstruction::LOAD) {
                auto load = dynamic_cast<LoadInstruction *>(I); // Safe type check
                if (load && load->GetPointer()->GetOperandType() == BasicOperand::REG) {
                    int result = load->GetResultRegNo();
                    if (UpdateReg.find(result) != UpdateReg.end()) {
                        int new_result = UpdateReg[result];
                        // Update the mapping chain
                        while (UpdateReg.find(new_result) != UpdateReg.end()) {
                            new_result = UpdateReg[new_result];
                        }
                        UpdateReg[result] = new_result; // Chain update
                    }
                }
            }
        }
    }

    // clean up instructions based on DeleteSet
    for (auto [id, bb] : *C->block_map) {
        auto tmp_Instruction_list = bb->Instruction_list;
        bb->Instruction_list.clear();
        for (auto I : tmp_Instruction_list) {
            if (DeleteSet.find(I) == DeleteSet.end()) {
                bb->InsertInstruction(1, I);
            }
        }
    }

    // replace registers based on UpdateReg
    for (auto [id, bb] : *C->block_map) {
        for (auto I : bb->Instruction_list) {
            I->Replace_RegMap(UpdateReg);
        }
    }

    // Clear sets after the process
    DeleteSet.clear();
    UpdateReg.clear();
    usual_alloc.clear();
    phi_map.clear();



    
    /*
    // 映射从块ID到变量寄存器到其最新值的映射
    std::unordered_map<int, std::unordered_map<int, int>> workList;
    std::vector<bool> visitedBlocks(C->block_map., false);
    std::set<BasicInstruction*> eraseSet;
    std::unordered_map<int, int> mem2regMap;  // 寄存器映射，用于替换
 
    // 初始化工作列表，从入口块开始
    workList[0] = {};
 
    // 主迭代循环
    while (!workList.empty()) {
        auto it = workList.begin();
        int blockId = it->first;
        auto &incomingVals = it->second;
        workList.erase(it);
 
        if (visitedBlocks[blockId]) {
            continue;
        }
        visitedBlocks[blockId] = true;
 
        // 处理当前块中的指令
        for (auto &instruction : (*C->block_map)[blockId]->Instruction_list) {
            if (instruction->GetOpcode() == BasicInstruction::STORE) {
                auto storeI = dynamic_cast<StoreInstruction*>(instruction);
                Operand op = storeI->GetPointer();
                if (op->GetOperandType() != BasicOperand::GLOBAL) {
                    RegOperand* regOp = dynamic_cast<RegOperand*>(op);
                    if (regOp) {
                        int regno = regOp->GetRegNo();
                        // 更新存储的目标寄存器的值
                        incomingVals[regno] = ((RegOperand*)(storeI->GetValue()))->GetRegNo();
                        // 将此存储指令标记为待删除
                        eraseSet.insert(storeI);
                    }
                }
            } else if (instruction->GetOpcode() == BasicInstruction::LOAD) {
                auto loadI = dynamic_cast<LoadInstruction*>(instruction);
                Operand op = loadI->GetPointer();
                if (op->GetOperandType() != BasicOperand::GLOBAL) {
                    RegOperand* regOp = dynamic_cast<RegOperand*>(op);
                    if (regOp) {
                        int regno = regOp->GetRegNo();
                        // 更新加载的寄存器为其最新值
                        if (incomingVals.find(regno) != incomingVals.end()) {
                            mem2regMap[loadI->GetResultRegNo()] = incomingVals[regno];
                            // 将此加载指令标记为待删除（或替换）
                            // 注意：这里我们暂时不直接删除，而是用映射替换
                            // eraseSet.insert(loadI); // 如果需要直接删除，可以取消注释这行代码
                        }
                    }
                }
            }
        }
 
        // 更新后继块的工作列表
        for (auto successor : C->G[blockId]) {
            int successorId = successor->block_id;
            workList[successorId].insert(incomingVals.begin(), incomingVals.end());
        }
    }
 
    // 遍历所有块，替换寄存器并删除不必要的指令
    for (auto &[blockId, block] : *C->block_map) {
        auto tmpInstructionList = block->Instruction_list;
        block->Instruction_list.clear();
        for (auto &instruction : tmpInstructionList) {
            if (eraseSet.find(instruction) == eraseSet.end()) {
                // 替换寄存器
                instruction->ReplaceRegByMap(mem2regMap);
                block->InsertInstruction(1, instruction); // 假设InsertInstruction(1, ...)表示在适当位置插入指令
            }
        }
    }
 
    // 清理
    eraseSet.clear();
    mem2regMap.clear();

    */
}

void Mem2RegPass::Mem2Reg(CFG *C) {
    InsertPhi(C);
    VarRename(C);
}

void Mem2RegPass::Execute() {
    for (auto [defI, cfg] : llvmIR->llvm_cfg) {
        std::cout << "Mem2RegPass begin" << std::endl;
        Mem2Reg(cfg);
    }
}