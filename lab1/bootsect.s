.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

SETUPLEN = 4				! setup程序代码占用扇区数
BOOTSEG  = 0x07c0			! bootsect程序代码所在内存原始地址
INITSEG  = 0x9000			! 将bootsect移动到0x9000处
SETUPSEG = 0x9020			! setup程序开始的地址

entry _start
_start:

! 下面这段代码将自身复制到0x9000处
	mov	ax,#BOOTSEG
	mov	ds,ax
	mov	ax,#INITSEG
	mov	es,ax
	mov	cx,#256
	sub	si,si
	sub	di,di
	rep
	movw
	
! 复制完成从0x9000的go标号处开始执行
	jmpi	go,INITSEG
go:	mov	ax,cs
	mov	ds,ax  !设置ds=es=cs
	mov	es,ax

! 加载setup.s程序
load_setup: 
	mov	dx,#0x0000		! drive 0, head 0
	mov	cx,#0x0002		! sector 2, track 0
	mov	bx,#0x0200		! address = 512, in INITSEG
	mov	ax,#0x0200+SETUPLEN	! service 2, nr of sectors
	int	0x13			! read it
	jnc	ok_load_setup		! ok - continue
	！加载错误
	mov	dx,#0x0000
	mov	ax,#0x0000		! reset the diskette
	int	0x13
	j	load_setup

ok_load_setup:

!输出一些信息

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#27
	mov	bx,#0x000c		! page 0, attribute c 
	mov	bp,#msg1		! es:bp 指向待显示 字符串
	mov	ax,#0x1301		! write string, move cursor
	int	0x10
!开始执行setup代码
	jmpi 0,SETUPSEG

msg1:
	.byte 13,10
	.ascii "Tonatus is booting..."
	.byte 13,10,13,10

.org 510
boot_flag:
	.word 0xAA55
.text
endtext:
.data
enddata:
.bss
endbss:
