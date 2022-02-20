; 关于multiboot的信息参考这里， http://www.cnblogs.com/chio/archive/2008/01/01/1022430.html

MBOOT_HEADER_MAGIC 	equ 	0x1BADB002 	; GRUB 会通过这个魔数判断该映像是否支持
MBOOT_PAGE_ALIGN 	equ 	1 << 0      ; boot module will be aligned to 4KB
MBOOT_MEM_INFO 		equ 	1 << 1    	; 通过 Multiboot 信息结构的 mem* 域包括可用内存的信息，告诉GRUB把内存空间的信息包含在Multiboot信息结构中
MBOOT_HEADER_FLAGS 	equ 	MBOOT_PAGE_ALIGN | MBOOT_MEM_INFO

; 域checksum是一个32位的无符号值，当与其他的magic域(也就是magic和flags)相加时，
; 要求其结果必须是32位的无符号值0 (即magic + flags + checksum = 0)

MBOOT_CHECKSUM 		equ 	-(MBOOT_HEADER_MAGIC + MBOOT_HEADER_FLAGS)

; 符合Multiboot规范的 OS 映象需要一个 magic Multiboot 头
; Multiboot 头的分布必须如下表所示：
; ----------------------------------------------------------
; 偏移量   类型   域名         备注
;   0     u32   magic       必需
;   4     u32   flags       必需
;   8     u32   checksum    必需
;   ......
;-----------------------------------------------------------


[BITS 32]  				; 下面生成的是32位的代码
section .init.text 		; 临时代码段从这里开始，这里是处于低地址区域的

; 在代码段的起始位置设置符合 Multiboot 规范的标记

dd MBOOT_HEADER_MAGIC
dd MBOOT_HEADER_FLAGS
dd MBOOT_CHECKSUM

[GLOBAL _start] 		; 内核代码入口，传给链接器
[GLOBAL mboot_ptr_tmp] 	; 全局的分页开启之前的 struct multiboot * 变量
[EXTERN kern_entry] 	; 声明内核 C 代码的入口函数

_start:
	cli  						; 此时还没有设置好保护模式的中断处理，所以必须关闭中断
	mov [mboot_ptr_tmp], ebx	; 将 ebx 中存储的指针存入 glb_mboot_ptr 变量，ebx 应该是grub提供的
	mov esp, STACK_TOP  		; 设置内核栈地址，按照 multiboot 规范，当需要使用堆栈时，OS 映象必须自己创建一个，这里的地址是分页开启之前的低地址区域的地址
	and esp, 0FFFFFFF0H			; 栈地址按照 16 字节对齐
	mov ebp, 0 					; 帧指针修改为 0，为了下面的跟踪调试方便
	call kern_entry				; 调用内核入口函数
;-----------------------------------------------------------------------------

section .init.data			; 开启分页前临时的数据段，位于低地址区域
stack:    times 1024 db 0  	; 这里作为临时内核栈
STACK_TOP equ $-stack-1 	; 内核栈顶，$符指代是当前地址

mboot_ptr_tmp: dd 0
