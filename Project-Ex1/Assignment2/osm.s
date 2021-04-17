	.file	"osm.cpp"
# GNU C++14 (Ubuntu 9.3.0-17ubuntu1~20.04) version 9.3.0 (x86_64-linux-gnu)
#	compiled by GNU C version 9.3.0, GMP version 6.2.0, MPFR version 4.0.2, MPC version 1.1.0, isl version isl-0.22.1-GMP

# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  -imultiarch x86_64-linux-gnu -D_GNU_SOURCE osm.cpp
# -mtune=generic -march=x86-64 -auxbase-strip osm.s -g -fverbose-asm
# -fasynchronous-unwind-tables -fstack-protector-strong -Wformat
# -Wformat-security -fstack-clash-protection -fcf-protection
# options enabled:  -fPIC -fPIE -faggressive-loop-optimizations
# -fassume-phsa -fasynchronous-unwind-tables -fauto-inc-dec -fcommon
# -fdelete-null-pointer-checks -fdwarf2-cfi-asm -fearly-inlining
# -feliminate-unused-debug-types -fexceptions -ffp-int-builtin-inexact
# -ffunction-cse -fgcse-lm -fgnu-runtime -fgnu-unique -fident
# -finline-atomics -fipa-stack-alignment -fira-hoist-pressure
# -fira-share-save-slots -fira-share-spill-slots -fivopts
# -fkeep-static-consts -fleading-underscore -flifetime-dse
# -flto-odr-type-merging -fmath-errno -fmerge-debug-strings -fpeephole
# -fplt -fprefetch-loop-arrays -freg-struct-return
# -fsched-critical-path-heuristic -fsched-dep-count-heuristic
# -fsched-group-heuristic -fsched-interblock -fsched-last-insn-heuristic
# -fsched-rank-heuristic -fsched-spec -fsched-spec-insn-heuristic
# -fsched-stalled-insns-dep -fschedule-fusion -fsemantic-interposition
# -fshow-column -fshrink-wrap-separate -fsigned-zeros
# -fsplit-ivs-in-unroller -fssa-backprop -fstack-clash-protection
# -fstack-protector-strong -fstdarg-opt -fstrict-volatile-bitfields
# -fsync-libcalls -ftrapping-math -ftree-cselim -ftree-forwprop
# -ftree-loop-if-convert -ftree-loop-im -ftree-loop-ivcanon
# -ftree-loop-optimize -ftree-parallelize-loops= -ftree-phiprop
# -ftree-reassoc -ftree-scev-cprop -funit-at-a-time -funwind-tables
# -fverbose-asm -fzero-initialized-in-bss -m128bit-long-double -m64 -m80387
# -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387 -mfxsr
# -mglibc -mieee-fp -mlong-double-80 -mmmx -mno-sse4 -mpush-args -mred-zone
# -msse -msse2 -mstv -mtls-direct-seg-refs -mvzeroupper

	.text
.Ltext0:
	.globl	_Z18osm_operation_timej
	.type	_Z18osm_operation_timej, @function
_Z18osm_operation_timej:
.LFB0:
	.file 1 "osm.cpp"
	.loc 1 11 52
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$80, %rsp	#,
	movl	%edi, -68(%rbp)	# iterations, iterations
# osm.cpp:11: double osm_operation_time(unsigned int iterations) {
	.loc 1 11 52
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp104
	movq	%rax, -8(%rbp)	# tmp104, D.2538
	xorl	%eax, %eax	# tmp104
# osm.cpp:12:     if (iterations == 0) {
	.loc 1 12 5
	cmpl	$0, -68(%rbp)	#, iterations
	jne	.L2	#,
# osm.cpp:13:         return EXIT_FAILURE;
	.loc 1 13 16
	movsd	.LC0(%rip), %xmm0	#, _13
	jmp	.L8	#
.L2:
# osm.cpp:17:     gettimeofday(&start_time, nullptr);
	.loc 1 17 17
	leaq	-48(%rbp), %rax	#, tmp94
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp94,
	call	gettimeofday@PLT	#
.LBB2:
# osm.cpp:19:         unsigned int counter = 0;
	.loc 1 19 22
	movl	$0, -64(%rbp)	#, counter
.LBB3:
# osm.cpp:20:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 20 27
	movl	$0, -60(%rbp)	#, i
