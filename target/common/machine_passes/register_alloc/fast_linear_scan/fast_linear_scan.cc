#include "fast_linear_scan.h"
//比较两个LiveInterval实例的优先级，起始位置较大的区间优先级较高
bool IntervalsPrioCmp(LiveInterval a, LiveInterval b) { return a.begin()->begin > b.begin()->begin; }
FastLinearScan::FastLinearScan(MachineUnit *unit, PhysicalRegistersAllocTools *phy, SpillCodeGen *spiller)
    : RegisterAllocation(unit, phy, spiller), unalloc_queue(IntervalsPrioCmp) {}
//unalloc_queue 是一个优先队列，使用 IntervalsPrioCmp 函数作为比较器，用于管理未分配的活跃区间

//进行寄存器分配的主要入口点
bool FastLinearScan::DoAllocInCurrentFunc() {
    //记录是否发生了溢出
    bool spilled = false;
    //获取当前处理的机器函数对象 mfun 并打印其名称
    auto mfun = current_func;
    //PRINT("FastLinearScan: %s", mfun->getFunctionName().c_str());
    // std::cerr<<"FastLinearScan: "<<mfun->getFunctionName()<<"\n";
    phy_regs_tools->clear();
    for (auto interval : intervals) {
        int first_reg_no = interval.first.reg_no;
        int second_reg_no = interval.second.getReg().reg_no;
        
        std::cout << "Checking interval: first=" << first_reg_no << "  is_virtual: " << interval.first.is_virtual << std::endl;
        std::cout << "second.getReg()=" << second_reg_no << "  is_virtual: " << interval.second.getReg().is_virtual << std::endl;
                
        if (first_reg_no != second_reg_no) {
            std::cout << "Mismatch found: interval.first.reg_no=" << first_reg_no 
                    << ", interval.second.getReg().reg_no=" << second_reg_no << std::endl;
            // 打印更多关于 interval.second 的详细信息
            //std::cout << "Interval details: " << interval.second.getReg() << std::endl; // 假设有toString方法
        }

        Assert(interval.first == interval.second.getReg());
        
        if (interval.first.is_virtual) {
            unalloc_queue.push(interval.second);
            if (!interval.second.getReg().is_virtual){
                std::cout << "!interval.second.getReg().is_virtual " << std::endl;
            }
        } else {
            phy_regs_tools->OccupyReg(interval.first.reg_no, interval.second);
        }
    }
    // TODO: 进行线性扫描寄存器分配, 为每个虚拟寄存器选择合适的物理寄存器或者将其溢出到合适的栈地址中
    // 该函数中只需正确设置alloc_result，并不需要实际生成溢出代码
    //TODO("LinearScan");

    //遍历未分配的活跃区间
    while(!unalloc_queue.empty()){
        //取出并移除最高优先级活跃区间
        auto active_interval = unalloc_queue.top();
        unalloc_queue.pop();

        //获取活跃区间寄存器信息
        auto active_reg = active_interval.getReg();

        //为当前活跃区间分配一个空闲的物理寄存器
        int preg_id = phy_regs_tools->getIdleReg(active_interval);

        //如果成功找到可用的物理寄存器
        if (preg_id >= 0){
            //占用该物理寄存器，将分配结果写入alloc_result
            phy_regs_tools->OccupyReg(preg_id, active_interval);
            AllocPhyReg(mfun, active_reg, preg_id);
        }
        else{
            //如果没找到可用的物理寄存器，溢出
            spilled = true;

            //获取并占用一个空闲的内存位置用于存储溢出的数据
            int mem = phy_regs_tools->getIdleMem(active_interval);
            phy_regs_tools->OccupyMem(mem, active_reg.getDataWidth(), active_interval);
        
            //将虚拟寄存器溢出到栈上的指定位置
            AllocStack(mfun, active_reg, mem);

            //计算当前活跃区间的溢出权重，并尝试寻找一个更优的溢出候选者
            //即具有最小溢出权重的活跃区间，尽量减少溢出带来的性能损失
            double spill_weight = CalculateSpillWeight(active_interval);
            
            //默认情况下，如果找不到更好的候选者，当前区间将是溢出的候选者
            auto spill_interval = active_interval;

            bool sign = 0;

            //遍历与当前活跃区间 active_interval 冲突的所有其他活跃区间
            for (auto other : phy_regs_tools->getConflictIntervals(active_interval)) {
                double other_weight = CalculateSpillWeight(other);
                //当前找到的最佳溢出候选者的溢出权重大于当前冲突区间的溢出权重
                //并且该冲突区间是一个虚拟寄存器，则考虑它作为更优的溢出候选者
                if (spill_weight > other_weight && other.getReg().is_virtual) {
                    spill_weight = other_weight;
                    spill_interval = other;
                    sign = 1;
                }
            }

            //如果找到了一个比当前活跃区间更适合溢出的区间
            if (sign == 1){
                //将spill的物理寄存器腾出来，为当前active_interval分配该寄存器
                //将spill数据移动到内存中指定位置mem
                phy_regs_tools->swapRegspill(getAllocResultInReg(mfun, spill_interval.getReg()), spill_interval, mem, active_reg.getDataWidth(), active_interval);
                swapAllocResult(mfun, active_interval.getReg(), spill_interval.getReg());
            
                //获取一个空闲内存位置spill_mem存放spill_interval的数据
                int spill_mem = phy_regs_tools->getIdleMem(spill_interval);
                //标记该内存位置已被占用
                phy_regs_tools->OccupyMem(spill_mem, spill_interval.getReg().getDataWidth(), spill_interval);
                //将分配结果写入alloc_result
                AllocStack(mfun, spill_interval.getReg(), spill_mem);
            }
        }
    }

    // 返回是否发生溢出
    return spilled;
}

// 计算溢出权重
double FastLinearScan::CalculateSpillWeight(LiveInterval interval) {
    return (double)interval.getReferenceCount() / interval.getIntervalLen();
}
