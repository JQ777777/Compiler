#ifndef RISCV64_INSTSELECT_H
#define RISCV64_INSTSELECT_H
#include "../../common/machine_passes/machine_selector.h"
#include "../riscv64.h"
class RiscV64Selector : public MachineSelector {
private:
    int cur_offset;    // 局部变量在栈中的偏移
    // 你需要保证在每个函数的指令选择结束后, cur_offset的值为局部变量所占栈空间的大小
    
    // TODO(): 添加更多你需要的成员变量和函数
    std::map<int, Register> ir_riscv_regtable;//ir寄存器到目的寄存器映射
    std::map<int, int> ir_riscv_alloca;//ir寄存器编号到栈上偏移量的映射
    std::map<Register, Instruction> cmp_context;//管理比较指令及其结果所使用的寄存器之间的映射

public:
    RiscV64Selector(MachineUnit *dest, LLVMIR *IR) : MachineSelector(dest, IR) {}
    void SelectInstructionAndBuildCFG();
    void ClearFunctionSelectState();
    template <class INSPTR> void ConvertAndAppend(INSPTR);
    int CompareImmediateOperands(ImmI32Operand *op1, ImmI32Operand *op2, BasicInstruction::IcmpCond cur_cond);
    void HandleZeroImmediate(Register op1_reg, BasicInstruction::IcmpCond &cur_cond, Register result_reg);
    void HandleRegisterOperands(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg) ;
    void HandleImmediateOperandWithReg(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg);
    void HandleImmediateOperand(Operand op1, Operand op2, BasicInstruction::IcmpCond &cur_cond, Register result_reg);
    void LoadImmediateToFloatReg(uint32_t imm_val, Register dest_reg) ;
};
#endif