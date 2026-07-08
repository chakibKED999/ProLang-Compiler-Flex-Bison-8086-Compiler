#include "quad.h"


Quad quad[1000];
int  qc      = 0;
int  cptTemp = 0;


void ajouterQuad(char oper[], char op1[], char op2[], char res[])
{
    if (qc >= 1000) {
        fprintf(stderr, "Erreur : trop de quadruplets.\n");
        exit(1);
    }
    strcpy(quad[qc].oper, oper);
    strcpy(quad[qc].op1,  op1);
    strcpy(quad[qc].op2,  op2);
    strcpy(quad[qc].res,  res);
    qc++;
}


void updateQuad(int num_quad, int colon_quad, char val[])
{
    if      (colon_quad == 0) strcpy(quad[num_quad].oper, val);
    else if (colon_quad == 1) strcpy(quad[num_quad].op1,  val);
    else if (colon_quad == 2) strcpy(quad[num_quad].op2,  val);
    else if (colon_quad == 3) strcpy(quad[num_quad].res,  val);
}

void nouveauTemp(char *buf)
{
    sprintf(buf, "T%d", ++cptTemp);
}


void afficherQuads(void)
{
    int i;
    printf("\n*********************Les Quadruplets***********************\n");
    printf("------------------------------------------------------------\n");
    for (i = 0; i < qc; i++) {
        printf(" %d - ( %s , %s , %s , %s )\n",
               i, quad[i].oper, quad[i].op1, quad[i].op2, quad[i].res);
        printf("------------------------------------------------------------\n");
    }
}


ExprInfo *creerExpr(const char *type, const char *place,
                    const char *cmpOp,
                    int isConst, int isInt, int ival, float fval)
{
    ExprInfo *e = (ExprInfo *)malloc(sizeof(ExprInfo));
    if (e == NULL) { fprintf(stderr, "Erreur memoire.\n"); exit(1); }

    strcpy(e->type,  type   ? type   : "");
    strncpy(e->place, place ? place  : "", 63); e->place[63] = '\0';
    strncpy(e->cmpOp, cmpOp ? cmpOp  : "BZ", 7); e->cmpOp[7] = '\0';

    e->isConst = isConst;
    e->isInt   = isInt;
    e->ival    = ival;
    e->fval    = fval;
    return e;
}

void libererExpr(ExprInfo *e)
{
    if (e) free(e);
}

/* ============================================================
  
     x > y  est faux  quand x <= y  → BLE
     x < y  est faux  quand x >= y  → BGE
     x >= y est faux  quand x <  y  → BLT
     x <= y est faux  quand x >  y  → BGT
     x == y est faux  quand x != y  → BNZ
     x != y est faux  quand x == y  → BZ
   ============================================================ */
static const char *sautInverse(const char *op)
{
    if (strcmp(op, ">")  == 0) return "BLE";
    if (strcmp(op, "<")  == 0) return "BGE";
    if (strcmp(op, ">=") == 0) return "BLT";
    if (strcmp(op, "<=") == 0) return "BGT";
    if (strcmp(op, "==") == 0) return "BNZ";
    if (strcmp(op, "!=") == 0) return "BZ";
    return "BZ";  /* par defaut */
}

ExprInfo *exprArith(ExprInfo *a, ExprInfo *b, char op)
{
    char temp[32], opstr[4];

    if (a == NULL || b == NULL) {
        libererExpr(a); libererExpr(b);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    if (op == '/' && b->isConst && b->isInt && b->ival == 0)
        fprintf(stderr, "Erreur semantique : division par zero\n");

    nouveauTemp(temp);
    opstr[0] = op; opstr[1] = '\0';
    ajouterQuad(opstr, a->place, b->place, temp);

    if (op == '/' || strcmp(a->type,"float")==0 || strcmp(b->type,"float")==0) {
        ExprInfo *r = creerExpr("float", temp, "BZ", 0, 0, 0, 0.0f);
        libererExpr(a); libererExpr(b);
        return r;
    } else {
        ExprInfo *r = creerExpr("integer", temp, "BZ", 0, 1, 0, 0.0f);
        libererExpr(a); libererExpr(b);
        return r;
    }
}


ExprInfo *exprComp(ExprInfo *a, ExprInfo *b, const char *opstr, int code)
{
    char temp[32];
    const char *saut;
    ExprInfo *r;
    (void)code;

    if (a == NULL || b == NULL) {
        libererExpr(a); libererExpr(b);
        return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);
    }

    saut = sautInverse(opstr);

    nouveauTemp(temp);
    ajouterQuad((char*)opstr, a->place, b->place, temp);

    
    r = creerExpr("integer", temp, saut, 0, 1, 0, 0.0f);

    

    libererExpr(a); libererExpr(b);
    return r;
}