.L5:
# osm.cpp:20:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 20 36 discriminator 3
	movl	-60(%rbp), %eax	# i, tmp95
	cmpl	-68(%rbp), %eax	# iterations, tmp95
	jnb	.L4	#,
# osm.cpp:21:             ++counter;
	.loc 1 21 13 discriminator 2
	addl	$1, -64(%rbp)	#, counter
# osm.cpp:20:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 20 9 discriminator 2
	addl	$1, -60(%rbp)	#, i
	jmp	.L5	#
.L4:
.LBE3:
.LBE2:
# osm.cpp:24:     gettimeofday(&end_time, nullptr);
	.loc 1 24 17
	leaq	-32(%rbp), %rax	#, tmp96
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp96,
	call	gettimeofday@PLT	#
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 30
	movq	-32(%rbp), %rdx	# end_time.tv_sec, _1
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 50
	movq	-48(%rbp), %rax	# start_time.tv_sec, _2
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 37
	subq	%rax, %rdx	# _2, _1
	movq	%rdx, %rax	# _1, _3
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 58
	imulq	$1000000000, %rax, %rdx	#, _3, _4
# osm.cpp:27:             (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 27 23
	movq	-24(%rbp), %rcx	# end_time.tv_usec, _5
# osm.cpp:27:             (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 27 44
	movq	-40(%rbp), %rax	# start_time.tv_usec, _6
# osm.cpp:27:             (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 27 31
	subq	%rax, %rcx	# _6, _5
	movq	%rcx, %rax	# _5, _7
# osm.cpp:27:             (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 27 53
	imulq	$1000, %rax, %rax	#, _7, _8
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 75
	addq	%rdx, %rax	# _4, _9
# osm.cpp:26:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 26 12
	cvtsi2sdq	%rax, %xmm0	# _9, tmp97
	movsd	%xmm0, -56(%rbp)	# tmp97, delta
# osm.cpp:28:     return delta / iterations;
	.loc 1 28 18
	movl	-68(%rbp), %eax	# iterations, tmp98
	testq	%rax, %rax	# tmp98
	js	.L6	#,
	cvtsi2sdq	%rax, %xmm0	# tmp98, _10
	jmp	.L7	#
.L6:
	movq	%rax, %rdx	# tmp98, tmp100
	shrq	%rdx	# tmp100
	andl	$1, %eax	#, tmp101
	orq	%rax, %rdx	# tmp101, tmp100
	cvtsi2sdq	%rdx, %xmm0	# tmp100, tmp99
	addsd	%xmm0, %xmm0	# tmp99, _10
.L7:
# osm.cpp:28:     return delta / iterations;
	.loc 1 28 20
	movsd	-56(%rbp), %xmm1	# delta, tmp102
	divsd	%xmm0, %xmm1	# _10, tmp102
	movapd	%xmm1, %xmm0	# tmp102, _13
.L8:
# osm.cpp:29: }
	.loc 1 29 1 discriminator 1
	movq	-8(%rbp), %rax	# D.2538, tmp105
	xorq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp105
	je	.L9	#,
# osm.cpp:29: }
	.loc 1 29 1 is_stmt 0
	call	__stack_chk_fail@PLT	#
.L9:
	leave	
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE0:
	.size	_Z18osm_operation_timej, .-_Z18osm_operation_timej
	.globl	_Z10cdecl_funcv
	.type	_Z10cdecl_funcv, @function
_Z10cdecl_funcv:
.LFB1:
	.loc 1 33 19 is_stmt 1
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
# osm.cpp:33: void cdecl_func() {}
	.loc 1 33 20
	nop	
	popq	%rbp	#
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE1:
	.size	_Z10cdecl_funcv, .-_Z10cdecl_funcv
	.globl	_Z17osm_function_timej
	.type	_Z17osm_function_timej, @function
_Z17osm_function_timej:
.LFB2:
	.loc 1 35 51
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	subq	$80, %rsp	#,
	movl	%edi, -68(%rbp)	# iterations, iterations
# osm.cpp:35: double osm_function_time(unsigned int iterations) {
	.loc 1 35 51
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp104
	movq	%rax, -8(%rbp)	# tmp104, D.2540
	xorl	%eax, %eax	# tmp104
# osm.cpp:36:     if (iterations == 0) {
	.loc 1 36 5
	cmpl	$0, -68(%rbp)	#, iterations
	jne	.L12	#,
# osm.cpp:37:         return EXIT_FAILURE;
	.loc 1 37 16
	movsd	.LC0(%rip), %xmm0	#, _12
	jmp	.L18	#
