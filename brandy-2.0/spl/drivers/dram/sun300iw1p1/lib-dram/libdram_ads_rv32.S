	.file	"mctl_hal.c"
	.option nopic
	.option verbatim
	.option cmodel_medany
	.text
	.section	.text.handler_super_standby,"ax",@progbits
	.align	1
	.weak	handler_super_standby
	.type	handler_super_standby, @function
handler_super_standby:
	ret
	.size	handler_super_standby, .-handler_super_standby
	.section	.text.get_pmu_exist,"ax",@progbits
	.align	1
	.weak	get_pmu_exist
	.type	get_pmu_exist, @function
get_pmu_exist:
	li	a0,-1
	ret
	.size	get_pmu_exist, .-get_pmu_exist
	.section	.text.memcpy_self,"ax",@progbits
	.align	1
	.globl	memcpy_self
	.type	memcpy_self, @function
memcpy_self:
	li	a5,0
	.align	1
.L4:
	sub	a4,a2,a5
	bgt	a4,zero,.L5
	ret
	.align	1
.L5:
	add	a4,a1,a5
	lbu	a3,0(a4)
	add	a4,a0,a5
	sb	a3,0(a4)
	addi	a5,a5,1
	j	.L4
	.size	memcpy_self, .-memcpy_self
	.section	.text.dram_udelay,"ax",@progbits
	.align	1
	.globl	dram_udelay
	.type	dram_udelay, @function
dram_udelay:
	tail	__usdelay
	.size	dram_udelay, .-dram_udelay
	.section	.text.paraconfig,"ax",@progbits
	.align	1
	.globl	paraconfig
	.type	paraconfig, @function
paraconfig:
	lw	a5,0(a0)
	not	a1,a1
	and	a1,a1,a5
	or	a1,a1,a2
	sw	a1,0(a0)
	ret
	.size	paraconfig, .-paraconfig
	.section	.text.dram_enable_all_master,"ax",@progbits
	.align	1
	.globl	dram_enable_all_master
	.type	dram_enable_all_master, @function
dram_enable_all_master:
	li	a5,1125130240
	li	a4,-1
	sw	a4,32(a5)
	li	a4,255
	sw	a4,36(a5)
	li	a4,65536
	addi	a4,a4,-1
	li	a0,10
	sw	a4,40(a5)
	tail	__usdelay
	.size	dram_enable_all_master, .-dram_enable_all_master
	.section	.text.dram_disable_all_master,"ax",@progbits
	.align	1
	.globl	dram_disable_all_master
	.type	dram_disable_all_master, @function
dram_disable_all_master:
	li	a5,1125130240
	li	a4,1
	sw	a4,32(a5)
	sw	zero,36(a5)
	li	a0,10
	sw	zero,40(a5)
	tail	__usdelay
	.size	dram_disable_all_master, .-dram_disable_all_master
	.section	.text.eye_delay_compensation,"ax",@progbits
	.align	1
	.globl	eye_delay_compensation
	.type	eye_delay_compensation, @function
eye_delay_compensation:
	call	t0,__riscv_save_2
	lw	a5,88(a0)
	lw	a4,84(a0)
	bfoz	a3,a5,1,4
	mv	s1,a0
	bfoz	a0,a4,9,12
	or	a0,a0,a3
	li	a2,1125134336
	li	a3,1125134336
	addi	a2,a2,784
	addi	a6,a3,820
	.align	1
.L14:
	lw	a1,0(a2)
	or	a1,a0,a1
	sw	a1,0(a2)
	addi	a2,a2,4
	bne	a2,a6,.L14
	li	a1,8192
	addi	a1,a1,-512
	slli	a2,a4,5
	and	a2,a2,a1
	srli	a1,a5,4
	bfoz	a1,a1,1,4
	li	s0,1125134336
	or	a2,a2,a1
	addi	a3,a3,912
	addi	a0,s0,948
	.align	1
.L15:
	lw	a1,0(a3)
	or	a1,a2,a1
	sw	a1,0(a3)
	addi	a3,a3,4
	bne	a3,a0,.L15
	addi	a0,s0,256
	lw	a2,0(a0)
	li	a1,-67108864
	addi	a1,a1,-1
	and	a2,a2,a1
	li	a6,8192
	srli	a1,a4,16
	sw	a2,0(a0)
	slli	t1,a1,9
	srli	a2,a5,16
	addi	a6,a6,-512
	lw	a7,820(s0)
	and	t1,t1,a6
	bfoz	a2,a2,1,4
	or	a2,a2,t1
	or	a7,a2,a7
	sw	a7,820(s0)
	lw	t1,824(s0)
	srli	a4,a4,20
	or	a2,a2,t1
	slli	a7,a4,9
	srli	a5,a5,20
	sw	a2,824(s0)
	and	a6,a7,a6
	lw	a2,0(a3)
	bfoz	a5,a5,1,4
	or	a5,a5,a6
	or	a2,a5,a2
	sw	a2,0(a3)
	addi	a3,s0,952
	lw	a2,0(a3)
	bfoz	a1,a1,25,28
	or	a5,a5,a2
	sw	a5,0(a3)
	addi	a5,s0,828
	lw	a3,0(a5)
	bfoz	a4,a4,25,28
	or	a1,a1,a3
	sw	a1,0(a5)
	addi	a5,s0,956
	lw	a3,0(a5)
	addi	s0,s0,576
	or	a4,a4,a3
	sw	a4,0(a5)
	lw	a5,0(a0)
	li	a4,67108864
	or	a5,a5,a4
	sw	a5,0(a0)
	li	a0,1
	call	__usdelay
	lw	a5,80(s1)
	li	a4,4096
	addi	a4,a4,-256
	slli	a2,a5,4
	and	a2,a2,a4
	li	a4,1125134336
	addi	a1,a4,636
	.align	1
.L16:
	lw	a3,0(s0)
	or	a3,a2,a3
	sw	a3,0(s0)
	addi	s0,s0,4
	bne	s0,a1,.L16
	li	a3,1125134336
	addi	a4,a4,552
	addi	a0,a3,576
	.align	1
.L17:
	lw	a1,0(a4)
	or	a1,a2,a1
	sw	a1,0(a4)
	addi	a4,a4,4
	bne	a4,a0,.L17
	addi	a1,a3,536
	li	a4,4096
	lw	a0,0(a1)
	addi	a4,a4,-256
	slli	a2,a5,8
	and	a2,a2,a4
	or	a2,a2,a0
	sw	a2,0(a1)
	addi	a1,a3,540
	lw	a0,0(a1)
	and	a2,a5,a4
	or	a2,a2,a0
	addi	a3,a3,640
	sw	a2,0(a1)
	srli	a5,a5,4
	lw	a2,0(a3)
	and	a5,a5,a4
	or	a5,a5,a2
	sw	a5,0(a3)
	tail	__riscv_restore_2
	.size	eye_delay_compensation, .-eye_delay_compensation
	.section	.text.bit_delay_compensation,"ax",@progbits
	.align	1
	.globl	bit_delay_compensation
	.type	bit_delay_compensation, @function
bit_delay_compensation:
	call	t0,__riscv_save_1
	addi	sp,sp,-352
	lui	a1,%hi(.LANCHOR0)
	addi	s0,a1,%lo(.LANCHOR0)
	li	a2,176
	addi	a1,a1,%lo(.LANCHOR0)
	mv	a0,sp
	call	memcpy
	li	a2,176
	addi	a1,s0,176
	addi	a0,sp,176
	call	memcpy
	li	a5,1125134336
	addi	a2,a5,256
	lw	a4,0(a2)
	li	a3,-67108864
	addi	a3,a3,-1
	and	a4,a4,a3
	sw	a4,0(a2)
	li	a4,1125134336
	li	a0,44
	addi	a5,a5,784
	addi	a7,a4,1296
	.align	1
.L24:
	addi	a3,a0,-44
	mv	a1,a5
	.align	1
.L25:
	add	a6,sp,a3
	lw	a2,0(a1)
	lw	a6,0(a6)
	addi	s0,sp,176
	add	a6,a2,a6
	add	a2,s0,a3
	lw	a2,0(a2)
	addi	a3,a3,4
	slli	a2,a2,8
	add	a2,a2,a6
	sw	a2,0(a1)
	addi	a1,a1,4
	bne	a0,a3,.L25
	addi	a5,a5,128
	addi	a0,a0,44
	bne	a5,a7,.L24
	addi	a5,a4,256
	lw	a4,0(a5)
	li	a3,67108864
	or	a4,a4,a3
	sw	a4,0(a5)
	addi	sp,sp,352
	tail	__riscv_restore_1
	.size	bit_delay_compensation, .-bit_delay_compensation
	.section	.text.set_master_priority_pad,"ax",@progbits
	.align	1
	.globl	set_master_priority_pad
	.type	set_master_priority_pad, @function
set_master_priority_pad:
	li	a5,1125130240
	lw	a4,12(a5)
	li	a3,-4096
	and	a3,a4,a3
	lw	a4,0(a0)
	addi	a2,a5,532
	srli	a4,a4,1
	addi	a4,a4,-1
	or	a4,a4,a3
	sw	a4,12(a5)
	li	a3,65536
	addi	a4,a5,512
	sw	a3,0(a4)
	li	a4,16777216
	addi	a4,a4,9
	addi	a3,a5,528
	sw	a4,0(a3)
	li	a3,5242880
	addi	a3,a3,100
	sw	a3,0(a2)
	li	a2,33554432
	addi	a1,a5,560
	addi	a2,a2,13
	sw	a2,0(a1)
	li	a2,6291456
	addi	a1,a5,564
	addi	a2,a2,256
	sw	a2,0(a1)
	addi	a2,a5,576
	sw	a4,0(a2)
	addi	a2,a5,580
	sw	a3,0(a2)
	li	a3,6553600
	addi	a2,a5,608
	addi	a3,a3,521
	sw	a3,0(a2)
	li	a3,2097152
	addi	a2,a5,612
	addi	a3,a3,64
	sw	a3,0(a2)
	addi	a3,a5,656
	sw	a4,0(a3)
	li	a4,4194304
	addi	a3,a5,660
	addi	a4,a4,128
	sw	a4,0(a3)
	addi	a4,a5,1136
	sw	zero,0(a4)
	addi	a5,a5,1140
	sw	zero,0(a5)
	li	a4,260059136
	li	a5,1125134336
	addi	a3,a5,448
	addi	a4,a4,-251
	sw	a4,0(a3)
	li	a4,251658240
	addi	a3,a5,456
	addi	a4,a4,255
	sw	a4,0(a3)
	li	a4,1056964608
	addi	a5,a5,464
	addi	a4,a4,95
	sw	a4,0(a5)
	ret
	.size	set_master_priority_pad, .-set_master_priority_pad
	.section	.text.auto_cal_timing,"ax",@progbits
	.align	1
	.globl	auto_cal_timing
	.type	auto_cal_timing, @function
auto_cal_timing:
	mul	a1,a0,a1
	li	a5,1000
	remu	a0,a1,a5
	divu	a1,a1,a5
	snez	a0,a0
	add	a0,a0,a1
	ret
	.size	auto_cal_timing, .-auto_cal_timing
	.section	.text.auto_set_timing_para,"ax",@progbits
	.align	1
	.globl	auto_set_timing_para
	.type	auto_set_timing_para, @function
