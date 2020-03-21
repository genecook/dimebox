#********************************************************************
# simple minded asm program to confirm timer interrupts,
# enter/wakeup from low power mode...
#
# to run:
#
#    dimebox -L wfi_timer.o -U --timer 0x8000
#********************************************************************
	
# set timer to cause interrupt, goto sleep 'til timer interrupt...

.global _start

.equ MTVEC_FUDGE_VAL,       0x20
.equ TIMER_BASE,            0x8000
.equ MTIME_CMP,             0x10
.equ WFI_ENABLE_BIT,        0x200000
.equ GLOBAL_INT_ENABLE_BIT, 0x8
.equ TIMER_INT_ENABLE_BIT,  0x80
	
# timer_compare_lo 0x8010
	
_start:
	# set exceptions vector base to address past wfi...
	
	li a0,MTVEC_FUDGE_VAL
	csrw mtvec,a0

	# set nice low value for timer compare...
	
	li a0,TIMER_BASE
	li a1,20
	sw a1,MTIME_CMP(a0)

	# enable global interrupt (MSTATUS.MIE),
	# wfi enable (actually MSTATUS.TW only applies in User Mode)...
	
	li a0,WFI_ENABLE_BIT | GLOBAL_INT_ENABLE_BIT
	csrr a1,mstatus
	or a1,a1,a0
	csrw mstatus,a1

	# enable timer interrupt (MIE.MTIE)...
	
	li a0,TIMER_INT_ENABLE_BIT
	csrr a1,mie
	or a1,a1,a0
	csrw mie,a1

	# enter low power mode. timer interrupt to wake up...
	
	wfi

_mtvec:
	# after timer interrupt execution should resume here or so...
	
	nop
	nop
	nop
	nop

	# branch to self ends test...
	
_end:	j _end
