;********************************************************************
; game23.asm
; Implementation of Game of 23
;********************************************************************

.MODEL SMALL, NOLANGUAGE  ; small memory model, no  automatic prologue or epilogue
 
LOCALS @@   ; local labels begin with @@
; JUMPS       ; Tasm quirk

; drawing symbols
TopLeft     EQU 201
TopRight    EQU 187
BottomLeft  EQU 200
BottomRight EQU 188
HSymbol     EQU 205
VSymbol     EQU 186


NRows EQU 4
NCols EQU 6

PieceWidth  EQU 4
PieceHeight EQU 3
BoardWidth  EQU (PieceWidth + 1)*NCols + 3
BoardHeight EQU PieceHeight*NRows + 2


MaxNum EQU  NRows*NCols-1

VideoBuf        EQU 0B800h
ScreenWidth     EQU 80
ScreenHeight    EQU 25

ESCAPE EQU 1Bh   ; quit key

; Extended ASCII keys
LEFT 	EQU 4Bh
RIGHT 	EQU 4Dh
UP		EQU 48h
DOWN	EQU 50h

;******************************************************
; structure for holding piece screen coordinate 
;
;*******************************************************
Coord STRUC
    X dw ?
    Y dw ?
Coord ENDS


;***************** MACROS ******************************
; macro for string output to the screen
; uses DOS interrupt 21h, function 09
;
;*******************************************************
OutMsg MACRO Msg
    mov ah, 9          ; function number
    mov dx, OFFSET Msg ; offset of message
    int 21h
ENDM



;********************************************************

.DATA           ; data segment
 WelcomeMsg     db "Welcome to the Game of 23", 0Ah, 0Dh, 0Ah, 0Dh, '$'
 Menu           db "1. Use arrow keys to move the pieces", 0Ah, 0Dh
                db "2. ESC to quit the game", 0Ah, 0Dh
                db  0Ah, 0Dh
                db  "Press any key to start the game", 0Ah, 0Dh, '$'
 WonMsg         db  "  Congratulations! You did it!!!", 0Ah, 0Dh, '$'
 
; drawing colors 
Background db 1    
Foreground db 14   

NRAND   dw 0 ; to hold generated random numbers

; parameters for the Lehmer random number generator
; X(n +1) = A*X(n) mod M ( see Wikipedia)
A       dw 5       ; Primitive root of M27      
M       dw  44497  ; 27th Mersenne's prime   

 
.DATA? ; unitialized data segment for the game board
Numbers db  NRows*NCols DUP(-1)
Pos     Coord NRows*NCols DUP(<>)

EROW   dw ?  ; Empty piece row and column 
ECOL   dw ?  ; 


.STACK 100h                             ; stack segment


.CODE                   ; code segment
.386                    ; allow to use some 386 machine instructions like movzx

main:                   ; program's entry point

    mov ax, @data      ; initialize data segment
    mov ds, ax         ;

    mov ax, 0003h      ; set text mode 3   
    int 10h            ; 
 
     OutMsg WelcomeMsg  ; display the user menu
     OutMsg Menu
     call getkey

    mov al, 1Eh        
    call clearscreen    ; 
   
    call hidecursor
    
    mov Background, 1  ; blue
    mov Foreground, 14  ; yellow

    call Populate
    ;call WinPopulate   ; for testing the solved puzzle


    push 5 25         ; game board position on the screen
    call DrawBoard

    call GetEmptyPieceRowCol
  

 Game_loop: 

    call IsSolved
    cmp ax, 1
    je @solved

    call getkey  
    cmp al, ESCAPE   ; quit?      
    je @done
    cmp al, 0        ; check if extended key
    je @extended_keys
    jmp Game_loop

@extended_keys:
    cmp ah, LEFT
    je @left
    cmp ah, RIGHT
    je @right
    cmp ah, UP
    je @up
    cmp ah, DOWN
    je @down


@left:
    call MoveLeft 
    jmp Game_loop
@right:
    call MoveRight 
    jmp Game_loop
@up:
    call MoveUp 
    jmp Game_loop
@down:
    call MoveDown 
    jmp Game_loop

@solved:
    OutMsg WonMsg
    call getkey
