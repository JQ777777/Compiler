#ifndef MACHINE_PHI_DESTRUCTION_H
#define MACHINE_PHI_DESTRUCTION_H
#include "machine_pass.h"
#include "../riscv64.h"
class MachinePhiDestruction : public MachinePass {
private:
    void PhiDestructionInCurrentFunction();

public:
    void Execute();
    MachinePhiDestruction(MachineUnit *unit) : MachinePass(unit) {}
    MachineFunction *cur_func;
};
#endif