auto_set_timing_para:
	call	t0,__riscv_save_12
	lw	a2,92(a0)
	addi	sp,sp,-64
	sw	a2,56(sp)
	lw	s1,0(a0)
	lw	s4,4(a0)
	mv	s2,a0
	bbc	a2,1,.L32
	lw	s7,40(a0)
	lw	s11,44(a0)
	bfoz	a2,s7,23,21
	lw	s0,48(a0)
	sw	a2,60(sp)
	bfoz	a2,s11,27,23
	sw	a2,12(sp)
	bfoz	a2,s11,19,15
	bfoz	s8,s7,20,15
	bfoz	s5,s7,14,11
	bfoz	s3,s7,10,6
	bfoz	s10,s11,22,20
	bfoz	a6,s11,14,11
	bfoz	s6,s11,10,6
	sw	a2,48(sp)
	andi	s7,s7,63
	bfoz	a2,s0,20,12
	andi	s11,s11,63
	bfoz	s0,s0,11,0
	sw	a2,16(sp)
	.align	1
.L33:
	lw	a2,36(s2)
	li	a5,6
	sw	a2,52(sp)
	lw	a2,28(s2)
	sw	a2,44(sp)
	beqc	s4,6,.L61
	bgtu	s4,a5,.L62
	beqc	s4,2,.L63
	beqc	s4,3,.L64
	.align	1
.L89:
	li	a2,8
	sw	a2,40(sp)
	li	a2,27
	sw	a2,36(sp)
	li	a2,12
	sw	a2,32(sp)
	li	a0,4
	li	a6,3
	li	t1,4
	li	s4,2
	li	t4,6
	li	a7,2
	li	a2,0
	li	a4,3
	li	a1,3
	li	t3,1
	li	a5,1
	li	t5,0
	li	t6,0
	li	t0,0
	li	a3,0
	li	s9,0
	sw	zero,28(sp)
	sw	zero,24(sp)
	sw	zero,20(sp)
	.align	1
.L65:
	lw	t2,48(sp)
	addi	s10,a1,2
	add	t2,s6,t2
	bgeu	t2,s10,.L70
	sub	t2,a1,s6
	addi	t2,t2,2
	sw	t2,48(sp)
	.align	1
.L70:
	lhu	t2,26(s2)
	bne	t2,zero,.L71
	sw	a3,24(s2)
	.align	1
.L71:
	lw	a3,44(sp)
	srli	a3,a3,16
	bne	a3,zero,.L72
	sw	t0,28(s2)
	.align	1
.L72:
	lhu	a3,34(s2)
	bne	a3,zero,.L73
	sw	t6,32(s2)
	.align	1
.L73:
	lw	a3,52(sp)
	srli	a3,a3,16
	bne	a3,zero,.L74
	sw	t5,36(s2)
	.align	1
.L74:
	lw	a3,24(s2)
	li	t5,65536
	addi	t6,t5,-1
	and	t5,a3,t6
	li	a3,1125134336
	sw	t5,48(a3)
	lw	t5,28(s2)
	lw	t0,32(sp)
	and	t5,t5,t6
	sw	t5,52(a3)
	lw	t5,32(s2)
	slli	s8,s8,16
	and	t5,t5,t6
	sw	t5,56(a3)
	lw	t5,36(s2)
	or	s11,s8,s11
	slli	s8,t0,24
	lw	t0,36(sp)
	and	t5,t5,t6
	sw	t5,60(a3)
	or	s11,s11,s8
	lw	t5,12(s2)
	slli	s8,t0,8
	slli	a1,a1,16
	lw	t0,12(sp)
	slli	a4,a4,24
	slli	a2,a2,16
	slli	a7,a7,12
	lw	t6,48(sp)
	or	a4,a4,a1
	or	a2,a2,a7
	lw	a1,40(sp)
	slli	t2,t0,16
	or	t4,a2,t4
	bfoz	t5,t5,5,4
	lw	a2,60(sp)
	sw	t5,44(a3)
	or	s7,t2,s7
	or	a4,a4,a1
	or	s8,s11,s8
	slli	t2,t6,8
	slli	t1,t1,8
	sw	s8,88(a3)
	or	t1,a4,t1
	or	t2,s7,t2
	slli	a4,a2,16
	slli	s3,s3,24
	sw	t2,92(a3)
	or	s3,s3,a4
	slli	a4,a0,24
	slli	a0,a0,16
	sw	t1,96(a3)
	or	a0,a4,a0
	or	s6,s3,s6
	slli	s5,s5,8
	sw	t4,100(a3)
	or	s5,s6,s5
	or	s4,a0,s4
	slli	a6,a6,8
	sw	s5,104(a3)
	or	a6,s4,a6
	sw	a6,108(a3)
	lw	a4,120(a3)
	li	a3,268369920
	and	a4,a4,a3
	li	a3,800
	bgtu	s1,a3,.L75
	li	a3,-268410880
	.align	1
.L90:
	addi	a3,a3,1536
	slli	a5,a5,16
	or	a4,a4,a3
	or	t3,a5,t3
	li	a5,33554432
	lw	s1,20(sp)
	li	a3,1125134336
	ori	a4,a4,16
	addi	a5,a5,256
	sw	a4,120(a3)
	or	t3,t3,a5
	addi	a5,a3,128
	sw	t3,0(a5)
	slli	a5,s1,20
	lw	s1,28(sp)
	or	a5,a5,s9
	sw	a5,80(a3)
	slli	a5,s1,20
	lw	s1,24(sp)
	or	a5,a5,s1
	lw	s1,16(sp)
	sw	a5,84(a3)
	slli	a5,s0,16
	or	s9,a5,s1
	addi	a5,a3,144
	sw	s9,0(a5)
	slli	s0,s0,15
	li	a5,268369920
	and	s0,s0,a5
	addi	a3,a3,148
	sw	s0,0(a3)
	addi	sp,sp,64
	tail	__riscv_restore_12
	.align	1
.L32:
	srli	s9,s1,1
	bnec	s4,3,.L34
	mv	a1,s9
	li	a0,8
	call	auto_cal_timing
	mv	a1,s9
	mv	s10,a0
	li	a0,15
	call	auto_cal_timing
	mv	s6,a0
	li	a0,8192
	mv	a1,s9
	addi	a0,a0,-392
	call	auto_cal_timing
	andi	s0,a0,31
	snez	s0,s0
	srli	a0,a0,5
	mv	a1,s9
	add	s0,s0,a0
	li	a0,350
	call	auto_cal_timing
	li	a5,800
	sw	a0,16(sp)
	mv	a1,s9
	bgtu	s1,a5,.L35
	li	a0,50
	call	auto_cal_timing
	mv	a1,s9
	mv	s8,a0
	li	a0,10
	call	auto_cal_timing
	li	a5,2
	mv	s5,a0
	bgeu	a0,a5,.L36
	li	s5,2
	.align	1
.L36:
	mv	a1,s9
	li	a0,53
	call	auto_cal_timing
	li	a5,2
	mv	s7,a0
	bgeu	s10,a5,.L37
	li	s10,2
	.align	1
.L37:
	li	a5,2
	mv	a6,s6
	bgeu	s6,a5,.L38
	li	a6,2
	.align	1
.L38:
	mv	a1,s9
	li	a0,38
	sw	a6,20(sp)
	mv	s3,s6
	call	auto_cal_timing
	mv	s11,a0
	sw	s10,12(sp)
	.align	1
.L86:
	lw	a6,20(sp)
	.align	1
.L39:
	slli	a4,s8,15
	slli	a5,s3,6
	or	a5,a5,a4
	or	a5,a5,s7
	slli	a4,s5,11
	or	a5,a5,a4
	li	a4,4194304
	or	a5,a5,a4
	sw	a5,40(s2)
	slli	a4,a6,11
	slli	a5,s6,6
	or	a5,a5,a4
	lw	a2,12(sp)
	slli	a4,s10,15
	or	a5,a5,s11
	or	a5,a5,a4
	slli	a4,s10,20
	or	a5,a5,a4
	slli	a4,a2,23
	or	a5,a5,a4
	sw	a5,44(s2)
	lw	a5,56(s2)
	li	a4,1
	bfoz	a5,a5,15,12
	beqc	a5,1,.L56
	beqc	a5,2,.L57
	.align	1
.L58:
	lw	a2,16(sp)
	sw	s10,48(sp)
	slli	a5,a2,12
	or	a5,a5,s0
	li	a2,2
	sw	a5,48(s2)
	sw	a2,60(sp)
	j	.L33
	.align	1
.L35:
	li	a0,35
	call	auto_cal_timing
	mv	a1,s9
	mv	s8,a0
	li	a0,6
	call	auto_cal_timing
	li	a5,2
	mv	s5,a0
	bgeu	a0,a5,.L40
	li	s5,2
	.align	1
.L40:
	mv	a1,s9
	li	a0,14
	call	auto_cal_timing
	mv	a1,s9
	mv	s3,a0
	li	a0,48
	call	auto_cal_timing
	li	a5,2
	mv	s7,a0
	bgeu	s10,a5,.L41
	li	s10,2
	.align	1
.L41:
	li	a5,2
	mv	a6,s6
	bgeu	s6,a5,.L42
	li	a6,2
	.align	1
.L42:
	mv	a1,s9
	li	a0,34
	sw	a6,20(sp)
	mv	s6,s3
	call	auto_cal_timing
	sw	s5,12(sp)
	mv	s11,a0
	j	.L86
	.align	1
.L34:
	bnec	s4,2,.L43
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	mv	a1,s9
	mv	s8,a0
	li	a0,10
	call	auto_cal_timing
	mv	a1,s9
	mv	s5,a0
	li	a0,20
	call	auto_cal_timing
	mv	a1,s9
	mv	s3,a0
	li	a0,65
	call	auto_cal_timing
	mv	a1,s9
	mv	s7,a0
	li	a0,8
	call	auto_cal_timing
	mv	a1,s9
	mv	s10,a0
	li	a0,15
	call	auto_cal_timing
	mv	a1,s9
	mv	s6,a0
	li	a0,45
	call	auto_cal_timing
	mv	s11,a0
	li	a0,8192
	mv	a1,s9
	addi	a0,a0,-392
	call	auto_cal_timing
	srli	s0,a0,5
	andi	a0,a0,31
	snez	a0,a0
	add	s0,s0,a0
	mv	a1,s9
	li	a0,328
	call	auto_cal_timing
	li	a2,2
	mv	a6,s6
	sw	a0,16(sp)
	sw	a2,12(sp)
	j	.L39
	.align	1
.L43:
	bnec	s4,6,.L44
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	li	a5,4
	mv	s8,a0
	bgeu	a0,a5,.L45
	li	s8,4
	.align	1
.L45:
	mv	a1,s9
	li	a0,10
	call	auto_cal_timing
	mv	s5,a0
	bne	a0,zero,.L46
	li	s5,1
	.align	1
.L46:
	mv	a1,s9
	li	a0,24
	call	auto_cal_timing
	li	a5,2
	mv	s3,a0
	bgeu	a0,a5,.L47
	li	s3,2
	.align	1
