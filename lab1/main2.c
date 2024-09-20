#include <stdio.h>
#include <stdlib.h>

#define MAX_FACT 1000  // 定义最大阶乘值，超过这个值可能会导致溢出
int main() {
    int n;
    double fact = 1.0;
    double arr[MAX_FACT];  // 数组用于存储中间结果
    int i;

    printf("请输入一个整数：");
    scanf("%d", &n);

    // 初始化数组
    for (i = 0; i < n; i++) 
    {
        arr[i] = i + 1;
    }

    // 计算阶乘
    for (i = 0; i < n; i++) {
        fact *= arr[i];
    }

    printf("阶乘结果是：%g\n", fact);

    return 0;
}
