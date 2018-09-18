;-----------------------------------------------------------
;ʵ��һʾ������                                            |
;                                                          |
;���ܣ���ȡ��������״̬ȡ�����͵�LED��ʾ                   |
;74LS244����8λ��λ���أ���ַΪ80H or 82H or 84H or 86H    |
;74LS273����8����������ܣ���ַΪ88H or 8AH or 8CH or 8EH  |
;                                                          |
;Designer��SYZ                                             |
;-----------------------------------------------------------
		.MODEL	SMALL			; �趨8086������ʹ��Small model
		.8086				; �趨����8086���ָ�
;-----------------------------------------------------------
;	���Ŷ���                                               |
;-----------------------------------------------------------
;
PortIn	EQU	80h	;��������˿ں�
PortOut	EQU	88h	;��������˿ں�
;-----------------------------------------------------------
;	�����ջ��                                             |
;-----------------------------------------------------------
		.stack 100h				; ����256�ֽ������Ķ�ջ

;-----------------------------------------------------------
;	�������ݶ�                                             |
;-----------------------------------------------------------
		.data					; �������ݶ�
;MyVar	DB	?					; ����MyVar����   


;-----------------------------------------------------------
;	��������                                             |
;-----------------------------------------------------------
		.code						; Code segment definition
;
;���¿�ʼ�����û�ָ�����
;

;-----------------------------------------------------------
;�����򲿷�,��ȡ������״̬ȡ��������ʾ                     |
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
		
		JMP Start			;��תѭ��ִ��

		END Again			;ָʾ�������������
