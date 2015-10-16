# 1 "keyboard.S"
# 1 "/home/capela/oslab/linux-0.11/kernel/chr_drv//"
# 1 "<built-in>"
# 1 "<command line>"
# 1 "keyboard.S"












# 1 "../../include/linux/config.h" 1





















# 36 "../../include/linux/config.h"

# 47 "../../include/linux/config.h"

# 14 "keyboard.S" 2

.text
.globl keyboard_interrupt




size	= 1024		

head = 4
tail = 8
proc_list = 12
buf = 16

mode:	.byte 0		
leds:	.byte 2		
e0:	.byte 0






keyboard_interrupt:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	xor %al,%al		
	inb $0x60,%al
	cmpb $0xe0,%al
	je set_e0
	cmpb $0xe1,%al
	je set_e1
	call key_table(,%eax,4)
	movb $0,e0
e0_e1:	inb $0x61,%al
	jmp 1f
1:	jmp 1f
1:	orb $0x80,%al
	jmp 1f
1:	jmp 1f
1:	outb %al,$0x61
	jmp 1f
1:	jmp 1f
1:	andb $0x7F,%al
	outb %al,$0x61
	movb $0x20,%al
	outb %al,$0x20
	pushl $0
	call do_tty_interrupt
	addl $4,%esp
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret
set_e0:	movb $1,e0
	jmp e0_e1
set_e1:	movb $2,e0
	jmp e0_e1






put_queue:
	pushl %ecx
	pushl %edx
	movl table_list,%edx		# read-queue for console
	movl head(%edx),%ecx
1:	movb %al,buf(%edx,%ecx)
	incl %ecx
	andl $size-1,%ecx
	cmpl tail(%edx),%ecx		# buffer full - discard everything
	je 3f
	shrdl $8,%ebx,%eax
	je 2f
	shrl $8,%ebx
	jmp 1b
2:	movl %ecx,head(%edx)
	movl proc_list(%edx),%ecx
	testl %ecx,%ecx
	je 3f
	movl $0,(%ecx)
3:	popl %edx
	popl %ecx
	ret

ctrl:	movb $0x04,%al
	jmp 1f
alt:	movb $0x10,%al
1:	cmpb $0,e0
	je 2f
	addb %al,%al
2:	orb %al,mode
	ret
unctrl:	movb $0x04,%al
	jmp 1f
unalt:	movb $0x10,%al
1:	cmpb $0,e0
	je 2f
	addb %al,%al
2:	notb %al
	andb %al,mode
	ret

lshift:
	orb $0x01,mode
	ret
unlshift:
	andb $0xfe,mode
	ret
rshift:
	orb $0x02,mode
	ret
unrshift:
	andb $0xfd,mode
	ret

caps:	testb $0x80,mode
	jne 1f
	xorb $4,leds
	xorb $0x40,mode
	orb $0x80,mode
set_leds:
	call kb_wait
	movb $0xed,%al		
	outb %al,$0x60
	call kb_wait
	movb leds,%al
	outb %al,$0x60
	ret
uncaps:	andb $0x7f,mode
	ret
scroll:
	xorb $1,leds
	jmp set_leds
num:	xorb $2,leds
	jmp set_leds





cursor:
	subb $0x47,%al
	jb 1f
	cmpb $12,%al
	ja 1f
	jne cur2		
	testb $0x0c,mode
	je cur2
	testb $0x30,mode
	jne reboot
cur2:	cmpb $0x01,e0		
	je cur
	testb $0x02,leds	
	je cur
	testb $0x03,mode	
	jne cur
	xorl %ebx,%ebx
	movb num_table(%eax),%al
	jmp put_queue
1:	ret

cur:	movb cur_table(%eax),%al
	cmpb $'9,%al
	ja ok_cur
	movb $'~,%ah
ok_cur:	shll $16,%eax
	movw $0x5b1b,%ax
	xorl %ebx,%ebx
	jmp put_queue





num_table:
	.ascii "789 456 1230,"

cur_table:
	.ascii "HA5 DGC YB623"