.L47:
	mv	a1,s9
	li	a0,70
	call	auto_cal_timing
	mv	a1,s9
	mv	s7,a0
	li	a0,8
	call	auto_cal_timing
	sw	a0,12(sp)
	bne	a0,zero,.L48
	li	s0,1
	sw	s0,12(sp)
	.align	1
.L48:
	li	a5,2
	mv	s10,a0
	bgeu	a0,a5,.L49
	li	s10,2
	.align	1
.L49:
	mv	a1,s9
	li	a0,15
	call	auto_cal_timing
	li	a5,2
	mv	a6,a0
	bgeu	a0,a5,.L50
	li	a6,2
	.align	1
.L50:
	mv	a1,s9
	li	a0,17
	sw	a6,20(sp)
	call	auto_cal_timing
	mv	a1,s9
	mv	s6,a0
	li	a0,42
	call	auto_cal_timing
	mv	s11,a0
	li	a0,4096
	mv	a1,s9
	addi	a0,a0,-196
	call	auto_cal_timing
	srli	s0,a0,5
	andi	a0,a0,31
	snez	a0,a0
	add	s0,s0,a0
	mv	a1,s9
	li	a0,210
	call	auto_cal_timing
	sw	a0,16(sp)
	j	.L86
	.align	1
.L44:
	bnec	s4,7,.L77
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	li	a5,4
	mv	s8,a0
	bgeu	a0,a5,.L51
	li	s8,4
	.align	1
.L51:
	mv	a1,s9
	li	a0,10
	call	auto_cal_timing
	mv	s5,a0
	bne	a0,zero,.L52
	li	s5,1
	.align	1
.L52:
	mv	a1,s9
	li	a0,24
	call	auto_cal_timing
	li	a5,2
	mv	s3,a0
	bgeu	a0,a5,.L53
	li	s3,2
	.align	1
.L53:
	mv	a1,s9
	li	a0,70
	call	auto_cal_timing
	mv	a1,s9
	mv	s7,a0
	li	a0,8
	call	auto_cal_timing
	li	a5,2
	mv	s10,a0
	bgeu	a0,a5,.L54
	li	s10,2
	.align	1
.L54:
	mv	a1,s9
	li	a0,15
	call	auto_cal_timing
	li	a5,2
	mv	a6,a0
	bgeu	a0,a5,.L55
	li	a6,2
	.align	1
.L55:
	mv	a1,s9
	li	a0,17
	sw	a6,20(sp)
	call	auto_cal_timing
	mv	a1,s9
	mv	s6,a0
	li	a0,42
	call	auto_cal_timing
	mv	s11,a0
	li	a0,4096
	mv	a1,s9
	addi	a0,a0,-196
	call	auto_cal_timing
	srli	s0,a0,5
	andi	a0,a0,31
	snez	a0,a0
	add	s0,s0,a0
	mv	a1,s9
	li	a0,210
	call	auto_cal_timing
	sw	s10,12(sp)
	sw	a0,16(sp)
	j	.L86
	.align	1
.L77:
	li	s0,128
	li	a2,10
	sw	s0,16(sp)
	li	a6,8
	li	s0,98
	li	s10,3
	li	s6,6
	li	s11,14
	li	s8,16
	li	s7,20
	li	s3,6
	li	s5,3
	sw	a2,12(sp)
	j	.L39
	.align	1
.L56:
	addi	a4,s4,-2
	bgtu	a4,a5,.L60
	li	a5,2
	.align	1
.L60:
	divu	s0,s0,a5
	j	.L58
	.align	1
.L57:
	addi	a3,s4,-2
	bgtu	a3,a4,.L60
	li	a5,4
	j	.L60
	.align	1
.L62:
	bnec	s4,7,.L89
	li	a5,60
	divu	a1,s1,a5
	li	a5,800
	sw	a1,36(sp)
	bleu	s1,a5,.L84
	li	a4,4
	li	t3,3
	li	a5,6
	li	t6,12
	.align	1
.L69:
	li	a3,200
	mul	a3,s1,a3
	li	a1,1000
	addi	a6,a6,5
	addi	a3,a3,1
	mv	s9,a3
	li	a3,100
	mul	a3,s1,a3
	addi	a2,s10,5
	lw	t5,52(sp)
	divu	a3,a3,a1
	li	a0,5
	li	t1,13
	li	t4,12
	li	a7,5
	addi	a1,a3,1
	li	a3,11
	mul	a3,s1,a3
	sw	a1,20(sp)
	addi	a1,s1,1
	addi	a3,a3,1
	sw	a3,24(sp)
	sw	a1,28(sp)
	add	a3,a6,a4
	add	a1,a2,a4
	sw	a1,40(sp)
	li	a6,5
	mv	a1,s4
	li	a2,5
	li	s4,3
	sw	a3,32(sp)
	j	.L87
	.align	1
.L63:
	li	a5,30
	divu	a2,s1,a5
	li	a5,409
	sw	a2,36(sp)
	bgtu	s1,a5,.L80
	li	a3,4096
	li	a1,3
	li	a5,1
	addi	a3,a3,-1437
	.align	1
.L67:
	li	a4,400
	mul	a4,s1,a4
	li	a0,1000
	lw	t0,44(sp)
	addi	a2,a4,1
	li	a4,500
	mul	a4,s1,a4
	mv	s9,a2
	li	a7,2
	divu	a4,a4,a0
	li	t1,4
	li	a0,5
	li	s4,3
	li	t4,12
	li	t3,1
	li	t5,0
	li	t6,0
	addi	a2,a4,1
	li	a4,200
	mul	a4,s1,a4
	sw	a2,20(sp)
	addi	a2,a4,1
	sw	a2,24(sp)
	addi	a2,s1,1
	sw	a2,28(sp)
	addi	a2,a6,5
	sw	a2,32(sp)
	addi	a2,s10,5
	sw	a2,40(sp)
	li	a6,4
	li	a2,0
	li	a4,3
	j	.L65
	.align	1
.L80:
	li	a3,4096
	li	a5,2
	li	a1,4
	addi	a3,a3,-397
	j	.L67
	.align	1
.L64:
	li	a5,30
	divu	a1,s1,a5
	li	a5,800
	sw	a1,36(sp)
	bleu	s1,a5,.L81
	li	a3,8192
	li	t3,3
	li	a1,7
	li	a5,5
	li	t6,32
	addi	a3,a3,-492
	.align	1
.L68:
	li	a4,500
	mul	a4,s1,a4
	li	a0,1000
	addi	a6,a6,2
	addi	a2,a4,1
	li	a4,360
	mul	a4,s1,a4
	mv	s9,a2
	divu	a4,a4,a0
	addi	a2,a4,1
	li	a4,200
	mul	a4,s1,a4
	sw	a2,20(sp)
	addi	a2,a4,1
	sw	a2,24(sp)
	addi	a2,s1,1
	sw	a2,28(sp)
	add	a2,a6,a5
	sw	a2,32(sp)
	addi	a2,s10,2
	add	a2,a2,a5
	sw	a2,40(sp)
	lw	a2,56(sp)
	bfoz	a4,a2,3,2
	beqc	a4,1,.L83
	li	a4,912
	bleu	s1,a4,.L83
	lw	t0,44(sp)
	mv	a4,a5
	li	a0,5
	li	a6,4
	li	t1,6
	.align	1
.L88:
	li	t4,12
	li	a7,4
	li	a2,0
	li	t5,0
	j	.L65
	.align	1
.L81:
	li	a3,8192
	li	a1,6
	li	t3,2
	li	a5,4
	li	t6,24
	addi	a3,a3,-912
	j	.L68
	.align	1
.L61:
	li	a5,60
	divu	a1,s1,a5
	li	a5,200
	mul	a5,s1,a5
	li	a4,1000
	lw	t5,52(sp)
	li	t6,6
	li	a0,5
	li	t1,10
	li	s4,2
	li	t4,5
	li	a7,5
	li	a2,3
	li	t3,1
	sw	a1,36(sp)
	addi	a1,a5,1
	li	a5,100
	mul	a5,s1,a5
	mv	s9,a1
	divu	a5,a5,a4
	li	a4,2
	addi	a1,a5,1
	li	a5,11
	mul	a5,s1,a5
	sw	a1,20(sp)
	addi	a1,a5,1
	sw	a1,24(sp)
	addi	a1,s1,1
	sw	a1,28(sp)
	addi	a1,a6,5
	sw	a1,32(sp)
	addi	a1,s10,5
	sw	a1,40(sp)
	li	a6,5
	li	a1,4
	li	a5,3
	.align	1
.L87:
	li	t0,195
	li	a3,0
	j	.L65
	.align	1
.L84:
	li	a4,3
	li	s4,6
	li	t3,2
	li	a5,5
	li	t6,10
	j	.L69
	.align	1
.L83:
	lw	t0,44(sp)
	mv	a4,a5
	li	a0,5
	li	a6,4
	li	t1,5
	j	.L88
	.align	1
.L75:
	li	a3,-268406784
	j	.L90
	.size	auto_set_timing_para, .-auto_set_timing_para
	.section	.text.ccm_get_hosc_freq,"ax",@progbits
	.align	1
	.globl	ccm_get_hosc_freq
	.type	ccm_get_hosc_freq, @function
ccm_get_hosc_freq:
	li	a5,1241579520
	addi	a5,a5,1032
	lw	a4,0(a5)
	li	a4,1
	li	a3,1241579520
	sw	a4,0(a5)
	.align	1
.L92:
	addi	a5,a3,1032
	lw	a4,0(a5)
	bbc	a4,1,.L92
	lw	a5,0(a5)
	li	a4,-57344
	addi	a4,a4,-1857
	bfoz	a5,a5,23,4
	add	a5,a5,a4
	li	a4,40960
	addi	a4,a4,-162
	li	a0,40
	bleu	a5,a4,.L91
	li	a0,24
	.align	1
.L91:
	ret
	.size	ccm_get_hosc_freq, .-ccm_get_hosc_freq
	.section	.srodata.cst4,"aM",@progbits,4
	.align	2
.LC2:
	.word	1103101952
	.align	2
.LC3:
	.word	1207959552
	.section	.text.ccm_get_sscg,"ax",@progbits
	.align	1
	.globl	ccm_get_sscg
	.type	ccm_get_sscg, @function
ccm_get_sscg:
	li	a5,24
	divu	a4,a1,a5
	mul	a5,a4,a5
	beq	a5,a1,.L104
	call	t0,__riscv_save_2
	lw	a5,0(a2)
	bbc	a5,1,.L99
	lw	a5,0(a3)
	li	a0,524288
	or	a5,a5,a0
	sw	a5,0(a3)
	.align	1
