#ifndef MachineBaseIns_H
#define MachineBaseIns_H
#include <assert.h>
#include <cstring>
#include <iostream>
#include <map>
#include <set>
#include <vector>

// #define ENABLE_PRINT
#ifdef ENABLE_PRINT
#ifndef PRINT
#define PRINT(...)                                                                                                     \
    do {                                                                                                               \
        char message[256];                                                                                             \
        sprintf(message, __VA_ARGS__);                                                                                 \
        std::cerr << message << "\n";                                                                                  \
    } while (0)
#endif
#else
#ifndef PRINT
#define PRINT(...)
#endif
#endif

#ifndef ERROR
#define ERROR(...)                                                                                                     \
    do {                                                                                                               \
        char message[256];                                                                                             \
        sprintf(message, __VA_ARGS__);                                                                                 \
        std::cerr << "\033[;31;1m";                                                                                    \
        std::cerr << "ERROR: ";                                                                                        \
        std::cerr << "\033[0;37;1m";                                                                                   \
        std::cerr << message << "\n";                                                                                  \
        std::cerr << "\033[0;33;1m";                                                                                   \
        std::cerr << "File: \033[4;37;1m" << __FILE__ << ":" << __LINE__ << "\n";                                      \
        std::cerr << "\033[0m";                                                                                        \
        assert(false);                                                                                                 \
    } while (0)
#endif

#ifndef TODO
#define TODO(...)                                                                                                      \
    do {                                                                                                               \
        char message[256];                                                                                             \
        sprintf(message, __VA_ARGS__);                                                                                 \
        std::cerr << "\033[;34;1m";                                                                                    \
        std::cerr << "TODO: ";                                                                                         \
        std::cerr << "\033[0;37;1m";                                                                                   \
        std::cerr << message << "\n";                                                                                  \
        std::cerr << "\033[0;33;1m";                                                                                   \
        std::cerr << "File: \033[4;37;1m" << __FILE__ << ":" << __LINE__ << "\n";                                      \
        std::cerr << "\033[0m";                                                                                        \
        assert(false);                                                                                                 \
    } while (0)
#endif

#define ENABLE_LOG
#ifdef ENABLE_LOG
#ifndef Log
#define Log(...)                                                                                                       \
    do {                                                                                                               \
        char message[256];                                                                                             \
        sprintf(message, __VA_ARGS__);                                                                                 \
        std::cerr << "\033[;35;1m[\033[4;33;1m" << __FILE__ << ":" << __LINE__ << "\033[;35;1m "                       \
                  << __PRETTY_FUNCTION__ << "]";                                                                       \
        std::cerr << "\033[0;37;1m ";                                                                                  \
        std::cerr << message << "\n";                                                                                  \
        std::cerr << "\033[0m";                                                                                        \
    } while (0)
#endif
#else
#ifndef Log
#define Log(...)
#endif
#endif

#define ENABLE_LOG
#ifdef ENABLE_LOG
#ifndef Lazy
#define Lazy(str)                                                                                                      \
    do {                                                                                                               \
        Log("\033[;31;1m%s\033[0m", str);                                                                              \
    } while (0)
#endif
#else
#ifndef Lazy
#define Lazy(str)
#endif
#endif

