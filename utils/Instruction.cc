#include "../include/Instruction.h"
#include "../include/basic_block.h"
#include <assert.h>
#include <unordered_map>

static std::unordered_map<int, RegOperand *> RegOperandMap;
static std::map<int, LabelOperand *> LabelOperandMap;
static std::map<std::string, GlobalOperand *> GlobalOperandMap;

RegOperand *GetNewRegOperand(int RegNo) {
    auto it = RegOperandMap.find(RegNo);
    if (it == RegOperandMap.end()) {
        auto R = new RegOperand(RegNo);
        RegOperandMap[RegNo] = R;
        return R;
    } else {
        return it->second;
    }
}

LabelOperand *GetNewLabelOperand(int LabelNo) {
    auto it = LabelOperandMap.find(LabelNo);
    if (it == LabelOperandMap.end()) {
        auto L = new LabelOperand(LabelNo);
        LabelOperandMap[LabelNo] = L;
        return L;
    } else {
        return it->second;
    }
}

GlobalOperand *GetNewGlobalOperand(std::string name) {
    auto it = GlobalOperandMap.find(name);
    if (it == GlobalOperandMap.end()) {
        auto G = new GlobalOperand(name);
        GlobalOperandMap[name] = G;
        return G;
    } else {
        return it->second;
    }
}

void IRgenArithmeticI32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                      GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                   GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticI32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int reg2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, new ImmI32Operand(val1),
                                                      GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32ImmLeft(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, int reg2,
                               int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, new ImmF32Operand(val1),
                                                   GetNewRegOperand(reg2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticI32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, int val1, int val2, int result_reg) {
    B->InsertInstruction(1, new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::I32, new ImmI32Operand(val1),
                                                      new ImmI32Operand(val2), GetNewRegOperand(result_reg)));
}

void IRgenArithmeticF32ImmAll(LLVMBlock B, BasicInstruction::LLVMIROpcode opcode, float val1, float val2,
                              int result_reg) {
    B->InsertInstruction(1,
                         new ArithmeticInstruction(opcode, BasicInstruction::LLVMType::FLOAT32, new ImmF32Operand(val1),
                                                   new ImmF32Operand(val2), GetNewRegOperand(result_reg)));
}

void IRgenIcmp(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new IcmpInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                GetNewRegOperand(reg2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFcmp(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, int reg2, int result_reg) {
    B->InsertInstruction(1, new FcmpInstruction(BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                GetNewRegOperand(reg2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenIcmpImmRight(LLVMBlock B, BasicInstruction::IcmpCond cmp_op, int reg1, int val2, int result_reg) {
    B->InsertInstruction(1, new IcmpInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(reg1),
                                                new ImmI32Operand(val2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFcmpImmRight(LLVMBlock B, BasicInstruction::FcmpCond cmp_op, int reg1, float val2, int result_reg) {
    B->InsertInstruction(1, new FcmpInstruction(BasicInstruction::LLVMType::FLOAT32, GetNewRegOperand(reg1),
                                                new ImmF32Operand(val2), cmp_op, GetNewRegOperand(result_reg)));
}

void IRgenFptosi(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new FptosiInstruction(GetNewRegOperand(dst), GetNewRegOperand(src)));
}

void IRgenSitofp(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new SitofpInstruction(GetNewRegOperand(dst), GetNewRegOperand(src)));
}

void IRgenZextI1toI32(LLVMBlock B, int src, int dst) {
    B->InsertInstruction(1, new ZextInstruction(BasicInstruction::LLVMType::I32, GetNewRegOperand(dst),
                                                BasicInstruction::LLVMType::I1, GetNewRegOperand(src)));
}

void IRgenGetElementptrIndexI32(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs) {
    B->InsertInstruction(1, new GetElementptrInstruction(type, GetNewRegOperand(result_reg), ptr, dims, indexs, BasicInstruction::I32));
}

void IRgenGetElementptrIndexI64(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr,
                        std::vector<int> dims, std::vector<Operand> indexs) {
    B->InsertInstruction(1, new GetElementptrInstruction(type, GetNewRegOperand(result_reg), ptr, dims, indexs, BasicInstruction::I64));
}

void IRgenLoad(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, Operand ptr) {
    B->InsertInstruction(1, new LoadInstruction(type, ptr, GetNewRegOperand(result_reg)));
}

void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, int value_reg, Operand ptr) {
    B->InsertInstruction(1, new StoreInstruction(type, ptr, GetNewRegOperand(value_reg)));
}

void IRgenStore(LLVMBlock B, BasicInstruction::LLVMType type, Operand value, Operand ptr) {
    B->InsertInstruction(1, new StoreInstruction(type, ptr, value));
}

void IRgenCall(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg,
               std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(result_reg), name, args));
}

void IRgenCallVoid(LLVMBlock B, BasicInstruction::LLVMType type,
                   std::vector<std::pair<enum BasicInstruction::LLVMType, Operand>> args, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(-1), name, args));
}

void IRgenCallNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, int result_reg, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(result_reg), name));
}