func:
	pushl %eax
	pushl %ecx
	pushl %edx
	call show_stat
	popl %edx
	popl %ecx
	popl %eax
	subb $0x3B,%al
	jb end_func
	cmpb $9,%al
	jbe ok_func
	subb $18,%al
	cmpb $10,%al
	jb end_func
	cmpb $11,%al
	ja end_func
ok_func:
	cmpl $4,%ecx		
	jl end_func
	movl func_table(,%eax,4),%eax
	xorl %ebx,%ebx
	jmp put_queue
end_func:
	ret




func_table:
	.long 0x415b5b1b,0x425b5b1b,0x435b5b1b,0x445b5b1b
	.long 0x455b5b1b,0x465b5b1b,0x475b5b1b,0x485b5b1b
	.long 0x495b5b1b,0x4a5b5b1b,0x4b5b5b1b,0x4c5b5b1b

# 294 "keyboard.S"

key_map:
	.byte 0,27
	.ascii "1234567890-="
	.byte 127,9
	.ascii "qwertyuiop[]"
	.byte 13,0
	.ascii "asdfghjkl;'"
	.byte '`,0
	.ascii "\\zxcvbnm,./"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		
	.byte '-,0,0,0,'+	
	.byte 0,0,0,0,0,0,0	
	.byte '<
	.fill 10,1,0


shift_map:
	.byte 0,27
	.ascii "!@#$%^&*()_+"
	.byte 127,9
	.ascii "QWERTYUIOP{}"
	.byte 13,0
	.ascii "ASDFGHJKL:\""
	.byte '~,0
	.ascii "|ZXCVBNM<>?"
	.byte 0,'*,0,32		/* 36-39 */
	.fill 16,1,0		
	.byte '-,0,0,0,'+	
	.byte 0,0,0,0,0,0,0	
	.byte '>
	.fill 10,1,0

alt_map:
	.byte 0,0
	.ascii "\0@\0$\0\0{[]}\\\0"
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte '~,13,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0
	.byte 0,0,0,0,0,0,0,0,0,0,0
	.byte 0,0,0,0		
	.fill 16,1,0		
	.byte 0,0,0,0,0		
	.byte 0,0,0,0,0,0,0	
	.byte '|
	.fill 10,1,0

# 449 "keyboard.S"




do_self:
	lea alt_map,%ebx
	testb $0x20,mode		
	jne 1f
	lea shift_map,%ebx
	testb $0x03,mode
	jne 1f
	lea key_map,%ebx
1:	movb (%ebx,%eax),%al
	orb %al,%al
	je none
	testb $0x4c,mode		
	je 2f
	cmpb $'a,%al
	jb 2f
	cmpb $'},%al
	ja 2f
	subb $32,%al
2:	testb $0x0c,mode		
	je 3f
	cmpb $64,%al
	jb 3f
	cmpb $64+32,%al
	jae 3f
	subb $64,%al
3:	testb $0x10,mode		
	je 4f
	orb $0x80,%al
4:	andl $0xff,%eax
	xorl %ebx,%ebx
	call put_queue
none:	ret






minus:	cmpb $1,e0
	jne do_self
	movl $'/,%eax
	xorl %ebx,%ebx
	jmp put_queue






key_table:
	.long none,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,ctrl,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,lshift,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,do_self,do_self,do_self	
	.long do_self,minus,rshift,do_self	
	.long alt,do_self,caps,func		
	.long func,func,func,func		
	.long func,func,func,func		
	.long func,num,scroll,cursor		
	.long cursor,cursor,do_self,cursor	
	.long cursor,cursor,do_self,cursor	
	.long cursor,cursor,cursor,cursor	
	.long none,none,do_self,func		
	.long func,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,unctrl,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,unlshift,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,unrshift,none		
	.long unalt,none,uncaps,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		
	.long none,none,none,none		





kb_wait:
	pushl %eax
1:	inb $0x64,%al
	testb $0x02,%al
	jne 1b
	popl %eax
	ret




reboot:
	call kb_wait
	movw $0x1234,0x472	
	movb $0xfc,%al		
	outb %al,$0x64
die:	jmp die
