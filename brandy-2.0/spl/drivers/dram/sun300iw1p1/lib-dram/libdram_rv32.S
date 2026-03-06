	.file	"mctl_hal.c"
	.option nopic
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
.L4:
	sub	a4,a2,a5
	bgt	a4,zero,.L5
	ret
.L5:
	lrbu	a4,a1,a5,0
	srb	a4,a0,a5,0
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
	sw	a4,40(a5)
	li	a0,10
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
	sw	zero,40(a5)
	li	a0,10
	tail	__usdelay
	.size	dram_disable_all_master, .-dram_disable_all_master
	.section	.text.eye_delay_compensation,"ax",@progbits
	.align	1
	.globl	eye_delay_compensation
	.type	eye_delay_compensation, @function
eye_delay_compensation:
	lw	a5,84(a0)
	lw	a4,88(a0)
	li	a3,8192
	addi	a3,a3,-512
	slli	a1,a5,9
	and	a1,a1,a3
	slli	a3,a4,1
	andi	a3,a3,30
	addi	sp,sp,-16
	or	a1,a1,a3
	li	a2,1125134336
	li	a3,1125134336
	sw	s1,4(sp)
	sw	ra,12(sp)
	sw	s0,8(sp)
	mv	s1,a0
	addi	a2,a2,784
	addi	a6,a3,820
.L11:
	lw	a0,0(a2)
	or	a0,a1,a0
	swia	a0,(a2),4,0
	bne	a2,a6,.L11
	li	a1,8192
	addi	a1,a1,-512
	slli	a2,a5,5
	and	a2,a2,a1
	srli	a1,a4,3
	andi	a1,a1,30
	li	s0,1125134336
	or	a2,a2,a1
	addi	a3,a3,912
	addi	a0,s0,948
.L12:
	lw	a1,0(a3)
	or	a1,a2,a1
	swia	a1,(a3),4,0
	bne	a3,a0,.L12
	lw	a2,256(s0)
	li	a1,-67108864
	addi	a1,a1,-1
	and	a2,a2,a1
	sw	a2,256(s0)
	addi	a0,s0,768
	srli	a2,a5,16
	li	a7,8192
	lw	t1,52(a0)
	srli	a1,a4,15
	slli	t3,a2,9
	addi	a7,a7,-512
	and	t3,t3,a7
	andi	a1,a1,30
	or	a1,a1,t3
	or	t1,a1,t1
	sw	t1,52(a0)
	lw	t1,56(a0)
	srli	a5,a5,20
	srli	a4,a4,19
	or	a1,a1,t1
	sw	a1,56(a0)
	lw	a1,0(a3)
	slli	t1,a5,9
	and	a7,t1,a7
	andi	a4,a4,30
	or	a4,a4,a7
	or	a1,a4,a1
	sw	a1,0(a3)
	addi	a3,s0,896
	lw	a1,56(a3)
	slli	a2,a2,25
	slli	a5,a5,25
	or	a4,a4,a1
	sw	a4,56(a3)
	lw	a1,60(a0)
	li	a4,503316480
	and	a2,a2,a4
	or	a2,a2,a1
	sw	a2,60(a0)
	lw	a2,60(a3)
	and	a5,a5,a4
	li	a4,67108864
	or	a5,a5,a2
	sw	a5,60(a3)
	lw	a5,256(s0)
	li	a0,1
	addi	s0,s0,576
	or	a5,a5,a4
	sw	a5,-320(s0)
	call	__usdelay
	lw	a5,80(s1)
	li	a4,4096
	addi	a4,a4,-256
	slli	a2,a5,4
	and	a2,a2,a4
	li	a4,1125134336
	addi	a1,a4,636
.L13:
	lw	a3,0(s0)
	or	a3,a2,a3
	swia	a3,(s0),4,0
	bne	s0,a1,.L13
	li	a3,1125134336
	addi	a4,a4,552
	addi	a0,a3,576
.L14:
	lw	a1,0(a4)
	or	a1,a2,a1
	swia	a1,(a4),4,0
	bne	a4,a0,.L14
	addi	a1,a3,512
	lw	a0,24(a1)
	li	a4,4096
	addi	a4,a4,-256
	slli	a2,a5,8
	and	a2,a2,a4
	or	a2,a2,a0
	sw	a2,24(a1)
	lw	a0,28(a1)
	and	a2,a5,a4
	addi	a3,a3,640
	or	a2,a2,a0
	sw	a2,28(a1)
	lw	a2,0(a3)
	srli	a5,a5,4
	lw	ra,12(sp)
	lw	s0,8(sp)
	and	a5,a5,a4
	or	a5,a5,a2
	sw	a5,0(a3)
	lw	s1,4(sp)
	addi	sp,sp,16
	jr	ra
	.size	eye_delay_compensation, .-eye_delay_compensation
	.section	.text.bit_delay_compensation,"ax",@progbits
	.align	1
	.globl	bit_delay_compensation
	.type	bit_delay_compensation, @function
bit_delay_compensation:
	addi	sp,sp,-368
	li	a2,176
	lla	a1,.LANCHOR0
	mv	a0,sp
	sw	ra,364(sp)
	call	memcpy
	addi	a6,sp,176
	li	a2,176
	mv	a0,a6
	lla	a1,.LANCHOR0+176
	call	memcpy
	li	a3,1125134336
	lw	a5,256(a3)
	li	a4,-67108864
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,281284608
	sw	a5,256(a3)
	mv	a6,a0
	addi	a3,a4,-828
	li	a5,0
	addi	a2,a4,-700
.L21:
	slli	a1,a3,2
	addi	a0,a5,44
.L22:
	lrw	a4,a6,a5,0
	lw	a7,0(a1)
	slli	a4,a4,8
	add	a4,a4,a7
	lrw	a7,sp,a5,0
	addi	a5,a5,4
	add	a4,a4,a7
	swia	a4,(a1),4,0
	bne	a0,a5,.L22
	addi	a3,a3,32
	beq	a3,a2,.L23
	mv	a5,a0
	j	.L21
.L23:
	li	a4,1125134336
	lw	a5,256(a4)
	lw	ra,364(sp)
	li	a3,67108864
	or	a5,a5,a3
	sw	a5,256(a4)
	addi	sp,sp,368
	jr	ra
	.size	bit_delay_compensation, .-bit_delay_compensation
	.section	.text.set_master_priority_pad,"ax",@progbits
	.align	1
	.globl	set_master_priority_pad
	.type	set_master_priority_pad, @function
set_master_priority_pad:
	li	a4,1125130240
	lw	a5,12(a4)
	li	a3,-4096
	li	a2,5242880
	and	a3,a5,a3
	lw	a5,0(a0)
	addi	a2,a2,100
	li	a1,33554432
	srli	a5,a5,1
	addi	a5,a5,-1
	or	a5,a5,a3
	sw	a5,12(a4)
	li	a3,65536
	addi	a5,a4,512
	sw	a3,0(a5)
	li	a3,16777216
	addi	a3,a3,9
	sw	a3,16(a5)
	sw	a2,20(a5)
	addi	a1,a1,13
	sw	a1,48(a5)
	li	a1,6291456
	addi	a1,a1,256
	sw	a1,52(a5)
	sw	a3,64(a5)
	sw	a2,68(a5)
	li	a2,6553600
	addi	a2,a2,521
	sw	a2,96(a5)
	li	a2,2097152
	addi	a2,a2,64
	sw	a2,100(a5)
	addi	a2,a4,640
	li	a5,4194304
	sw	a3,16(a2)
	addi	a5,a5,128
	sw	a5,20(a2)
	sw	zero,1136(a4)
	sw	zero,1140(a4)
	li	a4,260059136
	li	a5,1125134336
	addi	a4,a4,-251
	sw	a4,448(a5)
	li	a4,251658240
	addi	a4,a4,255
	sw	a4,456(a5)
	li	a4,1056964608
	addi	a4,a4,95
	sw	a4,464(a5)
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
	lw	a5,92(a0)
	addi	sp,sp,-128
	sw	s1,116(sp)
	sw	s2,112(sp)
	sw	s4,104(sp)
	sw	a5,56(sp)
	sw	ra,124(sp)
	sw	s0,120(sp)
	sw	s3,108(sp)
	sw	s5,100(sp)
	sw	s6,96(sp)
	sw	s7,92(sp)
	sw	s8,88(sp)
	sw	s9,84(sp)
	sw	s10,80(sp)
	sw	s11,76(sp)
	andi	a5,a5,2
	lw	s1,0(a0)
	lw	s4,4(a0)
	mv	s2,a0
	beq	a5,zero,.L29
	lw	s8,40(a0)
	lw	a2,44(a0)
	lw	s0,48(a0)
	extu	a5,s8,21+3-1,21
	sw	a5,60(sp)
	extu	a5,a2,23+5-1,23
	sw	a5,12(sp)
	li	a0,4096
	extu	a5,a2,15+5-1,15
	sw	a5,48(sp)
	addi	a0,a0,-1
	extu	a5,s0,12+9-1,12
	extu	s6,s8,15+6-1,15
	extu	s5,s8,11+4-1,11
	extu	s3,s8,6+5-1,6
	extu	s11,a2,20+3-1,20
	andi	s8,s8,63
	extu	t1,a2,11+4-1,11
	extu	s7,a2,6+5-1,6
	andi	s10,a2,63
	sw	a5,16(sp)
	and	s0,s0,a0
.L30:
	lw	a5,36(s2)
	sw	a5,52(sp)
	lw	a5,28(s2)
	sw	a5,44(sp)
	li	a5,6
	beq	s4,a5,.L58
	bgtu	s4,a5,.L59
	li	a5,2
	beq	s4,a5,.L60
	li	a5,3
	beq	s4,a5,.L61
