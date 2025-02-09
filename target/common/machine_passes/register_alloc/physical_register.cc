#include "physical_register.h"
bool PhysicalRegistersAllocTools::OccupyReg(int phy_id, LiveInterval interval) {
    // 你需要保证interval不与phy_id已有的冲突
    // 或者增加判断分配失败返回false的代码
    phy_occupied[phy_id].push_back(interval);
    return true;
}

bool PhysicalRegistersAllocTools::ReleaseReg(int phy_id, LiveInterval interval) { 
    //TODO("ReleaseReg"); 
    auto it = phy_occupied[phy_id].begin();
    for (; it != phy_occupied[phy_id].end(); ++it) {
        if (*it == interval) {
            phy_occupied[phy_id].erase(it);
            return true;
        }
    }
    return false;
}

//用于在内存中为一个活跃区间分配空间
bool PhysicalRegistersAllocTools::OccupyMem(int offset, int size, LiveInterval interval) {
    //TODO("OccupyMem");

    //将字节数转换为32位的字的数量
    size /= 4;
    //标记这些位置已被当前活跃区间占用
    for (int i = offset; i < offset + size; i++) {
        //如果当前内存位置 i 超过了 mem_occupied 向量的当前大小，则通过 push_back({}) 方法扩展 mem_occupied 向量
        while (i >= mem_occupied.size()) {
            mem_occupied.push_back({});
        }
        //表示i被活跃区间占用
        mem_occupied[i].push_back(interval);
    }
    return true;
}

bool PhysicalRegistersAllocTools::ReleaseMem(int offset, int size, LiveInterval interval) {
    size /= 4;
    for (int i = offset; i < offset + size; i++) {
        auto it = mem_occupied[i].begin();
        for (; it != mem_occupied[i].end(); ++it) {
            if (*it == interval) {
                mem_occupied[i].erase(it);
                break;
            }
        }
    }
    return true;
}

//在寄存器分配过程中为一个活跃区间寻找一个可用的物理寄存器
int PhysicalRegistersAllocTools::getIdleReg(LiveInterval interval) {
    //TODO("getIdleReg");

    //分别记录已经尝试过的寄存器和有效的寄存器
    std::map<int, int> reg_tried, reg_valid;

    //获取所有可能有效的物理寄存器
    for (auto i : getValidRegs(interval)) {
        reg_valid[i] = 1;
        if (reg_tried[i])
            continue;
        int sign = true;

        //对于每个有效的寄存器，如果未尝试过，则进行冲突检测
        for (auto conflict_j : getAliasRegs(i)) {
            for (auto other_interval : phy_occupied[conflict_j]) {
                if (interval & other_interval) {
                    sign = false;
                    break;
                }
            }
        }
        if (sign) {
            return i;
        }
    }
    //未找到
    return -1;
}

//为给定的 LiveInterval 找到一个合适的内存位置
int PhysicalRegistersAllocTools::getIdleMem(LiveInterval interval) { 
    //TODO("getIdleMem"); 

    std::vector<bool> ok;
    ok.resize(mem_occupied.size(), true);
    for (int i = 0; i < mem_occupied.size(); i++) {
        ok[i] = true;
        for (auto other_interval : mem_occupied[i]) {
            if (interval & other_interval) {
                ok[i] = false;
                break;
            }
        }
    }
    int free_cnt = 0;
    for (int offset = 0; offset < ok.size(); offset++) {
        if (ok[offset]) {
            free_cnt++;
        } else {
            free_cnt = 0;
        }
        if (free_cnt == interval.getReg().getDataWidth() / 4) {
            return offset - free_cnt + 1;
        }
    }
    return mem_occupied.size() - free_cnt;
}

int PhysicalRegistersAllocTools::swapRegspill(int p_reg1, LiveInterval interval1, int offset_spill2, int size,
                                              LiveInterval interval2) {

    //TODO("swapRegspill");

    ReleaseReg(p_reg1, interval1);
    ReleaseMem(offset_spill2, size, interval2);
    OccupyReg(p_reg1, interval2);
    return 0;
}

std::vector<LiveInterval> PhysicalRegistersAllocTools::getConflictIntervals(LiveInterval interval) {
    std::vector<LiveInterval> result;
    for (auto phy_intervals : phy_occupied) {
        for (auto other_interval : phy_intervals) {
            if (interval.getReg().type == other_interval.getReg().type && (interval & other_interval)) {
                result.push_back(other_interval);
            }
        }
    }
    return result;
}
