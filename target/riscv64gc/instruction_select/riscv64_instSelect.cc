#include "riscv64_instSelect.h"
#include "../../common/machine_instruction_structures/machine.h"
#include <sstream>

//访存指令
template <> void RiscV64Selector::ConvertAndAppend<LoadInstruction *>(LoadInstruction *ins) {
    //TODO("Implement this if you need");

    //获取结果寄存器
    auto result_operand = (RegOperand*)ins->GetResult();
    int result_operand_regno = result_operand->GetRegNo();
    int op;

    if (ir_riscv_regtable.find(result_operand_regno) == ir_riscv_regtable.end()){
        //如果没找到ir到目标寄存器映射，则新建一个寄存器(%2)
        if (ins->GetDataType() == I32 || ins->GetDataType() == PTR){
            ir_riscv_regtable[result_operand_regno] = cur_func->GetNewReg(INT64);
        }
        else if (ins->GetDataType() == FLOAT32){
            ir_riscv_regtable[result_operand_regno] = cur_func->GetNewReg(FLOAT64);
        }
    }

    Register riscv_reg = ir_riscv_regtable[result_operand_regno];

    if (ins->GetDataType() == I32){
        op = RISCV_LW;
    }
    else if (ins->GetDataType() == FLOAT32){
        op = RISCV_FLW;
    }
    else if (ins->GetDataType() == PTR){
        op = RISCV_LD;
    }

    //1、全局变量或常量
    // %t1 = load i32* @a ====> lui %1, %hi(a)
                            //lw %2,%lo(a)(%1)
    if (ins->GetPointer()->GetOperandType()==BasicOperand::GLOBAL){
        //获取全局变量操作数
        auto global_operand = (GlobalOperand *)ins->GetPointer();
        
        //存储全局变量地址高20位的寄存器(%1)
        Register ad_hi_reg = cur_func->GetNewReg(INT64);

        //生成lui指令，将全局变量的高20位加载到ad_hi中
        auto lui_ins = rvconstructor->ConstructULabel(RISCV_LUI, ad_hi_reg, RiscVLabel(global_operand->GetName(), true));
        
        //生成lw指令，加载低12位偏移量并读取数据
        auto lw_ins = rvconstructor->ConstructILabel(op, riscv_reg, ad_hi_reg, RiscVLabel(global_operand->GetName(), false));

        //当前基本块加入lui和lw指令
        cur_block->push_back(lui_ins);
        cur_block->push_back(lw_ins);  
    }

    //%t1 = load i32* %t2 ====> lw %1, 0(%2) 
    else if (ins->GetPointer()->GetOperandType()==BasicOperand::REG){
        //获取操作数和结果寄存器
        auto ptr_operand = (RegOperand *)ins->GetPointer();//%t2
        int ptr_operand_regno = ptr_operand->GetRegNo();

        //2、加载一个栈中的临时变量
        if (ir_riscv_alloca.find(ptr_operand_regno) != ir_riscv_alloca.end()){
            //获取该寄存器对应的栈上偏移量 offset
            auto offset = ir_riscv_alloca[ptr_operand_regno];
            //从栈指针加上偏移量的位置加载数据到目标寄存器 
            auto lw_ins = rvconstructor->ConstructIImm(op, riscv_reg, GetPhysicalReg(RISCV_sp), offset);
            //记录涉及到alloca指令生成的局部变量
            ((RiscV64Function *)cur_func)->addlist_alloca_ins(lw_ins);
            cur_block->push_back(lw_ins);
        }

        //3、加载寄存器中的变量
        else {
            //获取指针寄存器
            if (ir_riscv_regtable.find(ptr_operand_regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[ptr_operand_regno] = cur_func->GetNewReg(INT64);
            }
            Register ptr_reg = ir_riscv_regtable[ptr_operand_regno];
            auto lw_ins = rvconstructor->ConstructIImm(op, riscv_reg, ptr_reg, 0);
            cur_block->push_back(lw_ins);
        }
    }


}

//访存指令
template <> void RiscV64Selector::ConvertAndAppend<StoreInstruction *>(StoreInstruction *ins) {
    //TODO("Implement this if you need");
    int op;
    if (ins->GetDataType() == I32){
        op = RISCV_SW;
    }
    else if (ins->GetDataType() == FLOAT32){
        op = RISCV_FSW;
    }
    else if (ins->GetDataType() == PTR){
        op = RISCV_SD;
    }

    Register val_reg;
    if (ins->GetValue()->GetOperandType() == BasicOperand::IMMI32){
        auto imm_val = ((ImmI32Operand *)ins->GetValue())->GetIntImmVal();
        val_reg = cur_func->GetNewReg(INT64);
        if (imm_val >= -2048 && imm_val <= 2047) {
            // 如果立即数在ADDIW范围内，直接使用ADDIW指令
            auto copy_ins = rvconstructor->ConstructIImm(RISCV_ADDIW, val_reg, val_reg, imm_val);
            cur_block->push_back(copy_ins);
        } else {
            // 如果立即数超出了ADDIW的范围，使用LUI加上ADDIW
            // 首先，加载高位部分
            int upper_imm = imm_val & 0xFFFFF000;  // 取出高20位
            auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, val_reg, upper_imm);
            cur_block->push_back(lui_instr);
            
            // 然后，使用ADDIW来加上低位部分
            int lower_imm = imm_val & 0xFFF;  // 取出低12位
            auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, val_reg, val_reg, lower_imm);
            cur_block->push_back(addiw_instr);
        }
    }
    else if (ins->GetValue()->GetOperandType() == BasicOperand::IMMF32){
        auto imm_val = ((ImmF32Operand *)ins->GetValue())->GetFloatVal();
        //val_reg = cur_func->GetNewReg(INT64);
        // 1. 将浮点立即数的二进制表示转化为整数
        uint32_t imm_as_int = *reinterpret_cast<uint32_t *>(&imm_val);


        // 2. 将整数加载到整数寄存器
        auto temp_reg = cur_func->GetNewReg(INT32); // 32 位浮点数使用 INT32
        auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, temp_reg, (imm_as_int >> 12)); // 高 20 位
        cur_block->push_back(lui_instr);
        auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, (imm_as_int & 0xFFF)); // 低 12 位
        cur_block->push_back(addi_instr);

        // 3. 将整数寄存器值移动到浮点寄存器
        val_reg = cur_func->GetNewReg(FLOAT64); // 单精度浮点数寄存器
        auto fmv_instr = rvconstructor->ConstructR2(RISCV_FMV_W_X, val_reg, temp_reg);
        cur_block->push_back(fmv_instr);

    }
    else if (ins->GetValue()->GetOperandType() == BasicOperand::REG){
        int val_reg_no = ((RegOperand *)ins->GetValue())->GetRegNo();
        if (ir_riscv_regtable.find(val_reg_no) == ir_riscv_regtable.end()){
            //如果没找到ir到目标寄存器映射，则新建一个寄存器(%2)
            if (ins->GetDataType() == I32 || ins->GetDataType() == PTR){
                ir_riscv_regtable[val_reg_no] = cur_func->GetNewReg(INT64);
            }
            else if (ins->GetDataType() == FLOAT32){
                ir_riscv_regtable[val_reg_no] = cur_func->GetNewReg(FLOAT64);
            }
        }
        val_reg = ir_riscv_regtable[val_reg_no];
    }

    //1、全局变量或常量
    // store i32 %t1, i32* @a ====> lui %1, %hi(a)
                            //sw %2,%lo(a)(%1)
    if (ins->GetPointer()->GetOperandType()==BasicOperand::GLOBAL){
        //获取全局变量操作数
        auto global_operand = (GlobalOperand *)ins->GetPointer();
        
        //存储全局变量地址高20位的寄存器(%1)
        Register ad_hi_reg = cur_func->GetNewReg(INT64);

        //生成lui指令，将全局变量的高20位加载到ad_hi中
        auto lui_ins = rvconstructor->ConstructULabel(RISCV_LUI, ad_hi_reg, RiscVLabel(global_operand->GetName(), true));
        
        //生成lw指令，加载低12位偏移量并读取数据
        auto sw_ins = rvconstructor->ConstructSLabel(op, val_reg, ad_hi_reg, RiscVLabel(global_operand->GetName(), false));

        //当前基本块加入lui和lw指令
        cur_block->push_back(lui_ins);
        cur_block->push_back(sw_ins);  
    }

    //store i32 %t1, i32* %t2 -> sw %1, 0(%2)
    else if (ins->GetPointer()->GetOperandType()==BasicOperand::REG){
        //获取操作数和结果寄存器
        auto ptr_operand = (RegOperand *)ins->GetPointer();//%t2
        int ptr_operand_regno = ptr_operand->GetRegNo();

        //2、加载一个栈中的临时变量
        if (ir_riscv_alloca.find(ptr_operand_regno) != ir_riscv_alloca.end()){
            //获取该寄存器对应的栈上偏移量 offset
            auto offset = ir_riscv_alloca[ptr_operand_regno];
            //将寄存器中的值存到栈指针加上偏移量的位置
            auto sw_ins = rvconstructor->ConstructSImm(op, val_reg, GetPhysicalReg(RISCV_sp), offset);
            //记录涉及到alloca指令生成的局部变量
            ((RiscV64Function *)cur_func)->addlist_alloca_ins(sw_ins);
            cur_block->push_back(sw_ins);
        }

        //3、加载寄存器中的变量
        else {
            //获取指针寄存器
            if (ir_riscv_regtable.find(ptr_operand_regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[ptr_operand_regno] = cur_func->GetNewReg(INT64);
            }
            Register ptr_reg = ir_riscv_regtable[ptr_operand_regno];
            auto sw_ins = rvconstructor->ConstructSImm(op, val_reg, ptr_reg, 0);
            cur_block->push_back(sw_ins);
        }
    }

}

//内存分配指令，为局部变量在栈上分配空间
template <> void RiscV64Selector::ConvertAndAppend<AllocaInstruction *>(AllocaInstruction *ins) {
    //TODO("Implement this if you need");

    auto result_operand = (RegOperand *)ins->GetResult();
    int result_operand_regno = result_operand->GetRegNo();
    ir_riscv_alloca[result_operand_regno] = cur_offset;
    int size = 1;
    for (auto d : ins->GetDims()){
        size *= d;
    }
    cur_offset += (size << 2);
}