.L99:
	fcvt.s.wu	fa5,a1
	lui	a5,%hi(.LC2)
	flw	fa4,%lo(.LC2)(a5)
	lui	a5,%hi(.LC3)
	li	a0,20
	mv	s0,a3
	fdiv.s	fa5,fa5,fa4
	mv	s1,a2
	fcvt.s.w	fa4,a4
	li	a4,655360
	addi	a4,a4,-1
	fsub.s	fa5,fa5,fa4
	flw	fa4,%lo(.LC3)(a5)
	fmul.s	fa5,fa5,fa4
	fcvt.w.s a5,fa5,rtz
	sw	a5,0(a3)
	lw	a5,0(a3)
	and	a5,a5,a4
	li	a4,-536477696
	or	a5,a5,a4
	sw	a5,0(a3)
	call	__usdelay
	lw	a5,0(s1)
	li	a4,16777216
	or	a5,a5,a4
	sw	a5,0(s1)
	lw	a0,0(s0)
	bfoz	a0,a0,16,0
	tail	__riscv_restore_2
	.align	1
.L104:
	li	a0,0
	ret
	.size	ccm_get_sscg, .-ccm_get_sscg
	.section	.text.ccm_set_pll_sscg,"ax",@progbits
	.align	1
	.globl	ccm_set_pll_sscg
	.type	ccm_set_pll_sscg, @function
ccm_set_pll_sscg:
	call	t0,__riscv_save_0
	lw	a5,0(a1)
	li	a4,-1572864
	addi	a6,a4,-1
	lw	a4,0(a2)
	bfoz	a5,a5,2,1
	addi	sp,sp,-16
	and	a4,a4,a6
	bne	a5,zero,.L110
	.align	1
.L122:
	sw	a4,0(a2)
	.align	1
.L111:
	lw	a5,92(a0)
	li	a4,4
	bfoz	a5,a5,22,20
	addi	a5,a5,-1
	bgtu	a5,a4,.L113
	lui	a4,%hi(.L115)
	addi	a4,a4,%lo(.L115)
	lea.w	a5,a4,a5
	lw	a5,0(a5)
	jr	a5
	.section	.rodata
	.align	2
	.align	2
.L115:
	.word	.L119
	.word	.L118
	.word	.L117
	.word	.L113
	.word	.L114
	.section	.text.ccm_set_pll_sscg
	.align	1
.L110:
	bnec	a5,1,.L112
	li	a6,524288
	or	a4,a4,a6
	j	.L122
	.align	1
.L112:
	slli	a5,a5,19
	or	a5,a4,a5
	sw	a5,0(a2)
	j	.L111
	.align	1
.L119:
	li	a5,-922300416
	addi	a5,a5,-820
	.align	1
.L123:
	sw	a5,0(a2)
	li	a0,20
	li	a5,-2147483648
	sw	a1,12(sp)
	sw	a5,0(a3)
	call	__usdelay
	lw	a1,12(sp)
	li	a4,16777216
	lw	a5,0(a1)
	li	a0,0
	or	a5,a5,a4
	sw	a5,0(a1)
	addi	sp,sp,16
	tail	__riscv_restore_0
	.align	1
.L118:
	li	a5,-771317760
	addi	a5,a5,-1639
	j	.L123
	.align	1
.L117:
	li	a5,-620339200
	addi	a5,a5,1638
	j	.L123
	.align	1
.L114:
	li	a5,-351928320
	j	.L123
	.align	1
.L113:
	li	a5,-469356544
	addi	a5,a5,819
	j	.L123
	.size	ccm_set_pll_sscg, .-ccm_set_pll_sscg
	.section	.text.ccm_set_pll_ddr_clk,"ax",@progbits
	.align	1
	.globl	ccm_set_pll_ddr_clk
	.type	ccm_set_pll_ddr_clk, @function
ccm_set_pll_ddr_clk:
	call	t0,__riscv_save_4
	lw	a5,92(a1)
	bfoz	a5,a5,6,6
	bne	a5,a0,.L125
	lw	s0,0(a1)
	.align	1
.L133:
	slli	s0,s0,1
	call	ccm_get_hosc_freq
	divu	s0,s0,a0
	li	a5,11
	mv	s3,a0
	li	s1,1
	bgtu	s0,a5,.L127
	slli	s0,s0,2
	li	s1,2
	.align	1
.L127:
	li	a4,1241579520
	addi	a4,a4,128
	lw	a3,0(a4)
	li	a2,-134742016
	addi	a1,s1,-1
	addi	a2,a2,248
	and	a3,a3,a2
	slli	a5,a1,1
	or	a5,a5,a3
	addi	a3,s0,-1
	slli	a3,a3,8
	or	a5,a5,a1
	or	a5,a5,a3
	li	a3,-1073741824
	or	a5,a5,a3
	sw	a5,0(a4)
	li	a3,-536870912
	lw	a5,0(a4)
	addi	a3,a3,-1
	and	a5,a5,a3
	sw	a5,0(a4)
	lw	a5,0(a4)
	li	a3,536870912
	or	a5,a5,a3
	sw	a5,0(a4)
	li	a4,1241579520
	.align	1
.L128:
	lw	a5,128(a4)
	addi	s2,a4,128
	bbc	a5,28,.L128
	li	a0,20
	call	__usdelay
	mul	a0,s0,s3
	lw	a5,0(s2)
	li	a4,134217728
	divu	a0,a0,s1
	or	a5,a5,a4
	li	a3,1107300352
	sw	a5,0(s2)
	li	a4,-117637120
	lw	a5,4(a3)
	addi	a4,a4,-32
	and	a5,a5,a4
	li	a4,-2130706432
	or	a5,a5,a4
	sw	a5,4(a3)
	divu	a0,a0,s1
	tail	__riscv_restore_4
	.align	1
.L125:
	lw	s0,76(a1)
	j	.L133
	.size	ccm_set_pll_ddr_clk, .-ccm_set_pll_ddr_clk
	.section	.rodata.str1.4,"aMS",@progbits,1
	.align	2
.LC4:
	.string	"ddr_clk0 = %d MHz\n"
	.section	.text.mctl_sys_init,"ax",@progbits
	.align	1
	.globl	mctl_sys_init
	.type	mctl_sys_init, @function
mctl_sys_init:
	call	t0,__riscv_save_7
	li	s0,1107300352
	lw	a4,148(s0)
	li	a5,-4096
	addi	a5,a5,-1
	and	a4,a4,a5
	sw	a4,148(s0)
	lw	a4,144(s0)
	addi	s1,s0,132
	andi	a4,a4,-9
	sw	a4,144(s0)
	lw	a4,0(s1)
	li	s6,134217728
	and	a5,a4,a5
	sw	a5,0(s1)
	lw	a5,128(s0)
	mv	s5,a0
	andi	a5,a5,-9
	sw	a5,128(s0)
	lw	a5,4(s0)
	li	a0,10
	bfoz	a5,a5,30,0
	sw	a5,4(s0)
	lw	a4,4(s0)
	addi	s3,s0,148
	or	a4,a4,s6
	sw	a4,4(s0)
	call	__usdelay
	mv	a1,s5
	li	a0,0
	call	ccm_set_pll_ddr_clk
	mv	a1,a0
	srli	a5,a0,1
	lui	a0,%hi(.LC4)
	addi	a0,a0,%lo(.LC4)
	sw	a5,0(s5)
	call	printf
	li	a0,100
	call	__usdelay
	call	dram_disable_all_master
	lw	a4,144(s0)
	li	a5,4096
	ori	a4,a4,8
	sw	a4,144(s0)
	lw	a3,148(s0)
	li	a0,5
	or	a3,a3,a5
	sw	a3,148(s0)
	lw	a3,128(s0)
	addi	s4,s0,144
	ori	a3,a3,8
	sw	a3,128(s0)
	lw	a4,0(s1)
	addi	s2,s0,128
	or	a4,a4,a5
	sw	a4,0(s1)
	lw	a4,4(s0)
	li	a5,-2147483648
	or	a4,a4,a5
	sw	a4,4(s0)
	lw	a5,4(s0)
	or	a5,a5,s6
	sw	a5,4(s0)
	call	__usdelay
	li	a5,1125134336
	li	a4,32768
	li	a0,10
	sw	a4,12(a5)
	call	__usdelay
	li	a0,0
	tail	__riscv_restore_7
	.size	mctl_sys_init, .-mctl_sys_init
	.section	.text.mctl_com_init,"ax",@progbits
	.align	1
	.globl	mctl_com_init
	.type	mctl_com_init, @function
mctl_com_init:
	li	a4,1125130240
	lw	a5,8(a4)
	li	a3,-16384
	addi	a3,a3,255
	and	a5,a5,a3
	li	a3,8192
	or	a5,a5,a3
	lw	a2,20(a0)
	lw	a3,4(a0)
	sw	a5,8(a4)
	li	a1,-16773120
	lw	a5,0(a4)
	addi	a1,a1,-1
	andi	a6,a2,1
	and	a1,a5,a1
	seqz	a4,a6
	bfoz	a5,a3,16,18
	or	a5,a5,a1
	slli	a4,a4,12
	or	a4,a5,a4
	addi	a3,a3,-6
	li	a5,1
	bgtu	a3,a5,.L138
	li	a5,4718592
	.align	1
.L164:
	or	a5,a5,a4
	li	a4,1125130240
	sw	a5,0(a4)
	srli	t3,a2,12
	li	a7,1
	bbc	a2,8,.L140
	andi	a5,t3,15
	bnec	a5,1,.L140
	li	a7,2
	.align	1
.L140:
	li	a5,4096
	lw	t4,16(a0)
	andi	t3,t3,3
	slli	a7,a7,4
	li	a3,0
	li	t1,1125130240
	li	t5,-4096
	li	t6,4
	addi	t0,a5,-1792
	addi	t2,a5,-1536
	addi	a1,a5,-2048
	.align	1
.L147:
	lw	a4,0(t1)
	addi	a5,a3,12
	and	a4,a4,t5
	srl	a5,t4,a5
	bfoz	a5,a5,0,2
	or	a4,a4,t3
	or	a4,a5,a4
	addi	a5,a3,4
	srl	a5,t4,a5
	addi	a5,a5,-1
	bfoz	a5,a5,4,7
	or	a5,a5,a4
	srl	a4,t4,a3
	andi	a4,a4,15
	or	a2,a5,t0
	beqc	a4,4,.L146
	bgtu	a4,t6,.L142
	ori	a2,a5,1792
	beqc	a4,1,.L146
	or	a2,a5,a1
	beqc	a4,2,.L146
	.align	1
.L145:
	ori	a2,a5,1536
	j	.L146
	.align	1
.L138:
	lw	a5,92(a0)
	li	a3,524288
	slli	a5,a5,14
	and	a5,a5,a3
	or	a5,a5,a4
	li	a4,4194304
	j	.L164
	.align	1
.L142:
	or	a2,a5,t2
	bnec	a4,8,.L145
	.align	1
.L146:
	sw	a2,0(t1)
	addi	a3,a3,16
	addi	t1,t1,4
	bne	a7,a3,.L147
	li	a5,1125130240
	lw	a5,0(a5)
	li	a4,771
	bbs	a5,0,.L148
	li	a4,513
	.align	1
.L148:
	li	a5,1125134336
	addi	a3,a5,288
	sw	a4,0(a3)
	beq	a6,zero,.L149
	addi	a5,a5,964
	sw	zero,0(a5)
	.align	1
.L149:
	lw	a5,56(a0)
	beq	a5,zero,.L136
	li	a4,1125130240
	lw	a2,0(a4)
	bfoz	a3,a5,25,26
	or	a3,a3,a2
	sw	a3,0(a4)
	slli	a5,a5,10
	lw	a3,4(a4)
	li	a2,2093056
	and	a5,a5,a2
	or	a5,a5,a3
	sw	a5,4(a4)
	.align	1