.L12:
# osm.cpp:41:     gettimeofday(&start_time, nullptr);
	.loc 1 41 17
	leaq	-48(%rbp), %rax	#, tmp94
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp94,
	call	gettimeofday@PLT	#
.LBB4:
.LBB5:
# osm.cpp:43:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 43 27
	movl	$0, -60(%rbp)	#, i
.L15:
# osm.cpp:43:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 43 36 discriminator 3
	movl	-60(%rbp), %eax	# i, tmp95
	cmpl	-68(%rbp), %eax	# iterations, tmp95
	jnb	.L14	#,
# osm.cpp:44:             cdecl_func();
	.loc 1 44 23 discriminator 2
	call	_Z10cdecl_funcv	#
# osm.cpp:43:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 43 9 discriminator 2
	addl	$1, -60(%rbp)	#, i
	jmp	.L15	#
.L14:
.LBE5:
.LBE4:
# osm.cpp:47:     gettimeofday(&end_time, nullptr);
	.loc 1 47 17
	leaq	-32(%rbp), %rax	#, tmp96
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp96,
	call	gettimeofday@PLT	#
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 30
	movq	-32(%rbp), %rdx	# end_time.tv_sec, _1
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 50
	movq	-48(%rbp), %rax	# start_time.tv_sec, _2
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 37
	subq	%rax, %rdx	# _2, _1
	movq	%rdx, %rax	# _1, _3
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 58
	imulq	$1000000000, %rax, %rdx	#, _3, _4
# osm.cpp:50:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 50 30
	movq	-24(%rbp), %rcx	# end_time.tv_usec, _5
# osm.cpp:50:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 50 51
	movq	-40(%rbp), %rax	# start_time.tv_usec, _6
# osm.cpp:50:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 50 38
	subq	%rax, %rcx	# _6, _5
	movq	%rcx, %rax	# _5, _7
# osm.cpp:50:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 50 60
	imulq	$1000, %rax, %rax	#, _7, _8
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 75
	addq	%rdx, %rax	# _4, _9
# osm.cpp:49:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 49 12
	cvtsi2sdq	%rax, %xmm0	# _9, tmp97
	movsd	%xmm0, -56(%rbp)	# tmp97, delta
# osm.cpp:51:     return delta / iterations;
	.loc 1 51 18
	movl	-68(%rbp), %eax	# iterations, tmp98
	testq	%rax, %rax	# tmp98
	js	.L16	#,
	cvtsi2sdq	%rax, %xmm0	# tmp98, _10
	jmp	.L17	#
.L16:
	movq	%rax, %rdx	# tmp98, tmp100
	shrq	%rdx	# tmp100
	andl	$1, %eax	#, tmp101
	orq	%rax, %rdx	# tmp101, tmp100
	cvtsi2sdq	%rdx, %xmm0	# tmp100, tmp99
	addsd	%xmm0, %xmm0	# tmp99, _10
.L17:
# osm.cpp:51:     return delta / iterations;
	.loc 1 51 20
	movsd	-56(%rbp), %xmm1	# delta, tmp102
	divsd	%xmm0, %xmm1	# _10, tmp102
	movapd	%xmm1, %xmm0	# tmp102, _12
.L18:
# osm.cpp:52: }
	.loc 1 52 1 discriminator 1
	movq	-8(%rbp), %rax	# D.2540, tmp105
	xorq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp105
	je	.L19	#,
# osm.cpp:52: }
	.loc 1 52 1 is_stmt 0
	call	__stack_chk_fail@PLT	#
.L19:
	leave	
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE2:
	.size	_Z17osm_function_timej, .-_Z17osm_function_timej
	.globl	_Z16osm_syscall_timej
	.type	_Z16osm_syscall_timej, @function
_Z16osm_syscall_timej:
.LFB3:
	.loc 1 54 50 is_stmt 1
	.cfi_startproc
	endbr64	
	pushq	%rbp	#
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp	#,
	.cfi_def_cfa_register 6
	pushq	%rbx	#
	subq	$88, %rsp	#,
	.cfi_offset 3, -24
	movl	%edi, -84(%rbp)	# iterations, iterations
# osm.cpp:54: double osm_syscall_time(unsigned int iterations) {
	.loc 1 54 50
	movq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp108
	movq	%rax, -24(%rbp)	# tmp108, D.2541
	xorl	%eax, %eax	# tmp108
