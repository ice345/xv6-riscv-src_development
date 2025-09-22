#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// 使用 extern 关键字声明我们要在汇编中定义的函数
extern int add_asm(int, int);

int main(void)
{
    int a = 10;
    int b = 20;
    int result;

    printf("C: Calling add_asm(%d, %d)...\n", a, b);

    // 直接调用, 就像调用一个普通的C函数一样
    result = add_asm(a, b);

    printf("C: Result from asm is %d\n", result);

    if (result == a + b) {
        printf("Test PASSED!\n");
    } else {
        printf("Test FAILED!\n");
    }

    exit(0);
}