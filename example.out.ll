declare i32 @getint()
declare i32 @getch()
declare float @getfloat()
declare i32 @getarray(ptr)
declare i32 @getfarray(ptr)
declare void @putint(i32)
declare void @putch(i32)
declare void @putfloat(float)
declare void @putarray(i32,ptr)
declare void @putfarray(i32,ptr)
declare void @_sysy_starttime(i32)
declare void @_sysy_stoptime(i32)
declare void @llvm.memset.p0.i32(ptr,i8,i32,i1)
declare i32 @llvm.umax.i32(i32,i32)
declare i32 @llvm.umin.i32(i32,i32)
declare i32 @llvm.smax.i32(i32,i32)
declare i32 @llvm.smin.i32(i32,i32)
declare float @llvm.fmin.f32(float,float)
declare float @llvm.fmax.f32(float,float)
@M = global i32 zeroinitializer
@L = global i32 zeroinitializer
@N = global i32 zeroinitializer
define i32 @tran(ptr %r0,ptr %r1,ptr %r2,ptr %r3,ptr %r4,ptr %r5,ptr %r6,ptr %r7,ptr %r8)
{
L0:  ;
    br label %L1
L1:  ;
    %r12 = add i32 2,0
    %r13 = getelementptr float, ptr %r7, i32 %r12
    %r14 = add i32 1,0
    %r15 = getelementptr float, ptr %r2, i32 %r14
    %r16 = load float, ptr %r15
    store float %r16, ptr %r13
    %r17 = add i32 1,0
    %r18 = getelementptr float, ptr %r8, i32 %r17
    %r19 = add i32 2,0
    %r20 = getelementptr float, ptr %r1, i32 %r19
    %r21 = load float, ptr %r20
    store float %r21, ptr %r18
    %r22 = add i32 1,0
    %r23 = getelementptr float, ptr %r6, i32 %r22
    %r24 = add i32 0,0
    %r25 = getelementptr float, ptr %r1, i32 %r24
    %r26 = load float, ptr %r25
    store float %r26, ptr %r23
    %r27 = add i32 2,0
    %r28 = getelementptr float, ptr %r6, i32 %r27
    %r29 = add i32 0,0
    %r30 = getelementptr float, ptr %r2, i32 %r29
    %r31 = load float, ptr %r30
    store float %r31, ptr %r28
    %r32 = add i32 0,0
    %r33 = getelementptr float, ptr %r7, i32 %r32
    %r34 = add i32 1,0
    %r35 = getelementptr float, ptr %r0, i32 %r34
    %r36 = load float, ptr %r35
    store float %r36, ptr %r33
    %r37 = add i32 0,0
    %r38 = getelementptr float, ptr %r8, i32 %r37
    %r39 = add i32 2,0
    %r40 = getelementptr float, ptr %r0, i32 %r39
    %r41 = load float, ptr %r40
    store float %r41, ptr %r38
    %r42 = add i32 1,0
    %r43 = getelementptr float, ptr %r7, i32 %r42
    %r44 = add i32 1,0
    %r45 = getelementptr float, ptr %r1, i32 %r44
    %r46 = load float, ptr %r45
    store float %r46, ptr %r43
    %r47 = add i32 2,0
    %r48 = getelementptr float, ptr %r8, i32 %r47
    %r49 = add i32 2,0
    %r50 = getelementptr float, ptr %r2, i32 %r49
    %r51 = load float, ptr %r50
    store float %r51, ptr %r48
    %r52 = add i32 0,0
    %r53 = getelementptr float, ptr %r6, i32 %r52
    %r54 = add i32 0,0
    %r55 = getelementptr float, ptr %r0, i32 %r54
    %r56 = load float, ptr %r55
    store float %r56, ptr %r53
    %r57 = add i32 0,0
    ret i32 %r57
}
define i32 @main()
{
L0:  ;
    %r11 = alloca [3 x float]
    %r10 = alloca [3 x float]
    %r9 = alloca [6 x float]
    %r8 = alloca [3 x float]
    %r7 = alloca [3 x float]
    %r6 = alloca [3 x float]
    %r5 = alloca [3 x float]
    %r4 = alloca [3 x float]
    %r3 = alloca [3 x float]
    br label %L1
L1:  ;
    %r0 = add i32 3,0
    store i32 %r0, ptr @N
    %r1 = add i32 3,0
    store i32 %r1, ptr @M
    %r2 = add i32 3,0
    store i32 %r2, ptr @L
    %r14 = add i32 0,0
    br label %L2
L2:  ;
    %r102 = phi i32 [%r14,%L1],[%r44,%L3]
    %r16 = load i32, ptr @M
    %r17 = icmp slt i32 %r102,%r16
    br i1 %r17, label %L3, label %L4
L3:  ;
    %r19 = getelementptr [3 x float], ptr %r3, i32 0, i32 %r102
    %r21 = sitofp i32 %r102 to float
    store float %r21, ptr %r19
    %r23 = getelementptr [3 x float], ptr %r4, i32 0, i32 %r102
    %r25 = sitofp i32 %r102 to float
    store float %r25, ptr %r23
    %r27 = getelementptr [3 x float], ptr %r5, i32 0, i32 %r102
    %r29 = sitofp i32 %r102 to float
    store float %r29, ptr %r27
    %r31 = getelementptr [3 x float], ptr %r6, i32 0, i32 %r102
    %r33 = sitofp i32 %r102 to float
    store float %r33, ptr %r31
    %r35 = getelementptr [3 x float], ptr %r7, i32 0, i32 %r102
    %r37 = sitofp i32 %r102 to float
    store float %r37, ptr %r35
    %r39 = getelementptr [3 x float], ptr %r8, i32 0, i32 %r102
    %r41 = sitofp i32 %r102 to float
    store float %r41, ptr %r39
    %r43 = add i32 1,0
    %r44 = add i32 %r102,%r43
    br label %L2
L4:  ;
    %r45 = getelementptr [3 x float], ptr %r3, i32 0
    %r46 = getelementptr [3 x float], ptr %r4, i32 0
    %r47 = getelementptr [3 x float], ptr %r5, i32 0
    %r48 = getelementptr [3 x float], ptr %r6, i32 0
    %r49 = getelementptr [3 x float], ptr %r7, i32 0
    %r50 = getelementptr [3 x float], ptr %r8, i32 0
    %r51 = getelementptr [6 x float], ptr %r9, i32 0
    %r52 = getelementptr [3 x float], ptr %r10, i32 0
    %r53 = getelementptr [3 x float], ptr %r11, i32 0
    %r54 = call i32 @tran(ptr %r45,ptr %r46,ptr %r47,ptr %r48,ptr %r49,ptr %r50,ptr %r51,ptr %r52,ptr %r53)
    br label %L5
L5:  ;
    %r103 = phi i32 [%r54,%L4],[%r67,%L6]
    %r58 = load i32, ptr @N
    %r59 = icmp slt i32 %r103,%r58
    br i1 %r59, label %L6, label %L7
L6:  ;
    %r61 = getelementptr [6 x float], ptr %r9, i32 0, i32 %r103
    %r62 = load float, ptr %r61
    %r63 = fptosi float %r62 to i32
    call void @putint(i32 %r63)
    %r66 = add i32 1,0
    %r67 = add i32 %r103,%r66
    br label %L5
L7:  ;
    %r68 = add i32 10,0
    call void @putch(i32 %r68)
    %r70 = add i32 0,0
    br label %L8
L8:  ;
    %r104 = phi i32 [%r70,%L7],[%r81,%L9]
    %r72 = load i32, ptr @N
    %r73 = icmp slt i32 %r104,%r72
    br i1 %r73, label %L9, label %L10
L9:  ;
    %r75 = getelementptr [3 x float], ptr %r10, i32 0, i32 %r104
    %r76 = load float, ptr %r75
    %r77 = fptosi float %r76 to i32
    call void @putint(i32 %r77)
    %r80 = add i32 1,0
    %r81 = add i32 %r104,%r80
    br label %L8
L10:  ;
    %r82 = add i32 10,0
    %r83 = add i32 0,0
    call void @putch(i32 %r82)
    br label %L11
L11:  ;
    %r105 = phi i32 [%r83,%L10],[%r95,%L12]
    %r86 = load i32, ptr @N
    %r87 = icmp slt i32 %r105,%r86
    br i1 %r87, label %L12, label %L13
L12:  ;
    %r89 = getelementptr [3 x float], ptr %r11, i32 0, i32 %r105
    %r90 = load float, ptr %r89
    %r91 = fptosi float %r90 to i32
    call void @putint(i32 %r91)
    %r94 = add i32 1,0
    %r95 = add i32 %r105,%r94
    br label %L11
L13:  ;
    %r96 = add i32 10,0
    call void @putch(i32 %r96)
    %r98 = add i32 0,0
    ret i32 %r98
}