# osm.cpp:55:     if (iterations == 0) {
	.loc 1 55 5
	cmpl	$0, -84(%rbp)	#, iterations
	jne	.L21	#,
# osm.cpp:56:         return EXIT_FAILURE;
	.loc 1 56 16
	movsd	.LC0(%rip), %xmm0	#, _12
	jmp	.L27	#
.L21:
# osm.cpp:60:     gettimeofday(&start_time, nullptr);
	.loc 1 60 17
	leaq	-64(%rbp), %rax	#, tmp94
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp94,
	call	gettimeofday@PLT	#
.LBB6:
.LBB7:
# osm.cpp:62:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 62 27
	movl	$0, -76(%rbp)	#, i
.L24:
# osm.cpp:62:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 62 36 discriminator 3
	movl	-76(%rbp), %eax	# i, tmp95
	cmpl	-84(%rbp), %eax	# iterations, tmp95
	jnb	.L23	#,
# osm.cpp:63:             OSM_NULLSYSCALL;
	.loc 1 63 28 discriminator 2
	movl	$-1, %eax	#, tmp96
	movl	$0, %esi	#, tmp97
	movl	$0, %ecx	#, tmp98
	movl	$0, %edx	#, tmp99
	movl	%esi, %ebx	# tmp97, tmp97
#APP
# 63 "osm.cpp" 1
	int $0x80 
# 0 "" 2
# osm.cpp:62:         for (unsigned int i = 0; i < iterations; ++i) {
	.loc 1 62 9 discriminator 2
#NO_APP
	addl	$1, -76(%rbp)	#, i
	jmp	.L24	#
.L23:
.LBE7:
.LBE6:
# osm.cpp:66:     gettimeofday(&end_time, nullptr);
	.loc 1 66 17
	leaq	-48(%rbp), %rax	#, tmp100
	movl	$0, %esi	#,
	movq	%rax, %rdi	# tmp100,
	call	gettimeofday@PLT	#
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 30
	movq	-48(%rbp), %rdx	# end_time.tv_sec, _1
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 50
	movq	-64(%rbp), %rax	# start_time.tv_sec, _2
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 37
	subq	%rax, %rdx	# _2, _1
	movq	%rdx, %rax	# _1, _3
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 58
	imulq	$1000000000, %rax, %rdx	#, _3, _4
# osm.cpp:69:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 69 30
	movq	-40(%rbp), %rcx	# end_time.tv_usec, _5
# osm.cpp:69:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 69 51
	movq	-56(%rbp), %rax	# start_time.tv_usec, _6
# osm.cpp:69:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 69 38
	subq	%rax, %rcx	# _6, _5
	movq	%rcx, %rax	# _5, _7
# osm.cpp:69:                    (end_time.tv_usec - start_time.tv_usec) * MICROSEC_TO_NANOSEC;
	.loc 1 69 60
	imulq	$1000, %rax, %rax	#, _7, _8
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 75
	addq	%rdx, %rax	# _4, _9
# osm.cpp:68:     double delta = (end_time.tv_sec - start_time.tv_sec) * SEC_TO_NANOSEC +
	.loc 1 68 12
	cvtsi2sdq	%rax, %xmm0	# _9, tmp101
	movsd	%xmm0, -72(%rbp)	# tmp101, delta
# osm.cpp:70:     return delta / iterations;
	.loc 1 70 18
	movl	-84(%rbp), %eax	# iterations, tmp102
	testq	%rax, %rax	# tmp102
	js	.L25	#,
	cvtsi2sdq	%rax, %xmm0	# tmp102, _10
	jmp	.L26	#
.L25:
	movq	%rax, %rdx	# tmp102, tmp104
	shrq	%rdx	# tmp104
	andl	$1, %eax	#, tmp105
	orq	%rax, %rdx	# tmp105, tmp104
	cvtsi2sdq	%rdx, %xmm0	# tmp104, tmp103
	addsd	%xmm0, %xmm0	# tmp103, _10
.L26:
# osm.cpp:70:     return delta / iterations;
	.loc 1 70 20
	movsd	-72(%rbp), %xmm1	# delta, tmp106
	divsd	%xmm0, %xmm1	# _10, tmp106
	movapd	%xmm1, %xmm0	# tmp106, _12
.L27:
# osm.cpp:71: }
	.loc 1 71 1 discriminator 1
	movq	-24(%rbp), %rax	# D.2541, tmp109
	xorq	%fs:40, %rax	# MEM[(<address-space-1> long unsigned int *)40B], tmp109
	je	.L28	#,
