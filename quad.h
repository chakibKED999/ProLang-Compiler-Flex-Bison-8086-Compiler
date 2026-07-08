#ifndef QUAD_H
#define QUAD_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ============================================================
   STRUCTURE D'UN QUADRUPLET
   (operateur, operande1, operande2, resultat)
   ============================================================ */
typedef struct {
    char oper[100];
    char op1[100];
    char op2[100];
    char res[100];
} Quad;

/* ============================================================
   STRUCTURE ExprInfo
   Transporte le type, le "place" et l'operateur de comparaison
   d'une expression entre les regles Bison.

   NOUVEAU : champ cmpOp
   Quand une expression est une comparaison directe (x > y),
   on stocke ici l'operateur inverse pour le saut conditionnel.
   Ex : si condition = (x > y), on saute si x <= y  → BLE
   ============================================================ */
typedef struct {
    char  type[10];    /* "integer" ou "float"                    */
    char  place[64];   /* nom variable ou temporaire Tx           */
    char  cmpOp[8];    /* operateur de saut : "BZ","BNZ","BGT"... */
    int   isConst;
    int   isInt;
    int   ival;
    float fval;
} ExprInfo;

/* ============================================================
   TABLE DES QUADRUPLETS (max 1000)
   ============================================================ */
extern Quad quad[1000];
extern int  qc;
extern int  cptTemp;

/* ============================================================
   OPERATEURS DE SAUT CONDITIONNEL
   ============================================================
   BZ  : Branch if Zero       → saute si condition == 0 (faux)
   BNZ : Branch if Not Zero   → saute si condition != 0 (vrai)
   BGT : Branch if >          → saute si op1 >  op2
   BLT : Branch if <          → saute si op1 <  op2
   BGE : Branch if >=         → saute si op1 >= op2
   BLE : Branch if <=         → saute si op1 <= op2
   BR  : Branch unconditional → saute toujours
   ============================================================ */

/* ============================================================
   FONCTIONS DE BASE
   ============================================================ */
void ajouterQuad(char oper[], char op1[], char op2[], char res[]);
void updateQuad (int num_quad, int colon_quad, char val[]);
void nouveauTemp(char *buf);
void afficherQuads(void);

/* ============================================================
   GESTION DES EXPRESSIONS
   ============================================================ */
ExprInfo *creerExpr (const char *type, const char *place,
                     const char *cmpOp,
                     int isConst, int isInt, int ival, float fval);
void      libererExpr(ExprInfo *e);

ExprInfo *exprArith  (ExprInfo *a, ExprInfo *b, char op);
ExprInfo *exprComp   (ExprInfo *a, ExprInfo *b, const char *opstr, int code);
ExprInfo *exprLogique(ExprInfo *a, ExprInfo *b, const char *opstr, int code);
ExprInfo *exprNon    (ExprInfo *a);

/* ============================================================
   GESTION DU IF  (sans piles, avec BZ/BNZ/BGT/BLT/BGE/BLE)
   ============================================================ */
int  debut_if        (ExprInfo *cond);
int  partie_else     (int idx_bz);
void fin_if          (int idx_br);
void fin_if_sans_else(int idx_bz);

/* ============================================================
   GESTION DU WHILE  (sans piles)
   ============================================================ */
int  debut_while    (void);
int  conditionWhile (ExprInfo *cond);
void fin_while      (int idx_debut, int idx_bz);

/* ============================================================
   GESTION DU FOR  (sans piles)
   ============================================================ */
int  debut_for(const char *var, ExprInfo *init, ExprInfo *lim);
void fin_for  (int idx_bz, const char *var);

#endif
