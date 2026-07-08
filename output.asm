; Fichier genere automatiquement par le compilateur ProLang
TITLE ProLang_Prog

; -------- Segment de pile --------
PILE SEGMENT STACK
    DW 256 DUP(?)
BASE_PILE EQU $
PILE ENDS

; -------- Segment de donnees --------
; Note : les variables de type float sont stockees en
;        virgule fixe x100 (ex: 3.14 -> 314) car le 8086
;        ne possede pas d'unite flottante (FPU).
;        Addition/Soustraction : directe.
;        Multiplication        : IMUL puis IDIV 100.
;        Division              : IMUL 100 puis IDIV.
DONNEE SEGMENT
    a                    DW 5
    b                    DW 5
    c                    DW 5
    d                    DW 0
    e                    DW 0
    f                    DW 0
    T2                   DW 0
    T8                   DW 0
    INBUF                DB 8, ?, 8 DUP(?)
    _INPROMPT            DB '? $'
DONNEE ENDS

; -------- Segment de code --------
LECODE SEGMENT
Debut:
    ASSUME CS:LECODE, DS:DONNEE, SS:PILE
    MOV AX, DONNEE
    MOV DS, AX
    MOV AX, PILE
    MOV SS, AX
    MOV SP, BASE_PILE

L0:
    MOV AX, 10
    MOV a, AX
L1:
    MOV AX, 10
    MOV b, AX
L2:
    MOV AX, 10
    MOV c, AX
L3:
    MOV AX, 10
    MOV d, AX
L4:
    MOV AX, 10
    MOV BX, 5
    ADD AX, BX
    MOV e, AX
L5:
    MOV AX, 2
    MOV a, AX
L6:
    MOV AX, 3
    MOV b, AX
L7:
    MOV AX, 2
    MOV BX, 3
    ADD AX, BX
    MOV T2, AX
L8:
    MOV AX, T2
    MOV c, AX
L9:
    MOV AX, T2
    MOV d, AX
L10:
    MOV AX, T2
    MOV e, AX
L11:
    MOV AX, 3
    MOV a, AX
L12:
    MOV AX, T2
    MOV b, AX
L13:
    MOV AX, T2
    MOV c, AX
L14:
    MOV AX, T2
    MOV d, AX
L15:
    MOV AX, 20
    MOV a, AX
L16:
    MOV AX, 30
    MOV b, AX
L17:
    MOV AX, 20
    MOV BX, 30
    ADD AX, BX
    MOV T8, AX
L18:
    MOV AX, T8
    MOV c, AX
L19:
    MOV AX, T8
    MOV d, AX
L20:
    MOV AX, T8
    MOV e, AX
L21:
    MOV AX, 42
    MOV f, AX
L22:
    MOV AX, 5
    MOV a, AX
L23:
    MOV AX, 5
    MOV b, AX
L24:
    MOV AX, 5
    MOV c, AX
L25:
    MOV AX, 5
    MOV d, AX
L26:
    MOV AX, 5
    MOV e, AX
L27:
    MOV AX, 5
    MOV BX, 5
    ADD AX, BX
    MOV f, AX
L28:
    MOV AH, 4Ch
    INT 21h
L29:
    MOV AH, 4Ch
    INT 21h

; ===== Sous-routine PRINT_INT_NOSPACE =====
; Entree : AX = entier signe a afficher sans espace final
PRINT_INT_NOSPACE PROC NEAR
    PUSH AX
    PUSH BX
    PUSH CX
    PUSH DX
    CMP AX, 0
    JGE PINS_POS
    PUSH AX
    MOV AH, 02h
    MOV DL, '-'
    INT 21h
    POP AX
    NEG AX
PINS_POS:
    MOV BX, 10
    MOV CX, 0
PINS_LOOP:
    XOR DX, DX
    DIV BX
    PUSH DX
    INC CX
    CMP AX, 0
    JNE PINS_LOOP
PINS_PRINT:
    POP DX
    ADD DL, '0'
    MOV AH, 02h
    INT 21h
    LOOP PINS_PRINT
    POP DX
    POP CX
    POP BX
    POP AX
    RET
PRINT_INT_NOSPACE ENDP

; ===== Sous-routine PRINT_INT =====
; Entree : AX = entier signe a afficher avec espace final
PRINT_INT PROC NEAR
    CALL PRINT_INT_NOSPACE
    MOV AH, 02h
    MOV DL, ' '
    INT 21h
    RET
PRINT_INT ENDP

; ===== Sous-routine PRINT_FLOAT =====
; Entree : AX = nombre fixe x100
PRINT_FLOAT PROC NEAR
    PUSH AX
    PUSH BX
    PUSH DX
    CMP AX, 0
    JGE PF_POS
    MOV DL, '-'
    MOV AH, 02h
    INT 21h
    NEG AX
PF_POS:
    XOR DX, DX
    MOV BX, 256
    DIV BX
    PUSH DX
    CALL PRINT_INT_NOSPACE
    MOV DL, '.'
    MOV AH, 02h
    INT 21h
    POP AX
    CMP AX, 10
    JGE PF_DEC
    MOV DL, '0'
    MOV AH, 02h
    INT 21h
PF_DEC:
    CALL PRINT_INT_NOSPACE
    MOV AH, 02h
    MOV DL, ' '
    INT 21h
    POP DX
    POP BX
    POP AX
    RET
PRINT_FLOAT ENDP

; ===== Sous-routine STR_TO_INT =====
; Sortie : AX = entier lu dans INBUF
STR_TO_INT PROC NEAR
    PUSH BX
    PUSH CX
    PUSH DX
    PUSH SI
    MOV AX, 0
    MOV BX, 10
    MOV CX, 0          ; flag negatif
    LEA SI, INBUF+2
    MOV DL, [SI]
    CMP DL, '-'
    JNE STI_LOOP
    MOV CX, 1
    INC SI
STI_LOOP:
    MOV DL, [SI]
    CMP DL, 0Dh
    JE  STI_DONE
    CMP DL, 0
    JE  STI_DONE
    SUB DL, '0'
    PUSH DX
    MUL BX
    POP DX
    MOV DH, 0
    ADD AX, DX
    INC SI
    JMP STI_LOOP
STI_DONE:
    CMP CX, 1
    JNE STI_POS
    NEG AX
STI_POS:
    POP SI
    POP DX
    POP CX
    POP BX
    RET
STR_TO_INT ENDP

LECODE ENDS

; -------- Fin du programme --------
END Debut
