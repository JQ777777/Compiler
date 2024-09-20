#include <stdio.h>

int main() {
    int i, n, f;
    printf("请输入一个整数：");
    scanf("%d", &n);

    i = 2;
    f = 1;
    while (i <= n) {
        f = f * i;
        i = i + 1;
    }

    printf("阶乘结果是：%d\n", f);

    return 0;
}