template <> void RiscV64Selector::ConvertAndAppend<ArithmeticInstruction *>(ArithmeticInstruction *ins) {
    //TODO("Implement this if you need");
    //乘法
    if(ins->GetOpcode()== BasicInstruction::MUL){
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();
        int opcode = ins->GetOpcode();
        //获取regno
        auto MulI = dynamic_cast<ArithmeticInstruction*>(ins);
        int regno;
        if (result->GetOperandType() == BasicOperand::REG) {
            Operand op = MulI->GetResult();
            RegOperand* regOp = (RegOperand *)(op);
            regno = regOp->GetRegNo();
            }
        
        
        if(ir_riscv_regtable.find(regno)==ir_riscv_regtable.end())
        {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        //新建结果寄存器映射，获取一个与目标寄存器相对应的LLVM寄存器对象
        Register rd = ir_riscv_regtable[regno];
        

        
        // 检查操作数类型
        RegOperand *regOp1 = nullptr, *regOp2 = nullptr;
        Operand *immOp2 = nullptr;

        Register opReg1,opReg2;

        // op1 是寄存器
        if (op1->GetOperandType() == BasicOperand::REG) {
            //查找并建立操作数1的寄存器映射
            int regno1;
            if (op1->GetOperandType() == BasicOperand::REG) {
                Operand op = MulI->GetOperand1();
                RegOperand* regOp = (RegOperand *)(op1);
                regno1 = regOp->GetRegNo();
                }
        
            if(ir_riscv_regtable.find(regno1)==ir_riscv_regtable.end())
            {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }

            opReg1 = ir_riscv_regtable[regno1];

        } else if(op1->GetOperandType() == BasicOperand::IMMI32)
            {
                opReg1 = cur_func->GetNewReg(INT64);
                auto op1_Immi=(ImmI32Operand *)ins->GetOperand1();
                auto op1_val = op1_Immi->GetIntImmVal();
                //auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg1,op1_val,INT64);
                //cur_block->push_back(copy_ins);
                // 如果立即数在ADDI可表示范围内，直接使用ADDI
                if (op1_val >= -2048 && op1_val <= 2047) {
                    auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1,GetPhysicalReg(RISCV_x0), op1_val);
                    cur_block->push_back(addi_instr);
                } else {
                    // 否则，分两步加载：高20位用LUI，低12位用ADDI
                    uint64_t imm_as_uint = static_cast<uint64_t>(op1_val);
                    uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                    uint32_t low_12_bits = imm_as_uint & 0xFFF;

                    // 使用LUI加载高20位
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg1, high_20_bits);
                    cur_block->push_back(lui_instr);

                    // 使用ADDI加载低12位
                    auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, opReg1, low_12_bits);
                    cur_block->push_back(addi_instr);

                    // 如果是负数，需要特殊处理，确保正确的符号扩展
                    if (op1_val < 0) {
                        auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg1, opReg1, 32); // 左移32位
                        cur_block->push_back(slli_instr);
                        auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg1, opReg1, 32); // 右移32位，完成符号扩展
                        cur_block->push_back(srli_instr);
                    }
                }
            }
             else {
            ERROR("Unexpected operand type for MUL");
         }

        // op2 可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2;
            if (op2->GetOperandType() == BasicOperand::REG) {
                Operand op = MulI->GetOperand2();
                RegOperand* regOp = (RegOperand *)(op);
                regno2 = regOp->GetRegNo();
                }
        
            if(ir_riscv_regtable.find(regno2)==ir_riscv_regtable.end())
            {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }

            opReg2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMI32) {
                opReg2 = cur_func->GetNewReg(INT64);
                auto op2_Immi=(ImmI32Operand *)ins->GetOperand2();
                auto op2_val = op2_Immi->GetIntImmVal();
                 // 如果立即数在ADDI可表示范围内，直接使用ADDI
            if (op2_val >= -2048 && op2_val <= 2047) {
                // 使用ADDI指令将立即数加载到目标寄存器
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg2, GetPhysicalReg(RISCV_x0), op2_val);
                cur_block->push_back(addi_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint32_t imm_as_uint = static_cast<uint32_t>(op2_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg2, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg2, opReg2, low_12_bits);
                cur_block->push_back(addi_instr);

                // 如果是负数，需要特殊处理，确保正确的符号扩展
                if (op2_val < 0) {
                    // 对于负数，我们需要确保符号扩展正确
                    // 这里我们先将寄存器左移32位，然后右移32位以填充符号位
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg2, opReg2, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg2, opReg2, 32); // 右移32位，完成符号扩展
                    cur_block->push_back(srli_instr);
                }
            }
        } else {
            ERROR("Unexpected operand type for MUL");
        }

        auto mul_inst = rvconstructor->ConstructR(RISCV_MULW,rd, opReg1, opReg2);
        cur_block->push_back(mul_inst);
    }
    else //加法


    if (ins->GetOpcode() == BasicInstruction::ADD) 
    {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();
        int opcode = ins->GetOpcode();

        // 获取 regno
        auto AddI = dynamic_cast<ArithmeticInstruction*>(ins);
        int regno;
        if (result->GetOperandType() == BasicOperand::REG) {
            Operand op = AddI->GetResult();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno = regOp->GetRegNo();
        }

        //确保目标寄存器已经映射到 LLVM 寄存器
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rd = ir_riscv_regtable[regno];

        // 检查操作数类型
        RegOperand* regOp1 = nullptr, * regOp2 = nullptr;
        Operand* immOp2 = nullptr;

        Register opReg1, opReg2;
        bool is_imm = false;
        int imm_val = 0;

        // op1 是寄存器
        if (op1->GetOperandType() == BasicOperand::REG) {
            int regno1;
            Operand op = AddI->GetOperand1();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno1 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }

            opReg1 = ir_riscv_regtable[regno1];

        } else if (op1->GetOperandType() == BasicOperand::IMMI32) {
            /*
            opReg1 = cur_func->GetNewReg(INT64);
            auto op1_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            auto op1_val = op1_Immi->GetIntImmVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg1, op1_val, INT64);
            cur_block->push_back(copy_ins);
            */
            is_imm = true;
            imm_val = dynamic_cast<ImmI32Operand*>(op1)->GetIntImmVal();
        }else {
            ERROR("Unexpected operand type for ADD");
        }


        // op2 可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2;
            Operand op = AddI->GetOperand2();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno2 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }

            opReg2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMI32) {
            /*
            opReg2 = cur_func->GetNewReg(INT64);
            auto op2_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand2());
            auto op2_val = op2_Immi->GetIntImmVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg2, op2_val, INT64);
            cur_block->push_back(copy_ins);
            */
            if (!is_imm) { // 如果第一个操作数不是立即数，则交换位置
                std::swap(opReg1, opReg2);
                is_imm = true;
                imm_val = dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal();
            } else {
                // 如果两个都是立即数，可以在编译时计算它们的和
                imm_val = imm_val + dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal();
            }
        } else {
            ERROR("Unexpected operand type for ADD");
        }

        // 构造加法指令
        /*
        auto add_inst = rvconstructor->ConstructR(RISCV_ADDW, rd, opReg1, opReg2);
        cur_block->push_back(add_inst);
        */
         // 根据操作数类型构造加法指令
        if (is_imm) {
            if (imm_val != 0) {
                 if (imm_val >= -2048 && imm_val <= 2047) {
                // 使用 ADDIW 指令添加立即数
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, GetPhysicalReg(RISCV_x0), imm_val);
                cur_block->push_back(addiw_instr);
            } else {
                // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                int lui_val = (imm_val >> 12) & 0xFFFFF; // 提取高 20 位
                int addi_val = imm_val & 0xFFF;          // 提取低 12 位

                // 使用临时寄存器
                Register temp_reg = GetPhysicalReg(RISCV_t0);

                // 使用 LUI 指令加载高 20 位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, rd, lui_val);
                cur_block->push_back(lui_instr);

                // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                if (addi_val < -2048 || addi_val > 2047) {
                    // 如果 ADDI 的立即数部分超出范围，进一步处理
                // 分解成两个 ADDI 操作
                int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                int addi_part2 = addi_val - addi_part1;         // 剩余部分

                // 使用 ADDI 指令添加第一部分立即数
                auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, addi_part1);
                cur_block->push_back(addi_instr1);

                // 使用 ADDI 指令添加第二部分立即数
                auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, temp_reg, addi_part2);
                cur_block->push_back(addi_instr2);
                } else {
                    // 如果 ADDI 没有超出范围，直接更新目标寄存器
                    auto final_addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, rd, rd, addi_val);
                    cur_block->push_back(final_addi_instr);
                }
                
            }
            } else {
                // 如果立即数是0，只需复制源寄存器
                auto copy_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, GetPhysicalReg(RISCV_x0), 0);
                cur_block->push_back(copy_instr);
            }
        } else {
            // 使用 ADDW 指令添加两个寄存器
            auto addw_instr = rvconstructor->ConstructR(RISCV_ADDW, rd, opReg1, opReg2);
            cur_block->push_back(addw_instr);
        }
    } else //减法


    if (ins->GetOpcode() == BasicInstruction::SUB) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();
        int opcode = ins->GetOpcode();

        // 获取 regno
        auto SubI = dynamic_cast<ArithmeticInstruction*>(ins);
        int regno;
        //确保结果是寄存器类型
        if (result->GetOperandType() == BasicOperand::REG) {
            Operand op = SubI->GetResult();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno = regOp->GetRegNo();
        }

        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rd = ir_riscv_regtable[regno];

        // 检查操作数类型
        RegOperand* regOp1 = nullptr, * regOp2 = nullptr;
        Operand* immOp2 = nullptr;

        Register opReg1, opReg2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;
        int imm_val1 = 0;
        int imm_val2 = 0;

        // 处理第一个操作数（被减数）
        if (op1->GetOperandType() == BasicOperand::REG) {
            int regno1;
            Operand op = SubI->GetOperand1();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno1 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }

            opReg1 = ir_riscv_regtable[regno1];

        } else if (op1->GetOperandType() == BasicOperand::IMMI32) {
            
            opReg1 = cur_func->GetNewReg(INT64);
            auto op1_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            /*
            auto op1_val = op1_Immi->GetIntImmVal();
            // 根据立即数的值选择适当的指令
            if (op1_val == 0) {
                // 如果立即数是0，可以使用ADDI指令复制零寄存器的内容到目标寄存器
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, GetPhysicalReg(RISCV_x0), 0);
                cur_block->push_back(addi_instr);
            } else if (op1_val >= -2048 && op1_val <= 2047) {
                // 如果立即数在ADDIW可表示范围内，直接使用ADDIW进行符号扩展
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, opReg1, GetPhysicalReg(RISCV_x0), op1_val);
                cur_block->push_back(addiw_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint32_t imm_as_uint = static_cast<uint32_t>(op1_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg1, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, opReg1, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (op1_val < 0) {
                    // 左移32位再右移32位以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg1, opReg1, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg1, opReg1, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }

           */
            
            is_op1_imm = true;
            imm_val1 = dynamic_cast<ImmI32Operand*>(op1)->GetIntImmVal();
        } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
            /*
            opReg1 = cur_func->GetNewReg(FLOAT64);
            auto op1_Immi = dynamic_cast<ImmF32Operand*>(ins->GetOperand1());
            auto op1_val = op1_Immi->GetFloatVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg1, op1_val, FLOAT64);
            cur_block->push_back(copy_ins);
            */
            
            is_op1_imm = true;
            imm_val1 = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
        }
        else {
            ERROR("Unexpected operand type for SUB");
        }

        // 处理第二个操作数（减数），可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2;
            Operand op = SubI->GetOperand2();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno2 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }

            opReg2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMI32) {
            /*
            opReg2 = cur_func->GetNewReg(INT64);
            auto op2_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand2());
            auto op2_val = op2_Immi->GetIntImmVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg2, op2_val, INT64);
            cur_block->push_back(copy_ins);
            */
            is_op2_imm = true;
            imm_val2 = dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal();
        } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
            /*
            opReg2 = cur_func->GetNewReg(FLOAT64);
            auto op2_Immi = dynamic_cast<ImmF32Operand*>(ins->GetOperand2());
            auto op2_val = op2_Immi->GetFloatVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(opReg2, op2_val, FLOAT64);
            cur_block->push_back(copy_ins);
            */
            is_op2_imm = true;
            imm_val2 = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
        }
        else {
            ERROR("Unexpected operand type for SUB");
        }

        // 构造减法指令
        /*
        auto sub_inst = rvconstructor->ConstructR(RISCV_SUBW, rd, opReg1, opReg2);
        cur_block->push_back(sub_inst);
        */
        if (is_op1_imm && !is_op2_imm) {
            // 被减数是立即数，减数是寄存器
            // 将 SUB immediate, reg 转换为 -reg + (-immediate)
            // 即：rd = -rs2 + imm_val1

            // 先生成一条 ADDIW 指令来计算 -rs2 + imm_val1
            auto rd1 = cur_func->GetNewReg(INT64);
            
            // 检查 imm_val1 是否在 ADDIW 范围内 (-2048 到 2047)
            if (imm_val1 >= -2048 && imm_val1 <= 2047) {
                auto subw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd1, GetPhysicalReg(RISCV_x0), imm_val1);
                cur_block->push_back(subw_instr);
            } else {
                // 如果立即数超出 ADDIW 范围，使用 LUI 加上 ADDIW
                int upper_imm = imm_val1 & 0xFFFFF000;  // 取高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, rd1, upper_imm);
                cur_block->push_back(lui_instr);
                
                int lower_imm = imm_val1 & 0xFFF;  // 取低12位
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd1, rd1, lower_imm);
                cur_block->push_back(addiw_instr);
            }

            // 再生成一条 SUBW 指令来减去寄存器值
            auto final_subw_instr = rvconstructor->ConstructR(RISCV_SUBW, rd, rd1, opReg2);
            cur_block->push_back(final_subw_instr);
        } else if (!is_op1_imm && is_op2_imm) {
            // 被减数是寄存器，减数是立即数
            // 检查立即数是否超出 ADDIW 的范围
            int32_t lower_imm_val = -imm_val2;  // 处理负值
            bool imm_in_range = (lower_imm_val >= -2048 && lower_imm_val <= 2047);

            if (imm_val2 != 0) {
                if (imm_in_range) {
                    // 使用 ADDIW 指令来实现减法（即添加负的立即数）
                    auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, opReg1, -imm_val2);
                    cur_block->push_back(addiw_instr);
                } else {
                    // 立即数超出 ADDIW 范围，使用 LUI + ADDIW 组合
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, rd, lower_imm_val & 0xFFFFF000);  // 获取高位
                    cur_block->push_back(lui_instr);
                    
                    // 添加低位
                    auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, rd, lower_imm_val & 0xFFF);  // 获取低位
                    cur_block->push_back(addiw_instr);
                }
            } else {
                // 如果立即数是0，使用 ADDIW 指令来复制源寄存器并进行符号扩展
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, opReg1, 0);
                cur_block->push_back(addiw_instr);
            }
        } else if (!is_op1_imm && !is_op2_imm) {
            // 两个操作数都是寄存器
            auto subw_instr = rvconstructor->ConstructR(RISCV_SUBW, rd, opReg1, opReg2);
            cur_block->push_back(subw_instr);
        } else {
            // 两个操作数都是立即数
            // 这种情况可以在编译期计算结果，并生成一条 COPY 指令
            int result_val = imm_val1 - imm_val2;
            // 如果 result_val 在 ADDIW 可表示范围内，可以直接使用 ADDIW
            if (result_val >= -2048 && result_val <= 2047) {
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, rd, GetPhysicalReg(RISCV_x0), static_cast<int32_t>(result_val));
                cur_block->push_back(addiw_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint64_t imm_as_uint = static_cast<uint64_t>(result_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, rd, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, rd, rd, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (result_val < 0) {
                    // 左移32位再右移32位以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, rd, rd, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, rd, rd, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }
        }
    }else //除法


        if (ins->GetOpcode() == BasicInstruction::DIV) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();
        int opcode = ins->GetOpcode();

        // 获取 regno
        auto DivI = dynamic_cast<ArithmeticInstruction*>(ins);
        int regno;
        if (result->GetOperandType() == BasicOperand::REG) {
            Operand op = DivI->GetResult();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno = regOp->GetRegNo();
        }

        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rd = ir_riscv_regtable[regno];

        // 检查操作数类型
        RegOperand* regOp1 = nullptr, * regOp2 = nullptr;
        Operand* immOp2 = nullptr;

        Register opReg1, opReg2;

        // op1 是寄存器
        if (op1->GetOperandType() == BasicOperand::REG) {
            int regno1;
            Operand op = DivI->GetOperand1();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno1 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }

            opReg1 = ir_riscv_regtable[regno1];

        } else if (op1->GetOperandType() == BasicOperand::IMMI32) {
            opReg1 = cur_func->GetNewReg(INT64);
            auto op1_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            auto op1_val = op1_Immi->GetIntImmVal();
            // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW
            if (op1_val >= -2048 && op1_val <= 2047) {
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, opReg1, GetPhysicalReg(RISCV_x0), op1_val);
                cur_block->push_back(addiw_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint64_t imm_as_uint = static_cast<uint64_t>(op1_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg1, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, opReg1, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (op1_val < 0) {
                    // 左移32位再右移32位以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg1, opReg1, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg1, opReg1, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }
        } else {
            ERROR("Unexpected operand type for DIV");
        }

        // op2 可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2;
            Operand op = DivI->GetOperand2();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno2 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }

            opReg2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMI32) {
            opReg2 = cur_func->GetNewReg(INT64);
            auto op2_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand2());
            auto op2_val = op2_Immi->GetIntImmVal();
            
            if (op2_val >= -2048 && op2_val <= 2047) {
                // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW 指令
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, opReg2, GetPhysicalReg(RISCV_x0), op2_val);
                cur_block->push_back(addiw_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint64_t imm_as_uint = static_cast<uint64_t>(op2_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg2, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg2, opReg2, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (op2_val < 0) {
                    // 左移32位再右移32位以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg2, opReg2, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg2, opReg2, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }
        } else {
            ERROR("Unexpected operand type for DIV");
        }

        // 构造除法指令
        auto div_inst = rvconstructor->ConstructR(RISCV_DIVW, rd, opReg1, opReg2);
        cur_block->push_back(div_inst);
    }
    else //mod


        if (ins->GetOpcode() == BasicInstruction::MOD) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();
        int opcode = ins->GetOpcode();

        // 获取 regno
        auto ModI = dynamic_cast<ArithmeticInstruction*>(ins);
        int regno;
        if (result->GetOperandType() == BasicOperand::REG) {
            Operand op = ModI->GetResult();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno = regOp->GetRegNo();
        }

        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rd = ir_riscv_regtable[regno];

        // 检查操作数类型
        RegOperand* regOp1 = nullptr, * regOp2 = nullptr;
        Operand* immOp2 = nullptr;

        Register opReg1, opReg2;

        // op1 是寄存器
        if (op1->GetOperandType() == BasicOperand::REG) {
            int regno1;
            Operand op = ModI->GetOperand1();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno1 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }

            opReg1 = ir_riscv_regtable[regno1];

        } else if (op1->GetOperandType() == BasicOperand::IMMI32) {
            opReg1 = cur_func->GetNewReg(INT64);
            auto op1_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            auto op1_val = op1_Immi->GetIntImmVal();
            if (op1_val >= -2048 && op1_val <= 2047) {
                // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW 指令
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, opReg1, GetPhysicalReg(RISCV_x0), op1_val);
                cur_block->push_back(addiw_instr);
            } else if (op1_val >= -32768 && op1_val <= 32767) {
                // 如果立即数在 ADDI 可表示范围内，但不在 ADDIW 范围内，直接使用 ADDI 指令
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, GetPhysicalReg(RISCV_x0), op1_val);
                cur_block->push_back(addi_instr);
            } else {
                // 否则，立即数超出 ADDI 和 ADDIW 范围，使用 LUI 和 ADDI 组合

                // 将立即数转为无符号整数，便于处理高位和低位
                uint64_t imm_as_uint = static_cast<uint64_t>(op1_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                // 使用 LUI 加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg1, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用 ADDI 加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg1, opReg1, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (op1_val < 0) {
                    // 左移32位再右移32位，以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg1, opReg1, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg1, opReg1, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }
        }else{
            ERROR("Unexpected operand type for MOD");
        }

        // op2 可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2;
            Operand op = ModI->GetOperand2();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            regno2 = regOp->GetRegNo();

            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }

            opReg2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMI32) {
            opReg2 = cur_func->GetNewReg(INT64);
            auto op2_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand2());
            auto op2_val = op2_Immi->GetIntImmVal();
            
            if (op2_val >= -2048 && op2_val <= 2047) {
                // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW 指令
                auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, opReg2, GetPhysicalReg(RISCV_x0), op2_val);
                cur_block->push_back(addiw_instr);
            } else {
                // 否则，分两步加载：高20位用LUI，低12位用ADDI
                uint64_t imm_as_uint = static_cast<uint64_t>(op2_val);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;
                int32_t low_12_bits = imm_as_uint & 0xFFF;

                // 使用LUI加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, opReg2, high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用ADDI加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, opReg2, opReg2, low_12_bits);
                cur_block->push_back(addi_instr);

                // 对于负数，确保正确的符号扩展
                if (op2_val < 0) {
                    // 左移32位再右移32位以完成符号扩展
                    auto slli_instr = rvconstructor->ConstructIImm(RISCV_SLLI, opReg2, opReg2, 32); // 左移32位
                    cur_block->push_back(slli_instr);
                    auto srli_instr = rvconstructor->ConstructIImm(RISCV_SRLI, opReg2, opReg2, 32); // 右移32位
                    cur_block->push_back(srli_instr);
                }
            }
        }else{
            ERROR("Unexpected operand type for MOD");
        }

        // 构造模运算指令（REM）
        auto mod_inst = rvconstructor->ConstructR(RISCV_REMW, rd, opReg1, opReg2);
        cur_block->push_back(mod_inst);
    }else


    if (ins->GetOpcode() == BasicInstruction::FADD) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();

        // 确保结果是寄存器类型
        Assert(result->GetOperandType() == BasicOperand::REG);

        // 获取结果寄存器编号，并确保它已经被映射到 LLVM 寄存器
        int regno = dynamic_cast<RegOperand*>(result)->GetRegNo();
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(FLOAT64);
        }
        Register rd = ir_riscv_regtable[regno];

        Register rs1, rs2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;
        float imm_val1 = 0.0f;
        float imm_val2 = 0.0f;

        // 处理第一个操作数（加数1）
        if (op1->GetOperandType() == BasicOperand::REG) {
            RegOperand* regOp1 = dynamic_cast<RegOperand*>(op1);
            int regno1 = regOp1->GetRegNo();
            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(FLOAT64);
            }
            rs1 = ir_riscv_regtable[regno1];
        } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
            /*
            rs1 = cur_func->GetNewReg(FLOAT64);
            auto op1_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            auto op1_val = op1_Immi->GetIntImmVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(rs1, op1_val, FLOAT64);
            cur_block->push_back(copy_ins);
            */
            is_op1_imm = true;
            imm_val1 = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
        } else {
            ERROR("Unexpected operand type for FADD");
        }

        // 处理第二个操作数（加数2），可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            RegOperand* regOp2 = dynamic_cast<RegOperand*>(op2);
            int regno2 = regOp2->GetRegNo();
            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(FLOAT64);
            }
            rs2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
            /*
            rs2 = cur_func->GetNewReg(FLOAT64);
            auto op2_Immi = dynamic_cast<ImmI32Operand*>(ins->GetOperand1());
            auto op2_val = op2_Immi->GetIntImmVal();
            auto copy_ins = rvconstructor->ConstructCopyRegImmI(rs2, op2_val, FLOAT64);
            cur_block->push_back(copy_ins);
            */
            is_op2_imm = true;
            imm_val2 = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
        } else {
            ERROR("Unexpected operand type for FADD");
        }

        // 构造浮点加法指令
        if (is_op1_imm && is_op2_imm) {
            // 如果两个操作数都是立即数，可以在编译期计算它们的和
            float result_val = imm_val1 + imm_val2;
            // 获取浮点立即数值
            //double result_val = static_cast<ImmF32Operand*>(ins->GetOperand1())->GetDoubleVal();

            // 将浮点数转换为对应的比特模式（即整数形式）
            uint64_t imm_as_uint = *reinterpret_cast<uint64_t*>(&result_val);

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
            auto fmv_d_x_instr = rvconstructor->ConstructR2(RISCV_FMV_W_X, rd, temp_reg); // 假设有这样一个构造函数
            cur_block->push_back(fmv_d_x_instr);
        } else if (is_op1_imm || is_op2_imm) {
            // 如果有一个操作数是立即数，需要将其加载到临时寄存器中
            //Register temp_reg = cur_func->GetNewReg(FLOAT64);
            float imm_val = is_op1_imm ? imm_val1 : imm_val2;
            // 将浮点数转换为对应的比特模式（即整数形式）
            uint64_t imm_as_uint = *reinterpret_cast<uint64_t*>(&imm_val);

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

            // 使用 FADD.S 指令进行加法
            Register non_imm_reg = is_op1_imm ? rs2 : rs1;
            auto fadd_instr = rvconstructor->ConstructR(RISCV_FADD_S, rd, non_imm_reg, temp_reg);
            cur_block->push_back(fadd_instr);
        } else {
            // 两个操作数都是寄存器
            // 使用 FADD.S 指令进行加法
            auto fadd_instr = rvconstructor->ConstructR(RISCV_FADD_S, rd, rs1, rs2);
            cur_block->push_back(fadd_instr);
        }
    }else //FSUB

    if (ins->GetOpcode() == BasicInstruction::FSUB) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();  // 被减数
        auto op2 = ins->GetOperand2();  // 减数
        auto result = ins->GetResult();

        // 确保结果是寄存器类型
        Assert(result->GetOperandType() == BasicOperand::REG);

        // 获取结果寄存器编号，并确保它已经被映射到 LLVM 寄存器
        int regno = dynamic_cast<RegOperand*>(result)->GetRegNo();
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(FLOAT64);
        }
        Register rd = ir_riscv_regtable[regno];

        Register rs1, rs2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;
        float imm_val1 = 0.0f;
        float imm_val2 = 0.0f;

        // 处理第一个操作数（被减数）
        if (op1->GetOperandType() == BasicOperand::REG) {
            RegOperand* regOp1 = dynamic_cast<RegOperand*>(op1);
            int regno1 = regOp1->GetRegNo();
            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(FLOAT64);
            }
            rs1 = ir_riscv_regtable[regno1];
        } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
            is_op1_imm = true;
            /*
            rs1 = cur_func->GetNewReg(FLOAT64);
            float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            // 获取浮点立即数值
            //double imm_val = static_cast<ImmF32Operand*>(ins->GetOperand1())->GetDoubleVal();

            // 将浮点数转换为对应的比特模式（即整数形式）
            uint64_t imm_as_uint = *reinterpret_cast<uint64_t*>(&imm_val);

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
            }*/
        } else {
            ERROR("Unexpected operand type for FSUB");
        }

        // 处理第二个操作数（减数），可以是寄存器或立即数
        if (op2->GetOperandType() == BasicOperand::REG) {
            RegOperand* regOp2 = dynamic_cast<RegOperand*>(op2);
            int regno2 = regOp2->GetRegNo();
            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(FLOAT64);
            }
            rs2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
            is_op2_imm = true;
            /*
            rs2 = cur_func->GetNewReg(FLOAT64);
            float imm_val = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
            auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs2, imm_val, FLOAT64);
            cur_block->push_back(copy_imm_instr);
            */
        } else {
            ERROR("Unexpected operand type for FSUB");
        }

         // 特殊情况处理：如果一个操作数为0
        if (is_op1_imm && dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal() == 0.0f) {
            // 如果被减数为0，则结果是减数的负值
            cur_block->push_back(rvconstructor->ConstructR2(RISCV_FNEG_S, rd, rs2));
        } else if (is_op2_imm && dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal() == 0.0f) {
            // 如果减数为0，则结果是被减数
            cur_block->push_back(rvconstructor->ConstructR(RISCV_ADDW, rd, GetPhysicalReg(RISCV_x0),rs1));
        } 


        // 创建一个临时浮点寄存器来存储立即数
        Register temp_float_reg = cur_func->GetNewReg(FLOAT64); // 获取新的浮点寄存器

        if (is_op1_imm) {
            float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            
            // 将立即数转换为对应的比特模式（即整数形式）
            uint32_t imm_as_uint = *reinterpret_cast<uint32_t*>(&imm_val);

            // 加载立即数到临时浮点寄存器中
            LoadImmediateToFloatReg(imm_as_uint, temp_float_reg);
            
            // 使用临时寄存器作为第一个操作数构造浮点减法指令
            auto fsub_instr = rvconstructor->ConstructR(RISCV_FSUB_S, rd, temp_float_reg, rs2);
            cur_block->push_back(fsub_instr);
        } else if (is_op2_imm) {
            float imm_val = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
            
            // 将立即数转换为对应的比特模式（即整数形式）
            uint32_t imm_as_uint = *reinterpret_cast<uint32_t*>(&imm_val);

            // 加载立即数到临时浮点寄存器中
            LoadImmediateToFloatReg( imm_as_uint, temp_float_reg);
            
            // 使用临时寄存器作为第二个操作数构造浮点减法指令
            auto fsub_instr = rvconstructor->ConstructR(RISCV_FSUB_S, rd, rs1, temp_float_reg);
            cur_block->push_back(fsub_instr);
        } else {
            // 如果都不是立即数，则直接构造浮点减法指令
            auto fsub_instr = rvconstructor->ConstructR(RISCV_FSUB_S, rd, rs1, rs2);
            cur_block->push_back(fsub_instr);
        }
    
    }else


    //FMUL
    if (ins->GetOpcode() == BasicInstruction::FMUL) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();  // 第一个操作数（乘数）
        auto op2 = ins->GetOperand2();  // 第二个操作数（被乘数）
        auto result = ins->GetResult();

        // 确保结果是寄存器类型
        Assert(result->GetOperandType() == BasicOperand::REG);

        // 获取结果寄存器编号，并确保它已经被映射到 LLVM 寄存器
        int regno = dynamic_cast<RegOperand*>(result)->GetRegNo();
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(FLOAT64);
        }
        Register rd = ir_riscv_regtable[regno];

        // 初始化变量
        Register rs1, rs2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;

        // 处理两个立即数的情况
        if (op1->GetOperandType() == BasicOperand::IMMF32 &&
            op2->GetOperandType() == BasicOperand::IMMF32) {
            float imm1 = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            float imm2 = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
            /*
            auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rd, imm1 * imm2, FLOAT64);
            cur_block->push_back(copy_imm_instr);
            */
           LoadImmediateToFloatReg(imm1*imm2,rd);
        } else {
            // 处理第一个操作数（乘数）
            if (op1->GetOperandType() == BasicOperand::REG) {
                RegOperand* regOp1 = dynamic_cast<RegOperand*>(op1);
                int regno1 = regOp1->GetRegNo();
                if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[regno1] = cur_func->GetNewReg(FLOAT64);
                }
                rs1 = ir_riscv_regtable[regno1];
            } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
                is_op1_imm = true;
                /*
                rs1 = cur_func->GetNewReg(FLOAT64);
                float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
                auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs1, imm_val, FLOAT64);
                cur_block->push_back(copy_imm_instr);
                */
            } else {
                ERROR("Unexpected operand type for FMUL");
            }

            // 处理第二个操作数（被乘数）
            if (op2->GetOperandType() == BasicOperand::REG) {
                RegOperand* regOp2 = dynamic_cast<RegOperand*>(op2);
                int regno2 = regOp2->GetRegNo();
                if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[regno2] = cur_func->GetNewReg(FLOAT64);
                }
            rs2 = ir_riscv_regtable[regno2];
            } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
                is_op2_imm = true;
                /*
                rs2 = cur_func->GetNewReg(FLOAT64);
                float imm_val = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
                auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs2, imm_val, FLOAT64);
                cur_block->push_back(copy_imm_instr);
                */
            } else {
                ERROR("Unexpected operand type for FMUL");
            }

            // 特殊情况处理：如果一个操作数为0或1
            if (is_op1_imm && dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal() == 0.0f ||
                is_op2_imm && dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal() == 0.0f) {
                // 如果任意一个操作数为0，则结果为0
                auto zero_val = 0.0f;
                LoadImmediateToFloatReg(zero_val,rd);
                //cur_block->push_back(copy_zero_instr);
            } else if (is_op1_imm && dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal() == 1.0f) {
                // 如果乘数为1，则结果是被乘数
                LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal(), rd);
            } else if (is_op2_imm && dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal() == 1.0f) {
                // 如果被乘数为1，则结果是乘数
                LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal(), rd);
            } else if (is_op1_imm || is_op2_imm) {
            // 当至少一个操作数是立即数时，需要将其加载到寄存器中
                Register temp_reg;
                if (is_op1_imm) {
                    LoadImmediateToFloatReg( dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal(), temp_reg);
                    // 构造浮点乘法指令，使用临时寄存器作为第一个操作数
                    auto fmul_instr = rvconstructor->ConstructR(RISCV_FMUL_S, rd, temp_reg, rs2);
                    cur_block->push_back(fmul_instr);
                } else if (is_op2_imm) {
                    LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal(), temp_reg);
                    // 构造浮点乘法指令，使用临时寄存器作为第二个操作数
                    auto fmul_instr = rvconstructor->ConstructR(RISCV_FMUL_S, rd, rs1, temp_reg);
                    cur_block->push_back(fmul_instr);
                }else {
                    // 构造浮点乘法指令
                    auto fmul_instr = rvconstructor->ConstructR(RISCV_FMUL_S, rd, rs1, rs2);
                    cur_block->push_back(fmul_instr);
                }
            }
        }
    }else
        if (ins->GetOpcode() == BasicInstruction::FDIV) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();  // 被除数
        auto op2 = ins->GetOperand2();  // 除数
        auto result = ins->GetResult();

        // 确保结果是寄存器类型
        Assert(result->GetOperandType() == BasicOperand::REG);

        // 获取结果寄存器编号，并确保它已经被映射到 LLVM 寄存器
        int regno = dynamic_cast<RegOperand*>(result)->GetRegNo();
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(FLOAT64);
        }
        Register rd = ir_riscv_regtable[regno];

        // 初始化变量
        Register rs1, rs2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;

        // 处理两个立即数的情况
        if (op1->GetOperandType() == BasicOperand::IMMF32 &&
            op2->GetOperandType() == BasicOperand::IMMF32) {
            float imm1 = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            float imm2 = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
            if (imm2 == 0.0f) {
                ERROR("Division by zero detected in immediate operands");
            }
            LoadImmediateToFloatReg( imm1 / imm2, rd);
            //cur_block->push_back(copy_imm_instr);
        } else {
            // 处理第一个操作数（被除数）
            if (op1->GetOperandType() == BasicOperand::REG) {
                RegOperand* regOp1 = dynamic_cast<RegOperand*>(op1);
                int regno1 = regOp1->GetRegNo();
                if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[regno1] = cur_func->GetNewReg(FLOAT64);
                }
                rs1 = ir_riscv_regtable[regno1];
            } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
                is_op1_imm = true;
                /*
                rs1 = cur_func->GetNewReg(FLOAT64);
                float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
                auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs1, imm_val, FLOAT64);
                cur_block->push_back(copy_imm_instr);
                */
            } else {
                ERROR("Unexpected operand type for FDIV");
            }

            // 处理第二个操作数（除数）
            if (op2->GetOperandType() == BasicOperand::REG) {
                RegOperand* regOp2 = dynamic_cast<RegOperand*>(op2);
                int regno2 = regOp2->GetRegNo();
                if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[regno2] = cur_func->GetNewReg(FLOAT64);
                }
            } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
                is_op2_imm = true;
                /*
                float imm_val = dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal();
                if (imm_val == 0.0f) {
                    ERROR("Division by zero detected in immediate operand");
                }
                rs2 = cur_func->GetNewReg(FLOAT64);
                auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs2, imm_val, FLOAT64);
                cur_block->push_back(copy_imm_instr);
                */
            } else {
                ERROR("Unexpected operand type for FDIV");
            }

            // 特殊情况处理：如果除数为1
            if (is_op2_imm && dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal() == 1.0f) {
                // 如果除数为1，则结果是被除数
                LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal(), rd);
            } 
            else {
                 if(is_op2_imm)
                {
                    LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op2)->GetFloatVal(),rs2);
                }
                if(is_op1_imm)
                {
                    LoadImmediateToFloatReg(dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal(),rs1);
                }
                
                // 构造浮点除法指令
                auto fdiv_instr = rvconstructor->ConstructR(RISCV_FDIV_S, rd, rs1, rs2);
                cur_block->push_back(fdiv_instr);
            }
        }
    }else // BITXOR

    if (ins->GetOpcode() == BasicInstruction::BITXOR) {
        // 获取操作数类型
        auto op1 = ins->GetOperand1();
        auto op2 = ins->GetOperand2();
        auto result = ins->GetResult();
        enum BasicInstruction::LLVMType type = ins->GetDataType();

        // 确保结果是寄存器类型，并获取其编号
        Assert(result->GetOperandType() == BasicOperand::REG);
        RegOperand* regOpResult = dynamic_cast<RegOperand*>(result);
        int regno = regOpResult->GetRegNo();

        // 确保目标寄存器已经映射到 LLVM 寄存器
        if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
        }

        // 获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rd = ir_riscv_regtable[regno];

        // 初始化变量
        Register rs1, rs2;
        bool is_op1_imm = false;
        bool is_op2_imm = false;

        // 处理第一个操作数
        if (op1->GetOperandType() == BasicOperand::REG) {
            int regno1 = dynamic_cast<RegOperand*>(op1)->GetRegNo();
            if (ir_riscv_regtable.find(regno1) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno1] = cur_func->GetNewReg(INT64);
            }
            rs1 = ir_riscv_regtable[regno1];
        } else if (op1->GetOperandType() == BasicOperand::IMMF32) {
            is_op1_imm = true;
            //rs1 = cur_func->GetNewReg(FLOAT64);
            //float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            //auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs1, imm_val, FLOAT64);
            //cur_block->push_back(copy_imm_instr);
           
        }else if (op1->GetOperandType() == BasicOperand::IMMI32) {
            is_op1_imm = true;
            /*
            rs1 = cur_func->GetNewReg(INT64);
            float imm_val = dynamic_cast<ImmI32Operand*>(op1)->GetIntImmVal();
            auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs1, imm_val, INT64);
            cur_block->push_back(copy_imm_instr);
            */
        }
         else {
            ERROR("Unexpected operand type for BITXOR");
        }

        // 处理第二个操作数
        if (op2->GetOperandType() == BasicOperand::REG) {
            int regno2 = dynamic_cast<RegOperand*>(op2)->GetRegNo();
            if (ir_riscv_regtable.find(regno2) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[regno2] = cur_func->GetNewReg(INT64);
            }
            rs2 = ir_riscv_regtable[regno2];
        } else if (op2->GetOperandType() == BasicOperand::IMMF32) {
            is_op2_imm = true;
            /*
            rs2 = cur_func->GetNewReg(FLOAT64);
            float imm_val = dynamic_cast<ImmF32Operand*>(op1)->GetFloatVal();
            auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs2, imm_val, FLOAT64);
            cur_block->push_back(copy_imm_instr);
            */
        }else if (op2->GetOperandType() == BasicOperand::IMMI32) {
            is_op2_imm = true;
            /*
            rs2 = cur_func->GetNewReg(INT64);
            float imm_val = dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal();
            auto copy_imm_instr = rvconstructor->ConstructCopyRegImmF(rs2, imm_val, INT64);
            cur_block->push_back(copy_imm_instr);
            */
        } else {
            ERROR("Unexpected operand type for BITXOR");
        }

        // 处理两个立即数的情况
        if (is_op1_imm && is_op2_imm) {
            int imm1 = dynamic_cast<ImmI32Operand*>(op1)->GetIntImmVal();
            int imm2 = dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal();
             LoadImmediateToFloatReg(imm1^imm2,rd);
        } 
        // 处理一个立即数和一个寄存器的情况
        else {
            if (is_op1_imm ) 
            {
                LoadImmediateToFloatReg(dynamic_cast<ImmI32Operand*>(op1)->GetIntImmVal(),rs1);
            }
            if (is_op2_imm ) 
            {
                LoadImmediateToFloatReg(dynamic_cast<ImmI32Operand*>(op2)->GetIntImmVal(),rs2);
            }
           
            cur_block->push_back(rvconstructor->ConstructR(RISCV_XOR, rd, rs1, rs2));
        } 
        
    } else {
        Log("RV InstSelect For Opcode %d", ins->GetOpcode());
    }

        
}