.L86:
	li	a5,8
	mv	s11,a5
	li	a5,27
	sw	a5,40(sp)
	li	a5,12
	sw	a5,36(sp)
	li	a0,4
	li	a7,3
	li	t3,4
	li	s4,2
	li	t4,6
	li	t1,2
	li	a1,0
	li	a4,3
	li	a6,3
	li	a2,1
	li	a5,1
	li	t5,0
	li	t6,0
	li	t0,0
	li	a3,0
	sw	zero,32(sp)
	sw	zero,28(sp)
	sw	zero,24(sp)
	sw	zero,20(sp)
.L62:
	lw	t2,48(sp)
	addi	s9,a6,2
	add	t2,s7,t2
	bgeu	t2,s9,.L67
	sub	t2,a6,s7
	addi	t2,t2,2
	sw	t2,48(sp)
.L67:
	lhu	t2,26(s2)
	bne	t2,zero,.L68
	sw	a3,24(s2)
.L68:
	lw	a3,44(sp)
	srli	a3,a3,16
	bne	a3,zero,.L69
	sw	t0,28(s2)
.L69:
	lhu	a3,34(s2)
	bne	a3,zero,.L70
	sw	t6,32(s2)
.L70:
	lw	a3,52(sp)
	srli	a3,a3,16
	bne	a3,zero,.L71
	sw	t5,36(s2)
.L71:
	lw	t6,24(s2)
	li	t5,65536
	addi	t5,t5,-1
	li	a3,1125134336
	and	t6,t6,t5
	sw	t6,48(a3)
	lw	t6,28(s2)
	slli	s6,s6,16
	or	s6,s6,s10
	and	t6,t6,t5
	sw	t6,52(a3)
	lw	t6,32(s2)
	slli	a4,a4,24
	slli	a6,a6,16
	and	t6,t6,t5
	sw	t6,56(a3)
	lw	t6,36(s2)
	or	a4,a4,a6
	or	a4,a4,s11
	and	t5,t6,t5
	sw	t5,60(a3)
	lw	t5,12(s2)
	slli	t3,t3,8
	or	t3,a4,t3
	extu	t5,t5,4+2-1,4
	sw	t5,44(a3)
	lw	t5,36(sp)
	lw	a4,60(sp)
	slli	a1,a1,16
	slli	t5,t5,24
	or	s6,s6,t5
	lw	t5,40(sp)
	slli	a4,a4,16
	slli	t1,t1,12
	slli	t5,t5,8
	or	s6,s6,t5
	lw	t5,12(sp)
	sw	s6,88(a3)
	slli	s3,s3,24
	slli	t2,t5,16
	lw	t5,48(sp)
	or	s8,t2,s8
	or	s3,s3,a4
	slli	t2,t5,8
	or	t2,s8,t2
	sw	t2,92(a3)
	slli	a4,a0,24
	or	a1,a1,t1
	slli	a0,a0,16
	sw	t3,96(a3)
	or	a0,a4,a0
	or	t4,a1,t4
	or	s7,s3,s7
	slli	s5,s5,8
	sw	t4,100(a3)
	or	s5,s7,s5
	or	s4,a0,s4
	slli	a7,a7,8
	sw	s5,104(a3)
	or	a7,s4,a7
	sw	a7,108(a3)
	lw	a4,120(a3)
	li	a3,268369920
	and	a4,a4,a3
	li	a3,800
	bgtu	s1,a3,.L72
	li	a3,-268410880
.L87:
	addi	a3,a3,1536
	or	a4,a4,a3
	ori	a4,a4,16
	li	a3,1125134336
	sw	a4,120(a3)
	slli	a5,a5,16
	li	a4,33554432
	or	a5,a5,a2
	addi	a4,a4,256
	or	a5,a5,a4
	addi	a4,a3,128
	sw	a5,0(a4)
	lw	a5,24(sp)
	lw	a2,20(sp)
	lw	ra,124(sp)
	slli	a5,a5,20
	or	a5,a5,a2
	sw	a5,80(a3)
	lw	a5,32(sp)
	lw	a2,28(sp)
	lw	s1,116(sp)
	slli	a5,a5,20
	or	a5,a5,a2
	sw	a5,84(a3)
	lw	a3,16(sp)
	slli	a5,s0,16
	slli	s0,s0,15
	or	s9,a5,a3
	li	a5,268369920
	sw	s9,16(a4)
	and	s0,s0,a5
	sw	s0,20(a4)
	lw	s0,120(sp)
	lw	s2,112(sp)
	lw	s3,108(sp)
	lw	s4,104(sp)
	lw	s5,100(sp)
	lw	s6,96(sp)
	lw	s7,92(sp)
	lw	s8,88(sp)
	lw	s9,84(sp)
	lw	s10,80(sp)
	lw	s11,76(sp)
	addi	sp,sp,128
	jr	ra
.L29:
	li	a5,3
	srli	s9,s1,1
	bne	s4,a5,.L31
	mv	a1,s9
	li	a0,8
	call	auto_cal_timing
	mv	a1,s9
	mv	s11,a0
	li	a0,15
	call	auto_cal_timing
	mv	s7,a0
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
	sw	a0,16(sp)
	li	a5,800
	mv	a1,s9
	bgtu	s1,a5,.L32
	li	a0,50
	call	auto_cal_timing
	mv	s6,a0
	mv	a1,s9
	li	a0,10
	call	auto_cal_timing
	li	a5,2
	mv	s5,a0
	bgeu	a0,a5,.L33
	li	s5,2
.L33:
	mv	a1,s9
	li	a0,53
	call	auto_cal_timing
	li	a5,2
	mv	s8,a0
	bgeu	s11,a5,.L34
	li	s11,2
.L34:
	li	a5,2
	mv	t1,s7
	bgeu	s7,a5,.L35
	li	t1,2
.L35:
	mv	a1,s9
	li	a0,38
	sw	t1,20(sp)
	call	auto_cal_timing
	mv	s10,a0
	sw	s11,12(sp)
	mv	s3,s7
.L83:
	lw	t1,20(sp)
.L36:
	slli	a4,s6,15
	slli	a5,s3,6
	or	a5,a5,a4
	or	a5,a5,s8
	slli	a4,s5,11
	or	a5,a5,a4
	li	a4,4194304
	or	a5,a5,a4
	sw	a5,40(s2)
	slli	a4,t1,11
	slli	a5,s7,6
	or	a5,a5,a4
	or	a5,a5,s10
	slli	a4,s11,15
	or	a5,a5,a4
	slli	a4,s11,20
	or	a5,a5,a4
	lw	a4,12(sp)
	slli	a4,a4,23
	or	a5,a5,a4
	sw	a5,44(s2)
	lw	a5,56(s2)
	li	a4,1
	extu	a5,a5,12+4-1,12
	beq	a5,a4,.L53
	li	a3,2
	beq	a5,a3,.L54
.L55:
	lw	a5,16(sp)
	sw	s11,48(sp)
	slli	a5,a5,12
	or	a5,a5,s0
	sw	a5,48(s2)
	li	a5,2
	sw	a5,60(sp)
	j	.L30
.L32:
	li	a0,35
	call	auto_cal_timing
	mv	s6,a0
	mv	a1,s9
	li	a0,6
	call	auto_cal_timing
	li	a5,2
	mv	s5,a0
	bgeu	a0,a5,.L37
	li	s5,2
.L37:
	mv	a1,s9
	li	a0,14
	call	auto_cal_timing
	mv	s3,a0
	mv	a1,s9
	li	a0,48
	call	auto_cal_timing
	li	a5,2
	mv	s8,a0
	bgeu	s11,a5,.L38
	li	s11,2
.L38:
	li	a5,2
	mv	t1,s7
	bgeu	s7,a5,.L39
	li	t1,2
.L39:
	mv	a1,s9
	li	a0,34
	sw	t1,20(sp)
	call	auto_cal_timing
	mv	s10,a0
	sw	s5,12(sp)
	mv	s7,s3
	j	.L83
.L31:
	li	a5,2
	bne	s4,a5,.L40
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	mv	a1,s9
	mv	s6,a0
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
	mv	s8,a0
	li	a0,8
	call	auto_cal_timing
	mv	a1,s9
	mv	s11,a0
	li	a0,15
	call	auto_cal_timing
	mv	s7,a0
	mv	a1,s9
	li	a0,45
	call	auto_cal_timing
	mv	s10,a0
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
	li	a5,2
	sw	a0,16(sp)
	sw	a5,12(sp)
	mv	t1,s7
	j	.L36
.L40:
	li	a5,6
	bne	s4,a5,.L41
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	li	a5,4
	mv	s6,a0
	bgeu	a0,a5,.L42
	li	s6,4
.L42:
	mv	a1,s9
	li	a0,10
	call	auto_cal_timing
	mv	s5,a0
	bne	a0,zero,.L43
	li	s5,1
.L43:
	mv	a1,s9
	li	a0,24
	call	auto_cal_timing
	li	a5,2
	mv	s3,a0
	bgeu	a0,a5,.L44
	li	s3,2
.L44:
	mv	a1,s9
	li	a0,70
	call	auto_cal_timing
	mv	s8,a0
	mv	a1,s9
	li	a0,8
	call	auto_cal_timing
	sw	a0,12(sp)
	bne	a0,zero,.L45
	li	a5,1
	sw	a5,12(sp)
.L45:
	li	a5,2
	mv	s11,a0
	bgeu	a0,a5,.L46
	li	s11,2
