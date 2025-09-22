# user/add_asm.S

# 声明 .text section, 表示这是代码
.section .text

# 将 add_asm 符号声明为全局可见, 这样链接器才能找到它
.global add_asm

# 定义 add_asm 函数
add_asm:
    # RISC-V ABI规定, 前两个整型参数通过 a0 和 a1 寄存器传递
    # a0 = a0 + a1
    add a0, a0, a1

    # ABI规定, 返回值放在 a0 寄存器中
    # 我们要做的加法已经完成了, 结果就在 a0 中, 所以直接返回即可

    # ret 是一个伪指令, 相当于 jalr x0, ra, 0
    # 它会跳转到 ra (Return Address) 寄存器中存储的地址, 也就是返回到调用者那里
    ret