# osm.cpp:71: }
	.loc 1 71 1 is_stmt 0
	call	__stack_chk_fail@PLT	#
.L28:
	addq	$88, %rsp	#,
	popq	%rbx	#
	popq	%rbp	#
	.cfi_def_cfa 7, 8
	ret	
	.cfi_endproc
.LFE3:
	.size	_Z16osm_syscall_timej, .-_Z16osm_syscall_timej
	.section	.rodata
	.align 8
.LC0:
	.long	0
	.long	-1074790400
	.text
.Letext0:
	.file 2 "/usr/include/x86_64-linux-gnu/bits/types.h"
	.file 3 "/usr/include/x86_64-linux-gnu/bits/types/struct_timeval.h"
	.section	.debug_info,"",@progbits
.Ldebug_info0:
	.long	0x27c
	.value	0x4
	.long	.Ldebug_abbrev0
	.byte	0x8
	.uleb128 0x1
	.long	.LASF22
	.byte	0x4
	.long	.LASF23
	.long	.LASF24
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.long	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.long	.LASF0
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.long	.LASF1
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.long	.LASF2
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.long	.LASF3
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF4
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.long	.LASF5
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.string	"int"
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.long	.LASF6
	.uleb128 0x4
	.long	.LASF7
	.byte	0x2
	.byte	0xa0
	.byte	0x1a
	.long	0x5e
	.uleb128 0x4
	.long	.LASF8
	.byte	0x2
	.byte	0xa2
	.byte	0x1f
	.long	0x5e
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.long	.LASF9
	.uleb128 0x5
	.long	.LASF25
	.byte	0x10
	.byte	0x3
	.byte	0x8
	.byte	0x8
	.long	0xac
	.uleb128 0x6
	.long	.LASF10
	.byte	0x3
	.byte	0xa
	.byte	0xc
	.long	0x65
	.byte	0
	.uleb128 0x6
	.long	.LASF11
	.byte	0x3
	.byte	0xb
	.byte	0x11
	.long	0x71
	.byte	0x8
	.byte	0
	.uleb128 0x7
	.long	.LASF16
	.byte	0x1
	.byte	0x36
	.byte	0x8
	.long	.LASF18
	.long	0x132
	.quad	.LFB3
	.quad	.LFE3-.LFB3
	.uleb128 0x1
	.byte	0x9c
	.long	0x132
	.uleb128 0x8
	.long	.LASF20
	.byte	0x1
	.byte	0x36
	.byte	0x26
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -100
	.uleb128 0x9
	.long	.LASF12
	.byte	0x1
	.byte	0x3b
	.byte	0x14
	.long	0x84
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0x9
	.long	.LASF13
	.byte	0x1
	.byte	0x3b
	.byte	0x20
	.long	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x9
	.long	.LASF14
	.byte	0x1
	.byte	0x44
	.byte	0xc
	.long	0x132
	.uleb128 0x3
	.byte	0x91
	.sleb128 -88
	.uleb128 0xa
	.quad	.LBB7
	.quad	.LBE7-.LBB7
	.uleb128 0xb
	.string	"i"
	.byte	0x1
	.byte	0x3e
	.byte	0x1b
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -92
	.byte	0
	.byte	0
	.uleb128 0x2
	.byte	0x8
	.byte	0x4
	.long	.LASF15
	.uleb128 0x7
	.long	.LASF17
	.byte	0x1
	.byte	0x23
	.byte	0x8
	.long	.LASF19
	.long	0x132
	.quad	.LFB2
	.quad	.LFE2-.LFB2
	.uleb128 0x1
	.byte	0x9c
	.long	0x1be
	.uleb128 0x8
	.long	.LASF20
	.byte	0x1
	.byte	0x23
	.byte	0x27
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0x9
	.long	.LASF12
	.byte	0x1
	.byte	0x28
	.byte	0x14
	.long	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x9
	.long	.LASF13
	.byte	0x1
	.byte	0x28
	.byte	0x20
	.long	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x9
	.long	.LASF14
	.byte	0x1
	.byte	0x31
	.byte	0xc
	.long	0x132
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0xa
	.quad	.LBB5
	.quad	.LBE5-.LBB5
	.uleb128 0xb
	.string	"i"
	.byte	0x1
	.byte	0x2b
	.byte	0x1b
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.byte	0
	.byte	0
	.uleb128 0xc
	.long	.LASF26
	.byte	0x1
	.byte	0x21
	.byte	0x6
	.long	.LASF27
	.quad	.LFB1
	.quad	.LFE1-.LFB1
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0xd
	.long	.LASF28
	.byte	0x1
	.byte	0xb
	.byte	0x8
	.long	.LASF29
	.long	0x132
	.quad	.LFB0
	.quad	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.uleb128 0x8
	.long	.LASF20
	.byte	0x1
	.byte	0xb
	.byte	0x28
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -84
	.uleb128 0x9
	.long	.LASF12
	.byte	0x1
	.byte	0x10
	.byte	0x14
	.long	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -64
	.uleb128 0x9
	.long	.LASF13
	.byte	0x1
	.byte	0x10
	.byte	0x20
	.long	0x84
	.uleb128 0x2
	.byte	0x91
	.sleb128 -48
	.uleb128 0x9
	.long	.LASF14
	.byte	0x1
	.byte	0x1a
	.byte	0xc
	.long	0x132
	.uleb128 0x3
	.byte	0x91
	.sleb128 -72
	.uleb128 0xa
	.quad	.LBB2
	.quad	.LBE2-.LBB2
	.uleb128 0x9
	.long	.LASF21
	.byte	0x1
	.byte	0x13
	.byte	0x16
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -80
	.uleb128 0xa
	.quad	.LBB3
	.quad	.LBE3-.LBB3
	.uleb128 0xb
	.string	"i"
	.byte	0x1
	.byte	0x14
	.byte	0x1b
	.long	0x3b
	.uleb128 0x3
	.byte	0x91
	.sleb128 -76
	.byte	0
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",@progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x1b
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0x13
	.byte	0x1
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0xd
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x38
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0xb
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x2e
	.byte	0
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x39
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x7
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2116
	.uleb128 0x19
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_aranges,"",@progbits
	.long	0x2c
	.value	0x2
	.long	.Ldebug_info0
	.byte	0x8
	.byte	0
	.value	0
	.value	0
	.quad	.Ltext0
	.quad	.Letext0-.Ltext0
	.quad	0
	.quad	0
	.section	.debug_line,"",@progbits