.L46:
	mv	a1,s9
	li	a0,15
	call	auto_cal_timing
	li	a5,2
	mv	t1,a0
	bgeu	a0,a5,.L47
	li	t1,2
.L47:
	mv	a1,s9
	li	a0,17
	sw	t1,20(sp)
	call	auto_cal_timing
	mv	a1,s9
	mv	s7,a0
	li	a0,42
	call	auto_cal_timing
	mv	s10,a0
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
	j	.L83
.L41:
	li	a5,7
	bne	s4,a5,.L74
	mv	a1,s9
	li	a0,50
	call	auto_cal_timing
	li	a5,4
	mv	s6,a0
	bgeu	a0,a5,.L48
	li	s6,4
.L48:
	mv	a1,s9
	li	a0,10
	call	auto_cal_timing
	mv	s5,a0
	bne	a0,zero,.L49
	li	s5,1
.L49:
	mv	a1,s9
	li	a0,24
	call	auto_cal_timing
	li	a5,2
	mv	s3,a0
	bgeu	a0,a5,.L50
	li	s3,2
.L50:
	mv	a1,s9
	li	a0,70
	call	auto_cal_timing
	mv	s8,a0
	mv	a1,s9
	li	a0,8
	call	auto_cal_timing
	li	a5,2
	mv	s11,a0
	bgeu	a0,a5,.L51
	li	s11,2
.L51:
	mv	a1,s9
	li	a0,15
	call	auto_cal_timing
	li	a5,2
	mv	t1,a0
	bgeu	a0,a5,.L52
	li	t1,2
.L52:
	mv	a1,s9
	li	a0,17
	sw	t1,20(sp)
	call	auto_cal_timing
	mv	a1,s9
	mv	s7,a0
	li	a0,42
	call	auto_cal_timing
	mv	s10,a0
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
	sw	s11,12(sp)
	j	.L83
.L74:
	li	a5,128
	sw	a5,16(sp)
	li	a5,10
	li	s0,98
	sw	a5,12(sp)
	li	t1,8
	li	s11,3
	li	s7,6
	li	s10,14
	li	s6,16
	li	s8,20
	li	s3,6
	li	s5,3
	j	.L36
.L53:
	addi	a4,s4,-2
	bgtu	a4,a5,.L57
	li	a5,2
.L57:
	divu	s0,s0,a5
	j	.L55
.L54:
	addi	a3,s4,-2
	bgtu	a3,a4,.L57
	li	a5,4
	j	.L57
.L59:
	li	a5,7
	bne	s4,a5,.L86
	li	a5,60
	divu	a5,s1,a5
	sw	a5,40(sp)
	li	a5,800
	bleu	s1,a5,.L81
	li	a4,4
	li	a2,3
	li	a5,6
	li	t6,12
.L66:
	li	a3,200
	mul	a3,s1,a3
	li	a1,1000
	addi	t1,t1,5
	addi	a7,s11,5
	mv	a6,s4
	lw	t5,52(sp)
	li	a0,5
	li	t3,13
	li	s4,3
	li	t4,12
	addi	a3,a3,1
	sw	a3,20(sp)
	li	a3,100
	mul	a3,s1,a3
	divu	a3,a3,a1
	li	a1,5
	addi	a3,a3,1
	sw	a3,24(sp)
	li	a3,11
	mul	a3,s1,a3
	addi	a3,a3,1
	sw	a3,28(sp)
	addi	a3,s1,1
	sw	a3,32(sp)
	add	a3,t1,a4
	sw	a3,36(sp)
	add	a3,a7,a4
	mv	s11,a3
	li	a7,5
	li	t1,5
	j	.L84
.L60:
	li	a5,30
	divu	a5,s1,a5
	sw	a5,40(sp)
	li	a5,409
	bgtu	s1,a5,.L77
	li	a3,4096
	li	a6,3
	li	a5,1
	addi	a3,a3,-1437
.L64:
	li	a4,400
	mul	a4,s1,a4
	li	a1,1000
	lw	t0,44(sp)
	li	a0,5
	li	a7,4
	li	t3,4
	li	t4,12
	li	a2,1
	li	t5,0
	li	t6,0
	addi	a4,a4,1
	sw	a4,20(sp)
	li	a4,500
	mul	a4,s1,a4
	divu	a4,a4,a1
	li	a1,0
	addi	a4,a4,1
	sw	a4,24(sp)
	li	a4,200
	mul	a4,s1,a4
	addi	a4,a4,1
	sw	a4,28(sp)
	addi	a4,s1,1
	sw	a4,32(sp)
	addi	a4,t1,5
	sw	a4,36(sp)
	addi	a4,s11,5
	mv	s11,a4
	mv	t1,s4
	li	a4,3
	li	s4,3
	j	.L62
.L77:
	li	a3,4096
	li	a5,2
	li	a6,4
	addi	a3,a3,-397
	j	.L64
.L61:
	li	a5,30
	divu	a5,s1,a5
	sw	a5,40(sp)
	li	a5,800
	bleu	s1,a5,.L78
	li	a3,8192
	li	a2,3
	li	a6,7
	li	a5,5
	li	t6,32
	addi	a3,a3,-492
.L65:
	li	a4,500
	mul	a4,s1,a4
	li	a1,1000
	addi	t1,t1,2
	addi	a7,s11,2
	addi	a4,a4,1
	sw	a4,20(sp)
	li	a4,360
	mul	a4,s1,a4
	divu	a4,a4,a1
	addi	a4,a4,1
	sw	a4,24(sp)
	li	a4,200
	mul	a4,s1,a4
	addi	a4,a4,1
	sw	a4,28(sp)
	addi	a4,s1,1
	sw	a4,32(sp)
	add	a4,t1,a5
	sw	a4,36(sp)
	add	a4,a7,a5
	mv	s11,a4
	lw	a4,56(sp)
	extu	a1,a4,2+2-1,2
	li	a4,1
	beq	a1,a4,.L80
	li	a4,912
	bleu	s1,a4,.L80
	lw	t0,44(sp)
	mv	a4,a5
	li	a0,5
	li	a7,4
	li	t3,6
.L85:
	li	t4,12
	li	t1,4
	li	a1,0
	li	t5,0
	j	.L62
.L78:
	li	a3,8192
	li	a6,6
	li	a2,2
	li	a5,4
	li	t6,24
	addi	a3,a3,-912
	j	.L65
.L58:
	li	a5,60
	divu	a5,s1,a5
	li	a4,1000
	lw	t5,52(sp)
	li	t6,6
	li	a0,5
	li	a7,5
	li	t3,10
	li	s4,2
	li	t4,5
	li	a1,3
	li	a6,4
	li	a2,1
	sw	a5,40(sp)
	li	a5,200
	mul	a5,s1,a5
	addi	a5,a5,1
	sw	a5,20(sp)
	li	a5,100
	mul	a5,s1,a5
	divu	a5,a5,a4
	li	a4,2
	addi	a5,a5,1
	sw	a5,24(sp)
	li	a5,11
	mul	a5,s1,a5
	addi	a5,a5,1
	sw	a5,28(sp)
	addi	a5,s1,1
	sw	a5,32(sp)
	addi	a5,t1,5
	sw	a5,36(sp)
	addi	a5,s11,5
	mv	s11,a5
	li	t1,5
	li	a5,3
.L84:
	li	t0,195
	li	a3,0
	j	.L62
.L81:
	li	a4,3
	li	s4,6
	li	a2,2
	li	a5,5
	li	t6,10
	j	.L66
.L80:
	lw	t0,44(sp)
	mv	a4,a5
	li	a0,5
	li	a7,4
	li	t3,5
	j	.L85
.L72:
	li	a3,-268406784
	j	.L87
	.size	auto_set_timing_para, .-auto_set_timing_para
	.section	.text.ccm_get_hosc_freq,"ax",@progbits
	.align	1
	.globl	ccm_get_hosc_freq
	.type	ccm_get_hosc_freq, @function
ccm_get_hosc_freq:
	li	a5,1241579520
	lw	a4,1032(a5)
	li	a4,1
	sw	a4,1032(a5)
	li	a4,1241579520
.L89:
	lw	a5,1032(a4)
	addi	a3,a4,1032
	andi	a5,a5,2
	beq	a5,zero,.L89
	lw	a5,0(a3)
	li	a4,-57344
	addi	a4,a4,-1857
	extu	a5,a5,4+20-1,4
	add	a5,a5,a4
	li	a4,40960
	addi	a4,a4,-162
	li	a0,40
	bleu	a5,a4,.L88
	li	a0,24
.L88:
	ret
	.size	ccm_get_hosc_freq, .-ccm_get_hosc_freq
	.globl	__floatunsisf
	.globl	__divsf3
	.globl	__subsf3
	.globl	__mulsf3
	.globl	__fixsfsi
	.section	.text.ccm_get_sscg,"ax",@progbits
	.align	1
	.globl	ccm_get_sscg
	.type	ccm_get_sscg, @function
ccm_get_sscg:
	addi	sp,sp,-32
	li	a5,24
	sw	s2,16(sp)
	divu	s2,a1,a5
	sw	ra,28(sp)
	sw	s0,24(sp)
	sw	s1,20(sp)
	sw	s3,12(sp)
	li	a0,0
	mul	a5,s2,a5
	beq	a5,a1,.L95
	lw	a5,0(a2)
	extu	a5,a5,1+1-1,1
	beq	a5,zero,.L96
	lw	a5,0(a3)
	li	a4,524288
	or	a5,a5,a4
	sw	a5,0(a3)