template <> void RiscV64Selector::ConvertAndAppend<IcmpInstruction *>(IcmpInstruction *ins) {
    //TODO("Implement this if you need");
    int regno ;
    if(ins->GetResult()->GetOperandType() == BasicOperand::REG)
    {
        RegOperand* regOpResult = dynamic_cast<RegOperand*>(ins->GetResult());
        regno = regOpResult->GetRegNo();
    }

    // 确保目标寄存器已经映射到 LLVM 寄存器
    if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
    }

    // 获取一个与目标寄存器相对应的 LLVM 寄存器对象
    Register rd = ir_riscv_regtable[regno];
    cmp_context[rd] = ins;
}

//浮点数比较
template <> void RiscV64Selector::ConvertAndAppend<FcmpInstruction *>(FcmpInstruction *ins) {
    //TODO("Implement this if you need");
    int regno ;
    if(ins->GetResult()->GetOperandType() == BasicOperand::REG)
    {
        RegOperand* regOpResult = dynamic_cast<RegOperand*>(ins->GetResult());
        regno = regOpResult->GetRegNo();
    }

    // 确保目标寄存器已经映射到 LLVM 寄存器
    if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
    }

    // 获取一个与目标寄存器相对应的 LLVM 寄存器对象
    Register rd = ir_riscv_regtable[regno];
    cmp_context[rd] = ins;
}