.L136:
	ret
	.size	mctl_com_init, .-mctl_com_init
	.section	.rodata.str1.4
	.align	2
.LC5:
	.string	"\005\023\004\017\f\003\r\016\001\n\026\024\021\020\006\b\007\t\025\002\013\022"
	.section	.text.mctl_phy_ac_remapping,"ax",@progbits
	.align	1
	.globl	mctl_phy_ac_remapping
	.type	mctl_phy_ac_remapping, @function
mctl_phy_ac_remapping:
	call	t0,__riscv_save_1
	addi	sp,sp,-80
	lui	a1,%hi(.LC5)
	mv	s0,a0
	li	a2,22
	addi	a1,a1,%lo(.LC5)
	addi	a0,sp,32
	sw	zero,8(sp)
	sw	zero,12(sp)
	sw	zero,16(sp)
	sw	zero,20(sp)
	sw	zero,24(sp)
	sh	zero,28(sp)
	call	memcpy
	lui	a1,%hi(.LANCHOR0+352)
	li	a2,22
	addi	a1,a1,%lo(.LANCHOR0+352)
	addi	a0,sp,56
	call	memcpy
	lw	a5,92(s0)
	li	a2,22
	bbs	a5,18,.L166
	addi	a1,sp,32
	.align	1
.L170:
	addi	a0,sp,8
	call	memcpy_self
	lw	a5,4(s0)
	bnec	a5,3,.L165
	lbu	a5,8(sp)
	lbu	a4,9(sp)
	slli	a5,a5,5
	slli	a4,a4,10
	or	a5,a5,a4
	lbu	a4,10(sp)
	li	a3,1125130240
	slli	a4,a4,15
	or	a5,a5,a4
	lbu	a4,11(sp)
	slli	a4,a4,20
	or	a5,a5,a4
	lbu	a4,12(sp)
	slli	a4,a4,25
	or	a5,a5,a4
	addi	a4,a3,1280
	sw	a5,0(a4)
	lbu	a5,14(sp)
	lbu	a2,15(sp)
	slli	a5,a5,5
	slli	a2,a2,10
	or	a5,a5,a2
	lbu	a2,13(sp)
	or	a5,a5,a2
	lbu	a2,16(sp)
	slli	a2,a2,15
	or	a5,a5,a2
	lbu	a2,17(sp)
	slli	a2,a2,20
	or	a5,a5,a2
	lbu	a2,18(sp)
	slli	a2,a2,25
	or	a5,a5,a2
	addi	a2,a3,1284
	sw	a5,0(a2)
	lbu	a5,20(sp)
	lbu	a2,21(sp)
	slli	a5,a5,5
	slli	a2,a2,10
	or	a5,a5,a2
	lbu	a2,19(sp)
	or	a5,a5,a2
	lbu	a2,22(sp)
	slli	a2,a2,15
	or	a5,a5,a2
	lbu	a2,23(sp)
	slli	a2,a2,20
	or	a5,a5,a2
	addi	a2,a3,1288
	sw	a5,0(a2)
	lbu	a5,25(sp)
	lbu	a2,26(sp)
	slli	a5,a5,5
	slli	a2,a2,10
	or	a5,a5,a2
	lbu	a2,24(sp)
	addi	a3,a3,1292
	or	a5,a5,a2
	lbu	a2,27(sp)
	slli	a2,a2,15
	or	a5,a5,a2
	lbu	a2,28(sp)
	slli	a2,a2,20
	or	a5,a5,a2
	lbu	a2,29(sp)
	slli	a2,a2,25
	or	a5,a5,a2
	sw	a5,0(a3)
	lbu	a5,8(sp)
	lbu	a3,12(sp)
	slli	a5,a5,5
	slli	a3,a3,25
	or	a5,a5,a3
	lbu	a3,9(sp)
	slli	a3,a3,10
	or	a5,a5,a3
	lbu	a3,10(sp)
	slli	a3,a3,15
	or	a5,a5,a3
	lbu	a3,11(sp)
	slli	a3,a3,20
	or	a5,a5,a3
	ori	a5,a5,1
	sw	a5,0(a4)
	.align	1
.L165:
	addi	sp,sp,80
	tail	__riscv_restore_1
	.align	1
.L166:
	addi	a1,sp,56
	j	.L170
	.size	mctl_phy_ac_remapping, .-mctl_phy_ac_remapping
	.section	.rodata.str1.4
	.align	2
.LC7:
	.string	"ZQ calibration error,check external 240 ohm resistor.\n"
	.section	.text.mctl_channel_init,"ax",@progbits
	.align	1
	.globl	mctl_channel_init
	.type	mctl_channel_init, @function
mctl_channel_init:
	call	t0,__riscv_save_5
	mv	s2,a1
	lw	s1,92(a1)
	li	a1,1125130240
	lw	a4,12(a1)
	lw	a2,0(s2)
	li	a3,-4096
	and	a5,a4,a3
	srli	a4,a2,1
	addi	a4,a4,-1
	or	a4,a4,a5
	sw	a4,12(a1)
	li	a4,1125134336
	addi	a1,a4,264
	lw	a5,0(a1)
	addi	a3,a3,255
	and	a5,a5,a3
	ori	a5,a5,768
	sw	a5,0(a1)
	addi	a4,a4,836
	lw	a5,12(s2)
	lw	a4,0(a4)
	not	a5,a5
	andi	a4,a4,-49
	bfoz	a5,a5,0,5
	li	a3,672
	bfoz	s1,s1,3,2
	or	a4,a4,a5
	bgtu	a2,a3,.L172
	li	a3,-61440
	addi	a3,a3,-15
	and	a4,a4,a3
	.align	1
.L173:
	li	a3,1125134336
	addi	a1,a3,836
	addi	a3,a3,964
	sw	a4,0(a1)
	lw	a4,0(a3)
	andi	a4,a4,-49
	or	a5,a5,a4
	li	a4,672
	bgtu	a2,a4,.L174
	li	a4,-61440
	addi	a4,a4,-15
	and	a5,a5,a4
	.align	1
.L175:
	li	s0,1125134336
	addi	a4,s0,964
	sw	a5,0(a4)
	addi	a4,s0,520
	lw	a5,0(a4)
	mv	a0,s2
	ori	a5,a5,2
	sw	a5,0(a4)
	addi	s3,s0,264
	call	eye_delay_compensation
	beqc	s1,1,.L176
	beqc	s1,2,.L177
	lw	a5,0(s3)
	li	a0,10
	andi	a5,a5,-65
	sw	a5,0(s3)
	call	__usdelay
	lw	a5,0(s3)
	ori	a5,a5,192
	sw	a5,0(s3)
	j	.L179
	.align	1
.L172:
	li	a3,-61440
	addi	a3,a3,-1551
	and	a4,a4,a3
	ori	a4,a4,1024
	j	.L173
	.align	1
.L174:
	li	a4,-61440
	addi	a4,a4,-1551
	and	a5,a5,a4
	ori	a5,a5,1024
	j	.L175
	.align	1
.L176:
	lw	a5,0(s3)
	addi	s0,s0,188
	andi	a5,a5,-193
	sw	a5,0(s3)
	lw	a5,0(s0)
	andi	a5,a5,-264
	.align	1
.L217:
	sw	a5,0(s0)
	.align	1
.L179:
	lw	a5,4(s2)
	li	a4,1
	addi	a5,a5,-6
	bgtu	a5,a4,.L180
	li	a5,1125134336
	addi	a5,a5,284
	lw	a4,0(a5)
	bnec	s1,1,.L181
	li	a3,-134217728
	addi	a3,a3,-193
	and	a4,a4,a3
	li	a3,-2147483648
	.align	1
.L218:
	or	a4,a4,a3
	sw	a4,0(a5)
	.align	1
.L180:
	lw	a4,20(s2)
	li	a5,1125134336
	addi	a5,a5,192
	bbc	a4,12,.L182
	lw	a4,0(a5)
	li	a3,-268435456
	and	a4,a4,a3
	li	a3,50331648
	addi	a3,a3,1
	.align	1
.L219:
	or	a4,a4,a3
	sw	a4,0(a5)
	li	a5,1241513984
	lw	a5,448(a5)
	andi	a5,a5,15
	bnec	a5,1,.L184
	li	a5,1241518080
	addi	a5,a5,-1992
	lw	a4,0(a5)
	li	a0,10
	andi	a4,a4,-3
	sw	a4,0(a5)
	call	__usdelay
	.align	1
.L184:
	li	a4,1125134336
	addi	a2,a4,320
	lw	a5,0(a2)
	li	a3,-67108864
	and	a3,a5,a3
	lw	a5,8(s2)
	bfoz	a5,a5,23,0
	or	a5,a5,a3
	li	a3,33554432
	or	a5,a5,a3
	sw	a5,0(a2)
	bnec	s1,1,.L185
	li	a5,82
	sw	a5,0(a4)
	li	a5,83
	sw	a5,0(a4)
	.align	1
.L186:
	lw	a5,16(a4)
	bbc	a5,0,.L186
	li	a0,10
	call	__usdelay
	lw	a4,4(s2)
	li	a5,1312
	bnec	a4,3,.L187
	li	a5,1440
	.align	1
.L187:
	li	a4,1125134336
	sw	a5,0(a4)
	li	a0,10
	ori	a5,a5,1
	li	s0,1125134336
	sw	a5,0(a4)
	call	__usdelay
	.align	1
.L188:
	lw	a5,16(s0)
	andi	s2,a5,1
	bbc	a5,0,.L188
	li	a5,1241513984
	lw	a5,448(a5)
	andi	a5,a5,15
	bnec	a5,1,.L189
	addi	a3,s0,268
	lw	a5,0(a3)
	li	a4,-100663296
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,67108864
	or	a5,a5,a4
	li	a0,10
	sw	a5,0(a3)
	call	__usdelay
	lw	a5,4(s0)
	ori	a5,a5,1
	sw	a5,4(s0)
	li	s0,1125134336
	.align	1
.L190:
	lw	a5,24(s0)
	andi	a5,a5,7
	bnec	a5,3,.L190
	li	a5,1241518080
	addi	a5,a5,-1992
	lw	a4,0(a5)
	li	a0,10
	andi	a4,a4,-2
	sw	a4,0(a5)
	call	__usdelay
	lw	a5,4(s0)
	andi	a5,a5,-2
	sw	a5,4(s0)
	li	s0,1125134336
	.align	1
.L191:
	lw	a5,24(s0)
	andi	a5,a5,7
	bnec	a5,1,.L191
	li	a0,15
	call	__usdelay
	bnec	s1,1,.L189
	addi	a4,s0,264
	lw	a5,0(a4)
	addi	a3,s0,268
	andi	a5,a5,-193
	sw	a5,0(a4)
	li	a4,-100663296
	lw	a5,0(a3)
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,33554432
	or	a5,a5,a4
	li	a0,1
	sw	a5,0(a3)
	call	__usdelay
	li	a5,1025
	li	a4,1125134336
	sw	a5,0(s0)
	.align	1
