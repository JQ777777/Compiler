#ifndef LivEInterval_H
#define LivEInterval_H
#include "../machine_pass.h"
#include <assert.h>
#include <queue>
class LiveInterval {
private:
    Register reg;
    // 当begin和end不同时, 活跃区间为[begin,end), 即左闭右开
    // 当begin和end相同时, 表示[begin,end], 即一个单点 (这么做的原因是方便活跃区间计算)
    // 注意特殊判断begin和end相同时的情况
    struct LiveSegment {
        int begin;
        int end;
        //检查给定的位置是否在这个段内
        bool inside(int pos) const {
            if (begin == end) return begin == pos;
            return begin <= pos && pos < end; 
        }
        //重载运算符以检查两个段是否有重叠
        bool operator&(const struct LiveSegment &that) const {
            return this->inside(that.begin) || this->inside(that.end - 1 > that.begin ? that.end - 1 : that.begin) ||
                   that.inside(this->begin) || that.inside(this->end - 1 > this->begin ? this->end - 1 : this->begin);
        }
        //重载运算符以比较两个段是否相等
        bool operator==(const struct LiveSegment &that) const {
            return this->begin == that.begin && this->end == that.end;
        }
    };
    std::list<LiveSegment> segments{};
    int reference_count;

public:
    // 检测两个活跃区间是否重叠
    // 保证两个活跃区间各个段各自都是不降序（升序）排列的
    bool operator&(const LiveInterval &that) const {
        //TODO("& operator in LiveInterval");

        // 如果任意一方没有段，则直接返回false，因为不存在重叠
        if (segments.empty() || that.segments.empty()) {
            return false;
        }

        auto thisLI = segments.begin();  // 当前LiveInterval的段迭代器
        auto thatLI = that.segments.begin();  // 另一个LiveInterval的段迭代器

        while (thisLI != segments.end() && thatLI != that.segments.end()) {
            // 检查当前两段是否重叠
            if (*thisLI & *thatLI) {
                return true;  // 找到重叠，立即返回true
            }

            // 根据段的结束位置决定哪个迭代器前进
            if (thisLI->end <= thatLI->begin) {
                ++thisLI;  // 当前LiveInterval的段完全在另一个之前
            } else if (thatLI->end <= thisLI->begin) {
                ++thatLI;  // 另一个LiveInterval的段完全在当前之前
            }
        }

        return false;  // 遍历完所有段后未找到重叠
    }

    bool operator==(const LiveInterval &that) const {
        // TODO : Judge if *this and that are equal
        if (reg == that.reg) {
            Assert(segments == that.segments);
            return true;
        } else {
            return false;
        }
        return reg == that.reg;    // && segments == that.segments;
    }

    // 更新引用计数
    void IncreaseReferenceCount(int count) { reference_count += count; }
    //获取当前引用计数
    int getReferenceCount() { return reference_count; }
    // 返回活跃区间长度
    int getIntervalLen() {
        int ret = 0;
        for (auto seg : segments) {
            ret += (seg.end - seg.begin + 1);
        }
        return ret;
    }
    //返回该活跃区间对应的寄存器
    Register getReg() { return reg; }
    //提供无参构造函数和接受一个寄存器参数的构造函数
    LiveInterval() : reference_count(0) {}    // Temp
    LiveInterval(Register reg) : reg(reg), reference_count(0) {}

    //在 segments 列表的前面添加一个新的 LiveSegment
    void PushFront(int begin, int end) { segments.push_front({begin = begin, end = end}); }
    //设置第一个段的起始位置
    void SetMostBegin(int begin) { segments.begin()->begin = begin; }

    // 可以直接 for(auto segment : liveinterval)
    decltype(segments.begin()) begin() { return segments.begin(); }
    decltype(segments.end()) end() { return segments.end(); }
};

class Liveness {
private:
    MachineFunction *current_func;
    // 更新所有块DEF和USE集合
    void UpdateDefUse();
    // Key: Block_Number
    // 存储活跃变量分析的结果
    std::map<int, std::set<Register>> IN{}, OUT{}, DEF{}, USE{};

public:
    // 对所有块进行活跃变量分析并保存结果
    void Execute();
    Liveness(MachineFunction *mfun, bool calculate = true) : current_func(mfun) {
        if (calculate) {
            Execute();
        }
    }
    // 获取基本块的IN/OUT/DEF/USE集合
    std::set<Register> GetIN(int bid) { return IN[bid]; }
    std::set<Register> GetOUT(int bid) { return OUT[bid]; }
    std::set<Register> GetDef(int bid) { return DEF[bid]; }
    std::set<Register> GetUse(int bid) { return USE[bid]; }
};
#endif