//无条件跳转指令  br label %B6 ====> j .L6 
template <> void RiscV64Selector::ConvertAndAppend<BrUncondInstruction *>(BrUncondInstruction *ins) {
    //TODO("Implement this if you need");

    auto ir_dest_label = (LabelOperand *)ins->GetDestLabel();
    int ir_dest_label_regno = ir_dest_label->GetLabelNo();
    auto riscv_dest_label = RiscVLabel(ir_dest_label_regno);
    auto jar_ins = rvconstructor->ConstructJLabel(RISCV_JAL, GetPhysicalReg(RISCV_x0), riscv_dest_label);
    cur_block->push_back(jar_ins);
}

//有条件跳转指令   %t2 = icmp ne i32 %t1, 0
               //bne %1, x0, .L3
               //br i1 %t2, label %B3, label %B4 ====> j .L4
template <> void RiscV64Selector::ConvertAndAppend<BrCondInstruction *>(BrCondInstruction *ins) {
    //TODO("Implement this if you need");

    //获取条件寄存器和比较指令
    auto cond_reg = (RegOperand *)ins->GetCond();
    int cond_regno = cond_reg->GetRegNo();
    if (ir_riscv_regtable.find(cond_regno) == ir_riscv_regtable.end()){
        ir_riscv_regtable[cond_regno] = cur_func->GetNewReg(INT64);
    }
    auto br_reg = ir_riscv_regtable[cond_regno];
    //从cmp_context映射表中查找与条件寄存器相关的比较指令
    auto cmp_ins = cmp_context[br_reg];

    Register cmp_riscv_op1, cmp_riscv_op2, cmp_result;
    int opcode;
    //处理比较指令
    if (cmp_ins->GetOpcode() == BasicInstruction::ICMP){
        auto icmp_ins = (IcmpInstruction *)cmp_ins;
        //第一个操作数是寄存器类型
        if (icmp_ins->GetOp1()->GetOperandType() == BasicOperand::REG) {
            auto icmp_ins_op1 = (RegOperand *)icmp_ins->GetOp1();
            int icmp_ins_op1regno = icmp_ins_op1->GetRegNo();
            if (ir_riscv_regtable.find(icmp_ins_op1regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[icmp_ins_op1regno] = cur_func->GetNewReg(INT64);
            }
            cmp_riscv_op1 = ir_riscv_regtable[icmp_ins_op1regno];
        }
        //第一个操作数是立即数
        else if (icmp_ins->GetOp1()->GetOperandType() == BasicOperand::IMMI32) {
            auto icmp_ins_op1 = (ImmI32Operand *)icmp_ins->GetOp1();
            auto icmp_ins_op1imm = icmp_ins_op1->GetIntImmVal();
            //如果立即数不等于0
            if (icmp_ins_op1imm != 0){
                cmp_riscv_op1 = cur_func->GetNewReg(INT64);
                //构造一条将立即数值复制到新分配寄存器的指令
                LoadImmediateToFloatReg(icmp_ins_op1imm,cmp_riscv_op1);
    
            }
            else{
                cmp_riscv_op1 = GetPhysicalReg(RISCV_x0);
            }
        }

        //处理第二个操作数
        if (icmp_ins->GetOp2()->GetOperandType() == BasicOperand::REG) {
            auto icmp_ins_op2 = (RegOperand *)icmp_ins->GetOp2();
            int icmp_ins_op2regno = icmp_ins_op2->GetRegNo();
            if (ir_riscv_regtable.find(icmp_ins_op2regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[icmp_ins_op2regno] = cur_func->GetNewReg(INT64);
            }
            cmp_riscv_op2 = ir_riscv_regtable[icmp_ins_op2regno];
        }
        //第二个操作数是立即数
        else if (icmp_ins->GetOp2()->GetOperandType() == BasicOperand::IMMI32) {
            auto icmp_ins_op2 = (ImmI32Operand *)icmp_ins->GetOp2();
            auto icmp_ins_op2imm = icmp_ins_op2->GetIntImmVal();
            //如果立即数不等于0
            if (icmp_ins_op2imm != 0){
                cmp_riscv_op2 = cur_func->GetNewReg(INT64);
                //构造一条将立即数值复制到新分配寄存器的指令
                LoadImmediateToFloatReg(icmp_ins_op2imm,cmp_riscv_op2);
            }
            else{
                cmp_riscv_op2 = GetPhysicalReg(RISCV_x0);
            }
        }
        if (icmp_ins->GetCompareCondition() == BasicInstruction::eq){
            opcode = RISCV_BEQ;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::ne){
            opcode = RISCV_BNE;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::ugt){
            opcode = RISCV_BGTU;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::uge){
            opcode = RISCV_BGEU;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::ult){
            opcode = RISCV_BLTU;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::ule){
            opcode = RISCV_BLEU;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::sgt){
            opcode = RISCV_BGT;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::sge){
            opcode = RISCV_BGE;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::slt){
            opcode = RISCV_BLT;
        }
        else if (icmp_ins->GetCompareCondition() == BasicInstruction::sle){
            opcode = RISCV_BLE;
        }
    }
    if (cmp_ins->GetOpcode() == BasicInstruction::FCMP){
        auto fcmp_ins = (FcmpInstruction *)cmp_ins;
        //第一个操作数是寄存器类型
        if (fcmp_ins->GetOp1()->GetOperandType() == BasicOperand::REG) {
            auto fcmp_ins_op1 = (RegOperand *)fcmp_ins->GetOp1();
            int fcmp_ins_op1regno = fcmp_ins_op1->GetRegNo();
            if (ir_riscv_regtable.find(fcmp_ins_op1regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[fcmp_ins_op1regno] = cur_func->GetNewReg(FLOAT64);
            }
            cmp_riscv_op1 = ir_riscv_regtable[fcmp_ins_op1regno];
        }
        //第一个操作数是立即数
        else if (fcmp_ins->GetOp1()->GetOperandType() == BasicOperand::IMMF32) {
            auto fcmp_ins_op1 = (ImmF32Operand *)fcmp_ins->GetOp1();
            auto fcmp_ins_op1imm = fcmp_ins_op1->GetFloatVal();
            cmp_riscv_op1 = cur_func->GetNewReg(FLOAT64);
            //构造一条将立即数值复制到新分配寄存器的指令
            LoadImmediateToFloatReg(fcmp_ins_op1imm,cmp_riscv_op1);
        }

        //处理第二个操作数
        if (fcmp_ins->GetOp2()->GetOperandType() == BasicOperand::REG) {
            auto fcmp_ins_op2 = (RegOperand *)fcmp_ins->GetOp2();
            int fcmp_ins_op2regno = fcmp_ins_op2->GetRegNo();
            if (ir_riscv_regtable.find(fcmp_ins_op2regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[fcmp_ins_op2regno] = cur_func->GetNewReg(FLOAT64);
            }
            cmp_riscv_op2 = ir_riscv_regtable[fcmp_ins_op2regno];
        }
        //第二个操作数是立即数
        else if (fcmp_ins->GetOp2()->GetOperandType() == BasicOperand::IMMF32) {
            auto fcmp_ins_op2 = (ImmF32Operand *)fcmp_ins->GetOp2();
            auto fcmp_ins_op2imm = fcmp_ins_op2->GetFloatVal();
            cmp_riscv_op2 = cur_func->GetNewReg(FLOAT64);
            //构造一条将立即数值复制到新分配寄存器的指令
            LoadImmediateToFloatReg(fcmp_ins_op2imm,cmp_riscv_op2);
        }
        cmp_result = cur_func->GetNewReg(INT64);
        if (fcmp_ins->GetCond() == BasicInstruction::OEQ || fcmp_ins->GetCond() == BasicInstruction::UEQ){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FEQ_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BNE;
        }
        else if (fcmp_ins->GetCond() == BasicInstruction::OGT || fcmp_ins->GetCond() == BasicInstruction::UGT){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FLT_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BNE;
        }
        else if (fcmp_ins->GetCond() == BasicInstruction::OGE || fcmp_ins->GetCond() == BasicInstruction::UGE){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FLE_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BNE;
        }
        else if (fcmp_ins->GetCond() == BasicInstruction::OLT || fcmp_ins->GetCond() == BasicInstruction::ULT){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FLT_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BNE;
        }
        else if (fcmp_ins->GetCond() == BasicInstruction::OLE || fcmp_ins->GetCond() == BasicInstruction::ULE){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FLE_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BNE;
        }
        else if (fcmp_ins->GetCond() == BasicInstruction::ONE || fcmp_ins->GetCond() == BasicInstruction::UNE){
            auto cmp_ins = rvconstructor->ConstructR(RISCV_FEQ_S, cmp_result, cmp_riscv_op1, cmp_riscv_op2);
            cur_block->push_back(cmp_ins);
            opcode = RISCV_BEQ;
        }
        //将第一个操作数更新为计算出的比较结果寄存器
        cmp_riscv_op1 = cmp_result;
        //第二个操作数设置为零寄存器x0,后续跳转指令会基于这两个操作数进行判断
        cmp_riscv_op2 = GetPhysicalReg(RISCV_x0);
    }
    //获取标签操作数
    auto true_label = (LabelOperand *)ins->GetTrueLabel();
    auto false_label = (LabelOperand *)ins->GetFalseLabel();
    auto riscv_label = RiscVLabel(true_label->GetLabelNo(), false_label->GetLabelNo());
    //构造条件跳转指令
    auto br_ins = rvconstructor->ConstructBLabel(opcode, cmp_riscv_op1, cmp_riscv_op2, riscv_label);
    cur_block->push_back(br_ins);
    //构造无条件跳转指令
    auto br_uncond_ins = rvconstructor->ConstructJLabel(RISCV_JAL, GetPhysicalReg(RISCV_x0), RiscVLabel(false_label->GetLabelNo()));
    cur_block->push_back(br_uncond_ins);
}

//返回指令  ret i32 0 ====> li a0 0
         //ld ra, offset1(sp)
         //ld s0, offset2(sp)
         //......
         //addi sp, sp, #stack_size
         //jr ra
template <> void RiscV64Selector::ConvertAndAppend<RetInstruction *>(RetInstruction *ins) {
    if (ins->GetRetVal() != NULL) {
        if (ins->GetRetVal()->GetOperandType() == BasicOperand::IMMI32) {
            //直接将该立即数值加载到 a0 寄存器中
            auto retimm_op = (ImmI32Operand *)ins->GetRetVal();
            auto imm = retimm_op->GetIntImmVal();
            //构造一条加载立即数的指令

            if (imm >= -2048 && imm <= 2047) {
                // 如果 sp_offset 在 ADDI 可表示范围内，直接使用 ADDI 指令
                auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_sp), imm);
                //((RiscV64Function *)cur_func)->addlist_alloca_ins(ld_alloca);
                cur_block->push_back(ld_alloca);
            } else {
                // 否则，使用 LUI 和 ADDI 组合加载
                uint64_t imm_as_uint = static_cast<uint64_t>(imm);
                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                // 使用 LUI 加载高20位
                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_a2), high_20_bits);
                cur_block->push_back(lui_instr);

                // 使用 ADDI 加载低12位
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_a2), low_12_bits);
                cur_block->push_back(addi_instr);

                // 添加到函数的 alloca 指令列表
                //((RiscV64Function *)cur_func)->addlist_alloca_ins(lui_instr);
                //((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_instr);
            }

        } else if (ins->GetRetVal()->GetOperandType() == BasicOperand::IMMF32) {
            //TODO("Implement this if you need");
            //直接将该立即数值加载到 a0 寄存器中
            auto retimm_op = (ImmF32Operand *)ins->GetRetVal();
            auto imm = retimm_op->GetFloatVal();
            //构造一条加载立即数的指令
            // if (imm >= -2048 && imm <= 2047) {
            //     // 如果 sp_offset 在 ADDI 可表示范围内，直接使用 ADDI 指令
            //     auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_sp), imm);
            //     //((RiscV64Function *)cur_func)->addlist_alloca_ins(ld_alloca);
            //     cur_block->push_back(ld_alloca);
            // } else {
            //     // 否则，使用 LUI 和 ADDI 组合加载
            //     uint64_t imm_as_uint = static_cast<uint64_t>(imm);
            //     uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
            //     int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

            //     // 使用 LUI 加载高20位
            //     auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_a2), high_20_bits);
            //     cur_block->push_back(lui_instr);

            //     // 使用 ADDI 加载低12位
            //     auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_a2), low_12_bits);
            //     cur_block->push_back(addi_instr);

            //     // 添加到函数的 alloca 指令列表
            //     //((RiscV64Function *)cur_func)->addlist_alloca_ins(lui_instr);
            //     //((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_instr);
            // }

            // 将浮点数转换为对应的比特模式（即整数形式）
            uint64_t imm_as_uint = *reinterpret_cast<uint64_t*>(&imm);

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
            auto fmv_d_x_instr = rvconstructor->ConstructR2(RISCV_FMV_W_X, GetPhysicalReg(RISCV_a2), temp_reg); // 假设有这样一个构造函数
            cur_block->push_back(fmv_d_x_instr);

        } else if (ins->GetRetVal()->GetOperandType() == BasicOperand::REG) {
            //TODO("Implement this if you need");
            if (ins->GetType() == I32){
                auto return_val = (RegOperand *)ins->GetRetVal();
                auto return_regno = return_val->GetRegNo();
                if (ir_riscv_regtable.find(return_regno) == ir_riscv_regtable.end()){
                    ir_riscv_regtable[return_regno] = cur_func->GetNewReg(INT64);
                }
                auto return_reg = ir_riscv_regtable[return_regno];
                // 构造一条ADDI指令，将return_reg的值移动到a0寄存器
                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0), return_reg, 0);
                cur_block->push_back(addi_instr);
            }
            else if (ins->GetType() == FLOAT32){
                auto return_val = (RegOperand *)ins->GetRetVal();
                auto return_regno = return_val->GetRegNo();
                if (ir_riscv_regtable.find(return_regno) == ir_riscv_regtable.end()){
                    ir_riscv_regtable[return_regno] = cur_func->GetNewReg(FLOAT64);
                }
                auto return_reg = ir_riscv_regtable[return_regno];
                // 构造一条fmv.d指令，将return_reg的值移动到fa0寄存器
                auto fmv_instr = rvconstructor->ConstructR2(RISCV_FMV_D, GetPhysicalReg(RISCV_fa0), return_reg);
                cur_block->push_back(fmv_instr);
            }
        }
    }

    auto ret_instr = rvconstructor->ConstructIImm(RISCV_JALR, GetPhysicalReg(RISCV_x0), GetPhysicalReg(RISCV_ra), 0);
    if (ins->GetType() == BasicInstruction::I32) {
        ret_instr->setRetType(1);
    } else if (ins->GetType() == BasicInstruction::FLOAT32) {
        ret_instr->setRetType(2);
    } else {
        ret_instr->setRetType(0);
    }
    cur_block->push_back(ret_instr);
}