.L192:
	lw	a5,16(a4)
	bbc	a5,0,.L192
	.align	1
.L189:
	li	a5,1125134336
	lw	a5,16(a5)
	bfoz	a4,a5,27,20
	beq	a4,zero,.L193
	li	s2,1048576
	and	s2,a5,s2
	beq	s2,zero,.L193
	lui	a0,%hi(.LC7)
	addi	a0,a0,%lo(.LC7)
	li	s2,0
	call	printf
	.align	1
.L171:
	mv	a0,s2
	tail	__riscv_restore_5
	.align	1
.L177:
	lw	a5,0(s3)
	addi	a3,s0,188
	andi	a5,a5,-193
	ori	a5,a5,128
	sw	a5,0(s3)
	lw	a5,96(s0)
	lw	a4,0(a3)
	bfoz	a5,a5,20,16
	andi	a4,a4,-264
	addi	a5,a5,-2
	or	a5,a5,a4
	ori	a5,a5,256
	addi	s0,s0,284
	sw	a5,0(a3)
	lw	a5,0(s0)
	li	a4,134217728
	bfoz	a5,a5,30,0
	or	a5,a5,a4
	j	.L217
	.align	1
.L181:
	li	a3,-1996488704
	addi	a3,a3,-1
	and	a4,a4,a3
	li	a3,570425344
	j	.L218
	.align	1
.L182:
	lw	a4,0(a5)
	li	a3,-268435456
	and	a4,a4,a3
	li	a3,16789504
	addi	a3,a3,135
	j	.L219
	.align	1
.L185:
	li	a5,1241513984
	lw	a4,448(a5)
	li	a5,98
	andi	a4,a4,15
	beqc	a4,1,.L187
	lw	a4,4(s2)
	li	a5,370
	bnec	a4,3,.L187
	li	a5,498
	j	.L187
	.align	1
.L193:
	li	s0,1125134336
	.align	1
.L195:
	lw	a5,24(s0)
	bbc	a5,0,.L195
	lw	a4,140(s0)
	li	s4,-2147483648
	or	a4,a4,s4
	li	a0,10
	sw	a4,140(s0)
	call	__usdelay
	lw	a4,140(s0)
	li	a0,10
	bfoz	a4,a4,30,0
	sw	a4,140(s0)
	call	__usdelay
	li	a4,1125130240
	lw	a5,20(a4)
	li	a0,10
	or	a5,a5,s4
	sw	a5,20(a4)
	call	__usdelay
	addi	a3,s0,268
	lw	a5,0(a3)
	li	a4,-100663296
	addi	a4,a4,-1
	and	a5,a5,a4
	sw	a5,0(a3)
	addi	s3,s0,140
	bnec	s1,1,.L171
	addi	s0,s0,284
	lw	a5,0(s0)
	andi	a5,a5,-193
	ori	a5,a5,64
	sw	a5,0(s0)
	j	.L171
	.size	mctl_channel_init, .-mctl_channel_init
	.section	.text.DRAMC_get_dram_size,"ax",@progbits
	.align	1
	.globl	DRAMC_get_dram_size
	.type	DRAMC_get_dram_size, @function
DRAMC_get_dram_size:
	li	a4,1125130240
	lw	a5,0(a4)
	bfoz	a3,a5,7,4
	bfoz	a0,a5,11,8
	add	a0,a0,a3
	addi	a0,a0,-14
	bfoz	a3,a5,3,2
	add	a0,a0,a3
	andi	a5,a5,3
	li	a3,1
	sll	a0,a3,a0
	beq	a5,zero,.L221
	lw	a4,4(a4)
	mv	a5,a0
	andi	a2,a4,3
	beq	a2,zero,.L221
	bfoz	a5,a4,11,8
	bfoz	a2,a4,7,4
	add	a5,a5,a2
	addi	a5,a5,-14
	bfoz	a4,a4,3,2
	add	a5,a5,a4
	sll	a5,a3,a5
	.align	1
.L221:
	add	a0,a5,a0
	ret
	.size	DRAMC_get_dram_size, .-DRAMC_get_dram_size
	.section	.rodata.str1.4
	.align	2
.LC8:
	.string	"[AUTO DEBUG] two rank and full DQ!\n"
	.align	2
.LC9:
	.string	"[AUTO DEBUG] single rank and full DQ!\n"
	.align	2
.LC10:
	.string	"[AUTO DEBUG] single rank and half DQ!\n"
	.align	2
.LC11:
	.string	"[AUTO DEBUG] dual rank and half DQ!\n"
	.align	2
.LC12:
	.string	"DX0 state:%d\n"
	.align	2
.LC13:
	.string	"DX1 state:%d\n"
	.section	.text.dqs_gate_detect,"ax",@progbits
	.align	1
	.globl	dqs_gate_detect
	.type	dqs_gate_detect, @function
dqs_gate_detect:
	call	t0,__riscv_save_1
	li	a5,1125134336
	lw	a4,16(a5)
	bbc	a4,22,.L228
	addi	a4,a5,840
	lw	a1,0(a4)
	addi	a5,a5,968
	lw	s0,0(a5)
	bfoz	a1,a1,25,24
	bfoz	s0,s0,25,24
	bnec	a1,2,.L237
	lw	a5,20(a0)
	li	a4,-61440
	addi	a4,a4,-16
	and	a5,a5,a4
	bnec	s0,2,.L232
	sw	a5,20(a0)
	lui	a0,%hi(.LC9)
	addi	a0,a0,%lo(.LC9)
	j	.L239
	.align	1
.L228:
	lw	a5,20(a0)
	li	a4,4096
	andi	a5,a5,-16
	or	a5,a5,a4
	sw	a5,20(a0)
	lui	a0,%hi(.LC8)
	addi	a0,a0,%lo(.LC8)
	.align	1
.L239:
	call	printf
	li	a0,1
	.align	1
.L227:
	tail	__riscv_restore_1
	.align	1
.L232:
	ori	a5,a5,1
	sw	a5,20(a0)
	lui	a0,%hi(.LC10)
	addi	a0,a0,%lo(.LC10)
	j	.L239
	.align	1
.L237:
	bne	a1,zero,.L233
	lw	a5,20(a0)
	li	a4,4096
	andi	a5,a5,-16
	addi	a4,a4,1
	or	a5,a5,a4
	sw	a5,20(a0)
	lui	a0,%hi(.LC11)
	addi	a0,a0,%lo(.LC11)
	j	.L239
	.align	1
.L233:
	lw	a5,92(a0)
	li	a0,536870912
	and	a0,a0,a5
	beq	a0,zero,.L227
	lui	a0,%hi(.LC12)
	addi	a0,a0,%lo(.LC12)
	call	printf
	lui	a0,%hi(.LC13)
	mv	a1,s0
	addi	a0,a0,%lo(.LC13)
	call	printf
	li	a0,0
	j	.L227
	.size	dqs_gate_detect, .-dqs_gate_detect
	.section	.rodata.str1.4
	.align	2
.LC14:
	.string	"DRAM simple test FAIL.\n"
	.align	2
.LC15:
	.string	"%x != %x at address %x\n"
	.align	2
.LC16:
	.string	"DRAM simple test OK.\n"
	.section	.text.dramc_simple_wr_test,"ax",@progbits
	.align	1
	.globl	dramc_simple_wr_test
	.type	dramc_simple_wr_test, @function
dramc_simple_wr_test:
	call	t0,__riscv_save_2
	srli	a0,a0,1
	li	s0,19087360
	li	a3,-19087360
	addi	sp,sp,-16
	slli	a0,a0,20
	li	a4,-2147483648
	li	a5,0
	addi	s0,s0,1383
	addi	a3,a3,-1384
	.align	1
.L241:
	bne	a5,a1,.L242
	li	a4,-38178816
	add	a5,a5,s0
	li	s1,-2147483648
	addi	a4,a4,1329
	.align	1
.L243:
	bne	a5,s0,.L247
	lui	a0,%hi(.LC16)
	addi	a0,a0,%lo(.LC16)
	call	printf
	li	a0,0
	j	.L240
	.align	1
.L242:
	add	a2,a5,s0
	sw	a2,0(a4)
	add	a6,a5,a3
	add	a2,a0,a4
	addi	a5,a5,1
	addi	a4,a4,4
	sw	a6,0(a2)
	j	.L241
	.align	1
.L247:
	add	a3,a0,s1
	lw	a1,0(a3)
	add	a2,s0,a4
	beq	a2,a1,.L244
	lui	a0,%hi(.LC14)
	addi	a0,a0,%lo(.LC14)
	sw	a1,12(sp)
	sw	a2,8(sp)
	sw	a3,4(sp)
	call	printf
	lw	a3,4(sp)
	lw	a2,8(sp)
	lw	a1,12(sp)
	.align	1
.L249:
	lui	a0,%hi(.LC15)
	addi	a0,a0,%lo(.LC15)
	call	printf
	li	a0,1
	.align	1
.L240:
	addi	sp,sp,16
	tail	__riscv_restore_2
	.align	1
.L244:
	lw	a1,0(s1)
	addi	a3,s1,4
	beq	a1,s0,.L246
	lui	a0,%hi(.LC14)
	addi	a0,a0,%lo(.LC14)
	sw	a1,4(sp)
	call	printf
	lw	a1,4(sp)
	mv	a3,s1
	mv	a2,s0
	j	.L249
	.align	1
.L246:
	addi	s0,s0,1
	mv	s1,a3
	j	.L243
	.size	dramc_simple_wr_test, .-dramc_simple_wr_test
	.section	.text.mctl_vrefzq_init,"ax",@progbits
	.align	1
	.globl	mctl_vrefzq_init
	.type	mctl_vrefzq_init, @function
mctl_vrefzq_init:
	lw	a2,92(a0)
	bbs	a2,17,.L250
	li	a5,1125134336
	addi	a1,a5,272
	lw	a4,0(a1)
	li	a3,-2139062272
	addi	a3,a3,128
	and	a4,a4,a3
	lw	a3,60(a0)
	or	a4,a4,a3
	sw	a4,0(a1)
	bbs	a2,16,.L250
	addi	a5,a5,276
	lw	a4,0(a5)
	andi	a3,a4,-128
	lw	a4,64(a0)
	andi	a4,a4,127
	or	a4,a4,a3
	sw	a4,0(a5)
	.align	1
.L250:
	ret
	.size	mctl_vrefzq_init, .-mctl_vrefzq_init
	.section	.text.mctl_core_init,"ax",@progbits
	.align	1
	.globl	mctl_core_init
	.type	mctl_core_init, @function
mctl_core_init:
	call	t0,__riscv_save_1
	mv	s0,a0
	call	mctl_sys_init
	mv	a0,s0
	call	mctl_vrefzq_init
	mv	a0,s0
	call	mctl_com_init
	mv	a0,s0
	call	auto_set_timing_para
	mv	a1,s0
	li	a0,0
	call	mctl_channel_init
	tail	__riscv_restore_1
	.size	mctl_core_init, .-mctl_core_init
	.section	.rodata.str1.4
	.align	2
.LC17:
	.string	"[ERROR DEBUG] DRAM initial error : 0!\n"
	.align	2