.L96:
	mv	a0,a1
	mv	s0,a3
	mv	s1,a2
	call	__floatunsisf
	lw	a1,.LC2
	call	__divsf3
	mv	s3,a0
	mv	a0,s2
	call	__floatunsisf
	mv	a1,a0
	mv	a0,s3
	call	__subsf3
	lw	a1,.LC3
	call	__mulsf3
	call	__fixsfsi
	sw	a0,0(s0)
	lw	a5,0(s0)
	li	a4,655360
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,-536477696
	or	a5,a5,a4
	sw	a5,0(s0)
	li	a0,20
	call	__usdelay
	lw	a5,0(s1)
	li	a4,16777216
	or	a5,a5,a4
	sw	a5,0(s1)
	lw	a0,0(s0)
	li	a5,131072
	addi	a5,a5,-1
	and	a0,a0,a5
.L95:
	lw	ra,28(sp)
	lw	s0,24(sp)
	lw	s1,20(sp)
	lw	s2,16(sp)
	lw	s3,12(sp)
	addi	sp,sp,32
	jr	ra
	.size	ccm_get_sscg, .-ccm_get_sscg
	.section	.text.ccm_set_pll_sscg,"ax",@progbits
	.align	1
	.globl	ccm_set_pll_sscg
	.type	ccm_set_pll_sscg, @function
ccm_set_pll_sscg:
	lw	a5,0(a1)
	addi	sp,sp,-32
	sw	ra,28(sp)
	extu	a5,a5,1+2-1,1
	bne	a5,zero,.L104
	lw	a5,0(a2)
	li	a4,-1572864
	addi	a4,a4,-1
	and	a5,a5,a4
.L116:
	sw	a5,0(a2)
	lw	a5,92(a0)
	li	a4,4
	extu	a5,a5,20+3-1,20
	addi	a5,a5,-1
	bgtu	a5,a4,.L107
	lla	a4,.L109
	lrw	a5,a4,a5,2
	add	a5,a5,a4
	jr	a5
	.section	.rodata
	.align	2
	.align	2
.L109:
	.word	.L113-.L109
	.word	.L112-.L109
	.word	.L111-.L109
	.word	.L107-.L109
	.word	.L108-.L109
	.section	.text.ccm_set_pll_sscg
.L104:
	li	a4,-1572864
	li	a7,1
	addi	a6,a4,-1
	bne	a5,a7,.L106
	lw	a5,0(a2)
	li	a4,524288
	and	a5,a5,a6
.L117:
	or	a5,a4,a5
	j	.L116
.L106:
	lw	a4,0(a2)
	slli	a5,a5,19
	and	a4,a4,a6
	j	.L117
.L113:
	li	a5,-922300416
	addi	a5,a5,-820
.L118:
	sw	a5,0(a2)
	li	a5,-2147483648
	sw	a5,0(a3)
	li	a0,20
	sw	a1,12(sp)
	call	__usdelay
	lw	a1,12(sp)
	lw	ra,28(sp)
	li	a4,16777216
	lw	a5,0(a1)
	li	a0,0
	or	a5,a5,a4
	sw	a5,0(a1)
	addi	sp,sp,32
	jr	ra
.L112:
	li	a5,-771317760
	addi	a5,a5,-1639
	j	.L118
.L111:
	li	a5,-620339200
	addi	a5,a5,1638
	j	.L118
.L108:
	li	a5,-351928320
	j	.L118
.L107:
	li	a5,-469356544
	addi	a5,a5,819
	j	.L118
	.size	ccm_set_pll_sscg, .-ccm_set_pll_sscg
	.section	.text.ccm_set_pll_ddr_clk,"ax",@progbits
	.align	1
	.globl	ccm_set_pll_ddr_clk
	.type	ccm_set_pll_ddr_clk, @function
ccm_set_pll_ddr_clk:
	lw	a5,92(a1)
	addi	sp,sp,-32
	sw	ra,28(sp)
	sw	s0,24(sp)
	sw	s1,20(sp)
	sw	s2,16(sp)
	sw	s3,12(sp)
	extu	a5,a5,6+1-1,6
	bne	a5,a0,.L120
	lw	s0,0(a1)
.L128:
	call	ccm_get_hosc_freq
	slli	s0,s0,1
	divu	s0,s0,a0
	li	a5,11
	mv	s2,a0
	li	s1,1
	bgtu	s0,a5,.L122
	slli	s0,s0,2
	li	s1,2
.L122:
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
	lw	a5,0(a4)
	li	a3,-536870912
	addi	a3,a3,-1
	and	a5,a5,a3
	sw	a5,0(a4)
	lw	a5,0(a4)
	li	a3,536870912
	or	a5,a5,a3
	sw	a5,0(a4)
	li	a3,268435456
	li	a4,1241579520
.L123:
	lw	a5,128(a4)
	addi	s3,a4,128
	and	a5,a5,a3
	beq	a5,zero,.L123
	li	a0,20
	call	__usdelay
	mul	a0,s0,s2
	lw	a5,0(s3)
	li	a4,134217728
	li	a3,1107300352
	or	a5,a5,a4
	sw	a5,0(s3)
	lw	a5,4(a3)
	li	a4,-117637120
	addi	a4,a4,-32
	lw	ra,28(sp)
	divu	a0,a0,s1
	lw	s0,24(sp)
	and	a5,a5,a4
	li	a4,-2130706432
	or	a5,a5,a4
	sw	a5,4(a3)
	lw	s2,16(sp)
	lw	s3,12(sp)
	divu	a0,a0,s1
	lw	s1,20(sp)
	addi	sp,sp,32
	jr	ra
.L120:
	lw	s0,76(a1)
	j	.L128
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
	addi	sp,sp,-32
	sw	s1,20(sp)
	li	s1,1107300352
	sw	s0,24(sp)
	addi	s0,s1,128
	lw	a4,20(s0)
	li	a5,-4096
	addi	a5,a5,-1
	sw	ra,28(sp)
	sw	s2,16(sp)
	sw	s3,12(sp)
	sw	s4,8(sp)
	and	a4,a4,a5
	sw	a4,20(s0)
	lw	a4,16(s0)
	li	s4,-2147483648
	li	s3,134217728
	andi	a4,a4,-9
	sw	a4,16(s0)
	lw	a4,4(s0)
	mv	s2,a0
	li	a0,10
	and	a5,a4,a5
	sw	a5,4(s0)
	lw	a5,0(s0)
	xori	a4,s4,-1
	andi	a5,a5,-9
	sw	a5,0(s0)
	lw	a5,4(s1)
	and	a5,a5,a4
	sw	a5,4(s1)
	lw	a3,4(s1)
	or	a3,a3,s3
	sw	a3,4(s1)
	call	__usdelay
	mv	a1,s2
	li	a0,0
	call	ccm_set_pll_ddr_clk
	srli	a5,a0,1
	mv	a1,a0
	sw	a5,0(s2)
	lla	a0,.LC4
	call	printf
	li	a0,100
	call	__usdelay
	call	dram_disable_all_master
	lw	a3,16(s0)
	li	a5,4096
	li	a0,5
	ori	a3,a3,8
	sw	a3,16(s0)
	lw	a2,20(s0)
	or	a2,a2,a5
	sw	a2,20(s0)
	lw	a2,0(s0)
	ori	a2,a2,8
	sw	a2,0(s0)
	lw	a3,4(s0)
	or	a3,a3,a5
	sw	a3,4(s0)
	lw	a4,4(s1)
	or	a4,a4,s4
	sw	a4,4(s1)
	lw	a5,4(s1)
	or	a5,a5,s3
	sw	a5,4(s1)
	call	__usdelay
	li	a5,1125134336
	li	a4,32768
	li	a0,10
	sw	a4,12(a5)
	call	__usdelay
	lw	ra,28(sp)
	lw	s0,24(sp)
	lw	s1,20(sp)
	lw	s2,16(sp)
	lw	s3,12(sp)
	lw	s4,8(sp)
	li	a0,0
	addi	sp,sp,32
	jr	ra
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
	addi	sp,sp,-16
	and	a5,a5,a3
	li	a3,8192
	or	a5,a5,a3
	sw	s0,12(sp)
	sw	s1,8(sp)
	sw	s2,4(sp)
	sw	a5,8(a4)
	lw	a5,0(a4)
	lw	a2,4(a0)
	lw	a3,20(a0)
	li	a1,-16773120
	addi	a1,a1,-1
	and	a1,a5,a1
	li	a4,458752
	slli	a5,a2,16
	andi	a7,a3,1
	and	a5,a5,a4
	seqz	a4,a7
	or	a5,a5,a1
	slli	a4,a4,12
	or	a4,a5,a4
	addi	a2,a2,-6
	li	a5,1
	bgtu	a2,a5,.L133
	li	a5,4718592
.L159:
	or	a5,a5,a4
	li	a4,1125130240
	srli	a6,a3,12
	sw	a5,0(a4)
	andi	a3,a3,256
	li	a2,1
	beq	a3,zero,.L135
	andi	a5,a6,15
	bne	a5,a2,.L135
	li	a2,2
.L135:
	lw	t3,16(a0)
	li	a1,4096
	addi	t6,a1,-1792
	addi	t2,a1,-1536
	andi	a6,a6,3
	slli	a2,a2,4
	li	a3,0
	li	t1,1125130240
	li	t5,-4096
	li	t4,4
	li	t0,8
	li	s0,1
	li	s1,2
	addi	a1,a1,-2048
