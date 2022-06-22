
 

	THUMB
	AREA |.text|, CODE, READONLY, ALIGN=4
     
	 
	 EXPORT		cnn1x1

	MACRO
	calc_1x1_x_line
	;r12 is kernel,r0 is line
	ldrsh     r3,[r12],#2 	
	
    ldrsh     r4,[r0,#0] 
	mla      r5,r3,r4,r5
	ldrsh     r4,[r0,#2]	
	mla      r6,r3,r4,r6 
	ldrsh     r4,[r0,#4]
	mla      r7,r3,r4,r7 
	ldrsh     r4,[r0,#6]
	mla      r8,r3,r4,r8
	ldrsh     r4,[r0,#8]
	mla      r9,r3,r4,r9
	ldrsh     r4,[r0,#10]
	mla      r10,r3,r4,r10
	ldrsh     r4,[r0,#12]
	mla      r11,r3,r4,r11
	MEND
	;void cnn1x1(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias);
cnn1x1 PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,-->lr
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	;sp[52+24+16] cb
	
	;sp[0]   
	;sp[4]  i of h
	;sp[8]    
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	
    str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	mul      r1,r2,r3
	
	lsl      r2,#1	 
	mul      r2,r2,r3  
	 
	 
; r1 i of imgsz ,r2 img adder	
next_1x1_x_wh	
	
 
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r5,[sp,#88]
	mov      r6,r5
	mov      r7,r5
	mov      r8,r5
	mov      r9,r5
	mov      r10,r5
	mov      r11,r5
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_1x1_x_c
    
;next line
	calc_1x1_x_line 
	 
		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1   ;i of c  
	bne		 next_1x1_x_c
	
	ldr      r0,[sp,#84]
	asr      r5,r0
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r5,[r0],#2
	strh     r6,[r0],#2
	strh     r7,[r0],#2
	strh     r8,[r0],#2
	strh     r9,[r0],#2
	strh     r10,[r0],#2
	strh     r11,[r0],#2
	str      r0,[sp,#80]
		 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#14
	str      r0,[sp,#16]
	
	sub      r1,#7 
	
	cmp      r1,#7
	bge      next_1x1_x_wh 
	
	cmp      r1,#0
	beq      start_1x1_x_next_h
	sub      r1,#7
	lsl      r1,#1
	add      r0,r1
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r1
	str      r7,[sp,#80] 
	
	mov      r1,#7 
	b        next_1x1_x_wh
start_1x1_x_next_h	     
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
		
	 
    MACRO
	calc_3x3_line
	;r12 is kernel,r0 is line
	ldrsh     r3,[r12],#2 	
	
    ldrsh     r4,[r0],#2	
	ldrsh     r5,[r0],#2
	ldrsh     r6,[r0],#2
	ldrsh     r7,[r0],#2
	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r4,[r0],#2
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	ldrsh     r3,[r12],#2 
	ldrsh     r5,[r0],#2
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11
	MEND
	 
  
	EXPORT		cnn3x3_outpad 

	;void cnn3x3_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias,int outpad);
cnn3x3_outpad PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,-->lr
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	;sp[52+24+16] outpad
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	
    str      lr,[sp,#36]
	
	ldr      r4,[sp,#92] ;outpad
	lsl      r4,#1
	str      r4,[sp,#92] ;outpad
	
	mov      r4,r3
	sub      r3,#2
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	lsl      r2,#1
	 
	mul      r3,r2,r4
	sub      r3,#12     
	sub      r3,r2
	sub      r3,r2
	str      r3,[sp,#8]
	
	sub      r2,#12
	;str      r2,[sp,#12]
	
	mov      r1,r2	;line adder 
	ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_3x3_outpad_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3_outpad_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
	
next_3x3_outpad_c
    
;next line
	calc_3x3_line
	add       r0,r1
	calc_3x3_line
	add       r0,r1
	calc_3x3_line
	 
		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1   ;i of c  
	bne		 next_3x3_outpad_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#6
	bge      next_3x3_outpad_w 
	
	cmp      r4,#2
	beq      start_3x3_outpad_next_h
	sub      r4,#6
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#6
	str      r4,[sp,#0]
	b        next_3x3_outpad_w
start_3x3_outpad_next_h	
	ldr      r7,[sp,#80] ;out
	ldr      r4,[sp,#92] ;outpad
	add      r7,r4
	str      r7,[sp,#80] 
  
	
	ldr      r0,[sp,#16]
	add      r0,#4
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3_outpad_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
  
		
		


 	EXPORT		pool3x3max_stride2_valid
 
	
	MACRO
	calc_3x3max_s2_valid_line
	;r0 is line 
	
    ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#4] 
	ldrsh     r6,[r0,#8] 
	ldrsh     r7,[r0,#12] 
	ldrsh     r12,[r0,#16]
	
	cmp       r4,r8
	it		  ge
	movge       r8,r4 
	cmp       r5,r8
	it		  ge
	movge       r8,r5 
	
	cmp       r5,r9
	it		  ge
	movge       r9,r5 
	cmp       r6,r9
	it		  ge
	movge       r9,r6
	
	cmp       r6,r10
	it		  ge
	movge       r10,r6 
	cmp       r7,r10
	it		  ge
	movge       r10,r7
	
	cmp       r7,r11
	it		  ge
	movge       r11,r7 
	cmp       r12,r11
	it		  ge
	movge       r11,r12
	
	ldrsh     r4,[r0,#2]
	ldrsh     r5,[r0,#6] 
	ldrsh     r6,[r0,#10] 
	ldrsh     r7,[r0,#14]
	
	cmp       r4,r8
	it		  ge
	movge       r8,r4  
	
	cmp       r5,r9
	it		  ge
	movge       r9,r5  
	
	cmp       r6,r10
	it		  ge
	movge       r10,r6  
	
	cmp       r7,r11
	it		  ge
	movge       r11,r7 
	 	
	MEND
	
	;void pool3x3max_stride2_valid( short *out , short *img,int w,int h);
pool3x3max_stride2_valid PROC
	;sp[0+24] out,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	 
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	 	
	sub      r3,#1
	lsr	     r3,#1
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#1
	lsr		 r2,#1
	str      r2,[sp,#32] ; out_w  
 
	       
; r1 line adder	
next_3x3max_s2_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3max_s2_v_w 
	 
	ldr      r0,[sp,#16]  ;img  ptr	 
	
	mov      r12,#0x10000
	mov      r8 ,#0
    sub      r8,r12
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
;next line
	calc_3x3max_s2_valid_line
	add       r0,r1 
	calc_3x3max_s2_valid_line
	add       r0,r1 
	calc_3x3max_s2_valid_line 
	
	
	ldr      r0,[sp,#24] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#24]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#16
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_3x3max_s2_v_w 
	
	cmp      r4,#0
	beq      start_3x3max_s2_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#24] ;out
	add      r7,r4
	str      r7,[sp,#24] 
	
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_3x3max_s2_v_w
start_3x3max_s2_v_next_h	

	ldr      r0,[sp,#28]
	add      r0,r1
	add      r0,r1 
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3max_s2_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
	EXPORT		pool3x3avg_stride2_valid
 
	
	MACRO
	calc_3x3avg_s2_valid_line
	;r0 is line 
	
  ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#4] 
	ldrsh     r6,[r0,#8] 
	ldrsh     r7,[r0,#12] 
	ldrsh     r12,[r0,#16]
	
	add       r8,r4 
	add       r8,r5
	
	add       r9,r5 
	add       r9,r6
	 
	add       r10,r6 
	add       r10,r7
	 
	add       r11,r7 
	add       r11,r12 
	
	
	ldrsh     r4,[r0,#2]
	ldrsh     r5,[r0,#6] 
	ldrsh     r6,[r0,#10] 
	ldrsh     r7,[r0,#14]
	
	add       r8,r4 	
	add       r9,r5 	 
	add       r10,r6  	 
	add       r11,r7   
	 	
	MEND
	
	;void pool3x3avg_stride2_valid( short *out , short *img,int w,int h);
pool3x3avg_stride2_valid PROC
	;sp[0+24] out,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	 
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	 	
	sub      r3,#1
	lsr	     r3,#1
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#1
	lsr		 r2,#1
	str      r2,[sp,#32] ; out_w  
 
	       
; r1 line adder	
next_3x3avg_s2_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3avg_s2_v_w 
	 
	ldr      r0,[sp,#16]  ;img  ptr	 
	 
	mov      r8 ,#0 
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
;next line
	calc_3x3avg_s2_valid_line
	add       r0,r1 
	calc_3x3avg_s2_valid_line
	add       r0,r1 
	calc_3x3avg_s2_valid_line 
	
	mov      r4,#9
	sdiv     r8,r8,r4
	sdiv     r9,r9,r4
	sdiv     r10,r10,r4
	sdiv     r11,r11,r4
	
	
	ldr      r0,[sp,#24] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#24]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#16
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_3x3avg_s2_v_w 
	
	cmp      r4,#0
	beq      start_3x3avg_s2_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#24] ;out
	add      r7,r4
	str      r7,[sp,#24] 
	
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_3x3avg_s2_v_w
start_3x3avg_s2_v_next_h	

	ldr      r0,[sp,#28]
	add      r0,r1
	add      r0,r1 
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3avg_s2_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP

	EXPORT		pool1x5avg_five_valid
	MACRO
	calc_1x5avg_five_valid_line
	;r0 is line 
	
    ldrsh     r4,[r0,#0]
	add    	  r7,r4
	
	ldrsh     r4,[r0,#2]
	add    	  r7,r4
	add    	  r8,r4
	
	ldrsh     r4,[r0,#4]
	add    	  r7,r4
	add    	  r8,r4
	add    	  r9,r4
	
	ldrsh     r4,[r0,#6]	
	add    	  r7,r4
	add    	  r8,r4
	add    	  r9,r4
	add    	  r10,r4
	
	ldrsh     r4,[r0,#8]
	add    	  r7,r4
	add    	  r8,r4
	add    	  r9,r4
	add    	  r10,r4
	add    	  r11,r4
	
	ldrsh     r4,[r0,#10]
	add    	  r8,r4
	add    	  r9,r4
	add    	  r10,r4
	add    	  r11,r4	 
	
	ldrsh     r4,[r0,#12] 
	add    	  r9,r4
	add    	  r10,r4
	add    	  r11,r4
	
	ldrsh     r4,[r0,#14]  
	add    	  r10,r4
	add    	  r11,r4
	
	ldrsh     r4,[r0,#16]  
	add    	  r11,r4
	 	
	MEND
	
	;void pool1x5avg_five_valid( short *out , short *img,int w,int h);
pool1x5avg_five_valid PROC
	;sp[0+24] out,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	 
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	 	 
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#4 
	str      r2,[sp,#32] ; out_w  
 
	       
; r1 line adder	
next_1x5avg_five_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_1x5avg_five_v_w 
	 
	ldr      r0,[sp,#16]  ;img  ptr	 
	 
	mov      r7 ,#0 
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
;next line
	calc_1x5avg_five_valid_line 
	
	mov      r4,#5
	sdiv     r7,r7,r4
	sdiv     r8,r8,r4
	sdiv     r9,r9,r4
	sdiv     r10,r10,r4
	sdiv     r11,r11,r4
	
	ldr      r0,[sp,#24] ;out
	strh     r7,[r0,#0]	
	strh     r8,[r0,#2]
	strh     r9,[r0,#4]
	strh     r10,[r0,#6]
	strh     r11,[r0,#8]
	add      r0,#10
	str      r0,[sp,#24]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#5
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#10
	str      r0,[sp,#16]
	
	cmp      r4,#5
	bge      next_1x5avg_five_v_w 
	
	cmp      r4,#0
	beq      start_1x5avg_five_v_next_h
	sub      r4,#5
	lsl      r4,#1
	
	ldr      r7,[sp,#24] ;out
	add      r7,r4
	str      r7,[sp,#24] 
	 
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#5
	str      r4,[sp,#0]
	b        next_1x5avg_five_v_w
start_1x5avg_five_v_next_h	

	ldr      r0,[sp,#28]
	add      r0,r1 
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_1x5avg_five_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
		
	EXPORT		pool3x3avg_valid
  MACRO
	calc_3x3avg_valid_line
	;r0 is line 
	
    ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#2] 
	ldrsh     r6,[r0,#4] 
	 
	
	add    r8,r4
	add    r8,r5
	add    r8,r6
	
	ldrsh     r4,[r0,#6]	
	add    r9,r4
	add    r9,r5
	add    r9,r6
	
	ldrsh     r5,[r0,#8]
	add    r10,r4
	add    r10,r5
	add    r10,r6
	
	ldrsh     r6,[r0,#10]
	add    r11,r4
	add    r11,r5
	add    r11,r6	 
	 	
	MEND
	
	;void pool3x3avg_valid( short *out , short *img,int w,int h);
pool3x3avg_valid PROC
	;sp[0+24] out,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	 
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	 	
	sub      r3,#2 
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#2 
	str      r2,[sp,#32] ; out_w  
 
	       
; r1 line adder	
next_3x3avg_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3avg_v_w 
	 
	ldr      r0,[sp,#16]  ;img  ptr	 
	 
	mov      r8 ,#0 
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
;next line
	calc_3x3avg_valid_line
	add       r0,r1 
	calc_3x3avg_valid_line
	add       r0,r1 
	calc_3x3avg_valid_line 
	
	mov      r4,#9
	sdiv     r8,r8,r4
	sdiv     r9,r9,r4
	sdiv     r10,r10,r4
	sdiv     r11,r11,r4
	
	ldr      r0,[sp,#24] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#24]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_3x3avg_v_w 
	
	cmp      r4,#0
	beq      start_3x3avg_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#24] ;out
	add      r7,r4
	str      r7,[sp,#24] 
	 
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_3x3avg_v_w
start_3x3avg_v_next_h	

	ldr      r0,[sp,#28]
	add      r0,r1 
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3avg_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
		
	EXPORT		pool3x3max_valid
 
	
	MACRO
	calc_3x3max_valid_line
	;r0 is line 
	
    ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#2] 
	ldrsh     r6,[r0,#4] 
	 
	
	cmp       r4,r8
	it		  ge
	movge       r8,r4 
	cmp       r5,r8
	it		  ge
	movge       r8,r5 
	cmp       r6,r8
	it		  ge
	movge       r8,r6 
	
	ldrsh     r4,[r0,#6] 
	cmp       r5,r9
	it		  ge
	movge       r9,r5 
	cmp       r6,r9
	it		  ge
	movge       r9,r6
	cmp       r4,r9
	it		  ge
	movge       r9,r4
	
	ldrsh     r5,[r0,#8]
	cmp       r6,r10
	it		  ge
	movge       r10,r6 
	cmp       r4,r10
	it		  ge
	movge       r10,r4
	cmp       r5,r10
	it		  ge
	movge       r10,r5
	
	ldrsh     r6,[r0,#10]
	cmp       r4,r11
	it		  ge
	movge       r11,r4 
	cmp       r5,r11
	it		  ge
	movge       r11,r5
	cmp       r6,r11
	it		  ge
	movge       r11,r6
	
	 
	 	
	MEND
	
	;void pool3x3max_valid( short *out , short *img,int w,int h);
pool3x3max_valid PROC
	;sp[0+24] out,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	 
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	 	
	sub      r3,#2 
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#2 
	str      r2,[sp,#32] ; out_w  
 
	       
; r1 line adder	
next_3x3max_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3max_v_w 
	 
	ldr      r0,[sp,#16]  ;img  ptr	 
	
	mov      r12,#0x10000
	mov      r8 ,#0
    sub      r8,r12
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
;next line
	calc_3x3max_valid_line
	add       r0,r1 
	calc_3x3max_valid_line
	add       r0,r1 
	calc_3x3max_valid_line 
	
	
	ldr      r0,[sp,#24] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#24]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_3x3max_v_w 
	
	cmp      r4,#0
	beq      start_3x3max_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#24] ;out
	add      r7,r4
	str      r7,[sp,#24] 
	 
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_3x3max_v_w
start_3x3max_v_next_h	

	ldr      r0,[sp,#28]
	add      r0,r1 
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3max_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
		
 	EXPORT		cnn3x3_stride2_valid

	MACRO
	cnn_3x3_s2_valid_line
	;r12 is kernel,r0 is line
		
	
    ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#4] 
	ldrsh     r6,[r0,#8] 
	ldrsh     r7,[r0,#12] 
	
	ldrsh     r3,[r12,#0]	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r4,[r0,#16]
	ldrsh     r3,[r12,#4]  
	
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	 
	
	ldrsh     r4,[r0,#2]
	ldrsh     r5,[r0,#6] 
	ldrsh     r6,[r0,#10] 
	ldrsh     r7,[r0,#14] 
	
	ldrsh     r3,[r12,#2]
	
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	
	MEND	
	;void cnn3x3_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
cnn3x3_stride2_valid PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	
	mov      r4,r3
	sub      r4,#2
	mul      r5,r1,r4    
	str      r5,[sp,#8]	;img_adder
	
	sub      r3,#1
	lsr	     r3,#1
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#1
	lsr		 r2,#1
	str      r2,[sp,#32] ; out_w 
	
	
	ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_3x3_s2_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x3_s2_v_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]  ;bias
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
	
next_3x3_s2_v_c
    
;next line
	cnn_3x3_s2_valid_line
	add       r0,r1
	add      r12,#6
	cnn_3x3_s2_valid_line
	add       r0,r1
	add      r12,#6
	cnn_3x3_s2_valid_line 
	add      r12,#6 
	   
	add      r0,r2 ;next img start addr 
	subs     lr,#1   ; i of c
	bne		 next_3x3_s2_v_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#16
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_3x3_s2_v_w 
	
	cmp      r4,#0
	beq      start_3x3_s2_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_3x3_s2_v_w
start_3x3_s2_v_next_h	
   
	
	ldr      r0,[sp,#28]
	add      r0,r1
	add      r0,r1
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_3x3_s2_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
	[ 0 > 1	
	EXPORT		cnn5x5_stride2_valid

	MACRO
	calc_5x5_s2_valid_line
	;r12 is kernel,r0 is line
		
	
    ldrsh     r4,[r0,#0]
	ldrsh     r5,[r0,#4] 
	ldrsh     r6,[r0,#8] 
	ldrsh     r7,[r0,#12] 
	
	ldrsh     r3,[r12,#0]	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r4,[r0,#16]
	ldrsh     r3,[r12,#4]  
	
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	
	ldrsh     r5,[r0,#20]
	ldrsh     r3,[r12,#8]
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11
	
	ldrsh     r4,[r0,#2]
	ldrsh     r5,[r0,#6] 
	ldrsh     r6,[r0,#10] 
	ldrsh     r7,[r0,#14] 
	
	ldrsh     r3,[r12,#2]
	
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r4,[r0,#18]
	ldrsh     r3,[r12,#6] 
	 
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11
	
	MEND
	
	
	;void cnn5x5_stride2_valid(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
cnn5x5_stride2_valid PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]   
	;sp[12]   
	;sp[16]  ptr of img
	;sp[20]   
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r1,[sp,#16]	;*img	
	
	mov      r1,r2	
	lsl      r1,#1  ;line adder 
	
	mov      r4,r3
	sub      r4,#4
	mul      r5,r1,r4    
	str      r5,[sp,#8]	;img_adder
	
	sub      r3,#4
	lsr	     r3,#1
	str      r3,[sp,#4]		; out_h
	
	
	sub      r2,#4
	lsr		 r2,#1
	str      r2,[sp,#32] ; out_w 
	
	
	ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_5x5_s2_v_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_5x5_s2_v_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]  ;bias
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
	
next_5x5_s2_v_c
    
;next line
	calc_5x5_s2_valid_line
	add       r0,r1
	add      r12,#10
	calc_5x5_s2_valid_line
	add       r0,r1
	add      r12,#10
	calc_5x5_s2_valid_line
	add       r0,r1
	add      r12,#10
	calc_5x5_s2_valid_line
	add       r0,r1
	add      r12,#10
	calc_5x5_s2_valid_line
	add      r12,#10
	   
	add      r0,r2 ;next img start addr 
	subs     lr,#1   ; i of c
	bne		 next_5x5_s2_v_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#16
	str      r0,[sp,#16]
	
	cmp      r4,#4
	bge      next_5x5_s2_v_w 
	
	cmp      r4,#0
	beq      start_5x5_s2_v_next_h
	sub      r4,#4
	lsl      r4,#1
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]	
	
	mov      r4,#4
	str      r4,[sp,#0]
	b        next_5x5_s2_v_w
start_5x5_s2_v_next_h	
   
	
	ldr      r0,[sp,#28]
	add      r0,r1
	add      r0,r1
	str      r0,[sp,#28]
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_5x5_s2_v_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		

	EXPORT		cnn5x5 

	MACRO
	calc_5x5_line
	;r12 is kernel,r0 is line
	ldrsh     r3,[r12],#2 	
	
    ldrsh     r4,[r0],#2	
	ldrsh     r5,[r0],#2
	ldrsh     r6,[r0],#2
	ldrsh     r7,[r0],#2
	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r4,[r0],#2
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	ldrsh     r3,[r12],#2 
	ldrsh     r5,[r0],#2
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r6,[r0],#2
	
	mla      r8,r3,r7,r8
	mla      r9,r3,r4,r9
	mla      r10,r3,r5,r10
	mla      r11,r3,r6,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r7,[r0],#2
	
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	MEND
	
	
	;void cnn5x5(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit ,int bias );
cnn5x5 PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	
	mov      r4,r3
	sub      r3,#4
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	lsl      r2,#1
	
	
	mul      r3,r2,r4
	sub      r3,#16    
    mov      r5,r2	
	mov      r6,#4
	mul      r5,r5,r6
	sub      r3,r5
	str      r3,[sp,#8]
	
	sub      r2,#16
	;str      r2,[sp,#12]
	
	mov      r1,r2	;line adder 
	ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_5x5_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_5x5_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
	
next_5x5_c
    
;next line
	calc_5x5_line
	add       r0,r1
	calc_5x5_line
	add       r0,r1
	calc_5x5_line
	add       r0,r1
	calc_5x5_line
	add       r0,r1
	calc_5x5_line
	 
		  
	add      r0,r2 ;next img start addr 
	subs     lr,#1   ; i of c
	bne		 next_5x5_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#8
	bge      next_5x5_w 
	
	cmp      r4,#4
	beq      start_5x5_next_h
	sub      r4,#8
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#8
	str      r4,[sp,#0]
	b        next_5x5_w
start_5x5_next_h	
   
	
	ldr      r0,[sp,#16]
	add      r0,#8
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_5x5_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
	]
	

	MACRO
	calc_1x7_line
	;r12 is kernel,r0 is line
	ldrsh     r3,[r12],#2 	
	
    ldrsh     r4,[r0],#2	
	ldrsh     r5,[r0],#2
	ldrsh     r6,[r0],#2
	ldrsh     r7,[r0],#2
	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r4,[r0],#2
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	ldrsh     r3,[r12],#2 
	ldrsh     r5,[r0],#2
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r6,[r0],#2
	
	mla      r8,r3,r7,r8
	mla      r9,r3,r4,r9
	mla      r10,r3,r5,r10
	mla      r11,r3,r6,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r7,[r0],#2
	
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r4,[r0],#2
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	ldrsh     r3,[r12],#2 
	ldrsh     r5,[r0],#2
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11 
	
	
	MEND
	
	[ 0 > 1
	EXPORT		cnn7x7 
	;void cnn7x7(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn7x7 PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	
	mov      r4,r3
	sub      r3,#6
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	lsl      r2,#1	
	
	mul      r3,r2,r4
	sub      r3,#20    
    mov      r5,r2	
	mov      r6,#6
	mul      r5,r5,r6
	sub      r3,r5
	str      r3,[sp,#8]
	
	sub      r2,#20
	;str      r2,[sp,#12]
	
	mov      r1,r2	;line adder 
	ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_7x7_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_7x7_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
	
next_7x7_c
    
;next line
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	add       r0,r1
	calc_1x7_line
	 
		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_7x7_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#10
	bge      next_7x7_w 
	
	cmp      r4,#6
	beq      start_7x7_next_h
	sub      r4,#10
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#10
	str      r4,[sp,#0]
	b        next_7x7_w
start_7x7_next_h	 
	
	ldr      r0,[sp,#16]
	add      r0,#12
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_7x7_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
	]
	
	EXPORT		cnn1x7_outpad 
	;void cnn1x7_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias,int outpad  );
cnn1x7_outpad PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	;sp[52+24+16] outpad
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	ldr      r7,[sp,#92] ;outpad
	lsl      r7,#1
	str      r7,[sp,#92]
	
	lsl      r2,#1	
	
	mul      r2,r2,r3
	sub      r2,#20     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_1x7_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_1x7_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_1x7_c
    
;next line
	calc_1x7_line	 
		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_1x7_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#10
	bge      next_1x7_w 
	
	cmp      r4,#6
	beq      start_1x7_next_h
	sub      r4,#10
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#10
	str      r4,[sp,#0]
	b        next_1x7_w
start_1x7_next_h	 
	ldr      r7,[sp,#80] ;out
	ldr      r4,[sp,#92] ;outpad
	add      r7,r4
	str      r7,[sp,#80] 
	
	ldr      r0,[sp,#16]
	add      r0,#12
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_1x7_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
			 
			 
			 
			 
			 
			 
			 
	MACRO
	calc_1x5_line
	;r12 is kernel,r0 is line
	ldrsh     r3,[r12],#2 	
	
    ldrsh     r4,[r0],#2	
	ldrsh     r5,[r0],#2
	ldrsh     r6,[r0],#2
	ldrsh     r7,[r0],#2
	 
	 
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r4,[r0],#2
	
	mla      r8,r3,r5,r8
	mla      r9,r3,r6,r9
	mla      r10,r3,r7,r10
	mla      r11,r3,r4,r11	
	
	ldrsh     r3,[r12],#2 
	ldrsh     r5,[r0],#2
	
	mla      r8,r3,r6,r8
	mla      r9,r3,r7,r9
	mla      r10,r3,r4,r10
	mla      r11,r3,r5,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r6,[r0],#2
	
	mla      r8,r3,r7,r8
	mla      r9,r3,r4,r9
	mla      r10,r3,r5,r10
	mla      r11,r3,r6,r11
	
	ldrsh     r3,[r12],#2 
	ldrsh     r7,[r0],#2
	
	mla      r8,r3,r4,r8
	mla      r9,r3,r5,r9
	mla      r10,r3,r6,r10
	mla      r11,r3,r7,r11 
	
	MEND
	EXPORT		cnn1x5_outpad 
	;void cnn1x5_outpad(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias,int outpad  );
cnn1x5_outpad PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	;sp[52+24+16] outpad
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	
    ldr      r7,[sp,#92] ;outpad
	lsl      r7,#1
	str      r7,[sp,#92]
	
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	
	mul      r2,r2,r3
	sub      r2,#16     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_1x5_pad_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_1x5_pad_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r8,[sp,#88]
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_1x5_pad_c
    
;next line
	calc_1x5_line	 
		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_1x5_pad_c
	
	ldr      r0,[sp,#84]
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r8,[r0,#0]
	strh     r9,[r0,#2]
	strh     r10,[r0,#4]
	strh     r11,[r0,#6]
	add      r0,#8
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#4
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#8
	str      r0,[sp,#16]
	
	cmp      r4,#8
	bge      next_1x5_pad_w 
	
	cmp      r4,#4
	beq      start_1x5_pad_next_h
	sub      r4,#8
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#8
	str      r4,[sp,#0]
	b        next_1x5_pad_w
start_1x5_pad_next_h	 
	
	ldr      r7,[sp,#80] ;out
	ldr      r4,[sp,#92] ; outpad
	add      r7,r4
	str      r7,[sp,#80] 
	
	
	ldr      r0,[sp,#16]
	add      r0,#8
	str      r0,[sp,#16]
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#0
	bne      next_1x5_pad_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
	 
	MACRO
	calc_nx1_six_line
	;r12 is kernel,r0 is line 
	ldrsh     r3,[r12],#2 	
	 	
	ldrsh     r5,[r0,#0] 
	mla      r6,r3,r5,r6
	ldrsh     r5,[r0,#2]
	mla      r7,r3,r5,r7
	ldrsh     r5,[r0,#4]	
	mla      r8,r3,r5,r8
	ldrsh     r5,[r0,#6]
	mla      r9,r3,r5,r9
	ldrsh     r5,[r0,#8]
	mla      r10,r3,r5,r10
	ldrsh     r5,[r0,#10]
	mla      r11,r3,r5,r11 
	
	MEND
	
	EXPORT		cnn5x1_six 
	;void cnn5x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn5x1_six PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	mov      r1,r2
	
	sub      r3,#4
	mul      r2,r2,r3     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_5x1_six_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_5x1_six_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r6,[sp,#88]
	mov      r7,r6
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_5x1_six_c
   
	calc_nx1_six_line	
	add       r0,r1
    calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	 
          		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_5x1_six_c
	
	ldr      r0,[sp,#84]
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r6,[r0,#0]
	strh     r7,[r0,#2]
	strh     r8,[r0,#4]
	strh     r9,[r0,#6]
	strh     r10,[r0,#8]
	strh     r11,[r0,#10]
	add      r0,#12
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#6
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#12
	str      r0,[sp,#16]
	
	cmp      r4,#6
	bge      next_5x1_six_w 
	
	cmp      r4,#0
	beq      start_5x1_six_next_h
	sub      r4,#6
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#6
	str      r4,[sp,#0]
	b        next_5x1_six_w
start_5x1_six_next_h	 
	 
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#5
	bge      next_5x1_six_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
	EXPORT		cnn7x1_six 
	;void cnn7x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn7x1_six PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	mov      r1,r2
	
	sub      r3,#6
	mul      r2,r2,r3     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_7x1_six_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_7x1_six_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r6,[sp,#88]
	mov      r7,r6
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_7x1_six_c
   
	calc_nx1_six_line	
	add       r0,r1
    calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line 
          		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_7x1_six_c
	
	ldr      r0,[sp,#84]
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r6,[r0,#0]
	strh     r7,[r0,#2]
	strh     r8,[r0,#4]
	strh     r9,[r0,#6]
	strh     r10,[r0,#8]
	strh     r11,[r0,#10]
	add      r0,#12
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#6
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#12
	str      r0,[sp,#16]
	
	cmp      r4,#6
	bge      next_7x1_six_w 
	
	cmp      r4,#0
	beq      start_7x1_six_next_h
	sub      r4,#6
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#6
	str      r4,[sp,#0]
	b        next_7x1_six_w
start_7x1_six_next_h	 
	 
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#7
	bge      next_7x1_six_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
	 
	 MACRO
	calc_nx1_seven_line
	;r12 is kernel,r0 is line 
	ldrsh     r3,[r12],#2 	
	 	
	ldrsh     r4,[r0,#0] 
	mla      r5,r3,r4,r5
	ldrsh     r4,[r0,#2] 
	mla      r6,r3,r4,r6
	ldrsh     r4,[r0,#4] 
	mla      r7,r3,r4,r7
	ldrsh     r4,[r0,#6] 	
	mla      r8,r3,r4,r8
	ldrsh     r4,[r0,#8] 
	mla      r9,r3,r4,r9
	ldrsh     r4,[r0,#10] 
	mla      r10,r3,r4,r10
	ldrsh     r4,[r0,#12] 
	mla      r11,r3,r4,r11 
	
	MEND
	EXPORT		cnn7x1_seven 
	;void cnn7x1_seven(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn7x1_seven PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	mov      r1,r2
	
	sub      r3,#6
	mul      r2,r2,r3     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_7x1_seven_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_7x1_seven_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r5,[sp,#88]
	mov      r6,r5
	mov      r7,r6
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_7x1_seven_c
   
	calc_nx1_seven_line	
	add       r0,r1
    calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line 
          		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_7x1_seven_c
	
	ldr      r0,[sp,#84]
	asr      r5,r0
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out
    strh     r5,[r0,#0]	
	strh     r6,[r0,#2]
	strh     r7,[r0,#4]
	strh     r8,[r0,#6]
	strh     r9,[r0,#8]
	strh     r10,[r0,#10]
	strh     r11,[r0,#12]
	add      r0,#14
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#7
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#14
	str      r0,[sp,#16]
	
	cmp      r4,#7
	bge      next_7x1_seven_w 
	
	cmp      r4,#0
	beq      start_7x1_seven_next_h
	sub      r4,#7
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#7
	str      r4,[sp,#0]
	b        next_7x1_seven_w
start_7x1_seven_next_h	 
	 
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#7
	bge      next_7x1_seven_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP		


	 
	EXPORT		cnn3x1_six 
	;void cnn3x1_six(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn3x1_six PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	mov      r1,r2
	
	sub      r3,#2
	mul      r2,r2,r3     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_3x1_six_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x1_six_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r6,[sp,#88]
	mov      r7,r6
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_3x1_six_c
   
	calc_nx1_six_line	
	add       r0,r1
    calc_nx1_six_line	
	add       r0,r1
	calc_nx1_six_line	 
          		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_3x1_six_c
	
	ldr      r0,[sp,#84]
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out	
	strh     r6,[r0,#0]
	strh     r7,[r0,#2]
	strh     r8,[r0,#4]
	strh     r9,[r0,#6]
	strh     r10,[r0,#8]
	strh     r11,[r0,#10]
	add      r0,#12
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#6
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#12
	str      r0,[sp,#16]
	
	cmp      r4,#6
	bge      next_3x1_six_w 
	
	cmp      r4,#0
	beq      start_3x1_six_next_h
	sub      r4,#6
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#6
	str      r4,[sp,#0]
	b        next_3x1_six_w
start_3x1_six_next_h	 
	 
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#3
	bge      next_3x1_six_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP
		
	 
	  
	EXPORT		cnn3x1_seven 
	;void cnn3x1_seven(short *kernels,short *img,int w,int h,int c,short *out,int shiftbit,int bias  );
cnn3x1_seven PROC
	;sp[0+24] kernels,
	;sp[4+24] img
	;sp[8+24] w;
	;sp[12+24] h,
	
	;sp[52+24+0] c
	;sp[52+24+4]  out
	;sp[52+24+8]  shiftbit 
	;sp[52+24+12] bias
	
	;sp[0]  i of w
	;sp[4]  i of h
	;sp[8]  next img adder 
	;sp[12]  next line adder
	;sp[16]  ptr of img
	;sp[20]  i of c
	
	push     {r0-r12}
	sub      sp,#24
	 
	str      lr,[sp,#36]
	 
	str      r3,[sp,#4]
	str      r1,[sp,#16]
	
	
	lsl      r2,#1	
	mov      r1,r2
	
	sub      r3,#2
	mul      r2,r2,r3     
	
	;sub      r2,#20
	;str      r2,[sp,#12]
	
	;mov      r1,r2	;line adder 
	;ldr      r2,[sp,#8] ; img adder 
; r1 line adder,r2 img adder	
next_3x1_seven_h	
	ldr      r4,[sp,#32]
	str      r4,[sp,#0]
next_3x1_seven_w
	ldr      lr,[sp,#76]  ;reset c
	
	ldr      r5,[sp,#88]
	mov      r6,r5
	mov      r7,r6
	mov      r8,r7
	mov      r9,r8
	mov      r10,r8
	mov      r11,r8
	 
	ldr      r0,[sp,#16]  ;img  ptr	
	ldr      r12,[sp,#24]  ;kernel
	
next_3x1_seven_c
   
	calc_nx1_seven_line	
	add       r0,r1
    calc_nx1_seven_line	
	add       r0,r1
	calc_nx1_seven_line	 
          		  
	add      r0,r2 ;next img start addr 
	subs      lr,#1        ; i of c
	bne		 next_3x1_seven_c
	
	ldr      r0,[sp,#84]
	asr      r5,r0
	asr      r6,r0
	asr      r7,r0
	asr      r8,r0
	asr      r9,r0
	asr      r10,r0
	asr      r11,r0
	
	ldr      r0,[sp,#80] ;out
    strh     r5,[r0,#0]	
	strh     r6,[r0,#2]
	strh     r7,[r0,#4]
	strh     r8,[r0,#6]
	strh     r9,[r0,#8]
	strh     r10,[r0,#10]
	strh     r11,[r0,#12]
	add      r0,#14
	str      r0,[sp,#80]
	
	
	ldr      r4,[sp,#0] ; i of w
	sub      r4,#7
	str      r4,[sp,#0]
	 
	
	ldr      r0,[sp,#16] ;ptr of  img 
	add      r0,#14
	str      r0,[sp,#16]
	
	cmp      r4,#7
	bge      next_3x1_seven_w 
	
	cmp      r4,#0
	beq      start_3x1_seven_next_h
	sub      r4,#7
	lsl      r4,#1
	add      r0,r4
	str      r0,[sp,#16]
	
	ldr      r7,[sp,#80] ;out
	add      r7,r4
	str      r7,[sp,#80] 
	
	mov      r4,#7
	str      r4,[sp,#0]
	b        next_3x1_seven_w
start_3x1_seven_next_h	 
	 
	ldr      r4,[sp,#4]   ;i of h
	sub      r4,#1
	str      r4,[sp,#4]  
	cmp      r4,#3
	bge      next_3x1_seven_h
	
	 
	add      sp,#24
	pop      {r0-r12}
	mov      lr,r3
	BX       lr
	ENDP	
		
	END	  ;end of file 