.LC18:
	.string	"[AUTO DEBUG] rank %d row = %d \n"
	.align	2
.LC19:
	.string	"[AUTO DEBUG] rank %d bank = %d \n"
	.align	2
.LC20:
	.string	"[AUTO DEBUG] rank %d page size = %d KB \n"
	.align	2
.LC21:
	.string	"rank1 config same as rank0\n"
	.align	2
.LC22:
	.string	"rank1 config different from rank0\n"
	.section	.text.auto_scan_dram_size,"ax",@progbits
	.align	1
	.globl	auto_scan_dram_size
	.type	auto_scan_dram_size, @function
auto_scan_dram_size:
	call	t0,__riscv_save_12
	addi	sp,sp,-16
	mv	s10,a0
	call	mctl_core_init
	bne	a0,zero,.L257
	mv	s1,a0
	lui	a0,%hi(.LC17)
	addi	a0,a0,%lo(.LC17)
	.align	1
.L310:
	call	printf
	.align	1
.L256:
	mv	a0,s1
	addi	sp,sp,16
	tail	__riscv_restore_12
	.align	1
.L257:
	lw	a5,20(s10)
	li	s0,1
	bfoz	a5,a5,15,12
	bnec	a5,1,.L259
	li	s0,2
	.align	1
.L259:
	li	s11,-4096
	li	s2,4096
	li	s4,0
	li	s9,0
	li	s1,-2147483648
	li	s5,-2147483648
	addi	s11,s11,3
	addi	s3,s2,-1376
	lui	s7,%hi(.LC20)
	.align	1
.L260:
	mv	a5,s5
	li	a2,0
	.align	1
.L263:
	mv	a1,a5
	bbs	a2,0,.L262
	not	a1,a5
	.align	1
.L262:
	sw	a1,0(a5)
	addi	a2,a2,1
	addi	a5,a5,4
	bnec	a2,64,.L263
	li	s8,1125130240
	lea.w	s8,s8,s9
	lw	a5,0(s8)
	li	a3,-4096
	addi	a3,a3,243
	and	a5,a5,a3
	ori	a5,a5,1776
	sw	a5,0(s8)
	.align	1
.L264:
	lw	a2,0(s8)
	bne	a5,a2,.L264
	li	a2,11
	li	t1,1
	not	s6,s1
	.align	1
.L287:
	addi	a1,a2,11
	sll	a1,t1,a1
	add	a1,a1,s5
	mv	a0,s6
	li	a5,0
	.align	1
.L267:
	not	a7,a0
	bbs	a5,0,.L265
	mv	a7,a0
	.align	1
.L265:
	lea.w	t3,a1,a5
	lw	t3,0(t3)
	bne	t3,a7,.L266
	addi	a5,a5,1
	addi	a0,a0,-4
	bnec	a5,64,.L267
	.align	1
.L288:
	li	a5,16
	bleu	a2,a5,.L268
	li	a2,16
	.align	1
.L268:
	lui	a3,%hi(.LC18)
	mv	a1,s9
	addi	a0,a3,%lo(.LC18)
	sw	a2,12(sp)
	call	printf
	addi	a1,s4,4
	li	a5,255
	lw	a0,16(s10)
	lw	a2,12(sp)
	sll	a5,a5,a1
	not	a5,a5
	and	a5,a5,a0
	sll	a2,a2,a1
	or	a2,a5,a2
	sw	a2,16(s10)
	bnec	s9,1,.L269
	li	a4,1125130240
	lw	a5,0(a4)
	li	s5,-2139095040
	and	a5,a5,s11
	ori	a5,a5,1700
	sw	a5,0(a4)
	.align	1
.L269:
	lw	a5,0(s8)
	and	a5,a5,s11
	ori	a5,a5,1700
	sw	a5,0(s8)
	.align	1
.L270:
	lw	a2,0(s8)
	bne	a2,a5,.L270
	addi	a5,s2,-2048
	add	a5,s5,a5
	mv	a1,s6
	li	a2,0
	.align	1
.L274:
	not	a0,a1
	bbs	a2,0,.L272
	mv	a0,a1
	.align	1
.L272:
	lw	a7,0(a5)
	bne	a7,a0,.L291
	addi	a2,a2,1
	addi	a1,a1,-4
	addi	a5,a5,4
	bnec	a2,64,.L274
	li	a7,0
	.align	1
.L273:
	addi	a2,a7,1
	lui	a3,%hi(.LC19)
	slli	a2,a2,2
	mv	a1,s9
	addi	a0,a3,%lo(.LC19)
	sw	a7,12(sp)
	call	printf
	addi	a2,s4,12
	li	a5,15
	lw	a1,16(s10)
	lw	a7,12(sp)
	sll	a5,a5,a2
	not	a5,a5
	and	a5,a5,a1
	sll	a7,a7,a2
	or	a5,a5,a7
	sw	a5,16(s10)
	bnec	s9,1,.L275
	li	a4,1125130240
	lw	a5,0(a4)
	li	s5,-2080374784
	and	a5,a5,s11
	or	a5,a5,s3
	sw	a5,0(a4)
	.align	1
.L275:
	lw	a5,0(s8)
	and	a5,a5,s11
	or	a5,a5,s3
	sw	a5,0(s8)
	.align	1
.L276:
	lw	a2,0(s8)
	bne	a2,a5,.L276
	li	a5,9
	li	t1,1
	.align	1
.L284:
	sll	a0,t1,a5
	add	a0,a0,s5
	mv	a1,s6
	li	a2,0
	.align	1
.L279:
	not	a7,a1
	bbs	a2,0,.L277
	mv	a7,a1
	.align	1
.L277:
	lea.w	t3,a0,a2
	lw	t3,0(t3)
	bne	t3,a7,.L278
	addi	a2,a2,1
	addi	a1,a1,-4
	bnec	a2,64,.L279
	beqc	a5,13,.L285
	li	s8,0
	beqc	a5,9,.L281
	.align	1
.L285:
	addi	a5,a5,-10
	li	s8,1
	sll	s8,s8,a5
	.align	1
.L281:
	mv	a2,s8
	mv	a1,s9
	addi	a0,s7,%lo(.LC20)
	call	printf
	li	a5,15
	lw	a2,16(s10)
	sll	a5,a5,s4
	not	a5,a5
	and	a5,a5,a2
	sll	a2,s8,s4
	or	a2,a5,a2
	addi	s9,s9,1
	sw	a2,16(s10)
	bne	s0,s9,.L282
	li	s1,1
	bnec	s0,2,.L256
	lw	a5,20(s10)
	li	a4,-4096
	addi	a4,a4,255
	and	a5,a5,a4
	srli	a4,a2,16
	bfoz	a2,a2,15,0
	bne	a4,a2,.L283
	lui	a0,%hi(.LC21)
	addi	a0,a0,%lo(.LC21)
	sw	a5,20(s10)
	j	.L310
	.align	1
.L282:
	bnec	s9,1,.L261
	li	a4,1125130240
	lw	a5,0(a4)
	li	s1,-2013265920
	and	a5,a5,s11
	ori	a5,a5,1776
	sw	a5,0(a4)
	lw	a5,4(a4)
	li	s5,-2013265920
	and	a5,a5,s11
	ori	a5,a5,1777
	sw	a5,4(a4)
	.align	1
.L261:
	addi	s4,s4,16
	j	.L260
	.align	1
.L291:
	li	a7,1
	j	.L273
	.align	1
.L283:
	ori	a5,a5,256
	lui	a0,%hi(.LC22)
	addi	a0,a0,%lo(.LC22)
	sw	a5,20(s10)
	j	.L310
	.align	1
.L278:
	addi	a5,a5,1
	bnec	a5,14,.L284
	li	a5,13
	j	.L285
	.align	1
.L266:
	addi	a2,a2,1
	bnec	a2,17,.L287
	j	.L288
	.size	auto_scan_dram_size, .-auto_scan_dram_size
	.section	.text.auto_scan_dram_rank_width,"ax",@progbits
	.align	1
	.globl	auto_scan_dram_rank_width
	.type	auto_scan_dram_rank_width, @function
auto_scan_dram_rank_width:
	call	t0,__riscv_save_3
	li	a5,11534336
	addi	a5,a5,176
	lw	s2,16(a0)
	sw	a5,16(a0)
	lw	a5,20(a0)
	lw	s1,92(a0)
	li	a4,4096
	andi	a5,a5,-16
	or	a5,a5,a4
	sw	a5,20(a0)
	andi	a5,s1,-9
	ori	a5,a5,5
	sw	a5,92(a0)
	mv	s0,a0
	call	mctl_core_init
	li	a5,1125134336
	lw	a5,16(a5)
	bbc	a5,20,.L312
	.align	1
.L314:
	li	a0,0
	.align	1
.L311:
	tail	__riscv_restore_3
	.align	1
.L312:
	mv	a0,s0
	call	dqs_gate_detect
	beq	a0,zero,.L314
	li	a0,1
	sw	s1,92(s0)
	sw	s2,16(s0)
	j	.L311
	.size	auto_scan_dram_rank_width, .-auto_scan_dram_rank_width
	.section	.rodata.str1.4
	.align	2
.LC23:
	.string	"[ERROR DEBUG] auto scan dram rank&width fail !\n"
	.align	2
.LC24:
	.string	"[ERROR DEBUG] auto scan dram size fail !\n"
	.section	.text.auto_scan_dram_config,"ax",@progbits
	.align	1
	.globl	auto_scan_dram_config
	.type	auto_scan_dram_config, @function
auto_scan_dram_config:
	call	t0,__riscv_save_2
	lw	a5,92(a0)
	mv	s1,a0
	bbs	a5,14,.L320
	call	auto_scan_dram_rank_width
	mv	s0,a0
	bne	a0,zero,.L320
	lui	a0,%hi(.LC23)
	addi	a0,a0,%lo(.LC23)
	.align	1
.L325:
	call	printf
	.align	1
.L319:
	mv	a0,s0
	tail	__riscv_restore_2
	.align	1
.L320:
	lw	a5,92(s1)
	bbs	a5,0,.L322
	mv	a0,s1
	call	auto_scan_dram_size
	mv	s0,a0
	bne	a0,zero,.L322
	lui	a0,%hi(.LC24)
	addi	a0,a0,%lo(.LC24)
	j	.L325
	.align	1
.L322:
	lw	a5,92(s1)
	li	s0,1
	bbs	a5,15,.L319
	li	a4,24576
	addi	a4,a4,1
	or	a5,a5,a4
	sw	a5,92(s1)
	j	.L319
	.size	auto_scan_dram_config, .-auto_scan_dram_config
	.section	.rodata.str1.4
	.align	2
.LC25:
	.string	"7.8"
	.align	2
.LC26:
	.string	"3.9"
	.align	2
.LC27:
	.string	"DRAM use internal ZQ!!\n"
	.align	2
.LC28:
	.string	"ZQ value = 0x%x\n"
	.align	2
.LC29:
	.string	"DRAM use external ZQ!!\n"
	.align	2
.LC30:
	.string	"V1.02"
	.align	2
.LC31:
	.string	"DRAM BOOT DRIVE INFO: %s\n"
	.align	2
.LC32:
	.string	"DRAM CLK = %d MHz\n"
	.align	2