void IRgenCallVoidNoArgs(LLVMBlock B, BasicInstruction::LLVMType type, std::string name) {
    B->InsertInstruction(1, new CallInstruction(type, GetNewRegOperand(-1), name));
}

void IRgenRetReg(LLVMBlock B, BasicInstruction::LLVMType type, int reg) {
    B->InsertInstruction(1, new RetInstruction(type, GetNewRegOperand(reg)));
}

void IRgenRetImmInt(LLVMBlock B, BasicInstruction::LLVMType type, int val) {
    B->InsertInstruction(1, new RetInstruction(type, new ImmI32Operand(val)));
}

void IRgenRetImmFloat(LLVMBlock B, BasicInstruction::LLVMType type, float val) {
    B->InsertInstruction(1, new RetInstruction(type, new ImmF32Operand(val)));
}

void IRgenRetVoid(LLVMBlock B) {
    B->InsertInstruction(1, new RetInstruction(BasicInstruction::LLVMType::VOID, nullptr));
}

void IRgenBRUnCond(LLVMBlock B, int dst_label) {
    B->InsertInstruction(1, new BrUncondInstruction(GetNewLabelOperand(dst_label)));
}

void IRgenBrCond(LLVMBlock B, int cond_reg, int true_label, int false_label) {
    B->InsertInstruction(1, new BrCondInstruction(GetNewRegOperand(cond_reg), GetNewLabelOperand(true_label),
                                                  GetNewLabelOperand(false_label)));
}

void IRgenAlloca(LLVMBlock B, BasicInstruction::LLVMType type, int reg) {
    B->InsertInstruction(0, new AllocaInstruction(type, GetNewRegOperand(reg)));
}

void IRgenAllocaArray(LLVMBlock B, BasicInstruction::LLVMType type, int reg, std::vector<int> dims) {
    B->InsertInstruction(0, new AllocaInstruction(type, dims, GetNewRegOperand(reg)));
}