#ifndef Assert
#define Assert(EXP)                                                                                                    \
    do {                                                                                                               \
        if (!(EXP)) {                                                                                                  \
            ERROR("Assertion failed: %s", #EXP);                                                                       \
        }                                                                                                              \
    } while (0)
#endif

typedef unsigned __int128 Uint128;
typedef unsigned long long Uint64;
typedef unsigned int Uint32;

struct MachineDataType {
    enum { INT, FLOAT };
    enum { B32, B64, B128 };
    unsigned data_type;
    unsigned data_length;
    MachineDataType() {}
    MachineDataType(const MachineDataType &other) {
        this->data_type = other.data_type;
        this->data_length = other.data_length;
    }
    MachineDataType operator=(const MachineDataType &other) {
        this->data_type = other.data_type;
        this->data_length = other.data_length;
        return *this;
    }
    MachineDataType(unsigned data_type, unsigned data_length) : data_type(data_type), data_length(data_length) {}
    bool operator==(const MachineDataType &other) const {
        return this->data_type == other.data_type && this->data_length == other.data_length;
    }
    int getDataWidth() {
        switch (data_length) {
        case B32:
            return 4;
        case B64:
            return 8;
        case B128:
            return 16;
        }
        return 0;
    }
    std::string toString() {
        std::string ret;
        if (data_type == INT)
            ret += 'i';
        if (data_type == FLOAT)
            ret += 'f';
        if (data_length == B32)
            ret += "32";
        if (data_length == B64)
            ret += "64";
        if (data_length == B128)
            ret += "128";
        return ret;
    }
};

extern MachineDataType INT32, INT64, INT128, FLOAT_32, FLOAT64, FLOAT128;

struct Register {
public:
    int reg_no;         // 寄存器编号
    bool is_virtual;    // 是否为虚拟寄存器
    MachineDataType type;
    MachineDataType Gettype(){return type;}
    Register() {}
    Register(bool is_virtual, int reg_no, MachineDataType type, bool save = false)
        : is_virtual(is_virtual), reg_no(reg_no), type(type) {}
    int getDataWidth() { return type.getDataWidth(); }
    Register(const Register &other) {
        this->is_virtual = other.is_virtual;
        this->reg_no = other.reg_no;
        this->type = other.type;
    }
    Register operator=(const Register &other) {
        this->is_virtual = other.is_virtual;
        this->reg_no = other.reg_no;
        this->type = other.type;
        return *this;
    }
    bool operator<(Register other) const {
        if (is_virtual != other.is_virtual)
            return is_virtual < other.is_virtual;
        if (reg_no != other.reg_no)
            return reg_no < other.reg_no;
        if (type.data_type != other.type.data_type)
            return type.data_type < other.type.data_type;
        if (type.data_length != other.type.data_length)
            return type.data_length < other.type.data_length;
        return false;
    }
    bool operator==(Register other) const {
        return reg_no == other.reg_no && is_virtual == other.is_virtual && type.data_type == other.type.data_type &&
               type.data_length == other.type.data_length;
    }
};

struct MachineBaseOperand {
    MachineDataType type;
    enum { REG, IMMI, IMMF, IMMD };
    int op_type;
    MachineBaseOperand(int op_type) : op_type(op_type) {}
    virtual std::string toString() = 0;
};

struct MachineRegister : public MachineBaseOperand {
    Register reg;
    MachineRegister(Register reg) : MachineBaseOperand(MachineBaseOperand::REG), reg(reg) {}
    std::string toString() {
        if (reg.is_virtual)
            return "%" + std::to_string(reg.reg_no);
        else
            return "phy_" + std::to_string(reg.reg_no);
    }
};

struct MachineImmediateInt : public MachineBaseOperand {
    int imm32;
    MachineImmediateInt(int imm32) : MachineBaseOperand(MachineBaseOperand::IMMI), imm32(imm32) {}
    std::string toString() { return std::to_string(imm32); }
};
struct MachineImmediateFloat : public MachineBaseOperand {
    float fimm32;
    MachineImmediateFloat(float fimm32) : MachineBaseOperand(MachineBaseOperand::IMMF), fimm32(fimm32) {}
    std::string toString() { return std::to_string(fimm32); }
};
struct MachineImmediateDouble : public MachineBaseOperand {
    double dimm64;
    MachineImmediateDouble(double dimm64) : MachineBaseOperand(MachineBaseOperand::IMMD), dimm64(dimm64) {}
    std::string toString() { return std::to_string(dimm64); }
};

class MachineBaseInstruction {
public:
    enum { ARM = 0, RiscV, PHI ,COPY ,ADDIW};
    const int arch;

private:
    int ins_number; // 指令编号, 用于活跃区间计算
    bool no_schedule;//指令是否可以被调度
public:
    void SetNoSchedule(bool no_schedule) { this->no_schedule = no_schedule; }
    bool CanSchedule() { return !no_schedule; }
    void setNumber(int ins_number) { this->ins_number = ins_number; }
    int getNumber() { return ins_number; }
    MachineBaseInstruction(int arch) : arch(arch) {}
    virtual std::vector<Register *> GetReadReg() = 0;     // 获得该指令所有读的寄存器
    virtual std::vector<Register *> GetWriteReg() = 0;    // 获得该指令所有写的寄存器
    virtual int GetLatency() = 0;    // 如果你不打算实现指令调度优化，可以忽略该函数
};

// 将一个操作数（源）复制到另一个操作数（目标）
class MachineCopyInstruction : public MachineBaseInstruction {
private:
    MachineDataType copy_type;
    MachineBaseOperand *src;
    MachineBaseOperand *dst;

public:
    std::vector<Register *> GetReadReg() { //返回一个包含所有被此指令读取的寄存器的向量。如果源操作数是寄存器类型，则返回一个包含该寄存器的向量；否则返回一个空向量
        if (src->op_type == MachineBaseOperand::REG)
            return std::vector<Register *>({&(((MachineRegister *)src)->reg)});
        return std::vector<Register *>();
    }
    std::vector<Register *> GetWriteReg() { //
        assert(dst->op_type == MachineBaseOperand::REG);
        return std::vector<Register *>({&(((MachineRegister *)dst)->reg)});
    }

    MachineCopyInstruction(MachineBaseOperand *src, MachineBaseOperand *dst, MachineDataType copy_type)
        : copy_type(copy_type), src(src), dst(dst), MachineBaseInstruction(MachineBaseInstruction::COPY) {}
    void output(std::ostream &s) {
        s << dst->toString() << " = " << copy_type.toString() << " COPY " << src->toString() << "\n";
    }
    MachineBaseOperand *GetSrc() { return src; }
    MachineBaseOperand *GetDst() { return dst; }
    MachineDataType GetCopyType() { return copy_type; }
    int GetLatency() { return 1; }
};

// 如果你没有实现优化的进阶要求，可以忽略下面的指令类
class MachinePhiInstruction : public MachineBaseInstruction {
private:
    Register result;
    std::vector<std::pair<int, MachineBaseOperand *>> phi_list;

public:
    std::vector<Register *> GetReadReg();
    std::vector<Register *> GetWriteReg();

    MachinePhiInstruction(Register result) : result(result), MachineBaseInstruction(MachineBaseInstruction::PHI) {}
    Register GetResult() { return result; }
    void SetResult(Register result) { this->result = result; }
    std::vector<std::pair<int, MachineBaseOperand *>> &GetPhiList() { return phi_list; }
    MachineBaseOperand *removePhiList(int label) {
        for (auto it = phi_list.begin(); it != phi_list.end(); ++it) {
            if (it->first == label) {
                auto ret = it->second;
                phi_list.erase(it);
                return ret;
            }
        }
        return nullptr;
    }
    void pushPhiList(int label, Register reg) { phi_list.push_back(std::make_pair(label, new MachineRegister(reg))); }
    void pushPhiList(int label, int imm32) {
        phi_list.push_back(std::make_pair(label, new MachineImmediateInt(imm32)));
    }
    void pushPhiList(int label, float immf32) {
        phi_list.push_back(std::make_pair(label, new MachineImmediateFloat(immf32)));
    }
    void pushPhiList(int label, MachineBaseOperand *op) { phi_list.push_back(std::make_pair(label, op)); }
    int GetLatency() { return 0; }
};

class MachineAddiwInstruction : public MachineBaseInstruction {
private:
    MachineBaseOperand *src; // 源操作数（寄存器）
    Register *dst;           // 目标操作数（寄存器）
    int imm;                 // 立即数值

public:
    // 获取读取的寄存器列表
    std::vector<Register *> GetReadReg() override {
        if (src->op_type == MachineBaseOperand::REG)
            return std::vector<Register *>({&(((MachineRegister *)src)->reg)});
        return std::vector<Register *>();
    }

    // 获取写入的寄存器列表
    std::vector<Register *> GetWriteReg() override {
        assert(dst != nullptr);
        return std::vector<Register *>({dst});
    }

    // 构造函数
    MachineAddiwInstruction(MachineBaseOperand *src, Register *dst, int imm)
        : src(src), dst(dst), imm(imm), MachineBaseInstruction(MachineBaseInstruction::ADDIW) {}

    // 输出指令的字符串表示形式
    void output(std::ostream &s) {
        s << (dst->type).toString() << " = ADDIW " << src->toString() << ", #" << imm << "\n";
    }

    // 获取源操作数
    MachineBaseOperand *GetSrc() { return src; }

    // 获取目标寄存器
    Register *GetDst() { return dst; }

    // 获取立即数值
    int GetImm() { return imm; }

    // 获取指令延迟
    int GetLatency() override { return 1; }
};
#endif