.L142:
	lw	a4,0(t1)
	addi	a5,a3,12
	srl	a5,t3,a5
	and	a4,a4,t5
	slli	a5,a5,2
	andi	a5,a5,4
	or	a4,a4,a6
	or	a4,a5,a4
	addi	a5,a3,4
	srl	a5,t3,a5
	addi	a5,a5,-1
	slli	a5,a5,4
	andi	a5,a5,255
	or	a5,a5,a4
	srl	a4,t3,a3
	andi	a4,a4,15
	or	s2,a5,t6
	beq	a4,t4,.L141
	bgtu	a4,t4,.L137
	ori	s2,a5,1792
	beq	a4,s0,.L141
	or	s2,a5,a1
	beq	a4,s1,.L141
.L140:
	ori	s2,a5,1536
	j	.L141
.L133:
	lw	a5,92(a0)
	li	a2,524288
	slli	a5,a5,14
	and	a5,a5,a2
	or	a5,a5,a4
	li	a4,4194304
	j	.L159
.L137:
	or	s2,a5,t2
	bne	a4,t0,.L140
.L141:
	swia	s2,(t1),4,0
	addi	a3,a3,16
	bne	a2,a3,.L142
	li	a5,1125130240
	lw	a5,0(a5)
	li	a4,771
	andi	a5,a5,1
	bne	a5,zero,.L143
	li	a4,513
.L143:
	li	a5,1125134336
	sw	a4,288(a5)
	beq	a7,zero,.L144
	sw	zero,964(a5)
.L144:
	lw	a5,56(a0)
	beq	a5,zero,.L131
	li	a3,1125130240
	lw	a2,0(a3)
	slli	a4,a5,25
	li	a1,100663296
	and	a4,a4,a1
	or	a4,a4,a2
	sw	a4,0(a3)
	lw	a4,4(a3)
	slli	a5,a5,10
	li	a2,2093056
	and	a5,a5,a2
	or	a5,a5,a4
	sw	a5,4(a3)
.L131:
	lw	s0,12(sp)
	lw	s1,8(sp)
	lw	s2,4(sp)
	addi	sp,sp,16
	jr	ra
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
	addi	sp,sp,-96
	sw	s0,88(sp)
	li	a2,22
	mv	s0,a0
	lla	a1,.LC5
	addi	a0,sp,32
	sw	ra,92(sp)
	sw	zero,8(sp)
	sw	zero,12(sp)
	sw	zero,16(sp)
	sw	zero,20(sp)
	sw	zero,24(sp)
	sh	zero,28(sp)
	call	memcpy
	li	a2,22
	lla	a1,.LANCHOR0+352
	addi	a0,sp,56
	call	memcpy
	lw	a4,92(s0)
	li	a5,262144
	li	a2,22
	and	a5,a5,a4
	addi	a1,sp,32
	beq	a5,zero,.L165
	addi	a1,sp,56
.L165:
	addi	a0,sp,8
	call	memcpy_self
	lw	a4,4(s0)
	li	a5,3
	bne	a4,a5,.L160
	lbu	a5,8(sp)
	lbu	a4,9(sp)
	slli	a5,a5,5
	slli	a4,a4,10
	or	a5,a5,a4
	lbu	a4,10(sp)
	slli	a4,a4,15
	or	a5,a5,a4
	lbu	a4,11(sp)
	slli	a4,a4,20
	or	a5,a5,a4
	lbu	a4,12(sp)
	slli	a4,a4,25
	or	a5,a5,a4
	li	a4,1125130240
	addi	a4,a4,1280
	sw	a5,0(a4)
	lbu	a5,14(sp)
	lbu	a3,15(sp)
	slli	a5,a5,5
	slli	a3,a3,10
	or	a5,a5,a3
	lbu	a3,13(sp)
	or	a5,a5,a3
	lbu	a3,16(sp)
	slli	a3,a3,15
	or	a5,a5,a3
	lbu	a3,17(sp)
	slli	a3,a3,20
	or	a5,a5,a3
	lbu	a3,18(sp)
	slli	a3,a3,25
	or	a5,a5,a3
	sw	a5,4(a4)
	lbu	a5,20(sp)
	lbu	a3,21(sp)
	slli	a5,a5,5
	slli	a3,a3,10
	or	a5,a5,a3
	lbu	a3,19(sp)
	or	a5,a5,a3
	lbu	a3,22(sp)
	slli	a3,a3,15
	or	a5,a5,a3
	lbu	a3,23(sp)
	slli	a3,a3,20
	or	a5,a5,a3
	sw	a5,8(a4)
	lbu	a5,25(sp)
	lbu	a3,26(sp)
	slli	a5,a5,5
	slli	a3,a3,10
	or	a5,a5,a3
	lbu	a3,24(sp)
	or	a5,a5,a3
	lbu	a3,27(sp)
	slli	a3,a3,15
	or	a5,a5,a3
	lbu	a3,28(sp)
	slli	a3,a3,20
	or	a5,a5,a3
	lbu	a3,29(sp)
	slli	a3,a3,25
	or	a5,a5,a3
	sw	a5,12(a4)
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
.L160:
	lw	ra,92(sp)
	lw	s0,88(sp)
	addi	sp,sp,96
	jr	ra
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
	addi	sp,sp,-32
	sw	s1,20(sp)
	sw	s2,16(sp)
	lw	s1,92(a1)
	mv	s2,a1
	li	a1,1125130240
	lw	a4,12(a1)
	lw	a2,0(s2)
	li	a3,-4096
	and	a5,a4,a3
	srli	a4,a2,1
	addi	a4,a4,-1
	or	a4,a4,a5
	sw	ra,28(sp)
	sw	s0,24(sp)
	sw	s3,12(sp)
	sw	s4,8(sp)
	sw	a4,12(a1)
	li	a4,1125134336
	lw	a5,264(a4)
	addi	a3,a3,255
	extu	s1,s1,2+2-1,2
	and	a5,a5,a3
	ori	a5,a5,768
	sw	a5,264(a4)
	lw	a5,12(s2)
	lw	a4,836(a4)
	li	a3,672
	not	a5,a5
	slli	a5,a5,5
	andi	a4,a4,-49
	andi	a5,a5,32
	or	a4,a4,a5
	bgtu	a2,a3,.L167
	li	a3,-61440
	addi	a3,a3,-15
	and	a4,a4,a3
.L168:
	li	a3,1125134336
	sw	a4,836(a3)
	lw	a4,964(a3)
	andi	a4,a4,-49
	or	a5,a5,a4
	li	a4,672
	bgtu	a2,a4,.L169
	li	a4,-61440
	addi	a4,a4,-15
	and	a5,a5,a4
.L170:
	li	s0,1125134336
	sw	a5,964(s0)
	lw	a5,520(s0)
	mv	a0,s2
	ori	a5,a5,2
	sw	a5,520(s0)
	call	eye_delay_compensation
	li	a5,1
	beq	s1,a5,.L171
	li	a5,2
	beq	s1,a5,.L172
	addi	s0,s0,256
	lw	a5,8(s0)
	li	a0,10
	andi	a5,a5,-65
	sw	a5,8(s0)
	call	__usdelay
	lw	a5,8(s0)
	ori	a5,a5,192
	sw	a5,8(s0)
	j	.L174
.L167:
	li	a3,-61440
	addi	a3,a3,-1551
	and	a4,a4,a3
	ori	a4,a4,1024
	j	.L168
.L169:
	li	a4,-61440
	addi	a4,a4,-1551
	and	a5,a5,a4
	ori	a5,a5,1024
	j	.L170
.L171:
	addi	a4,s0,256
	lw	a5,8(a4)
	addi	s0,s0,128
	andi	a5,a5,-193
	sw	a5,8(a4)
	lw	a5,60(s0)
	andi	a5,a5,-264
	sw	a5,60(s0)
.L174:
	lw	a5,4(s2)
	li	a4,1
	addi	a5,a5,-6
	bgtu	a5,a4,.L175
	li	a3,1125134336
	lw	a5,284(a3)
	bne	s1,a4,.L176
	li	a4,-134217728
	addi	a4,a4,-193
	and	a5,a5,a4
	li	a4,-2147483648
.L212:
	or	a5,a5,a4
	sw	a5,284(a3)
.L175:
	lw	a4,20(s2)
	li	a5,4096
	li	a3,1125134336
	and	a5,a5,a4
	beq	a5,zero,.L177
	lw	a5,192(a3)
	li	a4,-268435456
	and	a5,a5,a4
	li	a4,50331648
	addi	a4,a4,1
.L213:
	or	a5,a5,a4
	sw	a5,192(a3)
	li	a5,1241513984
	lw	a5,448(a5)
	li	a4,1
	andi	a5,a5,15
	bne	a5,a4,.L179
	li	a4,1241518080
	lw	a5,-1992(a4)
	li	a0,10
	andi	a5,a5,-3
	sw	a5,-1992(a4)
	call	__usdelay
.L179:
	li	a4,1125134336
	lw	a5,320(a4)
	li	a3,-67108864
	li	a2,16777216
	and	a3,a5,a3
	lw	a5,8(s2)
	addi	a2,a2,-1
	and	a5,a5,a2
	or	a5,a5,a3
	li	a3,33554432
	or	a5,a5,a3
	sw	a5,320(a4)
	li	a3,1
	bne	s1,a3,.L180
	li	a5,82
	sw	a5,0(a4)
	li	a5,83
	sw	a5,0(a4)
.L181:
	lw	a5,16(a4)
	andi	a5,a5,1
	beq	a5,zero,.L181
	li	a0,10
	call	__usdelay
	lw	a3,4(s2)
	li	a4,3
	li	a5,1312
	bne	a3,a4,.L182
	li	a5,1440
