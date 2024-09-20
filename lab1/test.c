     .file   "test.c"
        .option nopic
        .attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0"
        .attribute unaligned_access, 0
        .attribute stack_align, 16
# GNU C17 () version 12.2.0 (riscv64-unknown-linux-gnu)
#       compiled by GNU C version 9.4.0, GMP version 6.2.0, MPFR version 4.0.2, MPC version 1.1.0, isl version none
# warning: GMP header version 6.2.0 differs from library version 6.2.1.
# warning: MPFR header version 4.0.2 differs from library version 4.1.0.
# warning: MPC header version 1.1.0 differs from library version 1.2.1.
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed: -mtune=rocket -mabi=lp64d -misa-spec=20191213 -march=rv64imafdc_zicsr
        .text
        .section        .rodata
        .align  3
.LC1:
        .string "\350\257\267\350\276\223\345\205\245\344\270\200\344\270\252\346\225\264\346\225\260\357\274\232"
        .align  3
.LC2:
        .string "%d"
        .align  3
.LC3:
        .string "\351\230\266\344\271\230\347\273\223\346\236\234\346\230\257\357\274\232%g\n"
        .text
        .align  1
        .globl  main
        .type   main, @function
main:
        addi    sp,sp,-32       #,,
        sd      ra,24(sp)       #,
        sd      s0,16(sp)       #,
        addi    s0,sp,32        #,,
        li      t0,-8192                #,
        addi    t0,t0,192       #,,
        add     sp,sp,t0        #,,
# test.c:7:     double fact = 1.0;
        lui     a5,%hi(.LC0)    # tmp79,
        fld     fa5,%lo(.LC0)(a5)       # tmp80,
        fsd     fa5,-24(s0)     # tmp80, fact
# test.c:11:     printf("请输入一个整数：");
        lui     a5,%hi(.LC1)    # tmp81,
        addi    a0,a5,%lo(.LC1) #, tmp81,
        call    printf          #

# test.c:12:     scanf("%d", &n);
        addi    a5,s0,-32       #, tmp82,
        mv      a1,a5   #, tmp82
        lui     a5,%hi(.LC2)    # tmp83,
        addi    a0,a5,%lo(.LC2) #, tmp83,
        call    __isoc99_scanf          #


# test.c:15:     for (i = 0; i < n; i++) 
        sw      zero,-28(s0)    #, i
# test.c:15:     for (i = 0; i < n; i++) 
        j       .L2             #


.L3:
# test.c:17:         arr[i] = i + 1;
        lw      a5,-28(s0)              # tmp86, i
        addiw   a5,a5,1 #, tmp84, tmp85
        sext.w  a5,a5   # _1, tmp84
# test.c:17:         arr[i] = i + 1;
        fcvt.d.w        fa5,a5  # _2, _1
        li      a5,-8192                # tmp87,
        addi    a5,a5,-16       #, tmp114, tmp87
        add     a4,a5,s0        #, tmp88, tmp114
        lw      a5,-28(s0)              # tmp89, i
slli    a5,a5,3 #, tmp90, tmp89
        add     a5,a4,a5        # tmp90, tmp90, tmp88
        fsd     fa5,176(a5)     # _2, arr[i_7]

# test.c:15:     for (i = 0; i < n; i++) 
        lw      a5,-28(s0)              # tmp93, i
        addiw   a5,a5,1 #, tmp91, tmp92
        sw      a5,-28(s0)      # tmp91, i

.L2:
# test.c:15:     for (i = 0; i < n; i++) 
        lw      a4,-32(s0)              # n.0_3, n
        lw      a5,-28(s0)              # tmp95, i
        sext.w  a5,a5   # tmp96, tmp94
        blt     a5,a4,.L3       #, tmp96, tmp97,
# test.c:21:     for (i = 0; i < n; i++) {
        sw      zero,-28(s0)    #, i
# test.c:21:     for (i = 0; i < n; i++) {
        j       .L4             #
.L5:
# test.c:22:         fact *= arr[i];
        li      a5,-8192                # tmp98,
        addi    a5,a5,-16       #, tmp115, tmp98
        add     a4,a5,s0        #, tmp99, tmp115
        lw      a5,-28(s0)              # tmp100, i
        slli    a5,a5,3 #, tmp101, tmp100
        add     a5,a4,a5        # tmp101, tmp101, tmp99
 fld     fa5,176(a5)     # _4, arr[i_8]
# test.c:22:         fact *= arr[i];
        fld     fa4,-24(s0)     # tmp103, fact
        fmul.d  fa5,fa4,fa5     # tmp102, tmp103, _4
        fsd     fa5,-24(s0)     # tmp102, fact
# test.c:21:     for (i = 0; i < n; i++) {
        lw      a5,-28(s0)              # tmp106, i
        addiw   a5,a5,1 #, tmp104, tmp105
        sw      a5,-28(s0)      # tmp104, i
.L4:
# test.c:21:     for (i = 0; i < n; i++) {
        lw      a4,-32(s0)              # n.1_5, n
        lw      a5,-28(s0)              # tmp108, i
        sext.w  a5,a5   # tmp109, tmp107
        blt     a5,a4,.L5       #, tmp109, tmp110,
# test.c:25:     printf("阶乘结果是：%g\n", fact);
        ld      a1,-24(s0)              #, fact
        lui     a5,%hi(.LC3)    # tmp111,
        addi    a0,a5,%lo(.LC3) #, tmp111,
        call    printf          #
# test.c:27:     return 0;
        li      a5,0            # _17,
# test.c:28: }
        mv      a0,a5   #, <retval>
        li      t0,8192         #,
        addi    t0,t0,-192      #,,
        add     sp,sp,t0        #,,
        ld      ra,24(sp)               #,
        ld      s0,16(sp)               #,
        addi    sp,sp,32        #,,
        jr      ra              #
        .size   main, .-main
        .section        .rodata
        .align  3
.LC0:
        .word   0
        .word   1072693248
        .ident  "GCC: () 12.2.0"
        .section        .note.GNU-stack,"",@progbits