template <> void RiscV64Selector::ConvertAndAppend<CallInstruction *>(CallInstruction *ins) {
    //TODO("Implement this if you need");
    rvconstructor->DisableSchedule();//设置为不可调度

    int int_count = 0;//整数寄存器参数计数
    int float_count = 0;//浮点数
    int skt_count = 0;//堆栈上分配参数计数

    //处理特殊函数，内存设置函数，填充内存
    if (ins->GetFunctionName() == std::string("llvm.memset.p0.i32")
        &&ins->GetParameterList().size()==4
        &&ins->GetParameterList()[0].second->GetOperandType()==BasicOperand::REG
        &&ins->GetParameterList()[1].second->GetOperandType() == BasicOperand::IMMI32 
        &&ins->GetParameterList()[3].second->GetOperandType() == BasicOperand::IMMI32)
        {
            //参数0,目标地址
            int reg_no1 = ((RegOperand *)ins->GetParameterList()[0].second)->GetRegNo();
            if (ir_riscv_alloca.find(reg_no1) == ir_riscv_alloca.end()) {
                //是寄存器，就将它的值拷贝到 RISCV_a0 寄存器
                if (ir_riscv_regtable.find(reg_no1) == ir_riscv_regtable.end()){
                    ir_riscv_regtable[reg_no1] = cur_func->GetNewReg(INT64);
                }
                auto return_reg = ir_riscv_regtable[reg_no1];
                cur_block->push_back(
                rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0), return_reg, 0));
            } else {
                //如果是一个 alloca（栈上的局部变量），则从栈上加载该地址并将其存储到 RISCV_a0 寄存器
                auto sp_offset = ir_riscv_alloca[reg_no1];
                // 检查立即数范围并使用合适的指令
                int64_t imm_val = sp_offset;  // 假设 sp_offset 是立即数
                if (imm_val >= -2048 && imm_val <= 2047) {
                    // 如果立即数在 ADDI 可表示范围内，直接使用 ADDI 指令
                    auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0), GetPhysicalReg(RISCV_sp), imm_val);
                    ((RiscV64Function *)cur_func)->addlist_alloca_ins(ld_alloca);
                    cur_block->push_back(ld_alloca);
                } else {
                    // 否则，使用 LUI 和 ADDI 组合加载
                    uint64_t imm_as_uint = static_cast<uint64_t>(imm_val);
                    uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                    int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                    // 使用 LUI 加载高20位
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_a0), high_20_bits);
                    cur_block->push_back(lui_instr);

                    // 使用 ADDI 加载低12位
                    auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0), GetPhysicalReg(RISCV_a0), low_12_bits);
                    cur_block->push_back(addi_instr);

                    // 添加到函数的 alloca 指令列表
                    ((RiscV64Function *)cur_func)->addlist_alloca_ins(lui_instr);
                    ((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_instr);
                }
            }


            //参数1.内存填充值，复制到 RISCV_a1 寄存器
            auto imm_op = (ImmI32Operand *)(ins->GetParameterList()[1].second);
            LoadImmediateToFloatReg(imm_op->GetIntImmVal(),GetPhysicalReg(RISCV_a1));


            //参数2,数组的大小
            if (ins->GetParameterList()[2].second->GetOperandType() == BasicOperand::IMMI32) {
                //是立即数，则直接复制到 RISCV_a2 寄存器
                int arr_sz = ((ImmI32Operand *)ins->GetParameterList()[2].second)->GetIntImmVal();
                LoadImmediateToFloatReg(arr_sz,GetPhysicalReg(RISCV_a2));
            } else {
                int sizereg_no = ((RegOperand *)ins->GetParameterList()[2].second)->GetRegNo();
                if (ir_riscv_alloca.find(sizereg_no) == ir_riscv_alloca.end()) {

                    //是寄存器，检查是否为 alloca，如果是，则从栈中加载该值
                    if (ir_riscv_regtable.find(sizereg_no) == ir_riscv_regtable.end()){
                        ir_riscv_regtable[sizereg_no] = cur_func->GetNewReg(INT64);
                    }
                    auto return_reg = ir_riscv_regtable[sizereg_no];
                    cur_block->push_back(
                        rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), return_reg, 0));
                } else {
                    // 获取 sp_offset
                    auto sp_offset = ir_riscv_alloca[sizereg_no];  // 假设这是需要加载的偏移值

                    if (sp_offset >= -2048 && sp_offset <= 2047) {
                        // 如果 sp_offset 在 ADDI 可表示范围内，直接使用 ADDI 指令
                        auto ld_alloca = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_sp), sp_offset);
                        ((RiscV64Function *)cur_func)->addlist_alloca_ins(ld_alloca);
                        cur_block->push_back(ld_alloca);
                    } else {
                        // 否则，使用 LUI 和 ADDI 组合加载
                        uint64_t imm_as_uint = static_cast<uint64_t>(sp_offset);
                        uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                        int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                        // 使用 LUI 加载高20位
                        auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_a2), high_20_bits);
                        cur_block->push_back(lui_instr);

                        // 使用 ADDI 加载低12位
                        auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a2), GetPhysicalReg(RISCV_a2), low_12_bits);
                        cur_block->push_back(addi_instr);

                        // 添加到函数的 alloca 指令列表
                        ((RiscV64Function *)cur_func)->addlist_alloca_ins(lui_instr);
                        ((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_instr);
                    }
                }


            }

            //参数三，填充大小.调用 memset 函数，并传递 3 个参数（目标地址、填充值、大小）
            cur_block->push_back(rvconstructor->ConstructCall(RISCV_CALL, "memset", 3, 0));
            return;




        }

        //处理其他函数调用的参数
        for (auto [type, arg_op] : ins->GetParameterList()) {
        if (type == I32 || type == PTR) {
            if (int_count < 8) {
                // 处理整数类型和指针类型的参数
                if (arg_op->GetOperandType() == BasicOperand::REG) {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_regno = arg_regop->GetRegNo();
                    //auto arg_reg = cur_func->GetNewReg(INT64);
                    if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                        ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(INT64);
                    }
                    auto arg_reg = ir_riscv_regtable[arg_regno];
                    if (ir_riscv_alloca.find(arg_regop->GetRegNo()) == ir_riscv_alloca.end()) {
                        auto arg_copy_instr =
                        rvconstructor->ConstructIImm(RISCV_ADDI,GetPhysicalReg(RISCV_a0 + int_count), arg_reg,0);
                        cur_block->push_back(arg_copy_instr);
                    } else {
                       // 获取 sp_offset
                        auto sp_offset = ir_riscv_alloca[arg_regop->GetRegNo()];  // 从 ir_riscv_alloca 获取偏移量

                        if (sp_offset >= -2048 && sp_offset <= 2047) {
                            // 如果 sp_offset 在 ADDI 可表示范围内，直接使用 ADDI 指令
                            cur_block->push_back(rvconstructor->ConstructIImm(
                                RISCV_ADDI, GetPhysicalReg(RISCV_a0 + int_count), GetPhysicalReg(RISCV_sp), sp_offset));
                        } else {
                            // 否则，超出 ADDI 范围，使用 LUI 和 ADDI 组合加载
                            uint64_t imm_as_uint = static_cast<uint64_t>(sp_offset);
                            uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                            int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                            // 使用 LUI 加载高20位
                            auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, GetPhysicalReg(RISCV_a0 + int_count), high_20_bits);
                            cur_block->push_back(lui_instr);

                            // 使用 ADDI 加载低12位
                            auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, GetPhysicalReg(RISCV_a0 + int_count), GetPhysicalReg(RISCV_a0 + int_count), low_12_bits);
                            cur_block->push_back(addi_instr);
                        }
                    }
                } else if (arg_op->GetOperandType() == BasicOperand::IMMI32) {
                    auto arg_immop = (ImmI32Operand *)arg_op;
                    auto arg_imm = arg_immop->GetIntImmVal();
                    LoadImmediateToFloatReg(arg_imm,GetPhysicalReg(RISCV_a0 + int_count));
                    
                } else if (arg_op->GetOperandType() == BasicOperand::GLOBAL) {
                    auto mid_reg = cur_func->GetNewReg(INT64);
                    auto arg_global = (GlobalOperand *)arg_op;
                    cur_block->push_back(
                    rvconstructor->ConstructULabel(RISCV_LUI, mid_reg, RiscVLabel(arg_global->GetName(), true)));
                    cur_block->push_back(rvconstructor->ConstructILabel(RISCV_ADDI, GetPhysicalReg(RISCV_a0 + int_count),
                                                                        mid_reg,
                                                                        RiscVLabel(arg_global->GetName(), false)));
                }

            } else {
                // 处理超出寄存器限制的参数
            }
            int_count++;
        } else if (type == FLOAT32) {
            if (float_count < 8) {
                // 处理浮点数类型参数
                 if (arg_op->GetOperandType() == BasicOperand::REG) {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_regno = arg_regop->GetRegNo();

                    //auto arg_reg = cur_func->GetNewReg(INT64);
                    if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                        ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(FLOAT64);
                    }
                    auto arg_reg = ir_riscv_regtable[arg_regno];
                    auto arg_copy_instr =
                    rvconstructor->ConstructR2(RISCV_FMV_D,GetPhysicalReg(RISCV_fa0 + float_count), arg_reg);
                    cur_block->push_back(arg_copy_instr);
                } else if (arg_op->GetOperandType() == BasicOperand::IMMF32) {
                    auto arg_immop = (ImmF32Operand *)arg_op;
                    auto arg_imm = arg_immop->GetFloatVal();
                    
                    LoadImmediateToFloatReg(arg_imm,GetPhysicalReg(RISCV_fa0 + float_count));
                   
                } else {
                    ERROR("Unexpected Operand type");
                }
            } else {
                // 处理超出寄存器限制的参数
            }
            float_count++;
        } else if (type == DOUBLE) {
            if (int_count< 8) {
                // 处理双精度浮点数参数
                 if (arg_op->GetOperandType() == BasicOperand::REG) {
                    auto arg_regop = (RegOperand *)arg_op;
                    auto arg_regno = arg_regop->GetRegNo();
                    if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                        ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(INT64);
                    }
                    auto arg_reg = ir_riscv_regtable[arg_regno];;
                    cur_block->push_back(
                    rvconstructor->ConstructR2(RISCV_FMV_X_D, GetPhysicalReg(RISCV_a0 + int_count), arg_reg));
                } else {
                    ERROR("Unexpected Operand Type");
                }
            } else {
                // 处理超出寄存器限制的参数
            }
            int_count++;
        } else {
            ERROR("Unexpected parameter type %d", type);
        }
    }

    //计算超出的参数数量
    if (int_count - 8 > 0)
        skt_count += (int_count - 8);

    if (float_count - 8 > 0)
            skt_count += (float_count - 8);

    //处理堆栈上的参数
    if (skt_count != 0) {
        // 堆栈上的参数
        int_count = float_count = 0;
        int arg_off = 0;
        // 处理堆栈中的参数
        for (auto [type, arg_op] : ins->GetParameterList()) {
            if (type == I32 || type == PTR) {
                if (int_count < 8) {
                } else {
                    if (arg_op->GetOperandType() == BasicOperand::REG) {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_regno = arg_regop->GetRegNo();
                        if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                            ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(INT64);
                        }
                        auto arg_reg = ir_riscv_regtable[arg_regno];
                        if (ir_riscv_alloca.find(arg_regop->GetRegNo()) == ir_riscv_alloca.end()) {
                            cur_block->push_back(
                            rvconstructor->ConstructSImm(RISCV_SD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                        } else {
                            auto sp_offset = ir_riscv_alloca[arg_regop->GetRegNo()];
                            auto mid_reg = cur_func->GetNewReg(INT64);

                            // 判断 sp_offset 是否在 ADDI 范围内
                            if (sp_offset >= -2048 && sp_offset <= 2047) {
                                // 如果在 ADDI 范围内，直接使用 ADDI 指令
                                cur_block->push_back(
                                    rvconstructor->ConstructIImm(RISCV_ADDI, mid_reg, GetPhysicalReg(RISCV_sp), sp_offset));
                            } else {
                                // 如果超出 ADDI 范围，使用 LUI 和 ADDI 组合
                                uint64_t imm_as_uint = static_cast<uint64_t>(sp_offset);
                                uint32_t high_20_bits = (imm_as_uint >> 12) & 0xFFFFF;  // 提取高20位
                                int32_t low_12_bits = imm_as_uint & 0xFFF;               // 提取低12位

                                // 使用 LUI 加载高20位
                                auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, mid_reg, high_20_bits);
                                cur_block->push_back(lui_instr);

                                // 使用 ADDI 加载低12位
                                auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, mid_reg, mid_reg, low_12_bits);
                                cur_block->push_back(addi_instr);
                            }

                            // 使用 mid_reg 存储的值进行存储操作
                            cur_block->push_back(
                                rvconstructor->ConstructSImm(RISCV_SD, mid_reg, GetPhysicalReg(RISCV_sp), arg_off));
                        }
                    } else if (arg_op->GetOperandType() == BasicOperand::IMMI32) {
                        auto arg_immop = (ImmI32Operand *)arg_op;
                        auto arg_imm = arg_immop->GetIntImmVal();
                        auto imm_reg = cur_func->GetNewReg(INT64);
                        LoadImmediateToFloatReg(arg_imm,imm_reg);
                        cur_block->push_back(
                        rvconstructor->ConstructSImm(RISCV_SD, imm_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } else if (arg_op->GetOperandType() == BasicOperand::GLOBAL) {
                        auto glb_reg1 = cur_func->GetNewReg(INT64);
                        auto glb_reg2 = cur_func->GetNewReg(INT64);
                        auto arg_glbop = (GlobalOperand *)arg_op;
                        cur_block->push_back(
                        rvconstructor->ConstructULabel(RISCV_LUI, glb_reg1, RiscVLabel(arg_glbop->GetName(), true)));
                        cur_block->push_back(rvconstructor->ConstructILabel(RISCV_ADDI, glb_reg2, glb_reg1,
                                                                            RiscVLabel(arg_glbop->GetName(), false)));
                        cur_block->push_back(
                        rvconstructor->ConstructSImm(RISCV_SD, glb_reg2, GetPhysicalReg(RISCV_sp), arg_off));
                    }
                    arg_off += 8;
                }
                int_count++;
            } else if (type == FLOAT32) {
                if (float_count < 8) {
                } else {
                    if (arg_op->GetOperandType() == BasicOperand::REG) {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_regno = arg_regop->GetRegNo();
                        if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                            ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(FLOAT64);
                        }
                        auto arg_reg = ir_riscv_regtable[arg_regno];
                        cur_block->push_back(
                        rvconstructor->ConstructSImm(RISCV_FSD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } else if (arg_op->GetOperandType() == BasicOperand::IMMF32) {
                        auto arg_immop = (ImmF32Operand *)arg_op;
                        auto arg_imm = arg_immop->GetFloatVal();
                        auto imm_reg = cur_func->GetNewReg(INT64);
                        LoadImmediateToFloatReg(*(int *)&arg_imm,imm_reg);
                        cur_block->push_back(
                        rvconstructor->ConstructSImm(RISCV_SD, imm_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } else {
                        ERROR("Unexpected Operand type");
                    }
                    arg_off += 8;
                }
                float_count++;
            } else if (type == DOUBLE) {
                if (int_count < 8) {
                } else {
                    if (arg_op->GetOperandType() == BasicOperand::REG) {
                        auto arg_regop = (RegOperand *)arg_op;
                        auto arg_regno = arg_regop->GetRegNo();
                        if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
                            ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(FLOAT64);
                        }
                        auto arg_reg = ir_riscv_regtable[arg_regno];
                        cur_block->push_back(
                        rvconstructor->ConstructSImm(RISCV_FSD, arg_reg, GetPhysicalReg(RISCV_sp), arg_off));
                    } else {
                        ERROR("Unexpected Operand type");
                    }
                    arg_off += 8;
                }
                int_count++;
            } else {
                ERROR("Unexpected parameter type %d", type);
            }
        }

    }

    //调用函数
    auto call_funcname = ins->GetFunctionName();
    if (int_count> 8) {
        int_count = 8;
    }
    if (float_count > 8) {
        float_count = 8;
    }
    cur_block->push_back(rvconstructor->ConstructCall(RISCV_CALL, call_funcname, int_count, float_count));
    cur_func->UpdateParaSize(skt_count * 8);
    auto return_type = ins->GetRetType();
    auto result_op = (RegOperand *)ins->GetResult();


    //返回值处理
    if (return_type == I32) {
        auto arg_regno = result_op->GetRegNo();
        if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()){
            ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(INT64);
        }
        auto arg_reg = ir_riscv_regtable[arg_regno];
        auto copy_ret_ins =
        rvconstructor->ConstructIImm(RISCV_ADDI,arg_reg, GetPhysicalReg(RISCV_a0), 0);
        cur_block->push_back(copy_ret_ins);
    } else if (return_type == FLOAT32) {
        // 处理浮点返回值
        auto arg_regno = result_op->GetRegNo();
        if (ir_riscv_regtable.find(arg_regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[arg_regno] = cur_func->GetNewReg(FLOAT64);
        }
        auto arg_reg = ir_riscv_regtable[arg_regno];
        auto copy_ret_ins = rvconstructor->ConstructR(RISCV_FSGNJ_S, arg_reg, GetPhysicalReg(RISCV_fa0), GetPhysicalReg(RISCV_fa0));
        cur_block->push_back(copy_ret_ins);
    } else if (return_type == VOID) {
    } else {
        ERROR("Unexpected return type %d", return_type);
    }


}

template <> void RiscV64Selector::ConvertAndAppend<FptosiInstruction *>(FptosiInstruction *ins) {
    //TODO("Implement this if you need");
     // 获取操作数
    auto result = ins->GetResult();
    auto value = ins->GetSrc();

    // 获取 regno
    int regno;
    if (result->GetOperandType() == BasicOperand::REG) {
        Operand op = ins->GetResult();
        RegOperand* regOp = dynamic_cast<RegOperand*>(op);
        regno = regOp->GetRegNo();
    }

    // 确保目标寄存器已经映射到 LLVM 寄存器
    if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[regno] = cur_func->GetNewReg(INT64);
    }


    // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
    Register rd = ir_riscv_regtable[regno];

    // 判断类型，执行转换
    if (value->GetOperandType() == BasicOperand::IMMF32) {
        // 对于 float 转换为 int32，
        auto opReg1 = cur_func->GetNewReg(FLOAT64);
        auto op1_Immi=(ImmF32Operand *)ins->GetSrc();
        auto op1_val = op1_Immi->GetFloatVal();
        LoadImmediateToFloatReg(op1_val,opReg1);
        //cur_block->push_back(copy_ins);
    } else if (value->GetOperandType() == BasicOperand::IMMI32) {
        // 对于int
        auto opReg1 = cur_func->GetNewReg(INT64);
        auto op1_Immi=(ImmI32Operand *)ins->GetSrc();
        auto op1_val = op1_Immi->GetIntImmVal();
        LoadImmediateToFloatReg(op1_val,opReg1);
        //cur_block->push_back(copy_ins);
    } else if (value->GetOperandType() == BasicOperand::REG)
    {
        int valregno;
        if (value->GetOperandType() == BasicOperand::REG) {
            Operand op = ins->GetSrc();
            RegOperand* regOp = dynamic_cast<RegOperand*>(op);
            valregno = regOp->GetRegNo();
        }
        // 确保目标寄存器已经映射到 LLVM 寄存器
        if (ir_riscv_regtable.find(valregno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[valregno] = cur_func->GetNewReg(FLOAT64);
        }


        // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
        Register rs = ir_riscv_regtable[valregno];

        auto fptoIns = rvconstructor->ConstructR2(RISCV_FCVT_W_S,rd,rs);
        cur_block->push_back(fptoIns);

    }
    else {
        ERROR("Unsupported operand type for FPTOSI: %d", value->GetOperandType());
        }
}

//整数类型转换为浮点数类型
template <> void RiscV64Selector::ConvertAndAppend<SitofpInstruction *>(SitofpInstruction *ins) {
    //TODO("Implement this if you need");
     // 获取操作数
    auto result = ins->GetResult();
    auto value = ins->GetSrc();

    // 获取 regno
    int regno;
    if (result->GetOperandType() == BasicOperand::REG) {
        RegOperand* regOp = dynamic_cast<RegOperand*>(result);
        regno = regOp->GetRegNo();
    }

    // 确保目标寄存器已经映射到 LLVM 寄存器
    if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[regno] = cur_func->GetNewReg(FLOAT64); // 结果应该是浮点数寄存器
    }

    // 新建结果寄存器映射，获取一个与目标寄存器相对应的 LLVM 寄存器对象
    Register rd = ir_riscv_regtable[regno];

    // 判断类型，执行转换
    if (value->GetOperandType() == BasicOperand::IMMI32) {
        // 对于 int32 转换为 float，
        auto opReg1 = cur_func->GetNewReg(INT64);
        auto op1_Immi=(ImmI32Operand *)ins->GetSrc();
        auto op1_val = op1_Immi->GetIntImmVal();
        LoadImmediateToFloatReg(op1_val,opReg1);
        //cur_block->push_back(copy_ins);

        // 将整数寄存器中的值转换为浮点数，并存储在浮点寄存器中
        auto sitofpIns = rvconstructor->ConstructR2(RISCV_FCVT_S_W, rd, opReg1);
        cur_block->push_back(sitofpIns);
    } else if (value->GetOperandType() == BasicOperand::REG) {
        int valregno;
        RegOperand* regOp = dynamic_cast<RegOperand*>(value);
        valregno = regOp->GetRegNo();

        // 确保源寄存器已经映射到 LLVM 寄存器
        if (ir_riscv_regtable.find(valregno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[valregno] = cur_func->GetNewReg(INT64); // 源是整数寄存器
        }

        // 新建源寄存器映射，获取一个与源寄存器相对应的 LLVM 寄存器对象
        Register rs = ir_riscv_regtable[valregno];

        // 将整数寄存器中的值转换为浮点数，并存储在浮点寄存器中
        auto sitofpIns = rvconstructor->ConstructR2(RISCV_FCVT_S_W, rd, rs);
        cur_block->push_back(sitofpIns);
    } else {
        ERROR("Unsupported operand type for SITOFP: %d", value->GetOperandType());
    }
}

template <> void RiscV64Selector::ConvertAndAppend<ZextInstruction *>(ZextInstruction *ins) {
    //TODO("Implement this if you need");
     // 确保源和目的都是寄存器操作数

    // 获取源寄存器编号，并确保其已被映射到LLVM寄存器
    int src_regno = ((RegOperand *)ins->GetSrc())->GetRegNo();
    if (ir_riscv_regtable.find(src_regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[src_regno] = cur_func->GetNewReg(INT32); // 假设源寄存器是32位
    }
    Register rs = ir_riscv_regtable[src_regno];

    // 获取目的寄存器编号，并确保其已被映射到LLVM寄存器
    int dst_regno = ((RegOperand *)ins->GetResult())->GetRegNo();
    if (ir_riscv_regtable.find(dst_regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[dst_regno] = cur_func->GetNewReg(INT64); // 目的寄存器应该是64位
    }
    Register rd = ir_riscv_regtable[dst_regno];

    //获取以前的比较指令
    auto cmp_inst = cmp_context[rs];

    if(cmp_inst->GetOpcode() == BasicInstruction::ICMP)
    {
        auto icmp_inst = (IcmpInstruction *)cmp_inst;
        auto cur_cond = icmp_inst->GetCond();
        auto op1 = icmp_inst->GetOp1();
        auto op2 = icmp_inst->GetOp2();
        if (op1->GetOperandType() == BasicOperand::IMMI32) {
            HandleImmediateOperand(op1, op2, cur_cond, rd);
            return;
        }

        if (op2->GetOperandType() == BasicOperand::IMMI32) {
            HandleImmediateOperandWithReg(op1, op2, cur_cond, rd);
            return;
        }

        HandleRegisterOperands(op1, op2, cur_cond, rd);
    }
    else if(cmp_inst->GetOpcode() == BasicInstruction::FCMP)
    {
        auto fcmp_inst = (FcmpInstruction *)cmp_inst;
        auto cur_cond = fcmp_inst->GetCond();
        auto op1 = fcmp_inst->GetOp1();
        auto op2 = fcmp_inst->GetOp2();
        Register rg_op1,rg_op2;
        if(op1->GetOperandType() == BasicOperand::REG)
        {
            int op1_regno = ((RegOperand *)op1)->GetRegNo();
            if (ir_riscv_regtable.find(op1_regno) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[op1_regno] = cur_func->GetNewReg(FLOAT64); 
            }
            rg_op1 = ir_riscv_regtable[op1_regno];
        }else if(op1->GetOperandType() == BasicOperand::IMMF32)
        {
            rg_op1 = cur_func->GetNewReg(FLOAT64);
            auto val_op1 = (ImmF32Operand *)fcmp_inst->GetOp1();
            auto val = val_op1->GetFloatVal();
            LoadImmediateToFloatReg( val,rg_op1);
            //cur_block->push_back(op1_inst);
        }
        else{
            ERROR("Unexpected FCMP op1 type");
        }

        if(op2->GetOperandType() == BasicOperand::REG)
        {
            int op2_regno = ((RegOperand *)op2)->GetRegNo();
            if (ir_riscv_regtable.find(op2_regno) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[op2_regno] = cur_func->GetNewReg(FLOAT64); 
            }
            rg_op2 = ir_riscv_regtable[op2_regno];
        }else if(op2->GetOperandType() == BasicOperand::IMMF32)
        {
            rg_op2 = cur_func->GetNewReg(FLOAT64);
            auto val_op2 = (ImmF32Operand *)fcmp_inst->GetOp1();
            auto val = val_op2->GetFloatVal();
            LoadImmediateToFloatReg( val,rg_op2);
        }
        else{
            ERROR("Unexpected FCMP op2 type");
        }

        switch(cur_cond)
        {
            case BasicInstruction::FcmpCond::OEQ:
            case BasicInstruction::FcmpCond::UEQ:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, rd, rg_op1, rg_op2));
                break;
            case BasicInstruction::FcmpCond::OGT:
            case BasicInstruction::FcmpCond::UGT:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, rd, rg_op2, rg_op1));
            break;
            case BasicInstruction::FcmpCond::OGE:
            case BasicInstruction::FcmpCond::UGE:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, rd, rg_op2, rg_op1));
                break;
            case BasicInstruction::FcmpCond::OLT:
            case BasicInstruction::FcmpCond::ULT:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FLT_S, rd, rg_op1, rg_op2));
                break;
            case BasicInstruction::FcmpCond::OLE:
            case BasicInstruction::FcmpCond::ULE:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FLE_S, rd, rg_op1, rg_op2));
                break;
            case BasicInstruction::FcmpCond::ONE:
            case BasicInstruction::FcmpCond::UNE:
                cur_block->push_back(rvconstructor->ConstructR(RISCV_FEQ_S, rd, rg_op1, rg_op2));
                break;
            case BasicInstruction::FcmpCond::ORD:
            case BasicInstruction::FcmpCond::UNO:
            case BasicInstruction::FcmpCond::TRUE:
            case BasicInstruction::FcmpCond::FALSE:
            default:
                ERROR("Unexpected FCMP cond");
            
        }
    }   
}

