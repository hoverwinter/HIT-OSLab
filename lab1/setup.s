.globl begtext, begdata, begbss, endtext, enddata, endbss
.text
begtext:
.data
begdata:
.bss
begbss:
.text

BOOTSEG  = 0x07c0			! original address of boot-sector
INITSEG  = 0x9000			! we move boot here - out of the way
SETUPSEG = 0x9020			! setup starts here

entry _start
_start:

!设置cs=ds=es
	mov	ax,cs
	mov	ds,ax
	mov	es,ax

	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	
	mov	cx,#28
	mov	bx,#0x000c		! page 0, attribute c 
	mov	bp,#msg1
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

! ok, the read went well so we get current cursor position and save it for
! posterity.
! 获取光标位置 =>  0x9000:0
	mov	ax,#INITSEG	! this is done in bootsect already, but...
	mov	ds,ax
	mov	ah,#0x03	! read cursor pos
	xor	bh,bh
	int	0x10		! save it in known place, con_init fetches
	mov	[0],dx		! it from 0x90000.

! Get memory size (extended mem, kB)
! 获取拓展内存大小 => 0x9000:2
	mov	ah,#0x88
	int	0x15
	mov	[2],ax

! Get hd0 data
! 获取硬盘参数 => 0x9000:80  大小：16B
	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x41]
	mov	ax,#INITSEG
	mov	es,ax
	mov	di,#0x0080
	mov	cx,#0x10
	rep
	movsb

! 前面修改了ds寄存器，这里将其设置为0x9000
	mov ax,#INITSEG
	mov ds,ax
	mov ax,#SETUPSEG
	mov	es,ax  

!显示 Cursor POS: 字符串
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#11
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#cur
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

!调用 print_hex 显示具体信息
	mov ax,[0]
	call print_hex
	call print_nl

!显示 Memory SIZE: 字符串
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#12
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#mem
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

!显示 具体信息
	mov ax,[2]
	call print_hex

!显示相应 提示信息
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#25
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#cyl
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

!显示具体信息
	mov ax,[0x80]
	call print_hex
	call print_nl

！显示 提示信息
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#8
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#head
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

！显示 具体信息
	mov ax,[0x80+0x02]
	call print_hex
	call print_nl

！显示 提示信息
	mov	ah,#0x03		! read cursor pos
	xor	bh,bh
	int	0x10
	mov	cx,#8
	mov	bx,#0x0007		! page 0, attribute c 
	mov	bp,#sect
	mov	ax,#0x1301		! write string, move cursor
	int	0x10

！显示 具体信息
	mov ax,[0x80+0x0e]
	call print_hex
	call print_nl

!死循环
l:  jmp l

!以16进制方式打印ax寄存器里的16位数
print_hex:
	mov cx,#4   ! 4个十六进制数字
	mov dx,ax   ! 将ax所指的值放入dx中，ax作为参数传递寄存器
print_digit:
	rol dx,#4  ! 循环以使低4比特用上 !! 取dx的高4比特移到低4比特处。
	mov ax,#0xe0f  ! ah = 请求的功能值,al = 半字节(4个比特)掩码。
	and al,dl ! 取dl的低4比特值。
	add al,#0x30  ! 给al数字加上十六进制0x30
	cmp al,#0x3a
	jl outp  !是一个不大于十的数字
	add al,#0x07  !是a~f,要多加7
outp:
	int 0x10
	loop print_digit
	ret

!打印回车换行
print_nl:
	mov ax,#0xe0d
	int 0x10
	mov al,#0xa
	int 0x10
	ret

msg1:
	.byte 13,10
	.ascii "Now we are in setup..."
	.byte 13,10,13,10
cur:
	.ascii "Cursor POS:"
mem:
	.ascii "Memory SIZE:"
cyl:
	.ascii "KB"
	.byte 13,10,13,10
	.ascii "HD Info"
	.byte 13,10
	.ascii "Cylinders:"
head:
	.ascii "Headers:"
sect:
	.ascii "Secotrs:"

.text
endtext:
.data
enddata:
.bss
endbss:
