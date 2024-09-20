	.file	"test.c"
	.option nopic
	.attribute arch, "rv64i2p1_m2p0_a2p1_f2p2_d2p2_c2p0_zicsr2p0"
	.attribute unaligned_access, 0
	.attribute stack_align, 16
	.text
	.section	.rodata
	.align	3
.LC1:
	.string	"\350\257\267\350\276\223\345\205\245\344\270\200\344\270\252\346\225\264\346\225\260\357\274\232"
	.align	3
.LC2:
	.string	"%d"
	.align	3
.LC3:
	.string	"\351\230\266\344\271\230\347\273\223\346\236\234\346\230\257\357\274\232%g\n"
	.text
	.align	1
	.globl	main
	.type	main, @function
main:
	addi	sp,sp,-32
	sd	ra,24(sp)
	sd	s0,16(sp)
	addi	s0,sp,32
	li	t0,-8192
	addi	t0,t0,192
	add	sp,sp,t0
	lui	a5,%hi(.LC0)
	fld	fa5,%lo(.LC0)(a5)
	fsd	fa5,-24(s0)
	lui	a5,%hi(.LC1)
	addi	a0,a5,%lo(.LC1)
	call	printf
	addi	a5,s0,-32
	mv	a1,a5
	lui	a5,%hi(.LC2)
	addi	a0,a5,%lo(.LC2)
	call	__isoc99_scanf
	sw	zero,-28(s0)
	j	.L2
.L3:
	lw	a5,-28(s0)
	addiw	a5,a5,1
	sext.w	a5,a5
	fcvt.d.w	fa5,a5
	li	a5,-8192
	addi	a5,a5,-16
	add	a4,a5,s0
	lw	a5,-28(s0)
	slli	a5,a5,3
	add	a5,a4,a5
	fsd	fa5,176(a5)
	lw	a5,-28(s0)
	addiw	a5,a5,1
	sw	a5,-28(s0)
.L2:
	lw	a4,-32(s0)
	lw	a5,-28(s0)
	sext.w	a5,a5
	blt	a5,a4,.L3
	sw	zero,-28(s0)
	j	.L4
.L5:
	li	a5,-8192
	addi	a5,a5,-16
	add	a4,a5,s0
	lw	a5,-28(s0)
	slli	a5,a5,3
	add	a5,a4,a5
	fld	fa5,176(a5)
	fld	fa4,-24(s0)
	fmul.d	fa5,fa4,fa5
	fsd	fa5,-24(s0)
	lw	a5,-28(s0)
	addiw	a5,a5,1
	sw	a5,-28(s0)
.L4:
	lw	a4,-32(s0)
	lw	a5,-28(s0)
	sext.w	a5,a5
	blt	a5,a4,.L5
	ld	a1,-24(s0)
	lui	a5,%hi(.LC3)
	addi	a0,a5,%lo(.LC3)
	call	printf
	li	a5,0
	mv	a0,a5
	li	t0,8192
	addi	t0,t0,-192
	add	sp,sp,t0
	ld	ra,24(sp)
	ld	s0,16(sp)
	addi	sp,sp,32
	jr	ra
	.size	main, .-main
	.section	.rodata
	.align	3
.LC0:
	.word	0
	.word	1072693248
	.ident	"GCC: () 12.2.0"
	.section	.note.GNU-stack,"",@progbits