/*
   AND / OR */
ExprInfo *exprLogique(ExprInfo *a, ExprInfo *b, const char *opstr, int code)
{
    char temp[32];
    (void)code;

    if (a == NULL || b == NULL) {
        libererExpr(a); libererExpr(b);
        return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);
    }

    nouveauTemp(temp);
    ajouterQuad((char*)opstr, a->place, b->place, temp);

    libererExpr(a); libererExpr(b);
    return creerExpr("integer", temp, "BZ", 0, 1, 0, 0.0f);
}


ExprInfo *exprNon(ExprInfo *a)
{
    char temp[32];
    if (a == NULL) return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);

    nouveauTemp(temp);
    ajouterQuad("NON", a->place, "vide", temp);

    libererExpr(a);
    return creerExpr("integer", temp, "BNZ", 0, 1, 0, 0.0f);
}


int debut_if(ExprInfo *cond)
{
    int idx = qc;

   
    char op1_cond[64], op2_cond[64];
    strcpy(op1_cond, quad[qc-1].op1);  
    strcpy(op2_cond, quad[qc-1].op2);  

    
    if (strcmp(cond->cmpOp, "BZ") == 0) {
        ajouterQuad("BZ", "vide", cond->place, "vide");
    } else {
        ajouterQuad(cond->cmpOp, "vide", op1_cond, op2_cond);
    }

    return idx;
}


int partie_else(int idx_bz)
{
    int idx_br = qc;
    char tmp[20];

    ajouterQuad("BR", "vide", "vide", "vide");

    sprintf(tmp, "%d", qc);
    updateQuad(idx_bz, 1, tmp);

    return idx_br;
}

void fin_if_sans_else(int idx_bz)
{
    char tmp[20];
    sprintf(tmp, "%d", qc);
    updateQuad(idx_bz, 1, tmp);
}

void fin_if(int idx_br)
{
    char tmp[20];
    sprintf(tmp, "%d", qc);
    updateQuad(idx_br, 1, tmp);
}



int debut_while(void)
{
    return qc;
}

int conditionWhile(ExprInfo *cond)
{
    int idx = qc;
    char op1_cond[64], op2_cond[64];

    strcpy(op1_cond, quad[qc-1].op1);
    strcpy(op2_cond, quad[qc-1].op2);

    if (strcmp(cond->cmpOp, "BZ") == 0) {
        ajouterQuad("BZ", "vide", cond->place, "vide");
    } else {
        ajouterQuad(cond->cmpOp, "vide", op1_cond, op2_cond);
    }

    return idx;
}

void fin_while(int idx_debut, int idx_bz)
{
    char tmp[20];

    sprintf(tmp, "%d", idx_debut);
    ajouterQuad("BR", tmp, "vide", "vide");

    sprintf(tmp, "%d", qc);
    updateQuad(idx_bz, 1, tmp);
}



int debut_for(const char *var, ExprInfo *init, ExprInfo *lim)
{
    char tempLim[32], tempCond[32], tmp[20];
    int  idx_debut, idx_bz;

    
    ajouterQuad(":=", init->place, "vide", (char*)var);

    nouveauTemp(tempLim);
    ajouterQuad(":=", lim->place, "vide", tempLim);

   
    idx_debut = qc;

    
    nouveauTemp(tempCond);
    ajouterQuad("<=", (char*)var, tempLim, tempCond);

   
    idx_bz = qc;
    sprintf(tmp, "%d", idx_debut);
    ajouterQuad("BGT", tmp, (char*)var, tempLim);
   

    return idx_bz;
}

void fin_for(int idx_bz, const char *var)
{
    char tempIncr[32], tmp[20];
    int  idx_debut;

    
    idx_debut = atoi(quad[idx_bz].op1);

    
    nouveauTemp(tempIncr);
    ajouterQuad("+",  (char*)var, "1",     tempIncr);
    ajouterQuad(":=", tempIncr,  "vide",   (char*)var);

    
    sprintf(tmp, "%d", idx_debut);
    ajouterQuad("BR", tmp, "vide", "vide");

   
    sprintf(tmp, "%d", qc);
    updateQuad(idx_bz, 1, tmp);
}