@done:
    
mov ax,4C00h       ; exit program
int 21h            ;


;********************************************************
; NAME: getkey;
; DESCR: Reads a symbol from the keyboard buffer,
;  	 waits for a keystroke if the buffer is empty
; RETURNS: AL = the symbols ASCII code or 0
; 	   if AL = 0 AH contains an extended ASCII code
;********************************************************
getkey PROC
	xor ah, ah
	int 16h
	ret
getkey ENDP 


;************************************************************
; clearscreen - clears the screen
; PARAMS:
;  AL - attribute to use (Background << 4) | Foreground 
;RETURNS:
;   
;**************************************************************
clearscreen PROC
        mov bh, al
        mov ah, 06h       ; BIOS function #
        mov al, 0         ; scroll up the whole window
        xor cx, cx
        mov dh, ScreenHeight
        mov dl, ScreenWidth
        dec dh            ; adjust to screen coordinates (row, column) 
        dec dl            ;  
        int 10h
        ret
clearscreen ENDP


hidecursor PROC       
        mov ah, 02h
        mov bh, 0       ; currant video page is always assumed 0     
        mov dh, ScreenHeight
        mov dl, 0
        int 10h
        ret
hidecursor ENDP


;**************************************************************************
; NAME:     putchar
; DESCR:    Writes a character to video memory
;           using the set Background and Foreground colors
; PARAMS: 	x, y - position in screen coordinates, c - character to put 
; RETURNS: 
;**************************************************************************
putchar PROC 
    ARG x:WORD, y:WORD, c:BYTE

    push bp
    mov bp, sp

    push es si 
    
    mov ax, VideoBuf        ; calculate position in the video buffer
    mov es, ax      
    mov ax, ScreenWidth
    xor dx, dx
    mov bx, y
    mul bx
    add ax, x
    shl ax, 1
    mov si, ax
    mov al, c    
    mov byte ptr es:[si], al

    mov al, Background           ; set the symbol attributes 
    shl al, 4                    ; 
    or al, Foreground            ; 
    inc si                       ; point to the attribute byte
    mov byte ptr es:[si], al        

    pop si es

    pop bp

   ret 6
putchar ENDP



;**************************************************************************
; NAME:     seed
; DESCR:    seeds the random number generator using the system timer
; PARAMS: 	
;          
; RETURNS:  
;**************************************************************************
seed PROC

    xor ax, ax
    int 1AH      ; CX:DX now hold number of clock ticks since midnight  
    mov NRAND, dx
    ret

seed ENDP

;**************************************************************************
; NAME:     rand
; DESCR:    generates random numbers using Lehmer random number generator:
;           X(n +1) = A*X(n) mod M
; PARAMS: 	
;          
; RETURNS:  
;**************************************************************************
rand PROC 
    mov ax, NRAND
    mul A    
    div M
    mov NRAND, dx
    ret 
rand ENDP


;**************************************************************************
; NAME:     Populate
; DESCR:    populated the array Numbers with random numbers betwee 0 and MaxNum 
;           
; PARAMS: 	
;          
; RETURNS:  
;**************************************************************************
Populate PROC
LOCAL c:WORD, z:WORD = LocalSpace

    enter LocalSpace, 0

    mov c, 0  ; counter for added numbers
    mov z, 0  ; flag for zero to represent the empty cell
    
    lea si, Numbers

    call seed


@@L1:
    cmp c, MaxNum + 1
    je @@done    ; all numbers initialized

    call rand       
    mov ax, NRAND
    xor dx, dx
    mov bx,  MaxNum + 1  ; modulo division to gen. numbers between 0 and MaxNum
    div bx
    mov ax, dx      ; remainder in dx
    mov cx, c   
    call CanAdd    
    jc @@L1
    inc c
    mov byte ptr [si], al
    inc si
    cmp al, 0
    jne @@not_zero
    mov z, 1
 @@not_zero:
     jmp @@L1  

@@done:
    cmp z, 0
    jne @@exit              ; zero have been added
    mov byte ptr Numbers[MaxNum], 0  ; otherwise set the last element to zero

@@exit:

    leave    
    ret
