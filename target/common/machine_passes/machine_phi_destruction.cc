#include "machine_phi_destruction.h"
extern bool optimize_flag;

void MachinePhiDestruction::Execute() {
    for (auto func : unit->functions) {
        current_func = func;
        PhiDestructionInCurrentFunction();
    }
}

void MachinePhiDestruction::PhiDestructionInCurrentFunction() { 
    //TODO("Implement this if you need"); 

    auto block_it = current_func->getMachineCFG()->getSeqScanIterator();
    block_it->open();

    while (block_it->hasNext()) {
        auto block = block_it->next()->Mblock;

        // 遍历当前基本块中的指令
        for (auto it = block->begin(); it != block->end(); ++it) {
            auto ins = *it;

            // 如果指令是 PHI 指令
            if (ins->arch == MachineBaseInstruction::PHI) {
                auto phi_Ins = static_cast<MachinePhiInstruction *>(ins);
                auto phi_resultreg = phi_Ins->GetResult();

                // 获取 PHI 指令的操作数列表
                auto phi_list = phi_Ins->GetPhiList();
                auto result_reg = current_func->GetNewReg(INT64);
                auto result_freg = current_func->GetNewReg(FLOAT64);
                
                // 在前驱基本块末尾添加 ADDIW 指令
                for (auto [phi_labelid, phi_operand] : phi_list) {
                    auto predecessor_block = current_func->getMachineCFG()->GetNodeByBlockId(phi_labelid)->Mblock;

                
                    auto insert_pos = predecessor_block->end();
                    std::advance(insert_pos, -1);
                    
                    if (phi_operand->op_type == MachineBaseOperand::REG)
                    {
                        auto* reg_operand = dynamic_cast<MachineRegister*>(phi_operand);
                        Register reg = reg_operand->reg;
                        if (reg.type.data_type == MachineDataType::INT){
                            auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, result_reg, reg, 0);

                            // 将 ADDIW 指令插入到前驱基本块的末尾
                            predecessor_block->insert(insert_pos, addiw_instr);
                        }
                        else if (reg.type.data_type == MachineDataType::FLOAT){
                            auto addiw_instr = rvconstructor->ConstructR(RISCV_FADD_S, result_freg, reg, GetPhysicalReg(RISCV_x0));
                            // 将 ADDIW 指令插入到前驱基本块的末尾
                            predecessor_block->insert(insert_pos, addiw_instr);

                        }

                    }else if (phi_operand->op_type == MachineBaseOperand::IMMI) {

                        auto* imm_operand = dynamic_cast<MachineImmediateInt*>(phi_operand);
                        int64_t immediate_value = imm_operand->imm32;
                        auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, result_reg, GetPhysicalReg(RISCV_x0), immediate_value);
                        predecessor_block->insert(insert_pos, addiw_instr);

                    }else if (phi_operand->op_type == MachineBaseOperand::IMMF) {

                        auto* imm_operand = dynamic_cast<MachineImmediateFloat*>(phi_operand);
                        auto immediate_value = imm_operand->fimm32;
                        //auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, result_reg, GetPhysicalReg(RISCV_x0), immediate_value);
                        //predecessor_block->insert(insert_pos, addiw_instr);

                        // 将浮点数转换为对应的比特模式（即整数形式）
                        uint64_t imm_as_uint = *reinterpret_cast<uint64_t*>(&immediate_value);

                        // 创建一个临时整数寄存器来存储浮点数的比特模式
                        Register temp_reg = cur_func->GetNewReg(INT64);

                        if (imm_as_uint >= -2048 && imm_as_uint <= 2047) {
                            // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW 指令
                            auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, temp_reg, GetPhysicalReg(RISCV_x0), static_cast<int32_t>(imm_as_uint));
                            cur_block->push_back(addiw_instr);
                        } else {
                            // 否则，分两步加载：高20位用LUI，低12位用ADDI
                            uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                            int32_t low_12_bits = imm_as_uint & 0xFFF;

                            // 使用LUI加载高20位
                            auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, temp_reg, high_20_bits);
                            cur_block->push_back(lui_instr);

                            // 使用ADDI加载低12位
                            auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, low_12_bits);
                            cur_block->push_back(addi_instr);
                        }

                        // 将整数寄存器内容解释为浮点数，并移动到浮点寄存器ConstructR2(RISCV_FMV_W_X, float_reg, temp_reg)
                        auto fmv_d_x_instr = rvconstructor->ConstructR2(RISCV_FMV_W_X, result_reg, temp_reg); // 假设有这样一个构造函数
                        predecessor_block->insert(insert_pos, fmv_d_x_instr);

                    }
                }

                if (phi_resultreg.type.data_type == MachineDataType::INT){
                    // 将 PHI 指令替换为 ADDIW 指令
                    auto addiw_instr_replace = rvconstructor->ConstructIImm(RISCV_ADDIW, phi_resultreg, result_reg, 0);

                    // 替换 PHI 指令
                    it = block->erase(it);
                    --it;
                    block->insert(block->begin(), addiw_instr_replace);
                }
                else if (phi_resultreg.type.data_type == MachineDataType::FLOAT){
                    // 将 PHI 指令替换为 ADDIW 指令
                    auto addiw_instr_replace = rvconstructor->ConstructR(RISCV_FADD_S, phi_resultreg, result_freg, GetPhysicalReg(RISCV_x0));
                    // 替换 PHI 指令
                    it = block->erase(it);
                    --it;
                    block->insert(block->begin(), addiw_instr_replace);

                }
                
            }
        }
    }
}