.L182:
	li	a4,1125134336
	sw	a5,0(a4)
	ori	a5,a5,1
	sw	a5,0(a4)
	li	a0,10
	call	__usdelay
	li	s2,1125134336
.L183:
	lw	s0,16(s2)
	andi	s0,s0,1
	beq	s0,zero,.L183
	li	a5,1241513984
	lw	a5,448(a5)
	li	a4,1
	andi	a5,a5,15
	bne	a5,a4,.L184
	addi	a3,s2,256
	lw	a5,12(a3)
	li	a4,-100663296
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,67108864
	or	a5,a5,a4
	sw	a5,12(a3)
	li	a0,10
	call	__usdelay
	lw	a5,4(s2)
	li	a4,3
	ori	a5,a5,1
	sw	a5,4(s2)
	li	s2,1125134336
.L185:
	lw	a5,24(s2)
	andi	a5,a5,7
	bne	a5,a4,.L185
	li	a4,1241518080
	lw	a5,-1992(a4)
	li	a0,10
	li	s3,1
	andi	a5,a5,-2
	sw	a5,-1992(a4)
	call	__usdelay
	lw	a5,4(s2)
	andi	a5,a5,-2
	sw	a5,4(s2)
	li	s2,1125134336
.L186:
	lw	a5,24(s2)
	andi	a5,a5,7
	bne	a5,s3,.L186
	li	a0,15
	call	__usdelay
	bne	s1,s3,.L184
	addi	a4,s2,256
	lw	a5,8(a4)
	li	a3,-100663296
	addi	a3,a3,-1
	andi	a5,a5,-193
	sw	a5,8(a4)
	lw	a5,12(a4)
	li	a0,1
	and	a5,a5,a3
	li	a3,33554432
	or	a5,a5,a3
	sw	a5,12(a4)
	call	__usdelay
	li	a5,1025
	sw	a5,0(s2)
	li	a4,1125134336
.L187:
	lw	a5,16(a4)
	andi	a5,a5,1
	beq	a5,zero,.L187
.L184:
	li	a5,1125134336
	lw	a5,16(a5)
	extu	a4,a5,20+8-1,20
	beq	a4,zero,.L188
	li	s0,1048576
	and	s0,a5,s0
	beq	s0,zero,.L188
	lla	a0,.LC7
	call	printf
	li	s0,0
.L166:
	lw	ra,28(sp)
	mv	a0,s0
	lw	s0,24(sp)
	lw	s1,20(sp)
	lw	s2,16(sp)
	lw	s3,12(sp)
	lw	s4,8(sp)
	addi	sp,sp,32
	jr	ra
.L172:
	addi	a4,s0,256
	lw	a5,8(a4)
	andi	a5,a5,-193
	ori	a5,a5,128
	sw	a5,8(a4)
	lw	a5,96(s0)
	addi	s0,s0,128
	lw	a3,60(s0)
	extu	a5,a5,16+5-1,16
	addi	a5,a5,-2
	andi	a3,a3,-264
	or	a5,a5,a3
	ori	a5,a5,256
	sw	a5,60(s0)
	lw	a5,28(a4)
	li	a3,-2147483648
	xori	a3,a3,-1
	and	a5,a5,a3
	li	a3,134217728
	or	a5,a5,a3
	sw	a5,28(a4)
	j	.L174
.L176:
	li	a4,-1996488704
	addi	a4,a4,-1
	and	a5,a5,a4
	li	a4,570425344
	j	.L212
.L177:
	lw	a5,192(a3)
	li	a4,-268435456
	and	a5,a5,a4
	li	a4,16789504
	addi	a4,a4,135
	j	.L213
.L180:
	li	a5,1241513984
	lw	a4,448(a5)
	li	a5,98
	andi	a4,a4,15
	beq	a4,a3,.L182
	lw	a3,4(s2)
	li	a4,3
	li	a5,370
	bne	a3,a4,.L182
	li	a5,498
	j	.L182
.L188:
	li	s2,1125134336
.L190:
	lw	a5,24(s2)
	andi	a5,a5,1
	beq	a5,zero,.L190
	lw	a5,140(s2)
	li	s4,-2147483648
	li	a0,10
	or	a5,a5,s4
	sw	a5,140(s2)
	call	__usdelay
	lw	a5,140(s2)
	xori	a4,s4,-1
	li	a0,10
	and	a5,a5,a4
	sw	a5,140(s2)
	call	__usdelay
	li	a4,1125130240
	lw	a5,20(a4)
	li	a0,10
	or	a5,a5,s4
	sw	a5,20(a4)
	call	__usdelay
	addi	a3,s2,256
	lw	a5,12(a3)
	li	a4,-100663296
	addi	a4,a4,-1
	and	a5,a5,a4
	sw	a5,12(a3)
	li	a5,1
	bne	s1,a5,.L166
	lw	a5,284(s2)
	andi	a5,a5,-193
	ori	a5,a5,64
	sw	a5,284(s2)
	j	.L166
	.size	mctl_channel_init, .-mctl_channel_init
	.section	.text.DRAMC_get_dram_size,"ax",@progbits
	.align	1
	.globl	DRAMC_get_dram_size
	.type	DRAMC_get_dram_size, @function
DRAMC_get_dram_size:
	li	a4,1125130240
	lw	a5,0(a4)
	extu	a3,a5,4+4-1,4
	extu	a0,a5,8+4-1,8
	add	a0,a0,a3
	addi	a0,a0,-14
	extu	a3,a5,2+2-1,2
	add	a0,a0,a3
	andi	a5,a5,3
	li	a3,1
	sll	a0,a3,a0
	beq	a5,zero,.L215
	lw	a4,4(a4)
	mv	a5,a0
	andi	a2,a4,3
	beq	a2,zero,.L215
	extu	a5,a4,8+4-1,8
	extu	a2,a4,4+4-1,4
	add	a5,a5,a2
	addi	a5,a5,-14
	extu	a4,a4,2+2-1,2
	add	a5,a5,a4
	sll	a5,a3,a5
.L215:
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
	li	a5,1125134336
	lw	a4,16(a5)
	addi	sp,sp,-16
	li	a3,4194304
	sw	ra,12(sp)
	sw	s0,8(sp)
	and	a4,a4,a3
	beq	a4,zero,.L222
	lw	a1,840(a5)
	lw	s0,968(a5)
	li	a5,2
	extu	a1,a1,24+2-1,24
	extu	s0,s0,24+2-1,24
	bne	a1,a5,.L231
	lw	a5,20(a0)
	li	a4,-61440
	addi	a4,a4,-16
	and	a5,a5,a4
	bne	s0,a1,.L226
	sw	a5,20(a0)
	lla	a0,.LC9
	j	.L233
.L222:
	lw	a5,20(a0)
	li	a4,4096
	andi	a5,a5,-16
	or	a5,a5,a4
	sw	a5,20(a0)
	lla	a0,.LC8
.L233:
	call	printf
	li	a0,1
.L221:
	lw	ra,12(sp)
	lw	s0,8(sp)
	addi	sp,sp,16
	jr	ra
.L226:
	ori	a5,a5,1
	sw	a5,20(a0)
	lla	a0,.LC10
	j	.L233
.L231:
	bne	a1,zero,.L227
	lw	a5,20(a0)
	li	a4,4096
	addi	a4,a4,1
	andi	a5,a5,-16
	or	a5,a5,a4
	sw	a5,20(a0)
	lla	a0,.LC11
	j	.L233
.L227:
	lw	a5,92(a0)
	li	a0,536870912
	and	a0,a0,a5
	beq	a0,zero,.L221
	lla	a0,.LC12
	call	printf
	mv	a1,s0
	lla	a0,.LC13
	call	printf
	li	a0,0
	j	.L221
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
	addi	sp,sp,-32
	sw	s0,24(sp)
	srli	a0,a0,1
	li	s0,19087360
	li	a3,-19087360
	sw	ra,28(sp)
	sw	s1,20(sp)
	slli	a0,a0,20
	li	a4,-2147483648
	li	a5,0
	addi	s0,s0,1383
	addi	a3,a3,-1384
.L235:
	bne	a5,a1,.L236
	li	a4,-38178816
	add	a5,a5,s0
	li	s1,-2147483648
	addi	a4,a4,1329
.L237:
	bne	a5,s0,.L241
	lla	a0,.LC16
	call	printf
	li	a0,0
	j	.L234
.L236:
	add	a2,a5,s0
	sw	a2,0(a4)
	add	a2,a5,a3
	srw	a2,a0,a4,0
	addi	a5,a5,1
	addi	a4,a4,4
	j	.L235
.L241:
	lrw	a1,a0,s1,0
	add	a2,s0,a4
	add	a3,a0,s1
	beq	a2,a1,.L238
	lla	a0,.LC14
	sw	a1,12(sp)
	sw	a2,8(sp)
	sw	a3,4(sp)
	call	printf
	lw	a3,4(sp)
	lw	a2,8(sp)
	lw	a1,12(sp)
.L243:
	lla	a0,.LC15
	call	printf
	li	a0,1
.L234:
	lw	ra,28(sp)
	lw	s0,24(sp)
	lw	s1,20(sp)
	addi	sp,sp,32
	jr	ra
.L238:
	mv	a3,s1
	lwia	a1,(a3),4,0
	beq	a1,s0,.L240
	lla	a0,.LC14
	sw	a1,4(sp)
	call	printf
	lw	a1,4(sp)
	mv	a3,s1
	mv	a2,s0
	j	.L243