Populate ENDP


;**************************************************************************
; NAME: CanAdd     
;
; DESCR: Determines if the given number can be added to Numbers 
;        in order to avoid duplicates   
; PARAMS: AL - number to check, CX- number of array entries to check against
;          
; RETURNS:  Carry flag is set if the number cannot be added
;**************************************************************************
CanAdd PROC

    jcxz @@exit       ; allow to add if cx == 0   
    mov bx, ds
    mov es, bx
    lea di, Numbers  ; es:di -> Numbers
    
    repne scasb 
    je @@set_carry       ; set carry if found 
    jmp @@exit             
@@set_carry:
    stc
    ret
@@exit:
    clc    
    ret

CanAdd ENDP


;**************************************************************************
; NAME:     HLine
; DESCR:    Draws a horizontal line 
; PARAMS: 	x, y - position, sym - symbol to use, cnt - count, 
;          
; RETURNS:  
;**************************************************************************
HLine PROC
    ARG x:WORD, y:WORD, sym:BYTE, cnt:BYTE

    push bp
    mov bp, sp

    push cx

    movzx cx, cnt

 @@nxt:
    movzx ax, sym
    push ax y x
    call putchar
    inc x
    loop @@nxt
    
    pop cx
    pop bp
    ret 8
HLine ENDP


;**************************************************************************
; NAME:     VLine
; DESCR:    Draws a verical line 
; PARAMS: 	x, y - position, sym - symbol to use, cnt - count, 
;          
; RETURNS:  
;**************************************************************************
VLine PROC
    ARG x:WORD, y:WORD, sym:BYTE, cnt:BYTE

    push bp
    mov bp, sp
    
    push cx
    movzx cx, cnt

 @@nxt:
    movzx ax, sym
    push ax y x
    call putchar
    inc y
    loop @@nxt
    
    pop cx
    pop bp
    ret 8
VLine ENDP


;**************************************************************************
; NAME:     Box
; DESCR:    Draws a box
; PARAMS: 	top, left - position of the upper-left corner
;           w- width, h- height
;          
; RETURNS:  
;**************************************************************************
Box PROC
    ARG x:WORD, y:WORD, w: WORD, h:WORD

    push bp
    mov bp, sp

    push cx

        ; draw corners
        push TopLeft y x
        call putchar

        dec w
        dec h

        mov ax, x
        add ax, w 
        push TopRight y ax
        call putchar

        mov ax, y
        add ax, h
        push BottomLeft ax x
        call putchar
    
        mov ax, x
        add ax, w
        mov cx, y
        add cx, h 
        push BottomRight cx ax
        call putchar

        ; horizontal lines
        mov ax, w
        dec ax
        mov cx, x
        inc cx
        push ax HSymbol y cx 
        call HLine

        mov ax, w
        dec ax
        mov cx, x
        inc cx
        mov bx, y
        add bx, h
        push ax HSymbol bx cx 
        call HLine

        ; vertical lines
        mov ax, h
        dec ax
        mov cx, y
        inc cx
        push ax VSymbol cx x 
        call VLine

        mov ax, h
        dec ax
        mov cx, x
        add cx, w
        mov bx, y
        inc bx
        push ax VSymbol bx cx 
        call VLine

    pop cx
    pop bp
    ret 8
Box ENDP



;**************************************************************************
; NAME:     SetPieceValue
; DESCR:    Sets the value of the game piece at x, y
; PARAMS: 	x, y - position, v - value to set
; RETURNS:  
;**************************************************************************
SetPieceValue PROC
    ARG x:WORD, y:WORD, n:BYTE
    LOCAL v1:BYTE, v2:BYTE=LocalSpace

    enter LocalSpace, 0

    cmp v, 0
    jne @@not_zero  
    mov v1, 20h ; space 
    mov v2, 20h ;

    jmp @@print   

@@not_zero:
       movzx ax, v 
       mov bl, 10
       div bl
                        ; al - quotient , ah -remainder
       cmp al, 0
       jne @@not_zero1
       mov v1, 20h
       jmp @@second_digit