void LoadInstruction::Replace_RegMap(const std::map<int, int> &ReMap)
{
      // 处理指针寄存器
    if (pointer->GetOperandType() == BasicOperand::REG) {
        auto pointer_reg = dynamic_cast<RegOperand *>(pointer); // 使用 dynamic_cast 进行类型安全检查
        if (pointer_reg && ReMap.find(pointer_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(pointer_reg->GetRegNo());
            this->pointer = GetNewRegOperand(new_reg);
        }
    }

    // 处理结果寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void StoreInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理指针寄存器
    if (pointer->GetOperandType() == BasicOperand::REG) {
        auto pointer_reg = dynamic_cast<RegOperand *>(pointer); // 使用 dynamic_cast 进行类型检查
        if (pointer_reg && ReMap.find(pointer_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(pointer_reg->GetRegNo());
            this->pointer = GetNewRegOperand(new_reg);
        }
    }

    // 处理值寄存器
    if (value->GetOperandType() == BasicOperand::REG) {
        auto value_reg = dynamic_cast<RegOperand *>(value); // 使用 dynamic_cast 进行类型检查
        if (value_reg && ReMap.find(value_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(value_reg->GetRegNo());
            this->value = GetNewRegOperand(new_reg);
        }
    }
}

void ArithmeticInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {

    // 处理 op2
    if (op2->GetOperandType() == BasicOperand::REG) {
        auto op2_reg = dynamic_cast<RegOperand *>(op2); // 使用 dynamic_cast 进行类型安全检查
        if (op2_reg && ReMap.find(op2_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op2_reg->GetRegNo());
            this->op2 = GetNewRegOperand(new_reg);
        }
    }

    // 处理 op1
    if (op1->GetOperandType() == BasicOperand::REG) {
        auto op1_reg = dynamic_cast<RegOperand *>(op1); // 使用 dynamic_cast 进行类型安全检查
        if (op1_reg && ReMap.find(op1_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op1_reg->GetRegNo());
            this->op1 = GetNewRegOperand(new_reg);
        }
    }

    // 处理结果寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void IcmpInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 op2
    if (op2->GetOperandType() == BasicOperand::REG) {
        auto op2_reg = dynamic_cast<RegOperand *>(op2); // 使用 dynamic_cast 进行类型安全检查
        if (op2_reg && ReMap.find(op2_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op2_reg->GetRegNo());
            this->op2 = GetNewRegOperand(new_reg);
        }
    }

    // 处理 op1
    if (op1->GetOperandType() == BasicOperand::REG) {
        auto op1_reg = dynamic_cast<RegOperand *>(op1); // 使用 dynamic_cast 进行类型安全检查
        if (op1_reg && ReMap.find(op1_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op1_reg->GetRegNo());
            this->op1 = GetNewRegOperand(new_reg);
        }
    }

    // 处理结果寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void FcmpInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 op2
    if (op2->GetOperandType() == BasicOperand::REG) {
        auto op2_reg = dynamic_cast<RegOperand *>(op2); // 使用 dynamic_cast 进行类型安全检查
        if (op2_reg && ReMap.find(op2_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op2_reg->GetRegNo());
            this->op2 = GetNewRegOperand(new_reg);
        }
    }

    // 处理 op1
    if (op1->GetOperandType() == BasicOperand::REG) {
        auto op1_reg = dynamic_cast<RegOperand *>(op1); // 使用 dynamic_cast 进行类型安全检查
        if (op1_reg && ReMap.find(op1_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(op1_reg->GetRegNo());
            this->op1 = GetNewRegOperand(new_reg);
        }
    }

    // 处理结果寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void PhiInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 遍历 phi_list，替换 phi 操作数
    for (auto &label_pair : phi_list) {
        // 替换 op1
        if (label_pair.first->GetOperandType() == BasicOperand::REG) {
            auto op1_reg = dynamic_cast<RegOperand *>(label_pair.first); // 使用 dynamic_cast 进行类型安全检查
            if (op1_reg && ReMap.find(op1_reg->GetRegNo()) != ReMap.end()) {
                int new_reg = ReMap.at(op1_reg->GetRegNo());
                label_pair.first = GetNewRegOperand(new_reg);
            }
        }
        if(label_pair.second != nullptr){
            // 替换 op2
            if (label_pair.second->GetOperandType() == BasicOperand::REG) {
                auto op2_reg = dynamic_cast<RegOperand *>(label_pair.second); // 使用 dynamic_cast 进行类型安全检查
                if (op2_reg && ReMap.find(op2_reg->GetRegNo()) != ReMap.end()) {
                    int new_reg = ReMap.at(op2_reg->GetRegNo());
                    label_pair.second = GetNewRegOperand(new_reg);
                }
            }
        }
    }

    // 替换结果寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void AllocaInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 使用 dynamic_cast 进行类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void BrCondInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    if (cond->GetOperandType() == BasicOperand::REG) {
        auto cond_reg = dynamic_cast<RegOperand *>(cond); // 使用 dynamic_cast 进行类型安全检查
        if (cond_reg && ReMap.find(cond_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(cond_reg->GetRegNo());
            this->cond = GetNewRegOperand(new_reg);
        }
    }
}

void BrUncondInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {}

void GlobalVarDefineInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {}

void GlobalStringConstInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {}

void CallInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 遍历 args，替换寄存器操作数
    for (auto &arg_pair : args) {
        if (arg_pair.second->GetOperandType() == BasicOperand::REG) {
            auto op = dynamic_cast<RegOperand *>(arg_pair.second); // 类型安全检查
            if (op && ReMap.find(op->GetRegNo()) != ReMap.end()) {
                int new_reg = ReMap.at(op->GetRegNo());
                arg_pair.second = GetNewRegOperand(new_reg);
            }
        }
    }

    // 替换结果寄存器
    if (result && result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(result_reg->GetRegNo());
            this->result = GetNewRegOperand(new_reg);
        }
    }
}

void RetInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 替换返回值寄存器
    if (ret_val && ret_val->GetOperandType() == BasicOperand::REG) {
        auto ret_reg = dynamic_cast<RegOperand *>(ret_val); // 类型安全检查
        if (ret_reg && ReMap.find(ret_reg->GetRegNo()) != ReMap.end()) {
            int new_reg = ReMap.at(ret_reg->GetRegNo());
            ret_val = GetNewRegOperand(new_reg);
        }
    }
}

void FunctionDefineInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {}

void FunctionDeclareInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {}

void FptosiInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 result 寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            this->result = GetNewRegOperand(ReMap.at(result_reg->GetRegNo()));
        }
    }

    // 处理 value 寄存器
    if (value->GetOperandType() == BasicOperand::REG) {
        auto value_reg = dynamic_cast<RegOperand *>(value); // 类型安全检查
        if (value_reg && ReMap.find(value_reg->GetRegNo()) != ReMap.end()) {
            this->value = GetNewRegOperand(ReMap.at(value_reg->GetRegNo()));
        }
    }
}

void SitofpInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 result 寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            this->result = GetNewRegOperand(ReMap.at(result_reg->GetRegNo()));
        }
    }

    // 处理 value 寄存器
    if (value->GetOperandType() == BasicOperand::REG) {
        auto value_reg = dynamic_cast<RegOperand *>(value); // 类型安全检查
        if (value_reg && ReMap.find(value_reg->GetRegNo()) != ReMap.end()) {
            this->value = GetNewRegOperand(ReMap.at(value_reg->GetRegNo()));
        }
    }
}

void ZextInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 result 寄存器
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            this->result = GetNewRegOperand(ReMap.at(result_reg->GetRegNo()));
        }
    }

    // 处理 value 寄存器
    if (value->GetOperandType() == BasicOperand::REG) {
        auto value_reg = dynamic_cast<RegOperand *>(value); // 类型安全检查
        if (value_reg && ReMap.find(value_reg->GetRegNo()) != ReMap.end()) {
            this->value = GetNewRegOperand(ReMap.at(value_reg->GetRegNo()));
        }
    }
}

void GetElementptrInstruction::Replace_RegMap(const std::map<int, int> &ReMap) {
    // 处理 result 操作数
    if (result->GetOperandType() == BasicOperand::REG) {
        auto result_reg = dynamic_cast<RegOperand *>(result); // 类型安全检查
        if (result_reg && ReMap.find(result_reg->GetRegNo()) != ReMap.end()) {
            this->result = GetNewRegOperand(ReMap.at(result_reg->GetRegNo()));
        }
    }

    // 处理 ptrval 操作数
    if (ptrval->GetOperandType() == BasicOperand::REG) {
        auto ptrval_reg = dynamic_cast<RegOperand *>(ptrval); // 类型安全检查
        if (ptrval_reg && ReMap.find(ptrval_reg->GetRegNo()) != ReMap.end()) {
            this->ptrval = GetNewRegOperand(ReMap.at(ptrval_reg->GetRegNo()));
        }
    }

    // 处理索引列表
    for (auto &idx_pair : indexes) {
        if (idx_pair->GetOperandType() == BasicOperand::REG) {
            auto idx_reg = dynamic_cast<RegOperand *>(idx_pair); // 类型安全检查
            if (idx_reg && ReMap.find(idx_reg->GetRegNo()) != ReMap.end()) {
                idx_pair = GetNewRegOperand(ReMap.at(idx_reg->GetRegNo()));
            }
        }
    }
}