.Ldebug_line0:
	.section	.debug_str,"MS",@progbits,1
.LASF18:
	.string	"_Z16osm_syscall_timej"
.LASF10:
	.string	"tv_sec"
.LASF5:
	.string	"short int"
.LASF13:
	.string	"end_time"
.LASF17:
	.string	"osm_function_time"
.LASF14:
	.string	"delta"
.LASF16:
	.string	"osm_syscall_time"
.LASF6:
	.string	"long int"
.LASF22:
	.string	"GNU C++14 9.3.0 -mtune=generic -march=x86-64 -g -fasynchronous-unwind-tables -fstack-protector-strong -fstack-clash-protection -fcf-protection"
.LASF0:
	.string	"unsigned char"
.LASF23:
	.string	"osm.cpp"
.LASF21:
	.string	"counter"
.LASF29:
	.string	"_Z18osm_operation_timej"
.LASF4:
	.string	"signed char"
.LASF2:
	.string	"unsigned int"
.LASF19:
	.string	"_Z17osm_function_timej"
.LASF27:
	.string	"_Z10cdecl_funcv"
.LASF1:
	.string	"short unsigned int"
.LASF9:
	.string	"char"
.LASF12:
	.string	"start_time"
.LASF26:
	.string	"cdecl_func"
.LASF3:
	.string	"long unsigned int"
.LASF15:
	.string	"double"
.LASF7:
	.string	"__time_t"
.LASF25:
	.string	"timeval"
.LASF11:
	.string	"tv_usec"
.LASF20:
	.string	"iterations"
.LASF28:
	.string	"osm_operation_time"
.LASF24:
	.string	"/home/dan-os/CLionProjects/OSProjects/HujiOS/Project-Ex1/Assignment2"
.LASF8:
	.string	"__suseconds_t"
	.ident	"GCC: (Ubuntu 9.3.0-17ubuntu1~20.04) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
	.section	.note.gnu.property,"a"
	.align 8
	.long	 1f - 0f
	.long	 4f - 1f
	.long	 5
0:
	.string	 "GNU"
1:
	.align 8
	.long	 0xc0000002
	.long	 3f - 2f
2:
	.long	 0x3
3:
	.align 8
4:
