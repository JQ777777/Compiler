#include "riscv64_lowerframe.h"

extern bool optimize_flag;

/*
    假设IR中的函数定义为f(i32 %r0, i32 %r1)
    则parameters应该存放两个虚拟寄存器%0,%1

    在LowerFrame后应当为
    add %0,a0,x0  (%0 <- a0)
    add %1,a1,x0  (%1 <- a1)

    对于浮点寄存器按照类似的方法处理即可
*/
void RiscV64LowerFrame::Execute() {
    // 在每个函数的开头处插入获取参数的指令
    for (auto func : unit->functions) {
        current_func = func;
        for (auto &b : func->blocks) {
            if (b->getLabelId() == 0) {    // 函数入口，需要插入获取参数的指令
                int i32_cnt = 0;
                int f32_cnt = 0;
                int offset = 0;
                for (auto para : func->GetParameters()) {    // 你需要在指令选择阶段正确设置parameters的值
                    if (para.type.data_type == INT64.data_type) {
                        if (i32_cnt < 8) {    // 插入使用寄存器传参的指令
                            b->push_front(rvconstructor->ConstructR(RISCV_ADD, para, GetPhysicalReg(RISCV_a0 + i32_cnt),
                                                                    GetPhysicalReg(RISCV_x0)));
                        }
                        if (i32_cnt >= 8) {    // 插入使用内存传参的指令
                            //TODO("Implement this if you need");
                            b->push_front(rvconstructor->ConstructIImm(RISCV_LD, para, GetPhysicalReg(RISCV_fp), offset));
                            offset += 8;
                        }
                        i32_cnt++;
                    } else if (para.type.data_type == FLOAT64.data_type) {    // 处理浮点数
                        //TODO("Implement this if you need");

                        if (f32_cnt < 8) {    // 插入使用寄存器传参的指令
                            b->push_front(rvconstructor->ConstructR(RISCV_FADD_S, para, GetPhysicalReg(RISCV_fa0 + f32_cnt),
                                                                    GetPhysicalReg(RISCV_x0)));
                        }
                        if (f32_cnt >= 8) {    // 插入使用内存传参的指令
                            b->push_front(rvconstructor->ConstructIImm(RISCV_FLD, para, GetPhysicalReg(RISCV_fp), offset));
                            offset += 8;
                        }
                        f32_cnt++;
                    } else {
                        ERROR("Unknown type");
                    }
                }
                Register para_reg = current_func->GetNewReg(INT64);
                if (offset != 0) {
                    current_func->SetHasInParaInStack(true);
                    cur_block->push_front(rvconstructor->ConstructR(RISCV_ADD, para_reg, GetPhysicalReg(RISCV_fp),  GetPhysicalReg(RISCV_x0)));
                }
            }
        }
    }
}

