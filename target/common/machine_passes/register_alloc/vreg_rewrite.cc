#include "basic_register_allocation.h"

void VirtualRegisterRewrite::Execute() {
    for (auto func : unit->functions) {
        current_func = func;
        ExecuteInFunc();
    }
}

void VirtualRegisterRewrite::ExecuteInFunc() {
    auto func = current_func;
    //当前函数的机器控制流图（MachineCFG）中获取一个顺序扫描迭代器block_it，用于遍历基本块
    auto block_it = func->getMachineCFG()->getSeqScanIterator();
    block_it->open();
    //仍有未处理的基本块时，进入循环
    while (block_it->hasNext()) {
        //获取下一个基本块
        auto block = block_it->next()->Mblock;
        //遍历当前基本块中的所有机器指令
        for (auto it = block->begin(); it != block->end(); ++it) {
            auto ins = *it;
            // 根据alloc_result将ins的虚拟寄存器重写为物理寄存器
            //TODO("VirtualRegisterRewrite");
            // 历它读取的所有寄存器
            auto operands = ins->GetReadReg();
            for (auto operand : operands) {
                if (operand->is_virtual == false) { // 如果不是虚拟寄存器
                    Assert(alloc_result.find(func) == alloc_result.end() || alloc_result.find(func)->second.find(*operand) == alloc_result.find(func)->second.end());
                    
                    continue;                   
                    
                }
                //从分配结果中查找当前虚拟寄存器对应的分配信息result
                auto result = alloc_result.find(func)->second.find(*operand)->second;
                // if(!result.in_mem)//不在内存中
                // {
                //     operand->is_virtual = false;//标记为非虚拟
                //     operand->reg_no = result.phy_reg_no; //设置其物理寄存器编号reg_no为分配结果中的物理寄存器编号
                // }
                if (result.in_mem == true) {
                    ERROR("Shouldn't reach here");
                } else {
                    operand->is_virtual = false;
                    operand->reg_no = result.phy_reg_no;
                }
            }

            auto regs = ins->GetWriteReg();
            for(auto reg :regs)
            {
                if (reg->is_virtual == false) { // 如果不是虚拟寄存器
                    Assert(alloc_result.find(func) == alloc_result.end() ||alloc_result.find(func)->second.find(*reg) == alloc_result.find(func)->second.end());
                    
                    continue;                   
                    
                }

                //从分配结果中查找当前虚拟寄存器对应的分配信息result
                auto result = alloc_result.find(func)->second.find(*reg)->second;
                // if(!result.in_mem)//不在内存中
                // {
                //     reg->is_virtual = false;//标记为非虚拟
                //     reg->reg_no = result.phy_reg_no; //设置其物理寄存器编号reg_no为分配结果中的物理寄存器编号
                // }
                if (result.in_mem == true) {
                    ERROR("Shouldn't reach here");
                } else {
                    reg->is_virtual = false;
                    reg->reg_no = result.phy_reg_no;
                }

            }


        }
    }
}

void SpillCodeGen::ExecuteInFunc(MachineFunction *function, std::map<Register, AllocResult> *alloc_result) {
    this->function = function;
    this->alloc_result = alloc_result;
    auto block_it = function->getMachineCFG()->getSeqScanIterator();
    block_it->open();
    while (block_it->hasNext()) {
        cur_block = block_it->next()->Mblock;
        for (auto it = cur_block->begin(); it != cur_block->end(); ++it) {
            auto ins = *it;
            // 根据alloc_result对ins溢出的寄存器生成溢出代码
            // 在溢出虚拟寄存器的read前插入load，在write后插入store
            // 注意load的结果寄存器是虚拟寄存器, 因为我们接下来要重新分配直到不再溢出
            //TODO("SpillCodeGen");

            // 处理读取寄存器的情况
            auto operands = ins->GetReadReg();
            for (auto operand : operands) {
                if (!operand->is_virtual) {
                    continue; // 如果不是虚拟寄存器，跳过处理
                }

                auto result = alloc_result->find(*operand)->second;
                if (result.in_mem) { // 如果寄存器溢出，生成加载指令
                   *operand = GenerateReadCode(it, result.stack_offset*4, operand->type);
                }
            }

            // 处理写入寄存器的情况
            auto regs = ins->GetWriteReg();
            for (auto reg : regs) {
                if (!reg->is_virtual) {
                    continue; // 如果不是虚拟寄存器，跳过处理
                }

                auto result = alloc_result->find(*reg)->second;
                if (result.in_mem) { // 如果寄存器溢出，生成存储指令
                    *reg = GenerateWriteCode(it, result.stack_offset * 4, reg->type);
                }
            }

        }
    }
}