.LC33:
	.string	"DRAM Type = %d (2:DDR2,3:DDR3)\n"
	.align	2
.LC34:
	.string	"DRAMC ZQ value: 0x%x\n"
	.align	2
.LC35:
	.string	"DRAMC read ODT off.\n"
	.align	2
.LC36:
	.string	"DRAM ODT value: 0x%x.\n"
	.align	2
.LC37:
	.string	"DRAM ODT off.\n"
	.align	2
.LC38:
	.string	"DRAM initial error : 1 !\n"
	.align	2
.LC39:
	.string	"trefi: 3.9us\n"
	.align	2
.LC40:
	.string	"trefi: 1.95us\n"
	.align	2
.LC41:
	.string	"trefi: %sus\n"
	.align	2
.LC42:
	.string	"DRAM Size = %d MB\n"
	.align	2
.LC43:
	.string	"Enable Auto SR\n"
	.section	.text.init_DRAM,"ax",@progbits
	.align	1
	.globl	init_DRAM
	.type	init_DRAM, @function
init_DRAM:
	call	t0,__riscv_save_3
	li	a5,1241513984
	lw	a5,64(a5)
	mv	s0,a1
	bnec	a5,2,.L327
	lw	a5,92(a1)
	bbs	a5,16,.L328
	li	a5,1241518080
	addi	a4,a5,-1992
	sw	zero,0(a4)
	addi	a5,a5,-1988
	li	s2,1124073472
	li	a0,150
	addi	s1,s2,352
	sw	zero,0(a5)
	call	__usdelay
	lw	a5,0(s1)
	li	a0,10
	andi	a5,a5,-4
	sw	a5,0(s1)
	call	__usdelay
	lw	a5,0(s1)
	li	a0,10
	andi	a5,a5,-261
	sw	a5,0(s1)
	lw	a5,0(s1)
	addi	s2,s2,364
	ori	a5,a5,2
	sw	a5,0(s1)
	call	__usdelay
	lw	a5,0(s1)
	li	a0,20
	ori	a5,a5,1
	sw	a5,0(s1)
	call	__usdelay
	lui	a0,%hi(.LC27)
	addi	a0,a0,%lo(.LC27)
	call	printf
	lui	a0,%hi(.LC28)
	lw	a1,0(s2)
	addi	a0,a0,%lo(.LC28)
	call	printf
	.align	1
.L329:
	lw	a5,92(s0)
	bbc	a5,0,.L330
	.align	1
.L333:
	lui	a1,%hi(.LC30)
	lui	a0,%hi(.LC31)
	addi	a1,a1,%lo(.LC30)
	addi	a0,a0,%lo(.LC31)
	call	printf
	lw	a1,0(s0)
	lui	a0,%hi(.LC32)
	addi	a0,a0,%lo(.LC32)
	call	printf
	lw	a1,4(s0)
	lui	a0,%hi(.LC33)
	addi	a0,a0,%lo(.LC33)
	call	printf
	lw	a5,12(s0)
	bbs	a5,0,.L331
	lui	a0,%hi(.LC35)
	addi	a0,a0,%lo(.LC35)
	call	printf
	j	.L335
	.align	1
.L328:
	lui	a0,%hi(.LC29)
	addi	a0,a0,%lo(.LC29)
	call	printf
	li	a5,1124073472
	addi	a3,a5,352
	lw	a4,0(a3)
	addi	a5,a5,360
	ori	a4,a4,256
	sw	a4,0(a3)
	li	a0,10
	sw	zero,0(a5)
	.align	1
.L371:
	call	__usdelay
	j	.L329
	.align	1
.L327:
	li	a5,1241518080
	addi	a4,a5,-1992
	sw	zero,0(a4)
	addi	a5,a5,-1988
	li	a0,150
	sw	zero,0(a5)
	j	.L371
	.align	1
.L330:
	mv	a0,s0
	call	auto_scan_dram_config
	bne	a0,zero,.L333
	.align	1
.L374:
	li	a0,0
	.align	1
.L326:
	tail	__riscv_restore_3
	.align	1
.L331:
	lui	a0,%hi(.LC34)
	lw	a1,8(s0)
	addi	a0,a0,%lo(.LC34)
	call	printf
	.align	1
.L335:
	lw	a1,28(s0)
	andi	a5,a1,68
	beq	a5,zero,.L336
	lui	a0,%hi(.LC36)
	addi	a0,a0,%lo(.LC36)
	call	printf
	.align	1
.L337:
	mv	a0,s0
	call	mctl_core_init
	bne	a0,zero,.L338
	lui	a0,%hi(.LC38)
	addi	a0,a0,%lo(.LC38)
	call	printf
	j	.L374
	.align	1
.L336:
	lui	a0,%hi(.LC37)
	addi	a0,a0,%lo(.LC37)
	call	printf
	j	.L337
	.align	1
.L338:
	lw	a5,56(s0)
	li	a4,1
	bfoz	a5,a5,15,12
	beqc	a5,1,.L339
	beqc	a5,2,.L340
	lw	a5,4(s0)
	addi	a5,a5,-2
	bleu	a5,a4,.L360
	lui	a1,%hi(.LC26)
	addi	a1,a1,%lo(.LC26)
	.align	1
.L343:
	lui	a0,%hi(.LC41)
	addi	a0,a0,%lo(.LC41)
	call	printf
	j	.L342
	.align	1
.L339:
	lui	a0,%hi(.LC39)
	addi	a0,a0,%lo(.LC39)
	.align	1
.L372:
	call	printf
	.align	1
.L342:
	lw	s1,20(s0)
	bge	s1,zero,.L344
	bfoz	s1,s1,30,16
	.align	1
.L345:
	lui	a0,%hi(.LC42)
	mv	a1,s1
	addi	a0,a0,%lo(.LC42)
	call	printf
	lw	a3,92(s0)
	li	a5,1125134336
	addi	a4,a5,160
	bbc	a3,30,.L346
	lw	a5,0(a4)
	lw	a5,72(s0)
	bne	a5,zero,.L347
	li	a5,268435456
	addi	a5,a5,512
	.align	1
.L347:
	li	a4,1125134336
	addi	a3,a4,160
	sw	a5,0(a3)
	addi	a5,a4,156
	li	a3,1034
	sw	a3,0(a5)
	lw	a5,4(a4)
	lui	a0,%hi(.LC43)
	ori	a5,a5,1
	addi	a0,a0,%lo(.LC43)
	sw	a5,4(a4)
	call	printf
	.align	1
.L348:
	li	a5,1125134336
	addi	a5,a5,256
	lw	a4,0(a5)
	li	a2,-61440
	lw	a3,92(s0)
	addi	a2,a2,-1
	lw	a1,4(s0)
	and	a4,a4,a2
	bbs	a3,9,.L349
	bnec	a1,6,.L350
	.align	1
.L349:
	li	a2,20480
	or	a4,a4,a2
	.align	1
.L350:
	sw	a4,0(a5)
	li	a5,1125134336
	addi	a2,a5,320
	lw	a4,0(a2)
	li	a0,-2147483648
	or	a4,a4,a0
	sw	a4,0(a2)
	bbc	a3,8,.L352
	addi	a5,a5,184
	lw	a4,0(a5)
	ori	a4,a4,768
	sw	a4,0(a5)
	.align	1
.L352:
	li	a5,1125134336
	addi	a5,a5,264
	lw	a4,0(a5)
	bbc	a3,26,.L353
	li	a2,-8192
	addi	a2,a2,-1
	and	a4,a4,a2
	.align	1
.L373:
	sw	a4,0(a5)
	bnec	a1,7,.L355
	li	a2,1125134336
	lw	a5,124(a2)
	li	a4,-983040
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,65536
	or	a5,a5,a4
	sw	a5,124(a2)
	.align	1
.L355:
	bbs	a3,27,.L356
	mv	a0,s0
	call	set_master_priority_pad
	.align	1
.L356:
	call	dram_enable_all_master
	lw	a5,92(s0)
	bbs	a5,28,.L357
	.align	1
.L358:
	call	handler_super_standby
	mv	a0,s1
	j	.L326
	.align	1
.L340:
	lui	a0,%hi(.LC40)
	addi	a0,a0,%lo(.LC40)
	j	.L372
	.align	1
.L360:
	lui	a1,%hi(.LC25)
	addi	a1,a1,%lo(.LC25)
	j	.L343
	.align	1
.L344:
	call	DRAMC_get_dram_size
	lhu	a5,20(s0)
	slli	a4,a0,16
	or	a5,a5,a4
	mv	s1,a0
	sw	a5,20(s0)
	j	.L345
	.align	1
.L346:
	lw	a3,0(a4)
	li	a2,-65536
	and	a3,a3,a2
	sw	a3,0(a4)
	lw	a4,4(a5)
	andi	a4,a4,-2
	sw	a4,4(a5)
	j	.L348
	.align	1
.L353:
	li	a2,8192
	or	a4,a4,a2
	j	.L373
	.align	1
.L357:
	li	a5,1241513984
	lw	a5,448(a5)
	andi	a5,a5,15
	beqc	a5,1,.L358
	li	a1,4096
	mv	a0,s1
	call	dramc_simple_wr_test
	beq	a0,zero,.L358
	j	.L374
	.size	init_DRAM, .-init_DRAM
	.section	.text.mctl_init,"ax",@progbits
	.align	1
	.globl	mctl_init
	.type	mctl_init, @function
mctl_init:
	call	t0,__riscv_save_0
	addi	sp,sp,-96
	mv	a1,sp
	li	a0,0
	call	init_DRAM
	seqz	a0,a0
	addi	sp,sp,96
	tail	__riscv_restore_0
	.size	mctl_init, .-mctl_init
	.section	.rodata
	.align	2
	.set	.LANCHOR0,. + 0
.LC0:
	.word	0
	.word	1
	.word	2
	.word	3
	.word	2
	.word	3
	.word	3
	.word	3
	.word	0
	.word	0
	.word	0
	.word	6
	.word	6
	.word	6
	.word	5
	.word	5
	.word	5
	.word	5
	.word	5
	.word	0
	.word	0
	.word	0
	.word	0
	.word	2
	.word	4
	.word	2
	.word	6
	.word	5
	.word	5
	.word	5
	.word	0
	.word	0
	.word	0
	.word	3
	.word	3
	.word	3
	.word	2
	.word	2
	.word	1
	.word	1
	.word	1
	.word	0
	.word	0
	.word	0
.LC1:
	.word	0
	.word	1
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
	.word	4
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
	.word	4
	.word	4
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
	.word	0
	.word	3
	.word	3
	.word	1
	.word	6
	.word	6
	.word	5
	.word	6
	.word	3
	.word	3
	.word	3
	.word	5
	.word	5
	.word	6
	.word	6
	.word	4
	.word	5
	.word	3
	.word	3
	.word	3
	.word	3
	.word	3
.LC6:
	.string	"\023\004\022\002\006\003\005\007\f\b\r\n\t"
	.string	""
	.string	""
	.ascii	"\026\001\013\021\025\024"
	.ident	"GCC: (2023-05-08_nds32le-linux-glibc-v5d-4356776f1) 10.4.0"
	.section	.note.GNU-stack,"",@progbits