.L240:
	addi	s0,s0,1
	mv	s1,a3
	j	.L237
	.size	dramc_simple_wr_test, .-dramc_simple_wr_test
	.section	.text.mctl_vrefzq_init,"ax",@progbits
	.align	1
	.globl	mctl_vrefzq_init
	.type	mctl_vrefzq_init, @function
mctl_vrefzq_init:
	lw	a3,92(a0)
	li	a5,131072
	and	a5,a3,a5
	bne	a5,zero,.L244
	li	a4,1125134336
	lw	a5,272(a4)
	li	a2,-2139062272
	addi	a2,a2,128
	and	a5,a5,a2
	lw	a2,60(a0)
	or	a5,a5,a2
	sw	a5,272(a4)
	li	a5,65536
	and	a3,a3,a5
	bne	a3,zero,.L244
	lw	a5,276(a4)
	andi	a3,a5,-128
	lw	a5,64(a0)
	andi	a5,a5,127
	or	a5,a5,a3
	sw	a5,276(a4)
.L244:
	ret
	.size	mctl_vrefzq_init, .-mctl_vrefzq_init
	.section	.text.mctl_core_init,"ax",@progbits
	.align	1
	.globl	mctl_core_init
	.type	mctl_core_init, @function
mctl_core_init:
	addi	sp,sp,-16
	sw	ra,12(sp)
	sw	s0,8(sp)
	mv	s0,a0
	call	mctl_sys_init
	mv	a0,s0
	call	mctl_vrefzq_init
	mv	a0,s0
	call	mctl_com_init
	mv	a0,s0
	call	auto_set_timing_para
	mv	a1,s0
	lw	s0,8(sp)
	lw	ra,12(sp)
	li	a0,0
	addi	sp,sp,16
	tail	mctl_channel_init
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
	addi	sp,sp,-80
	sw	s7,44(sp)
	sw	ra,76(sp)
	sw	s0,72(sp)
	sw	s1,68(sp)
	sw	s2,64(sp)
	sw	s3,60(sp)
	sw	s4,56(sp)
	sw	s5,52(sp)
	sw	s6,48(sp)
	sw	s8,40(sp)
	sw	s9,36(sp)
	sw	s10,32(sp)
	sw	s11,28(sp)
	mv	s7,a0
	call	mctl_core_init
	bne	a0,zero,.L251
	mv	s0,a0
	lla	a0,.LC17
.L301:
	call	printf
.L250:
	lw	ra,76(sp)
	mv	a0,s0
	lw	s0,72(sp)
	lw	s1,68(sp)
	lw	s2,64(sp)
	lw	s3,60(sp)
	lw	s4,56(sp)
	lw	s5,52(sp)
	lw	s6,48(sp)
	lw	s7,44(sp)
	lw	s8,40(sp)
	lw	s9,36(sp)
	lw	s10,32(sp)
	lw	s11,28(sp)
	addi	sp,sp,80
	jr	ra
.L251:
	lw	a5,20(s7)
	li	s1,1
	extu	a5,a5,12+4-1,12
	bne	a5,s1,.L253
	li	s1,2
.L253:
	li	a5,-2147483648
	li	s8,-4096
	li	s10,0
	li	s0,0
	sw	a5,8(sp)
	li	s9,-2147483648
	li	s2,64
	li	s11,1
	addi	s8,s8,3
.L254:
	mv	a4,s9
	li	a5,0
.L257:
	andi	a3,a5,1
	mv	a2,a4
	bne	a3,zero,.L256
	not	a2,a4
.L256:
	swia	a2,(a4),4,0
	addi	a5,a5,1
	bne	a5,s2,.L257
	li	s5,1125130240
	lrw	a5,s5,s0,2
	li	a4,-4096
	addi	a4,a4,243
	and	a5,a5,a4
	slli	s4,s0,2
	ori	a5,a5,1776
	add	s6,s4,s5
	srw	a5,s5,s0,2
.L258:
	lw	a2,0(s6)
	bne	a5,a2,.L258
	lw	a5,8(sp)
	li	a2,11
	li	t3,17
	not	s3,a5
.L281:
	addi	a1,a2,11
	sll	a1,s11,a1
	add	a1,a1,s9
	mv	a0,s3
	li	a5,0
.L261:
	andi	t4,a5,1
	not	t1,a0
	mveqz	t1, a0, t4
	lrw	t4,a1,a5,2
	bne	t4,t1,.L260
	addi	a5,a5,1
	addi	a0,a0,-4
	bne	a5,s2,.L261
.L282:
	li	a5,16
	bleu	a2,a5,.L262
	li	a2,16
.L262:
	mv	a1,s0
	lla	a0,.LC18
	sw	a2,12(sp)
	call	printf
	lw	a0,16(s7)
	addi	a1,s10,4
	lw	a2,12(sp)
	li	a5,255
	sll	a5,a5,a1
	not	a5,a5
	and	a5,a5,a0
	sll	a2,a2,a1
	or	a2,a5,a2
	sw	a2,16(s7)
	bne	s0,s11,.L263
	li	a2,1125130240
	lw	a5,0(a2)
	li	s9,-2139095040
	and	a5,a5,s8
	ori	a5,a5,1700
	sw	a5,0(a2)
.L263:
	lrw	a5,s4,s5,0
	and	a5,a5,s8
	ori	a5,a5,1700
	srw	a5,s4,s5,0
.L264:
	lw	a2,0(s6)
	bne	a2,a5,.L264
	li	a5,4096
	addi	a5,a5,-2048
	add	a5,s9,a5
	mv	a1,s3
	li	a2,0
.L268:
	andi	t1,a2,1
	not	a0,a1
	mveqz	a0, a1, t1
	lw	t1,0(a5)
	bne	t1,a0,.L285
	addi	a2,a2,1
	addi	a1,a1,-4
	addi	a5,a5,4
	bne	a2,s2,.L268
	li	t1,0
.L267:
	addi	a2,t1,1
	slli	a2,a2,2
	mv	a1,s0
	lla	a0,.LC19
	sw	t1,12(sp)
	call	printf
	lw	a1,16(s7)
	addi	a2,s10,12
	lw	t1,12(sp)
	li	a5,15
	sll	a5,a5,a2
	not	a5,a5
	and	a5,a5,a1
	sll	t1,t1,a2
	or	a5,a5,t1
	sw	a5,16(s7)
	bne	s0,s11,.L269
	li	a2,1125130240
	lw	a5,0(a2)
	li	a4,4096
	addi	a4,a4,-1376
	and	a5,a5,s8
	or	a5,a5,a4
	sw	a5,0(a2)
	li	s9,-2080374784
.L269:
	lrw	a5,s4,s5,0
	li	a4,4096
	addi	a4,a4,-1376
	and	a5,a5,s8
	or	a5,a5,a4
	srw	a5,s4,s5,0
.L270:
	lw	a4,0(s6)
	bne	a4,a5,.L270
	li	a5,9
	li	a0,14
.L278:
	sll	a2,s11,a5
	add	a2,a2,s9
	mv	a3,s3
	li	a4,0
.L273:
	andi	a7,a4,1
	not	a1,a3
	mveqz	a1, a3, a7
	lrw	a7,a2,a4,2
	bne	a7,a1,.L272
	addi	a4,a4,1
	addi	a3,a3,-4
	bne	a4,s2,.L273
	li	a4,13
	beq	a5,a4,.L279
	li	a4,9
	li	s3,0
	beq	a5,a4,.L275
.L279:
	addi	a5,a5,-10
	sll	s3,s11,a5
.L275:
	mv	a2,s3
	mv	a1,s0
	lla	a0,.LC20
	call	printf
	lw	a4,16(s7)
	li	a5,15
	sll	a5,a5,s10
	not	a5,a5
	and	a5,a5,a4
	sll	a2,s3,s10
	or	a2,a5,a2
	sw	a2,16(s7)
	addi	s0,s0,1
	bne	s1,s0,.L276
	li	a5,2
	li	s0,1
	bne	s1,a5,.L250
	lw	a5,20(s7)
	li	a4,-4096
	addi	a4,a4,255
	and	a5,a5,a4
	srli	a4,a2,16
	extu	a2,a2,15,0
	bne	a4,a2,.L277
	sw	a5,20(s7)
	lla	a0,.LC21
	j	.L301
.L276:
	bne	s0,s11,.L255
	li	a4,1125130240
	lw	a5,0(a4)
	li	s9,-2013265920
	and	a5,a5,s8
	ori	a5,a5,1776
	sw	a5,0(a4)
	lw	a5,4(a4)
	and	a5,a5,s8
	ori	a5,a5,1777
	sw	a5,4(a4)
	li	a5,-2013265920
	sw	a5,8(sp)
.L255:
	addi	s10,s10,16
	j	.L254
.L285:
	li	t1,1
	j	.L267
.L277:
	ori	a5,a5,256
	sw	a5,20(s7)
	lla	a0,.LC22
	j	.L301
.L272:
	addi	a5,a5,1
	bne	a5,a0,.L278
	li	a5,13
	j	.L279
.L260:
	addi	a2,a2,1
	bne	a2,t3,.L281
	j	.L282
	.size	auto_scan_dram_size, .-auto_scan_dram_size
	.section	.text.auto_scan_dram_rank_width,"ax",@progbits
	.align	1
	.globl	auto_scan_dram_rank_width
	.type	auto_scan_dram_rank_width, @function
auto_scan_dram_rank_width:
	addi	sp,sp,-16
	li	a5,11534336
	sw	s0,8(sp)
	sw	s1,4(sp)
	sw	s2,0(sp)
	sw	ra,12(sp)
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
	li	a4,1048576
	and	a5,a5,a4
	beq	a5,zero,.L303
.L305:
	li	a0,0
