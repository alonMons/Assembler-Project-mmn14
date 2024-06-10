; errors
mcr a
sub r1, r4
cmp K, 5
endmcr
.entry LIST 
.extern LIST
.extern W 
.extern W
.data 50000
.define sz = 10000
MAIN:	mov r3, LIST[sz]
LOOP: 	jmp W 
	prn #-5
	mov STR[5], STR[2] 
	sub r1, r4
	cmp K, #sz
	bne W
L1: 	inc L3 
.entry LOOP
	bne LOOP 
END: hlt
.entry ENTRY
1label: mov r2, r2
mov r2, 50
.define len = 4
STR: .string "abcdef" 
LIST: .data 6, -9, len 
K: .data 22 
.extern L3
.extern M K
; unprintabble
.string "AAA AAא"