@@not_zero1:
       add al, 30h
       mov v1, al

 @@second_digit:      
       add ah, 30h
       mov v2, ah 

 @@print:
       inc x
       inc y
       movzx ax, v1 
       push ax y x
       call putchar
    
       inc x
       movzx ax, v2
       push ax y x
       call putchar 
       
    leave
    ret 6
SetPieceValue ENDP


;**************************************************************************
; NAME:     SetPieceValueByIndex
; DESCR:    Sets the value of the game piece using its index in the Numbers array 
; PARAMS: 	i - index
; RETURNS:  
;**************************************************************************
SetPieceValueByIndex PROC
    ARG i:WORD

    enter 0, 0

    ; get value  Numbers[i]
    lea si, Numbers
    add si, i
    mov al, byte ptr [si]
    push ax 

    ; calculate position in the Pos array
    mov ax, i
    mov bl, SIZE Coord
    mul bl
    lea si, Pos
    add si, ax

    pop ax
    push ax
    mov ax, [si].Y
    push ax
    mov ax, [si].X
    push ax
    call SetPieceValue
       
    leave
    ret 2
SetPieceValueByIndex ENDP




;**************************************************************************
; NAME:     DrawPiece
; DESCR:    Draws the game piece at x, y
; PARAMS: 	x, y - position, v - the piece's value
; RETURNS:  
;**************************************************************************
DrawPiece PROC
    ARG x:WORD, y:WORD, v:BYTE

       push bp
       mov bp, sp 

       push PieceHeight PieceWidth y x
       call Box       

        movzx ax, v
        push ax y x
        call SetPieceValue

          
       pop bp
       ret 6
DrawPiece ENDP



;**************************************************************************
; NAME:     DrawPieces
; DESCR:    Draws the game pieces starting at x, y
; PARAMS: 	x, y
; RETURNS:  
;**************************************************************************
DrawPieces PROC
     ARG x:WORD, y:WORD
     push bp
     mov bp, sp

     push cx
     mov cx, NRows  

      lea si, Numbers 
      lea di, Pos ; coordinates of the pieces

@@L1:
     push cx
     push x  
     mov cx, NCols
@@L2:
        
        mov ax, x                   ; save coordinates of the pieces
        mov  [di].X, ax             ;
        mov ax, y                   ;
        mov  [di].Y, ax             ; 

        mov al, byte ptr [si]       
        push ax y x
        call DrawPiece 
        add x, PieceWidth +1 
        inc si
        add di, SIZE Coord
    loop @@L2

    pop x
    pop cx
    add y, PieceHeight
    loop @@L1


    pop cx
    pop bp
    ret 4
DrawPieces ENDP


;**************************************************************************
; NAME:     DrawBoard
; DESCR:    Draws the Game board
; PARAMS: 	x , y  the board position in the screen coordinates
; RETURNS:  
;**************************************************************************
DrawBoard PROC
    
    push bp
    mov bp, sp
  
    push BoardHeight BoardWidth y x
    call Box
    add x,2
    inc y

    push y x
    call DrawPieces

    pop bp
    ret 4  

 DrawBoard ENDP


;**************************************************************************
; NAME:     GetEmptyPieceRowCol
; DESCR:    finds the empty cell Row and Col that it occupies in the game grid 
; PARAMS: 	
;
; RETURNS:  Sets the global variables EROW and ECOL
;**************************************************************************
GetEmptyPieceRowCol PROC
    
    mov cx, NCols*NRows
    xor si, si

 @@L1:
    cmp byte ptr Numbers[si], 0 
    je @@done


    inc si
    loop @@L1

@@done:
    mov ax, si
    mov bl, NCols
    div bl
    
    movzx bx, ah
    mov ECOL, bx
    movzx bx, al
    mov EROW, bx

    ret 
GetEmptyPieceRowCol ENDP




;**************************************************************************
; NAME:     Swap
; DESCR:    Swaps values of the pieces with indeces i and j
; PARAMS: 	
;
; RETURNS:  
;**************************************************************************
Swap PROC
    ARG i:WORD, j:WORD
    enter 0, 0
    
    lea si, Numbers
    lea di, Numbers

    add si, i
    add di, j

    mov al, byte ptr [si]
    mov ah, byte ptr [di]

    mov  byte ptr [si], ah
    mov  byte ptr [di], al

    ;reflect the changes in the game board
    push i
    call SetPieceValueByIndex

    push j
    call SetPieceValueByIndex

    
    leave
    ret 4 