.L302:
	lw	ra,12(sp)
	lw	s0,8(sp)
	lw	s1,4(sp)
	lw	s2,0(sp)
	addi	sp,sp,16
	jr	ra
.L303:
	mv	a0,s0
	call	dqs_gate_detect
	beq	a0,zero,.L305
	sw	s1,92(s0)
	sw	s2,16(s0)
	li	a0,1
	j	.L302
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
	lw	a4,92(a0)
	addi	sp,sp,-16
	li	a5,16384
	sw	s1,4(sp)
	sw	ra,12(sp)
	sw	s0,8(sp)
	and	a5,a5,a4
	mv	s1,a0
	bne	a5,zero,.L311
	call	auto_scan_dram_rank_width
	mv	s0,a0
	lla	a0,.LC23
	beq	s0,zero,.L316
.L311:
	lw	a5,92(s1)
	andi	a5,a5,1
	bne	a5,zero,.L313
	mv	a0,s1
	call	auto_scan_dram_size
	mv	s0,a0
	bne	a0,zero,.L313
	lla	a0,.LC24
.L316:
	call	printf
.L310:
	lw	ra,12(sp)
	mv	a0,s0
	lw	s0,8(sp)
	lw	s1,4(sp)
	addi	sp,sp,16
	jr	ra
.L313:
	lw	a5,92(s1)
	li	a4,32768
	li	s0,1
	and	a4,a5,a4
	bne	a4,zero,.L310
	li	a4,24576
	addi	a4,a4,1
	or	a5,a5,a4
	sw	a5,92(s1)
	j	.L310
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
	li	a5,1241513984
	lw	a4,64(a5)
	addi	sp,sp,-16
	sw	s0,8(sp)
	sw	ra,12(sp)
	sw	s1,4(sp)
	li	a5,2
	mv	s0,a1
	bne	a4,a5,.L318
	lw	a4,92(a1)
	li	a5,65536
	and	a5,a5,a4
	bne	a5,zero,.L319
	li	a5,1241518080
	sw	zero,-1992(a5)
	sw	zero,-1988(a5)
	li	a0,150
	li	s1,1124073472
	addi	s1,s1,256
	call	__usdelay
	lw	a5,96(s1)
	li	a0,10
	andi	a5,a5,-4
	sw	a5,96(s1)
	call	__usdelay
	lw	a5,96(s1)
	li	a0,10
	andi	a5,a5,-261
	sw	a5,96(s1)
	lw	a5,96(s1)
	ori	a5,a5,2
	sw	a5,96(s1)
	call	__usdelay
	lw	a5,96(s1)
	li	a0,20
	ori	a5,a5,1
	sw	a5,96(s1)
	call	__usdelay
	lla	a0,.LC27
	call	printf
	lw	a1,108(s1)
	lla	a0,.LC28
	call	printf
.L320:
	lw	a5,92(s0)
	andi	a5,a5,1
	beq	a5,zero,.L321
.L324:
	lla	a1,.LC30
	lla	a0,.LC31
	call	printf
	lw	a1,0(s0)
	lla	a0,.LC32
	call	printf
	lw	a1,4(s0)
	lla	a0,.LC33
	call	printf
	lw	a5,12(s0)
	andi	a5,a5,1
	bne	a5,zero,.L322
	lla	a0,.LC35
	call	printf
	j	.L326
.L319:
	lla	a0,.LC29
	call	printf
	li	a5,1124073472
	lw	a4,352(a5)
	li	a0,10
	ori	a4,a4,256
	sw	a4,352(a5)
	sw	zero,360(a5)
.L359:
	call	__usdelay
	j	.L320
.L318:
	li	a5,1241518080
	sw	zero,-1992(a5)
	sw	zero,-1988(a5)
	li	a0,150
	j	.L359
.L321:
	mv	a0,s0
	call	auto_scan_dram_config
	bne	a0,zero,.L324
.L362:
	li	a0,0
.L317:
	lw	ra,12(sp)
	lw	s0,8(sp)
	lw	s1,4(sp)
	addi	sp,sp,16
	jr	ra
.L322:
	lw	a1,8(s0)
	lla	a0,.LC34
	call	printf
.L326:
	lw	a1,28(s0)
	andi	a5,a1,68
	beq	a5,zero,.L327
	lla	a0,.LC36
	call	printf
.L328:
	mv	a0,s0
	call	mctl_core_init
	bne	a0,zero,.L329
	lla	a0,.LC38
	call	printf
	j	.L362
.L327:
	lla	a0,.LC37
	call	printf
	j	.L328
.L329:
	lw	a5,56(s0)
	li	a4,1
	extu	a5,a5,12+4-1,12
	beq	a5,a4,.L330
	li	a3,2
	beq	a5,a3,.L331
	lw	a5,4(s0)
	lla	a1,.LC25
	addi	a5,a5,-2
	bleu	a5,a4,.L334
	lla	a1,.LC26
.L334:
	lla	a0,.LC41
	call	printf
	j	.L333
.L330:
	lla	a0,.LC39
.L360:
	call	printf
.L333:
	lw	s1,20(s0)
	bge	s1,zero,.L335
	extu	s1,s1,16+15-1,16
.L336:
	mv	a1,s1
	lla	a0,.LC42
	call	printf
	lw	a4,92(s0)
	li	a5,1073741824
	and	a5,a5,a4
	beq	a5,zero,.L337
	li	a5,1125134336
	lw	a5,160(a5)
	lw	a5,72(s0)
	bne	a5,zero,.L338
	li	a5,268435456
	addi	a5,a5,512
.L338:
	li	a4,1125134336
	addi	a3,a4,128
	sw	a5,32(a3)
	li	a5,1034
	sw	a5,28(a3)
	lw	a5,4(a4)
	lla	a0,.LC43
	ori	a5,a5,1
	sw	a5,4(a4)
	call	printf
.L339:
	lw	a5,92(s0)
	lw	a2,4(s0)
	li	a1,1125134336
	andi	a4,a5,512
	bne	a4,zero,.L340
	li	a4,6
	bne	a2,a4,.L341
.L340:
	lw	a4,256(a1)
	li	a3,-61440
	addi	a3,a3,-1
	and	a4,a4,a3
	li	a3,20480
	or	a4,a4,a3
.L361:
	sw	a4,256(a1)
	li	a4,1125134336
	lw	a3,320(a4)
	li	a1,-2147483648
	or	a3,a3,a1
	sw	a3,320(a4)
	andi	a3,a5,256
	beq	a3,zero,.L343
	lw	a3,184(a4)
	ori	a3,a3,768
	sw	a3,184(a4)
.L343:
	li	a4,67108864
	and	a4,a5,a4
	beq	a4,zero,.L344
	li	a1,1125134336
	lw	a4,264(a1)
	li	a3,-8192
	addi	a3,a3,-1
	and	a4,a4,a3
	sw	a4,264(a1)
.L345:
	li	a4,7
	bne	a2,a4,.L346
	li	a2,1125134336
	lw	a4,124(a2)
	li	a3,-983040
	addi	a3,a3,-1
	and	a4,a4,a3
	li	a3,65536
	or	a4,a4,a3
	sw	a4,124(a2)
.L346:
	li	a4,134217728
	and	a5,a5,a4
	bne	a5,zero,.L347
	mv	a0,s0
	call	set_master_priority_pad
.L347:
	call	dram_enable_all_master
	lw	a4,92(s0)
	li	a5,268435456
	and	a5,a5,a4
	bne	a5,zero,.L348
.L349:
	call	handler_super_standby
	mv	a0,s1
	j	.L317
.L331:
	lla	a0,.LC40
	j	.L360
.L335:
	call	DRAMC_get_dram_size
	lhu	a5,20(s0)
	slli	a4,a0,16
	mv	s1,a0
	or	a5,a5,a4
	sw	a5,20(s0)
	j	.L336
.L337:
	li	a5,1125134336
	addi	a3,a5,128
	lw	a4,32(a3)
	li	a2,-65536
	and	a4,a4,a2
	sw	a4,32(a3)
	lw	a4,4(a5)
	andi	a4,a4,-2
	sw	a4,4(a5)
	j	.L339
.L341:
	lw	a4,256(a1)
	li	a3,-61440
	addi	a3,a3,-1
	and	a4,a4,a3
	j	.L361
.L344:
	li	a3,1125134336
	lw	a4,264(a3)
	li	a1,8192
	or	a4,a4,a1
	sw	a4,264(a3)
	j	.L345
.L348:
	li	a5,1241513984
	lw	a5,448(a5)
	li	a4,1
	andi	a5,a5,15
	beq	a5,a4,.L349
	li	a1,4096
	mv	a0,s1
	call	dramc_simple_wr_test
	beq	a0,zero,.L349
	j	.L362
	.size	init_DRAM, .-init_DRAM
	.section	.text.mctl_init,"ax",@progbits
	.align	1
	.globl	mctl_init
	.type	mctl_init, @function
mctl_init:
	addi	sp,sp,-112
	mv	a1,sp
	li	a0,0
	sw	ra,108(sp)
	call	init_DRAM
	lw	ra,108(sp)
	seqz	a0,a0
	addi	sp,sp,112
	jr	ra
	.size	mctl_init, .-mctl_init
	.section	.rodata.cst4,"aM",@progbits,4
	.align	2
.LC2:
	.word	1103101952
	.align	2
.LC3:
	.word	1207959552
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
	.ident	"GCC: (Xuantie-900 linux-5.10.4 glibc gcc Toolchain V2.8.1 B-20240115) 10.4.0"
	.section	.note.GNU-stack,"",@progbits