//处理立即数与寄存器的比较
void RiscV64Selector::HandleImmediateOperand(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg) {
    auto t = op1;
    op1 = op2;
    op2 = t;
     switch (cur_cond) {
        // 如果原先是a > b，则交换后变成b < a，所以将sgt改为slt
        case BasicInstruction::IcmpCond::sgt: cur_cond = BasicInstruction::IcmpCond::slt; break;
        // 如果原先是a >= b，则交换后变成b <= a，所以将sge改为sle
        case BasicInstruction::IcmpCond::sge: cur_cond = BasicInstruction::IcmpCond::sle; break;
        // 如果原先是a < b，则交换后变成b > a，所以将slt改为sgt
        case BasicInstruction::IcmpCond::slt: cur_cond = BasicInstruction::IcmpCond::sgt; break;
        // 如果原先是a <= b，则交换后变成b >= a，所以将sle改为sge
        case BasicInstruction::IcmpCond::sle: cur_cond = BasicInstruction::IcmpCond::sge; break;
        case BasicInstruction::IcmpCond::eq:// 等于条件不需要改变，因为a == b 和 b == a 是等价的
        case BasicInstruction::IcmpCond::ne:// 不等于条件同样不需要改变，因为a != b 和 b != a 是等价的
            break;
        case BasicInstruction::IcmpCond::ugt:
        case BasicInstruction::IcmpCond::uge:
        case BasicInstruction::IcmpCond::ult:
        case BasicInstruction::IcmpCond::ule:
            ERROR("Unexpected ICMP cond");
    }

    if (op1->GetOperandType() == BasicOperand::IMMI32) {
        Assert(op2->GetOperandType() == BasicOperand::IMMI32);
        int rval = CompareImmediateOperands((ImmI32Operand *)op1, (ImmI32Operand *)op2, cur_cond);
        LoadImmediateToFloatReg( rval,result_reg);
        //cur_block->push_back(rvconstructor->ConstructCopyRegImmI(result_reg, rval, INT64));
    }
}