Swap ENDP


;**************************************************************************
; NAME:     MoveLeft
; DESCR:    Part of the keboard navigation logic 
; PARAMS: 	
;
; RETURNS:  
;**************************************************************************
MoveLeft PROC
   
    ; Can move?
    cmp ECOL, NCols - 1
    jae @@exit

    ; calculate position of the Empty Cell in Numbers

    push EROW ECOL
    call GetPosition

    push ax
    inc ax
    push ax
    call Swap

    inc ECOL
     
 @@exit:  
    ret  
MoveLeft ENDP

;**************************************************************************
; NAME:     MoveRight
; DESCR:    Part of the keboard navigation logic 
; PARAMS: 	
;
; RETURNS:  
;**************************************************************************
MoveRight PROC
   
    ; Can move?
    cmp ECOL, 0
    je @@exit

    ; calculate position of the Empty Cell in Numbers

    push EROW ECOL
    call GetPosition
    
    push ax
    dec ax
    push ax
    call Swap

    dec ECOL
     
 @@exit:  
    ret  
MoveRight ENDP


;**************************************************************************
; NAME:     MoveUp
; DESCR:    Part of the keboard navigation logic 
; PARAMS: 	
;
; RETURNS:  
;**************************************************************************
MoveUp PROC
   
    ; Can move?
    cmp EROW, NRows - 1
    je @@exit

    ; calculate position of the Empty Cell in Numbers

    push EROW ECOL
    call GetPosition
    
    push ax
    add ax, NCols 
    push ax
    call Swap

    inc EROW
     
 @@exit:  
    ret  
MoveUp ENDP


;**************************************************************************
; NAME:     MoveDown
; DESCR:    Part of the keboard navigation logic 
; PARAMS: 	
;
; RETURNS:  
;**************************************************************************
MoveDown PROC
   
    ; Can move?
    cmp EROW, 0
    jbe @@exit

    ; calculate position of the Empty Cell in Numbers

    push EROW ECOL
    call GetPosition
    
    push ax
    sub ax, NCols 
    push ax
    call Swap

    dec EROW
     
 @@exit:  
    ret  
MoveDown ENDP


;**************************************************************************
; NAME:     GetPosition
; DESCR:    Calculates position in array Numbers give row and column
; PARAMS: 	col - column, row - row
;
; RETURNS:  AX - index of the piece in Numbers
;**************************************************************************
GetPosition PROC
    ARG col:Word, row:WORD

    enter 0, 0

    mov ax, row
    mov bl, NCols
    mul bl
    add ax, col

    leave
    ret 4
GetPosition ENDP


;**************************************************************************
; NAME:     IsSolved 
; DESCR:    Checks if the user solves the puzzle
;           The winning condition: numbers must be aligned in ascending order
;           (1, 2, 3, ...... 23, 0)    
; PARAMS: 	
;
; RETURNS:  AX - 0 (no), 1(yes)
;**************************************************************************
IsSolved PROC
    
    lea si, Numbers
    mov cx, MaxNum - 1

@@L1:
    mov al, byte ptr [si]
    cmp al, byte ptr [si + 1]
    ja @@false
    inc si
    loop @@L1

    mov ax, 1
    jmp @@exit

@@false:
    mov ax, 0
 
@@exit:  
    ret 
IsSolved ENDP


;**************************************************************************
; NAME:     WinPopulate
; DESCR:    Populate array Numbers with ascending values
;           (1, 2, 3, ...... 23, 0)    
;           Used for testing purposes only  
; PARAMS: 	
;
; RETURNS:  AX - 0 (no), 1(yes)
;**************************************************************************

WinPopulate PROC

    lea si, Numbers
    mov cx, MaxNum
    mov al, 1
 @@L1:

    mov byte ptr [si], al
    inc si
    inc al
    loop @@L1      

    mov byte ptr [si], 0

    ret
WinPopulate ENDP 

END main     ; end of main