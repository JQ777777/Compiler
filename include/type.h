#ifndef TYPE_H
#define TYPE_H

#include <iostream>
#include <string>
#include <vector>

class Type {
public:
    // 我们认为数组的类型为PTR
    enum ty { VOID = 0, INT = 1, FLOAT = 2, BOOL = 3, PTR = 4, DOUBLE = 5 } type;
    std::string GetTypeInfo();
    Type() { type = VOID; }
};

class ConstValue {
public:
    bool ConstTag;
    union ConstVal {
        bool BoolVal;
        int IntVal;
        float FloatVal;
        double DoubleVal;
    } val;
    std::string GetConstValueInfo(Type ty);
    ConstValue() {
        val.IntVal = 0;
        val.FloatVal = 0.0;
        ConstTag = false;
    }
};

// 变量的属性
class VarAttribute {
public:
    Type::ty type;
    bool ConstTag = 0;
    std::vector<int> dims{};    // 存储数组类型的相关信息
    // 对于数组的初始化值，我们将高维数组看作一维后再存储 eg.([3 x [4 x i32]] => [12 x i32])
    std::vector<int> IntInitVals{}; 
    std::vector<float> FloatInitVals{};

    // TODO():也许你需要添加更多变量
    union Value {
        int IntVal;
        float FloatVal;
        Value() {}
        ~Value() {}
        Value(int val) : IntVal(val) {}
        Value(float val) : FloatVal(val) {}
    };
   
    // 使用一个布尔标志来表示值是否存在
    bool has_value = false;
    Value value;

    VarAttribute() {
        type = Type::VOID;
        ConstTag = false;
        has_value = false; 
    }

    // 设置整数值
    void setIntValue(int val) {
        value = Value(val);
        type = Type::INT;
        has_value = true;
    }

    // 设置浮点值
    void setFloatValue(float val) {
        value = Value(val);
        type = Type::FLOAT;
        has_value = true;
    }

    // 获取整数值
    int getIntValue() const {
        if (has_value && type == Type::INT) {
            return value.IntVal;
        }
        throw std::runtime_error("Invalid type or value not set");
    }

    // 获取浮点值
    float getFloatValue() const {
        if (has_value && type == Type::FLOAT) {
            return value.FloatVal;
        }
        throw std::runtime_error("Invalid type or value not set");
    }

    // 检查值是否存在
    bool hasValue() const {
        return has_value;
    }
};

// 语法树节点的属性
class NodeAttribute {
public:
    enum opcode {
        ADD = 0,     // +
        SUB = 1,     // -
        MUL = 2,     // *
        DIV = 3,     // /
        MOD = 4,     // %
        GEQ = 5,     // >=
        GT = 6,      // >
        LEQ = 7,     // <=
        LT = 8,      // <
        EQ = 9,      // ==
        NEQ = 10,     // !=
        OR = 11,     // ||
        AND = 12,    // &&
        NOT = 13,    // !
    };
    int line_number = -1;
    Type T;
    ConstValue V;
    std::string GetAttributeInfo();
};

#endif