int RiscV64Selector::CompareImmediateOperands(ImmI32Operand *op1, ImmI32Operand *op2, BasicInstruction::IcmpCond cur_cond) {
    int op1_val = op1->GetIntImmVal();
    int op2_val = op2->GetIntImmVal();
    int rval = 0;
    switch (cur_cond) {
        case BasicInstruction::IcmpCond::eq: rval = (op1_val == op2_val); break;
        case BasicInstruction::IcmpCond::ne: rval = (op1_val != op2_val); break;
        case BasicInstruction::IcmpCond::sgt: rval = (op1_val > op2_val); break;
        case BasicInstruction::IcmpCond::sge: rval = (op1_val >= op2_val); break;
        case BasicInstruction::IcmpCond::slt: rval = (op1_val < op2_val); break;
        case BasicInstruction::IcmpCond::sle: rval = (op1_val <= op2_val); break;
        case BasicInstruction::IcmpCond::ugt:
        case BasicInstruction::IcmpCond::uge:
        case BasicInstruction::IcmpCond::ult:
        case BasicInstruction::IcmpCond::ule:
            ERROR("Unexpected ICMP cond");
    }
}
     

//处理寄存器与立即数的比较
void RiscV64Selector::HandleImmediateOperandWithReg(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg) {
    int op1_regno = ((RegOperand *)op1)->GetRegNo();
    if (ir_riscv_regtable.find(op1_regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[op1_regno] = cur_func->GetNewReg(INT64); 
    }
    Register op1_reg = ir_riscv_regtable[op1_regno];

    if (((ImmI32Operand *)op2)->GetIntImmVal() == 0) {
        HandleZeroImmediate(op1_reg, cur_cond, result_reg);
    } else if (cur_cond == BasicInstruction::IcmpCond::slt) {
        int op2_imm = ((ImmI32Operand *)op2)->GetIntImmVal();
        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTI, result_reg, op1_reg, op2_imm));
        return;
    } else if (cur_cond == BasicInstruction::IcmpCond::ult) {
        int op2_imm = ((ImmI32Operand *)op2)->GetIntImmVal();
        cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, op1_reg, op2_imm));
        return;
    }
    Register op2_reg = cur_func->GetNewReg(INT64);
    LoadImmediateToFloatReg( ((ImmI32Operand *)op2)->GetIntImmVal(),op2_reg);
    //cur_block->push_back(
    //rvconstructor->ConstructCopyRegImmI(op2_reg, ((ImmI32Operand *)op2)->GetIntImmVal(), INT64));
}

void RiscV64Selector::HandleZeroImmediate(Register op1_reg, BasicInstruction::IcmpCond &cur_cond, Register result_reg) {
    auto not_reg = cur_func->GetNewReg(INT64);
    switch (cur_cond) {
        case BasicInstruction::IcmpCond::eq:
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, op1_reg, 1));
            return;
        case BasicInstruction::IcmpCond::ne:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLTU, result_reg, GetPhysicalReg(RISCV_x0), op1_reg));
            return;
        case BasicInstruction::IcmpCond::sgt:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, GetPhysicalReg(RISCV_x0), op1_reg));
            return;
        case BasicInstruction::IcmpCond::sge:    // sgez ~ not sltz
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, not_reg, op1_reg, GetPhysicalReg(RISCV_x0)));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, not_reg, 1));
            return;
        case BasicInstruction::IcmpCond::slt:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, op1_reg, GetPhysicalReg(RISCV_x0)));
            return;
        case BasicInstruction::IcmpCond::sle:    // slez ~ not sgtz
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, not_reg, GetPhysicalReg(RISCV_x0), op1_reg));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, not_reg, 1));
            return;
        default:
            ERROR("Unexpected ICMP cond");
    }
}

//处理寄存器与寄存器的比较
void RiscV64Selector::HandleRegisterOperands(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg) {
    int op1_regno = ((RegOperand *)op1)->GetRegNo();
    if (ir_riscv_regtable.find(op1_regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[op1_regno] = cur_func->GetNewReg(INT64); 
    }
    Register reg_1 = ir_riscv_regtable[op1_regno];
    Register reg_2;

    if(op2->GetOperandType() == BasicOperand::REG)
    {
        int op2_regno = ((RegOperand *)op2)->GetRegNo();
        if (ir_riscv_regtable.find(op2_regno) == ir_riscv_regtable.end()) {
            ir_riscv_regtable[op2_regno] = cur_func->GetNewReg(INT64); 
        }
        reg_2 = ir_riscv_regtable[op2_regno];
    }else
        {
            reg_2 = cur_func->GetNewReg(INT64);
        }


    auto mid_reg = cur_func->GetNewReg(INT64);
    switch (cur_cond) {
        case BasicInstruction::IcmpCond::eq:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SUBW, mid_reg, reg_1, reg_2));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_SLTIU, result_reg, mid_reg, 1));
            return;
        case BasicInstruction::IcmpCond::ne:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SUBW, mid_reg, reg_1, reg_2));
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLTU, result_reg, GetPhysicalReg(RISCV_x0), mid_reg));
            return;
        case BasicInstruction::IcmpCond::sgt:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, reg_2, reg_1));
            return;
        case BasicInstruction::IcmpCond::sge:    // reg_1 >= reg_2 <==> not reg_1 < reg_2
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, mid_reg, reg_1, reg_2));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, mid_reg, 1));
            return;
        case BasicInstruction::IcmpCond::slt:
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, result_reg, reg_1, reg_2));
            return;
        case BasicInstruction::IcmpCond::sle:    // 2 < 1  2 >= 1 1 <= 2
            cur_block->push_back(rvconstructor->ConstructR(RISCV_SLT, mid_reg, reg_2, reg_1));
            cur_block->push_back(rvconstructor->ConstructIImm(RISCV_XORI, result_reg, mid_reg, 1));
            return;
        default:
            ERROR("Unexpected ICMP cond");
    }
}

//计算指针偏移量以访问数组或结构体中的元素（获取元素指针）
template <> void RiscV64Selector::ConvertAndAppend<GetElementptrInstruction *>(GetElementptrInstruction *ins) {
    //TODO("Implement this if you need");
    //获取指令的基础指针和结果寄存器
    auto global_operand = (GlobalOperand *)ins->GetPtrVal();
    auto result_operand = (RegOperand *)ins->GetResult();
    int result_regno = result_operand->GetRegNo();
    //目标寄存器 存储最终指针地址
    if (ir_riscv_regtable.find(result_regno) == ir_riscv_regtable.end()) {
        ir_riscv_regtable[result_regno] = cur_func->GetNewReg(INT64); 
    }
    auto result_reg = ir_riscv_regtable[result_regno];

    //存储偏移量寄存器
    auto offset_reg = cur_func->GetNewReg(INT64);

    //计算所有维度乘积，用于后续索引计算
    int mul = 1;
    for (auto d : ins->GetDims()){
        mul *= d;
    }

    int offset = 0;
    //标志位标记
    int sign_offset_reg = 0;

    //立即数标志
    bool imm_sign = false;

    //处理索引
    for (int i = 0 ; i < ins->GetIndexes().size(); i++){
        //如果当前索引是立即数
        if (ins->GetIndexes()[i]->GetOperandType() == BasicOperand::IMMI32){
            auto cur_index = (ImmI32Operand *)ins->GetIndexes()[i];
            auto cur_index_val = cur_index->GetIntImmVal();
            offset += cur_index_val * mul;
            //如果不是第一次分配
            if (sign_offset_reg == 1){
                // 将 offset 累加到当前偏移寄存器中
                auto temp_reg = cur_func->GetNewReg(INT64);

                // 如果 offset 在 ADDI 的范围内，直接使用 ADDI
                if (offset >= -2048 && offset <= 2047) {
                    // 使用 ADDI 指令将 offset 加到 offset_reg 上
                    auto temp_ins = rvconstructor->ConstructIImm(RISCV_ADDI, temp_reg, offset_reg, offset);
                    cur_block->push_back(temp_ins);
                } else {
                    // 如果 offset 超出了 ADDI 范围，使用 LUI 和 ADDI 组合
                    uint64_t imm_as_uint = static_cast<uint64_t>(offset);

                    // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                    int lui_val = (imm_as_uint >> 12) & 0xFFFFF; // 提取高 20 位
                    int addi_val = imm_as_uint & 0xFFF;          // 提取低 12 位

                    // 使用临时寄存器
                    Register temp1_reg = GetPhysicalReg(RISCV_t0);

                    // 使用 LUI 指令加载高 20 位
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, temp_reg, lui_val);
                    cur_block->push_back(lui_instr);

                    // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                    if (addi_val < -2048 || addi_val > 2047) {
                        // 如果 ADDI 的立即数部分超出范围，进一步处理
                        // 分解成两个 ADDI 操作
                        int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                        int addi_part2 = addi_val - addi_part1;         // 剩余部分

                        // 使用 ADDI 指令添加第一部分立即数
                        auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part1);
                        cur_block->push_back(addi_instr1);

                        // 使用 ADDI 指令添加第二部分立即数
                        auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part2);
                        cur_block->push_back(addi_instr2);
                    }
                }

                // 将计算结果存回 offset_reg
                offset_reg = temp_reg;
            }
            else if (sign_offset_reg == 0){
                //首次分配寄存器，直接加载offset*4到offset_reg
                sign_offset_reg = 1;
                //auto reg_ins = rvconstructor->ConstructCopyRegImmI(offset_reg, offset * 4, INT64);
                //cur_block->push_back(reg_ins);
                LoadImmediateToFloatReg(offset * 4,offset_reg);
                imm_sign = true;
            }
        }
        else{
            //如果当前索引是寄存器
            //获取索引寄存器
            auto index_operand = (RegOperand *)ins->GetIndexes()[i];
            auto index_regno = index_operand->GetRegNo();
            if (ir_riscv_regtable.find(index_regno) == ir_riscv_regtable.end()) {
                ir_riscv_regtable[index_regno] = cur_func->GetNewReg(INT64); 
            }
            auto index_reg = ir_riscv_regtable[index_regno];
            //如果乘积不等于1,表示当前维度需要进行乘法运算来计算正确的偏移量
            //if (mul != 1){
                //构造一条指令将mul值加载到mul_reg
                auto mul_reg = cur_func->GetNewReg(INT64);
                //auto mul_ins = rvconstructor->ConstructCopyRegImmI(mul_reg, mul, INT64);
                //cur_block->push_back(mul_ins);
                LoadImmediateToFloatReg(mul,mul_reg);

                //构造一条指令，临时寄存器存储本次计算结果
                auto cur_reg = cur_func->GetNewReg(INT64);
                auto cur_ins = rvconstructor->ConstructR(RISCV_MUL, cur_reg, index_reg, mul_reg);
                cur_block->push_back(cur_ins);

                //如果不是第一次分配偏移量寄存器
                if (sign_offset_reg == 1){
                    //获取一个临时寄存器
                    auto temp_reg = cur_func->GetNewReg(INT64);
                    //将当前存储偏移量寄存器和本次计算结果
                    auto temp_ins = rvconstructor->ConstructR(RISCV_ADD, temp_reg, offset_reg, cur_reg);
                    cur_block->push_back(temp_ins);
                    //更新offset_reg，用于后续使用新的累积值
                    offset_reg = temp_reg;
                }//如果是第一次分配
                else if(sign_offset_reg == 0){
                    sign_offset_reg = 1;
                    //将临时寄存器（本次计算结果）的值复制到存储偏移量寄存器中
                    auto offset_ins = rvconstructor->ConstructIImm(RISCV_ADDI,offset_reg, cur_reg, 0);
                    cur_block->push_back(offset_ins);
                   
                }
        }
        if (i < ins->GetDims().size()) {
            mul /= ins->GetDims()[i];
        }
    }

    //指针值为全局变量
    if (ins->GetPtrVal()->GetOperandType() == BasicOperand::GLOBAL){
        //已经分配了偏移寄存器
        if (sign_offset_reg == 1){
            //构造加载高20位地址LUI指令
            auto hi_reg = cur_func->GetNewReg(INT64);
            auto hiLabel = RiscVLabel(global_operand->GetName(), true);
            auto hi_ins = rvconstructor->ConstructULabel(RISCV_LUI, hi_reg, hiLabel);
            cur_block->push_back(hi_ins);

            //构造加载低12位地址的ADDI指令
            auto base_reg = cur_func->GetNewReg(INT64);
            auto lowLabel = RiscVLabel(global_operand->GetName(), false);
            auto base_ins = rvconstructor->ConstructILabel(RISCV_ADDI, base_reg, hi_reg, lowLabel);
            cur_block->push_back(base_ins);

            //构造左移指令使偏移量乘4,如果都是立即数则不需要乘4
            auto offsetall_reg = cur_func->GetNewReg(INT64);
            if (imm_sign){
                offsetall_reg = offset_reg;
            }
            else{
                auto sll_ins = rvconstructor->ConstructIImm(RISCV_SLLI, offsetall_reg, offset_reg, 2);
                cur_block->push_back(sll_ins);
            }

            //全局变量基地址与偏移量相加,得到最终目标地址
            auto target_ins = rvconstructor->ConstructR(RISCV_ADD, result_reg, base_reg, offsetall_reg);
            cur_block->push_back(target_ins);
        }//没有分配偏移寄存器
        else if (sign_offset_reg == 0){
            auto resulthi_reg = cur_func->GetNewReg(INT64);
            auto hilabel = RiscVLabel(global_operand->GetName(), true);

            //加载全局变量基地址高20位lui指令
            auto lui_ins = rvconstructor->ConstructULabel(RISCV_LUI, resulthi_reg, hilabel);
            cur_block->push_back(lui_ins);

            //加载全局变量基地址低12位ADDI指令
            auto lowlabel = RiscVLabel(global_operand->GetName(), false);
            auto addi_ins = rvconstructor->ConstructILabel(RISCV_ADDI, result_reg, resulthi_reg, lowlabel);
            cur_block->push_back(addi_ins);
        }
    }
    else if (ins->GetPtrVal()->GetOperandType() == BasicOperand::REG){
        auto ptr_operand = (RegOperand *)ins->GetPtrVal();
        auto ptr_regno = ptr_operand->GetRegNo();
        auto offsetall_reg = cur_func->GetNewReg(INT64);
        //已经分配了偏移寄存器
        if (sign_offset_reg == 1){
            if (imm_sign){
                offsetall_reg = offset_reg;
            }
            auto sll_ins = rvconstructor->ConstructIImm(RISCV_SLLI, offsetall_reg, offset_reg, 2);

            //区分指针操作数是否是局部变量
            if (ir_riscv_alloca.find(ptr_regno) == ir_riscv_alloca.end()){
                //指针操作数不是通过alloca分配的局部变量
                //添加左移指令
                if (!imm_sign){
                    cur_block->push_back(sll_ins);
                }
                //获取指针操作数对应的寄存器
                if (ir_riscv_regtable.find(ptr_regno) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[ptr_regno] = cur_func->GetNewReg(INT64); 
                }
                auto ptr_reg = ir_riscv_regtable[ptr_regno];

                //构造加法指令
                auto add_ins = rvconstructor->ConstructR(RISCV_ADD, result_reg, ptr_reg, offsetall_reg);
                cur_block->push_back(add_ins);
            }
            else{
                //指针操作数是通过alloca分配的局部变量
                //获取栈帧偏移量
                auto sp_offset = ir_riscv_alloca[ptr_regno];
                auto ptr_reg = cur_func->GetNewReg(INT64);

                // 如果 sp_offset 在 ADDI 可表示的范围内
                if (sp_offset >= -2048 && sp_offset <= 2047) {
                    // 构造加载基础地址的 ADDI 指令
                    auto addi_ins = rvconstructor->ConstructIImm(RISCV_ADDI, ptr_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                    ((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_ins);
                    cur_block->push_back(addi_ins);
                } else {
                    // 如果 sp_offset 超出了 ADDI 范围，使用 LUI 和 ADDI 组合
                    uint64_t imm_as_uint = static_cast<uint64_t>(sp_offset);

                    // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                    int lui_val = (imm_as_uint >> 12) & 0xFFFFF; // 提取高 20 位
                    int addi_val = imm_as_uint & 0xFFF;          // 提取低 12 位

                    // 使用临时寄存器
                    Register temp1_reg = GetPhysicalReg(RISCV_t0);

                    // 使用 LUI 指令加载高 20 位
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, ptr_reg, lui_val);
                    cur_block->push_back(lui_instr);

                    // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                    if (addi_val < -2048 || addi_val > 2047) {
                        // 如果 ADDI 的立即数部分超出范围，进一步处理
                        // 分解成两个 ADDI 操作
                        int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                        int addi_part2 = addi_val - addi_part1;         // 剩余部分

                        // 使用 ADDI 指令添加第一部分立即数
                        auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part1);
                        cur_block->push_back(addi_instr1);

                        // 使用 ADDI 指令添加第二部分立即数
                        auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part2);
                        cur_block->push_back(addi_instr2);
                    }
                }

                //添加左移指令
                if (!imm_sign){
                    cur_block->push_back(sll_ins);
                }

                //构造加法指令
                auto add_ins = rvconstructor->ConstructR(RISCV_ADD, result_reg, ptr_reg, offsetall_reg);
                cur_block->push_back(add_ins);
            }
        }//没有分配偏移寄存器
        else if (sign_offset_reg == 0){
            if (ir_riscv_alloca.find(ptr_regno) == ir_riscv_alloca.end()){
                //指针操作数不是通过alloca分配的局部变量
                //获取指针操作数对应的寄存器
                if (ir_riscv_regtable.find(ptr_regno) == ir_riscv_regtable.end()) {
                    ir_riscv_regtable[ptr_regno] = cur_func->GetNewReg(INT64); 
                }
                auto ptr_reg = ir_riscv_regtable[ptr_regno];
                //将 ptr_op 对应的寄存器内容复制到 result_reg 中
                auto reg_ins = rvconstructor->ConstructIImm(RISCV_ADDI,result_reg, ptr_reg, 0);
                cur_block->push_back(reg_ins);
            }
            else{
                //指针操作数是通过alloca分配的局部变量
               auto sp_offset = ir_riscv_alloca[ptr_regno];
                auto result_reg = cur_func->GetNewReg(INT64);

                // 如果 sp_offset 在 ADDI 可表示的范围内
                if (sp_offset >= -2048 && sp_offset <= 2047) {
                    // 构造加载基础地址的 ADDI 指令
                    auto addi_ins = rvconstructor->ConstructIImm(RISCV_ADDI, result_reg, GetPhysicalReg(RISCV_sp), sp_offset);
                    ((RiscV64Function *)cur_func)->addlist_alloca_ins(addi_ins);
                    cur_block->push_back(addi_ins);
                } else {
                    // 如果 sp_offset 超出了 ADDI 范围，使用 LUI 和 ADDI 组合
                    uint64_t imm_as_uint = static_cast<uint64_t>(sp_offset);

                    // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
                    int lui_val = (imm_as_uint >> 12) & 0xFFFFF; // 提取高 20 位
                    int addi_val = imm_as_uint & 0xFFF;          // 提取低 12 位

                    // 使用临时寄存器
                    Register temp1_reg = GetPhysicalReg(RISCV_t0);

                    // 使用 LUI 指令加载高 20 位
                    auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, result_reg, lui_val);
                    cur_block->push_back(lui_instr);

                    // 再次判断 ADDI 是否超出范围，若超出，再次拆分
                    if (addi_val < -2048 || addi_val > 2047) {
                        // 如果 ADDI 的立即数部分超出范围，进一步处理
                        // 分解成两个 ADDI 操作
                        int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
                        int addi_part2 = addi_val - addi_part1;         // 剩余部分

                        // 使用 ADDI 指令添加第一部分立即数
                        auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part1);
                        cur_block->push_back(addi_instr1);

                        // 使用 ADDI 指令添加第二部分立即数
                        auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part2);
                        cur_block->push_back(addi_instr2);
                    }
                }

            }
        }
    }

}

