;-----------------------------------------------------------
;实验一示例程序                                            |
;                                                          |
;功能：读取开关量的状态取反后送到LED显示                   |
;74LS244带有8位拨位开关，地址为80H or 82H or 84H or 86H    |
;74LS273带有8个发光二极管，地址为88H or 8AH or 8CH or 8EH  |
;                                                          |
;Designer：SYZ                                             |
;-----------------------------------------------------------
		.MODEL	SMALL			; 设定8086汇编程序使用Small model
		.8086				; 设定采用8086汇编指令集
;-----------------------------------------------------------
;	符号定义                                               |
;-----------------------------------------------------------
;
PortIn	EQU	80h	;定义输入端口号
PortOut	EQU	88h	;定义输出端口号
;-----------------------------------------------------------
;	定义堆栈段                                             |
;-----------------------------------------------------------
		.stack 100h				; 定义256字节容量的堆栈

;-----------------------------------------------------------
;	定义数据段                                             |
;-----------------------------------------------------------
		.data					; 定义数据段
;MyVar	DB	?					; 定义MyVar变量   


;-----------------------------------------------------------
;	定义代码段                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
;
;以下开始放置用户指令代码
;

;-----------------------------------------------------------
;主程序部分,读取开关量状态取反后送显示                     |
;-----------------------------------------------------------

Again:
		MOV CX,8FFFH
		
	        MOV AL,36H
	        OUT PortOut,AL
Zero:		DEC CX
		JNZ Zero
		
Start:		
		MOV CX,8FFFH
		MOV AL,33H
		OUT PortOut,AL
One:		DEC CX
		JNZ One
		
		MOV CX,3FH
Two:		MOV AL,33H
		OUT PortOut,AL
		MOV DX,400H
Two_1:		DEC DX
		JNZ Two_1
		MOV AL,37H
		OUT PortOut,AL
		MOV DX,400H
Two_2:		DEC DX
		JNZ Two_2
		DEC CX
		JNZ Two
		
		MOV CX,8FFFH
		MOV AL,35H
		OUT PortOut,AL
Two_3:		DEC CX
		JNZ Two_3
		
		MOV CX,8FFFH
		MOV AL,1EH
		OUT PortOut,AL
Three:		DEC CX
	        JNZ Three
		
		MOV CX,3FH
Four:
		MOV AL,1EH
		OUT PortOut,AL
		MOV DX,400H
Four_1:		DEC DX
		JNZ Four_1
		MOV AL,3EH
		OUT PortOut,AL
		MOV DX,400H
Four_2:		DEC DX
		JNZ Four_2
		DEC CX
		JNZ Four
		
		MOV CX,8FFFH
		MOV AL,2EH
		OUT PortOut,AL
Four_3:		DEC CX
		JNZ Four_3
		
		JMP Start			;跳转循环执行

		END Again			;指示汇编程序结束编译