void RiscV64LowerStack::Execute() {
    // 在函数在寄存器分配后执行
    // TODO: 在函数开头保存 函数被调者需要保存的寄存器，并开辟栈空间
    // TODO: 在函数结尾恢复 函数被调者需要保存的寄存器，并收回栈空间
    // TODO: 开辟和回收栈空间
    // 具体需要保存/恢复哪些可以查阅RISC-V函数调用约定
    Log("RiscV64LowerStack");

    // 遍历所有函数
    for (auto func : unit->functions) {
        current_func = func;

        std::vector<std::vector<int>> saveregs_occurblockids(64), saveregs_rwblockids(64);
        bool need_save_ra = false;

        std::vector<int> store_offset;
        store_offset.resize(64);

        // 初始化需要保存的寄存器列表
        std::unordered_set<int> regs_to_save = {
            RISCV_s0, RISCV_s1, RISCV_s2, RISCV_s3, RISCV_s4, RISCV_s5, RISCV_s6,
            RISCV_s7, RISCV_s8, RISCV_s9, RISCV_s10, RISCV_s11, RISCV_fs0, RISCV_fs1,
            RISCV_fs2, RISCV_fs3, RISCV_fs4, RISCV_fs5, RISCV_fs6, RISCV_fs7, RISCV_fs8,
            RISCV_fs9, RISCV_fs10, RISCV_fs11, RISCV_ra
        };

        // 遍历函数中的每个基本块，确定哪些寄存器需要保存
        for (auto &b : func->blocks) {
            for (auto ins : *b) {
                for (auto reg : ins->GetWriteReg()) {
                    if (!reg->is_virtual && regs_to_save.count(reg->reg_no)) {
                        saveregs_occurblockids[reg->reg_no].push_back(b->getLabelId());
                        saveregs_rwblockids[reg->reg_no].push_back(b->getLabelId());
                        if (reg->reg_no == RISCV_ra) {
                            need_save_ra = true;
                        }
                    }
                }
                for (auto reg : ins->GetReadReg()) {
                    if (!reg->is_virtual && regs_to_save.count(reg->reg_no)) {
                        saveregs_rwblockids[reg->reg_no].push_back(b->getLabelId());
                        if (reg->reg_no == RISCV_ra) {
                            need_save_ra = true;
                        }
                    }
                }
            }
        }

        // 如果函数有参数在栈上，则也需要保存 fp 寄存器
        if (func->HasInParaInStack()) {
            saveregs_occurblockids[RISCV_fp].push_back(0);
            saveregs_rwblockids[RISCV_fp].push_back(0);
            regs_to_save.insert(RISCV_fp);
        }

        // 计算需要保存的寄存器数量并调整栈大小
        int num = 0;
        for (int i = 0; i < saveregs_occurblockids.size(); ++i) {
            if (!saveregs_occurblockids[i].empty()) num++;
        }
        func->AddStackSize(num * 8); // 每个寄存器占用8字节

        // 遍历每个基本块
        for (auto &block : func->blocks) {
            if (block->getLabelId() == 0) { // 函数入口块
                // 调整栈指针以开辟栈空间
                int stack_size = func->GetStackSize();
                if (stack_size >= -2048 && stack_size <= 2047) {
                    block->insert(block->begin(), rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_sp), -stack_size));
                } else {
                        // 否则，使用 LUI 和 ADDI 组合加载
                        uint64_t imm_as_uint = static_cast<uint64_t>(stack_size);

                         // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                        int lui_val = (imm_as_uint >> 12) & 0xFFFFF; // 提取高 20 位
                        int addi_val = imm_as_uint & 0xFFF;          // 提取低 12 位

                        // 使用临时寄存器
                        Register temp_reg = GetPhysicalReg(RISCV_a2);

                        // 使用 LUI 指令加载高 20 位
                        auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_sp), -lui_val);
                        //block->push_front(lui_instr);

                        // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                        if (addi_val < -2048 || addi_val > 2047) {
                            // 如果 ADDI 的立即数部分超出范围，进一步处理
                            // 分解成两个 ADDI 操作
                            int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                            int addi_part2 = addi_val - addi_part1;         // 剩余部分

                            // 使用 ADDI 指令添加第一部分立即数
                            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, -addi_part1);
                            //block->push_front(addi_instr1);

                            // 使用 ADDI 指令添加第二部分立即数
                            auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), temp_reg, -addi_part2);
                            block->push_front(addi_instr2);
                            block->push_front(addi_instr1);
                        }
                        block->push_front(lui_instr);
                }
                if (func->HasInParaInStack()) {
                    block->push_front(rvconstructor->ConstructR(RISCV_ADD, GetPhysicalReg(RISCV_fp), GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_x0)));
                }

                    int offset = 0;
                    for (const auto& id : regs_to_save) {
                        if (!saveregs_occurblockids[id].empty()) {
                            offset -= 8;
                            if (id >= RISCV_x0 && id <= RISCV_x31) {
                                block->insert(block->begin(), rvconstructor->ConstructSImm(RISCV_SD, GetPhysicalReg(id), GetPhysicalReg(RISCV_sp), offset));
                            } else {
                                block->insert(block->begin(), rvconstructor->ConstructSImm(RISCV_FSD, GetPhysicalReg(id), GetPhysicalReg(RISCV_sp), offset));
                            }
                        }
                    }
            }

            // 在返回指令前插入恢复栈指针的指令
            auto last_ins = *(block->ReverseBegin());
            bool is_return = last_ins->arch == MachineBaseInstruction::RiscV &&
                             static_cast<RiscV64Instruction*>(last_ins)->getOpcode() == RISCV_JALR &&
                             static_cast<RiscV64Instruction*>(last_ins)->getRd() == GetPhysicalReg(RISCV_x0) &&
                             static_cast<RiscV64Instruction*>(last_ins)->getRs1() == GetPhysicalReg(RISCV_ra) &&
                             static_cast<RiscV64Instruction*>(last_ins)->getImm() == 0;
            if (is_return) {
                block->pop_back();

                int stack_size = func->GetStackSize();
                if (stack_size >= -2048 && stack_size <= 2047){
                    block->push_back(rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), GetPhysicalReg(RISCV_sp), stack_size));
                } else {
                    auto stacksz_reg = GetPhysicalReg(RISCV_t0);

                        // 否则，使用 LUI 和 ADDI 组合加载
                        uint64_t imm_as_uint = static_cast<uint64_t>(stack_size);

                         // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                        int lui_val = (imm_as_uint >> 12) & 0xFFFFF; // 提取高 20 位
                        int addi_val = imm_as_uint & 0xFFF;          // 提取低 12 位

                        // 使用临时寄存器
                        Register temp_reg = GetPhysicalReg(RISCV_a2);

                        // 使用 LUI 指令加载高 20 位
                        auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_sp), lui_val);
                        block->push_back(lui_instr);

                        // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                        if (addi_val < -2048 || addi_val > 2047) {
                            // 如果 ADDI 的立即数部分超出范围，进一步处理
                            // 分解成两个 ADDI 操作
                            int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                            int addi_part2 = addi_val - addi_part1;         // 剩余部分

                            // 使用 ADDI 指令添加第一部分立即数
                            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, addi_part1);
                            block->push_back(addi_instr1);

                            // 使用 ADDI 指令添加第二部分立即数
                            auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_sp), temp_reg, addi_part2);
                            block->push_back(addi_instr2);
                        }

                }

                    int offset = 0;
                    for (const auto& id : regs_to_save) {
                        if (!saveregs_occurblockids[id].empty()) {
                            offset -= 8;
                            if (id >= RISCV_x0 && id <= RISCV_x31) {
                                block->push_back(rvconstructor->ConstructIImm(RISCV_LD, GetPhysicalReg(id), GetPhysicalReg(RISCV_sp), offset));
                            } else {
                                block->push_back(rvconstructor->ConstructIImm(RISCV_FLD, GetPhysicalReg(id), GetPhysicalReg(RISCV_sp), offset));
                            }
                        }
                    }
                block->push_back(last_ins);
            }
        }
    }

    // 到此我们就完成目标代码生成的所有工作了
}