template <> void RiscV64Selector::ConvertAndAppend<PhiInstruction *>(PhiInstruction *ins) {
    //TODO("Implement this if you need");
    //获取并处理结果操作数
    auto result_operand = (RegOperand *)ins->GetResult();
    auto result_regno = result_operand->GetRegNo();
    Register result_reg;
    if (ins->GetType() == I32 || ins->GetType() == PTR) {
        if (ir_riscv_regtable.find(result_regno) == ir_riscv_regtable.end()){
            ir_riscv_regtable[result_regno] = cur_func->GetNewReg(INT64);
        }
        result_reg = ir_riscv_regtable[result_regno];
    } else if (ins->GetType() == FLOAT32) {
        if (ir_riscv_regtable.find(result_regno) == ir_riscv_regtable.end()){
            ir_riscv_regtable[result_regno] = cur_func->GetNewReg(FLOAT64);
        }
        result_reg = ir_riscv_regtable[result_regno];
    }
    //创建新的机器phi指令
    auto riscv_phi = new MachinePhiInstruction(result_reg);
    for (auto [label, value]:ins->GetPhiList()){
        auto label_operand = (LabelOperand *)label;
        auto label_no = label_operand->GetLabelNo();
        MachineDataType op;
        if (ins->GetType() == I32 || ins->GetType() == PTR){
            op = INT64;
        }
        else if (ins->GetType() == FLOAT32){
            op = FLOAT64;
        }
        //寄存器
        if (value->GetOperandType() == BasicOperand::REG){
            auto reg_operand = (RegOperand *)value;
            auto regno = reg_operand->GetRegNo();
            if (ir_riscv_regtable.find(regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[regno] = cur_func->GetNewReg(op);
            }
            auto value_reg = ir_riscv_regtable[regno];
            //将寄存器编号添加到m_phi的phi列表中
            riscv_phi->pushPhiList(label_no, value_reg);
        }
        else if (value->GetOperandType() == BasicOperand::IMMI32){
            auto int_imm_op = (ImmI32Operand *)value;
            auto int_imm_op_val = int_imm_op->GetIntImmVal();
            riscv_phi->pushPhiList(label_no, int_imm_op_val);
        }
        else if (value->GetOperandType() == BasicOperand::IMMF32){
            auto float_imm_op = (ImmF32Operand *)value;
            auto float_imm_op_val = float_imm_op->GetFloatVal();
            riscv_phi->pushPhiList(label_no, float_imm_op_val);
        }
    }
    //插入phi指令
    cur_block->push_back(riscv_phi);
}

template <> void RiscV64Selector::ConvertAndAppend<Instruction>(Instruction inst) {
    switch (inst->GetOpcode()) {
    case BasicInstruction::LOAD:
        ConvertAndAppend<LoadInstruction *>((LoadInstruction *)inst);
        break;
    case BasicInstruction::STORE:
        ConvertAndAppend<StoreInstruction *>((StoreInstruction *)inst);
        break;
    case BasicInstruction::ADD:
    case BasicInstruction::SUB:
    case BasicInstruction::MUL:
    case BasicInstruction::DIV:
    case BasicInstruction::FADD:
    case BasicInstruction::FSUB:
    case BasicInstruction::FMUL:
    case BasicInstruction::FDIV:
    case BasicInstruction::MOD:
    case BasicInstruction::SHL:
    case BasicInstruction::BITXOR:
        ConvertAndAppend<ArithmeticInstruction *>((ArithmeticInstruction *)inst);
        break;
    case BasicInstruction::ICMP:
        ConvertAndAppend<IcmpInstruction *>((IcmpInstruction *)inst);
        break;
    case BasicInstruction::FCMP:
        ConvertAndAppend<FcmpInstruction *>((FcmpInstruction *)inst);
        break;
    case BasicInstruction::ALLOCA:
        ConvertAndAppend<AllocaInstruction *>((AllocaInstruction *)inst);
        break;
    case BasicInstruction::BR_COND:
        ConvertAndAppend<BrCondInstruction *>((BrCondInstruction *)inst);
        break;
    case BasicInstruction::BR_UNCOND:
        ConvertAndAppend<BrUncondInstruction *>((BrUncondInstruction *)inst);
        break;
    case BasicInstruction::RET:
        ConvertAndAppend<RetInstruction *>((RetInstruction *)inst);
        break;
    case BasicInstruction::ZEXT:
        ConvertAndAppend<ZextInstruction *>((ZextInstruction *)inst);
        break;
    case BasicInstruction::FPTOSI:
        ConvertAndAppend<FptosiInstruction *>((FptosiInstruction *)inst);
        break;
    case BasicInstruction::SITOFP:
        ConvertAndAppend<SitofpInstruction *>((SitofpInstruction *)inst);
        break;
    case BasicInstruction::GETELEMENTPTR:
        ConvertAndAppend<GetElementptrInstruction *>((GetElementptrInstruction *)inst);
        break;
    case BasicInstruction::CALL:
        ConvertAndAppend<CallInstruction *>((CallInstruction *)inst);
        break;
    case BasicInstruction::PHI:
        ConvertAndAppend<PhiInstruction *>((PhiInstruction *)inst);
        break;
    default:
        ERROR("Unknown LLVM IR instruction");
    }
}

void RiscV64Selector::SelectInstructionAndBuildCFG() {
    // 与中间代码生成一样, 如果你完全无从下手, 可以先看看输出是怎么写的
    // 即riscv64gc/instruction_print/*  common/machine_passes/machine_printer.h

    // 指令选择除了一些函数调用约定必须遵守的情况需要物理寄存器，其余情况必须均为虚拟寄存器
    dest->global_def = IR->global_def;
    // 遍历每个LLVM IR函数
    for (auto [defI,cfg] : IR->llvm_cfg) {
        if(cfg == nullptr){
            ERROR("LLVMIR CFG is Empty, you should implement BuildCFG in MidEnd first");
        }
        //为每个函数创建一个RiscV64Function实例，并设置其父节点为dest。
        //然后将其添加到目标函数列表中。同时为当前函数创建一个新的机器控制流图（MachineCFG）并关联给cur_func
        std::string name = cfg->function_def->GetFunctionName();

        cur_func = new RiscV64Function(name);
        cur_func->SetParent(dest);
        // 你可以使用cur_func->GetNewRegister来获取新的虚拟寄存器
        dest->functions.push_back(cur_func);

        auto cur_mcfg = new MachineCFG;
        cur_func->SetMachineCFG(cur_mcfg);


        // 清空指令选择状态(可能需要自行添加初始化操作)
        ClearFunctionSelectState();

        // TODO: 添加函数参数(推荐先阅读一下riscv64_lowerframe.cc中的代码和注释)
        // See MachineFunction::AddParameter()
        //TODO("Add function parameter if you need");
        
        for (int i = 0; i < defI->formals.size(); i++) {
            MachineDataType type;
            if (defI->formals[i] == LLVMType::I32 || defI->formals[i] == LLVMType::PTR) {
                type = INT64;
            } else if (defI->formals[i] == LLVMType::FLOAT32) {
                type = FLOAT64;
            } else {
                ERROR("Unknown llvm type");
            }
            auto formal_reg = (RegOperand *)defI->formals_reg[i];
            auto formal_regno = formal_reg->GetRegNo();
            if (ir_riscv_regtable.find(formal_regno) == ir_riscv_regtable.end()){
                ir_riscv_regtable[formal_regno] = cur_func->GetNewReg(type);
            }
            auto result_reg = ir_riscv_regtable[formal_regno];
            cur_func->AddParameter(result_reg);
        }

        // 遍历每个LLVM IR基本块
        //函数中的每一个基本块，创建一个RiscV64Block实例，并将其添加到当前函数的机器控制流图中
        for (auto [id, block] : *(cfg->block_map)) {
            cur_block = new RiscV64Block(id);
            // 将新块添加到Machine CFG中
            cur_mcfg->AssignEmptyNode(id, cur_block);
            cur_func->UpdateMaxLabel(id);

            cur_block->setParent(cur_func);
            cur_func->blocks.push_back(cur_block);

            // 指令选择主要函数, 请注意指令选择时需要维护变量cur_offset
            // 指令选择，并将其转换为对应的目标机器指令，同时维护当前偏移量cur_offset
            for (auto instruction : block->Instruction_list) {
                // Log("Selecting Instruction");
                ConvertAndAppend<Instruction>(instruction);
            }
        }

        // RISCV 8字节对齐（）
        if (cur_offset % 8 != 0) {
            cur_offset = ((cur_offset + 7) / 8) * 8;
        }
        cur_func->SetStackSize(cur_offset + cur_func->GetParaSize());

        // 控制流图连边
        // 根据原始控制流图中的边信息，在机器控制流图中建立相应的边连接，以保持两个图的一致性
        for (auto [i, block] : *(cfg->block_map))  {
            const auto &arcs = cfg->G[i];
            for (auto arc : arcs) {
                cur_mcfg->MakeEdge(i, arc->block_id);
            }
        }
    }
}

void RiscV64Selector::ClearFunctionSelectState() { 
    cur_offset = 0; 
    ir_riscv_regtable.clear();
    ir_riscv_alloca.clear();
    cmp_context.clear();
}


// 函数用于将立即数加载到浮点寄存器中
void RiscV64Selector::LoadImmediateToFloatReg( uint32_t imm_val, Register dest_reg) {
    // 创建一个临时整数寄存器来存储浮点数的比特模式

    if (imm_val >= -2048 && imm_val <= 2047) {
        // 如果立即数在 ADDIW 可表示范围内，直接使用 ADDIW 指令
        auto addiw_instr = rvconstructor->ConstructIImm(RISCV_ADDIW, dest_reg, GetPhysicalReg(RISCV_x0), static_cast<int32_t>(imm_val));
        cur_block->push_back(addiw_instr);
    } else {
        // // 否则，分两步加载：高20位用LUI，低12位用ADDI
        // uint32_t high_20_bits = (imm_val >> 12) & 0xFFFFF;
        // int32_t low_12_bits = imm_val & 0xFFF;

        // // 使用LUI加载高20位
        // auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, dest_reg, high_20_bits);
        // cur_block->push_back(lui_instr);

        // // 使用ADDI加载低12位
        // auto addi_instr = rvconstructor->ConstructIImm(RISCV_ADDI, dest_reg, dest_reg, low_12_bits);
        // cur_block->push_back(addi_instr);

        // 如果立即数超出 ADDIW 范围，使用 LUI 和 ADDI 指令组合
        int lui_val = (imm_val >> 12) & 0xFFFFF; // 提取高 20 位
        int addi_val = imm_val & 0xFFF;          // 提取低 12 位

        // 使用临时寄存器
        Register temp1_reg = GetPhysicalReg(RISCV_t0);

        // 使用 LUI 指令加载高 20 位
        auto lui_instr = rvconstructor->ConstructUImm(RISCV_LUI, dest_reg, lui_val);
        cur_block->push_back(lui_instr);

        // 再次判断 ADDI 是否超出范围，若超出，再次拆分
        if (addi_val < -2048 || addi_val > 2047) {
            // 如果 ADDI 的立即数部分超出范围，进一步处理
            // 分解成两个 ADDI 操作
            int addi_part1 = addi_val >= 0 ? 2047 : -2048; // 第一部分立即数
            int addi_part2 = addi_val - addi_part1;         // 剩余部分

            // 使用 ADDI 指令添加第一部分立即数
            auto addi_instr1 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part1);
            cur_block->push_back(addi_instr1);

            // 使用 ADDI 指令添加第二部分立即数
            auto addi_instr2 = rvconstructor->ConstructIImm(RISCV_ADDI, temp1_reg, temp1_reg, addi_part2);
            cur_block->push_back(addi_instr2);
        }
    }

}