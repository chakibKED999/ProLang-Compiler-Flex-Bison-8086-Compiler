%{
#include "ts.h"
#include "quad.h"
#include "optim.h"
#include "codegen.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

extern int ligne;
extern int colonne;
extern int nb_err_lex;

int  yylex(void);
void yyerror(const char *msg);

int nb_err_syn = 0;
int nb_err_sem = 0;

char SauvType[20];
char attente[128][15];
int  nb_attente = 0;

/* 
   OUTILS SEMANTIQUES
 */

static int type_numerique(const char *t)
{
    if (t == NULL) return 0;
    if (strcmp(t, "integer") == 0) return 1;
    if (strcmp(t, "float") == 0) return 1;
    return 0;
}

static int type_compatible(const char *g, const char *d)
{
    if (g == NULL || d == NULL) return 0;
    if (strcmp(g, d) == 0) return 1;
    if (strcmp(g, "float") == 0 && strcmp(d, "integer") == 0) return 1;
    return 0;
}

static int expr_valide(ExprInfo *e)
{
    if (e == NULL) return 0;
    if (strlen(e->type) == 0) return 0;
    if (strlen(e->place) == 0) return 0;
    return 1;
}

static void erreur_semantique(const char *msg, const char *entite)
{
    nb_err_sem++;
    fprintf(stderr,
            "Erreur semantique : %s, ligne %d, colonne %d, entite %s\n",
            msg, ligne, colonne, entite ? entite : "");
}

static void ajouterAttente(const char *nom)
{
    if (nb_attente < 128) {
        strncpy(attente[nb_attente], nom, 14);
        attente[nb_attente][14] = '\0';
        nb_attente++;
    }
}

static void viderAttente(void)
{
    nb_attente = 0;
}

static void appliquerTypeVar(void)
{
    int i;

    for (i = 0; i < nb_attente; i++) {
        if (rechercheType(attente[i])) {
            erreur_semantique("double declaration", attente[i]);
        } else {
            insererType(attente[i], SauvType);
            mettreAjourCode(attente[i], "var");
        }
    }
}

static void sauvegarder_valeur_expr(const char *nom, ExprInfo *e)
{
    char buf[64];

    if (e == NULL || !e->isConst) {
        insererValeur(nom, "");
        return;
    }

    if (e->isInt)
        sprintf(buf, "%d", e->ival);
    else
        sprintf(buf, "%g", (double)e->fval);

    insererValeur(nom, buf);
}

static int verifierDeclaration(const char *nom)
{
    if (!rechercheType(nom)) {
        erreur_semantique("identificateur non declare", nom);
        supprimerEntite(nom);
        return 0;
    }

    return 1;
}

static int verifierVariableSimple(const char *nom)
{
    if (!verifierDeclaration(nom))
        return 0;

    if (estTableau(nom)) {
        erreur_semantique("tableau utilise sans indice", nom);
        return 0;
    }

    return 1;
}

static int verifierConstanteModifiable(const char *nom)
{
    if (!rechercheType(nom))
        return 0;

    if (estConstante(nom)) {
        erreur_semantique("modification d'une constante", nom);
        return 0;
    }

    return 1;
}

static int verifierTypeAffectation(const char *nom, ExprInfo *e)
{
    const char *t;

    if (e == NULL)
        return 0;

    t = getType(nom);
    if (t == NULL || strlen(t) == 0)
        return 0;

    if (!type_compatible(t, e->type)) {
        erreur_semantique("incompatibilite de types dans affectation", nom);
        return 0;
    }

    return 1;
}

static int verifierTableauIndice(const char *nom, ExprInfo *idx)
{
    if (!verifierDeclaration(nom))
        return 0;

    if (!estTableau(nom)) {
        erreur_semantique("identificateur non tableau utilise avec indice", nom);
        return 0;
    }

    if (idx == NULL)
        return 0;

    if (strcmp(idx->type, "integer") != 0) {
        erreur_semantique("indice de tableau non entier", nom);
        return 0;
    }

    if (idx->isConst && idx->isInt) {
        if (idx->ival < 0 || idx->ival >= getTaille(nom)) {
            erreur_semantique("index hors limites", nom);
            return 0;
        }
    }

    return 1;
}

static int expr_est_zero(ExprInfo *e)
{
    if (e == NULL || !e->isConst)
        return 0;

    if (e->isInt)
        return (e->ival == 0);

    return (e->fval == 0.0f);
}

static int lire_valeur_ts(const char *nom, ExprInfo *e)
{
    const char *v;
    const char *t;

    if (e == NULL)
        return 0;

    v = getValeur(nom);
    t = getType(nom);

    if (v == NULL || t == NULL || strlen(v) == 0 || strlen(t) == 0)
        return 0;

    if (strcmp(t, "integer") == 0) {
        e->isConst = 1;
        e->isInt   = 1;
        e->ival    = atoi(v);
        e->fval    = (float)e->ival;
        return 1;
    }

    if (strcmp(t, "float") == 0) {
        e->isConst = 1;
        e->isInt   = 0;
        e->fval    = (float)atof(v);
        return 1;
    }

    return 0;
}



static ExprInfo *traiter_expr_arith(ExprInfo *a, ExprInfo *b, char op)
{
    if (a == NULL || b == NULL) {
        libererExpr(a);
        libererExpr(b);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    if (!type_numerique(a->type) || !type_numerique(b->type)) {
        erreur_semantique("operandes non numeriques", "expression");
        libererExpr(a);
        libererExpr(b);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    if (op == '/' && expr_est_zero(b))
        erreur_semantique("division par zero", "expression");

    return exprArith(a, b, op);
}

static ExprInfo *traiter_expr_comp(ExprInfo *a, ExprInfo *b, const char *opstr, int code)
{
    if (a == NULL || b == NULL) {
        libererExpr(a);
        libererExpr(b);
        return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);
    }

    if (!type_numerique(a->type) || !type_numerique(b->type)) {
        erreur_semantique("comparaison entre types non compatibles", "expression");
        libererExpr(a);
        libererExpr(b);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    return exprComp(a, b, opstr, code);
}

static ExprInfo *traiter_expr_logique(ExprInfo *a, ExprInfo *b, const char *opstr, int code)
{
    if (a == NULL || b == NULL) {
        libererExpr(a);
        libererExpr(b);
        return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);
    }

    if (strcmp(a->type, "integer") != 0 || strcmp(b->type, "integer") != 0) {
        erreur_semantique("operandes logiques invalides", "expression");
        libererExpr(a);
        libererExpr(b);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    return exprLogique(a, b, opstr, code);
}

static ExprInfo *traiter_expr_non(ExprInfo *a)
{
    if (a == NULL)
        return creerExpr("integer", "", "BZ", 0, 1, 0, 0.0f);

    if (strcmp(a->type, "integer") != 0) {
        erreur_semantique("operande NON invalide", "NON");
        libererExpr(a);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    return exprNon(a);
}

static ExprInfo *traiter_moins_unaire(ExprInfo *a)
{
    ExprInfo *zero;

    if (a == NULL)
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);

    if (!type_numerique(a->type)) {
        erreur_semantique("operande signe invalide", "expression");
        libererExpr(a);
        return creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
    }

    zero = creerExpr("integer", "0", "BZ", 1, 1, 0, 0.0f);
    return exprArith(zero, a, '-');
}
%}

%union {
    int   entier;
    float reel;
    char *str;
    void *expr;
    int   idx;
}

%token mc_beginproject mc_endproject mc_setup mc_run
%token mc_define mc_const mc_if mc_then mc_else mc_endif
%token mc_loop mc_while mc_endloop mc_for mc_to mc_endfor
%token mc_and mc_or mc_non mc_out mc_in err
%token affect infeg supeg egal diff tok_pipe

%token <str>    idf mc_integer mc_float tok_chaine
%token <entier> cst_int
%token <reel>   cst_reel

%left mc_or
%left mc_and
%right mc_non
%nonassoc '<' '>' infeg supeg egal diff
%left '+' '-'
%left '*' '/'
%right UMINUS

%type <expr> EXPR VALEUR_LITTERALE CONDITION
%type <idx>  IF_PREFIX WHILE_PREFIX FOR_PREFIX

%start S

%%

S
    : mc_beginproject idf ';' mc_setup ':' DECLARATIONS mc_run ':' '{' INSTRUCTIONS '}' mc_endproject ';'
      {
          ajouterQuad("FIN", "vide", "vide", "vide");
          supprimerEntite($2);
          free($2);
          YYACCEPT;
      }
    ;

DECLARATIONS
    : /* vide */
    | DECLARATIONS DEC
    | DECLARATIONS error ';'
      {
          yyerrok;
      }
    ;

DEC
    : DECL_CONST
    | DECL_VARIABLE
    | DECL_TABLEAU
    ;

DECL_CONST
    : mc_const idf ':' TYPE '=' VALEUR_LITTERALE ';'
      {
          if (rechercheType($2)) {
              erreur_semantique("double declaration", $2);
          } else {
              if (!type_compatible(SauvType, ((ExprInfo *)$6)->type)) {
                  erreur_semantique("incompatibilite de types dans declaration constante", $2);
                  supprimerEntite($2);
              } else {
                  mettreAjourCode($2, "const");
                  insererType($2, SauvType);
                  marquerConstante($2, 1);
                  sauvegarder_valeur_expr($2, (ExprInfo *)$6);
              }
          }

          free($2);
          libererExpr((ExprInfo *)$6);
      }
    ;

DECL_VARIABLE
    : mc_define idf ':' TYPE ';'
      {
          if (rechercheType($2)) {
              erreur_semantique("double declaration", $2);
          } else {
              insererType($2, SauvType);
              mettreAjourCode($2, "var");
          }

          free($2);
      }
    | mc_define idf ':' TYPE '=' VALEUR_LITTERALE ';'
      {
          if (rechercheType($2)) {
              erreur_semantique("double declaration", $2);
          } else {
              if (!type_compatible(SauvType, ((ExprInfo *)$6)->type)) {
                  erreur_semantique("incompatibilite de types dans initialisation", $2);
                  supprimerEntite($2);
              } else {
                  insererType($2, SauvType);
                  mettreAjourCode($2, "var");
                  sauvegarder_valeur_expr($2, (ExprInfo *)$6);
                  ajouterQuad(":=", ((ExprInfo *)$6)->place, "vide", $2);
              }
          }

          free($2);
          libererExpr((ExprInfo *)$6);
      }
    | mc_define idf SUITE_IDF ':' TYPE ';'
      {
          ajouterAttente($2);
          appliquerTypeVar();
          viderAttente();
          free($2);
      }
    ;

SUITE_IDF
    : tok_pipe idf
      {
          viderAttente();
          ajouterAttente($2);
          free($2);
      }
    | SUITE_IDF tok_pipe idf
      {
          ajouterAttente($3);
          free($3);
      }
    ;

DECL_TABLEAU
    : mc_define idf ':' '[' TYPE ';' cst_int ']' ';'
      {
          if (rechercheType($2)) {
              erreur_semantique("double declaration", $2);
          } else {
              if ($7 <= 0) {
                  erreur_semantique("taille tableau positive requise", $2);
                  supprimerEntite($2);
              } else {
                  insererType($2, SauvType);
                  insererTaille($2, $7);
                  mettreAjourCode($2, "tab");
                  marquerTableau($2, 1);
              }
          }

          free($2);
      }
    ;

TYPE
    : mc_integer
      {
          strcpy(SauvType, "integer");
          free($1);
      }
    | mc_float
      {
          strcpy(SauvType, "float");
          free($1);
      }
    ;

INSTRUCTIONS
    : /* vide */
    | INSTRUCTIONS INSTRUCTION
    | INSTRUCTIONS error ';'
      {
          yyerrok;
      }
    ;

INSTRUCTION
    : instaff
    | instif
    | instwhile
    | instfor
    | instout
    | instin
    ;

instaff
    : idf affect
      {
          $<idx>$ = nb_err_sem;
      }
      EXPR ';'
      {
          int ok;

          ok = (nb_err_sem == $<idx>3);

          if (!verifierVariableSimple($1))
              ok = 0;

          if (!verifierConstanteModifiable($1))
              ok = 0;

          if (!verifierTypeAffectation($1, (ExprInfo *)$4))
              ok = 0;

          if (!expr_valide((ExprInfo *)$4))
              ok = 0;

          if (ok) {
              ajouterQuad(":=", ((ExprInfo *)$4)->place, "vide", $1);
              sauvegarder_valeur_expr($1, (ExprInfo *)$4);
          }

          free($1);
          libererExpr((ExprInfo *)$4);
      }
    | idf '['
      {
          $<idx>$ = nb_err_sem;
      }
      EXPR ']' affect EXPR ';'
      {
          int ok;

          ok = (nb_err_sem == $<idx>3);

          if (!verifierTableauIndice($1, (ExprInfo *)$4))
              ok = 0;

          if (!verifierConstanteModifiable($1))
              ok = 0;

          if (!verifierTypeAffectation($1, (ExprInfo *)$7))
              ok = 0;

          if (!expr_valide((ExprInfo *)$4) || !expr_valide((ExprInfo *)$7))
              ok = 0;

          if (ok) {
              ajouterQuad("WRITETAB",
                          ((ExprInfo *)$7)->place,
                          ((ExprInfo *)$4)->place,
                          $1);
          }

          free($1);
          libererExpr((ExprInfo *)$4);
          libererExpr((ExprInfo *)$7);
      }
    ;

IF_PREFIX
    : mc_if '('
      {
          $<idx>$ = nb_err_sem;
      }
      CONDITION ')' mc_then ':'
      {
          int ok;

          ok = (nb_err_sem == $<idx>3);

          if (strcmp(((ExprInfo *)$4)->type, "integer") != 0) {
              erreur_semantique("condition if invalide", "if");
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)$4))
              ok = 0;

          if (ok)
              $$ = debut_if((ExprInfo *)$4);
          else
              $$ = -1;

          libererExpr((ExprInfo *)$4);
      }
    ;

instif
    : IF_PREFIX '{' INSTRUCTIONS '}' mc_endif ';'
      {
          if ($1 >= 0)
              fin_if_sans_else($1);
      }
    | IF_PREFIX '{' INSTRUCTIONS '}' mc_else
      {
          if ($1 >= 0)
              $<idx>$ = partie_else($1);
          else
              $<idx>$ = -1;
      }
      '{' INSTRUCTIONS '}' mc_endif ';'
      {
          if ($<idx>6 >= 0)
              fin_if($<idx>6);
      }
    ;

WHILE_PREFIX
    : mc_loop mc_while
      {
          $<idx>$ = debut_while();
      }
      '('
      {
          $<idx>$ = nb_err_sem;
      }
      CONDITION ')'
      {
          int idx_debut;
          int idx_bz;
          int ok;

          idx_debut = $<idx>3;
          ok = (nb_err_sem == $<idx>5);

          if (strcmp(((ExprInfo *)$6)->type, "integer") != 0) {
              erreur_semantique("condition while invalide", "while");
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)$6))
              ok = 0;

          if (ok) {
              idx_bz = conditionWhile((ExprInfo *)$6);
              $$ = idx_debut * 10000 + idx_bz;
          } else {
              $$ = -1;
          }

          libererExpr((ExprInfo *)$6);
      }
    ;

instwhile
    : WHILE_PREFIX '{' INSTRUCTIONS '}' mc_endloop ';'
      {
          int idx_debut;
          int idx_bz;

          if ($1 >= 0) {
              idx_debut = $1 / 10000;
              idx_bz    = $1 % 10000;
              fin_while(idx_debut, idx_bz);
          }
      }
    ;

FOR_PREFIX
    : mc_for idf mc_in
      {
          $<idx>$ = nb_err_sem;
      }
      EXPR mc_to EXPR
      {
          int ok;

          ok = (nb_err_sem == $<idx>4);

          if (!verifierVariableSimple($2))
              ok = 0;

          if (!verifierConstanteModifiable($2))
              ok = 0;

          if (strcmp(getType($2), "integer") != 0) {
              erreur_semantique("variable du for non entiere", $2);
              ok = 0;
          }

          if (strcmp(((ExprInfo *)$5)->type, "integer") != 0 ||
              strcmp(((ExprInfo *)$7)->type, "integer") != 0) {
              erreur_semantique("bornes du for non entieres", $2);
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)$5) || !expr_valide((ExprInfo *)$7))
              ok = 0;

          if (ok) {
              $$ = debut_for($2, (ExprInfo *)$5, (ExprInfo *)$7);
              insererValeur($2, "");
          } else {
              $$ = -1;
          }

          free($2);
          libererExpr((ExprInfo *)$5);
          libererExpr((ExprInfo *)$7);
      }
    ;

instfor
    : FOR_PREFIX '{' INSTRUCTIONS '}' mc_endfor ';'
      {
          if ($1 >= 0 && $1 < qc)
              fin_for($1, quad[$1].op2);
      }
    ;

instout
    : mc_out '('
      {
          ajouterQuad("BEGIN_OUT", "vide", "vide", "vide");
      }
      LIST_ARGS ')' ';'
      {
          ajouterQuad("END_OUT", "vide", "vide", "vide");
      }
    ;

instin
    : mc_in '(' idf ')' ';'
      {
          if (verifierVariableSimple($3) &&
              verifierConstanteModifiable($3)) {
              ajouterQuad("IN", "vide", "vide", $3);
              insererValeur($3, "");
          }

          free($3);
      }
    ;

LIST_ARGS
    : arg
    | LIST_ARGS ',' arg
    ;

arg
    : {
          $<idx>$ = nb_err_sem;
      }
      EXPR
      {
          if (nb_err_sem == $<idx>1 && expr_valide((ExprInfo *)$2))
              ajouterQuad("OUT", ((ExprInfo *)$2)->place, "vide", "vide");

          libererExpr((ExprInfo *)$2);
      }
    | tok_chaine
      {
          ajouterQuad("OUT", $1, "vide", "vide");
          free($1);
      }
    ;

CONDITION
    : EXPR
      {
          $$ = $1;
      }
    ;

EXPR
    : EXPR '+' EXPR
      {
          $$ = (void *)traiter_expr_arith((ExprInfo *)$1, (ExprInfo *)$3, '+');
      }
    | EXPR '-' EXPR
      {
          $$ = (void *)traiter_expr_arith((ExprInfo *)$1, (ExprInfo *)$3, '-');
      }
    | EXPR '*' EXPR
      {
          $$ = (void *)traiter_expr_arith((ExprInfo *)$1, (ExprInfo *)$3, '*');
      }
    | EXPR '/' EXPR
      {
          $$ = (void *)traiter_expr_arith((ExprInfo *)$1, (ExprInfo *)$3, '/');
      }
    | EXPR '<' EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, "<", 1);
      }
    | EXPR '>' EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, ">", 2);
      }
    | EXPR infeg EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, "<=", 3);
      }
    | EXPR supeg EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, ">=", 4);
      }
    | EXPR egal EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, "==", 5);
      }
    | EXPR diff EXPR
      {
          $$ = (void *)traiter_expr_comp((ExprInfo *)$1, (ExprInfo *)$3, "!=", 6);
      }
    | EXPR mc_and EXPR
      {
          $$ = (void *)traiter_expr_logique((ExprInfo *)$1, (ExprInfo *)$3, "AND", 1);
      }
    | EXPR mc_or EXPR
      {
          $$ = (void *)traiter_expr_logique((ExprInfo *)$1, (ExprInfo *)$3, "OR", 2);
      }
    | mc_non '(' EXPR ')'
      {
          $$ = (void *)traiter_expr_non((ExprInfo *)$3);
      }
    | '-' EXPR %prec UMINUS
      {
          $$ = (void *)traiter_moins_unaire((ExprInfo *)$2);
      }
    | '(' EXPR ')'
      {
          $$ = $2;
      }
    | idf
      {
          char place[64];
          ExprInfo *e;

          if (verifierVariableSimple($1)) {
              strncpy(place, $1, 63);
              place[63] = '\0';

              e = creerExpr(getType($1), place, "BZ", 0, 0, 0, 0.0f);
              lire_valeur_ts($1, e);
          } else {
              e = creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
          }

          $$ = (void *)e;
          free($1);
      }
    | idf '[' EXPR ']'
      {
          char temp[32];
          ExprInfo *e;

          if (verifierTableauIndice($1, (ExprInfo *)$3)) {
              nouveauTemp(temp);
              ajouterQuad("READTAB", $1, ((ExprInfo *)$3)->place, temp);
              e = creerExpr(getType($1), temp, "BZ", 0, 0, 0, 0.0f);
          } else {
              e = creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
          }

          $$ = (void *)e;
          free($1);
          libererExpr((ExprInfo *)$3);
      }
    | cst_int
      {
          char place[64];
          sprintf(place, "%d", $1);
          $$ = (void *)creerExpr("integer", place, "BZ", 1, 1, $1, (float)$1);
      }
    | cst_reel
      {
          char place[64];
          sprintf(place, "%g", (double)$1);
          $$ = (void *)creerExpr("float", place, "BZ", 1, 0, 0, $1);
      }
    ;

VALEUR_LITTERALE
    : cst_int
      {
          char place[64];
          sprintf(place, "%d", $1);
          $$ = (void *)creerExpr("integer", place, "BZ", 1, 1, $1, (float)$1);
      }
    | cst_reel
      {
          char place[64];
          sprintf(place, "%g", (double)$1);
          $$ = (void *)creerExpr("float", place, "BZ", 1, 0, 0, $1);
      }
    ;

%%

void yyerror(const char *msg)
{
    nb_err_syn++;
    fprintf(stderr,
            "Erreur syntaxique : %s | Ligne %d, Colonne %d\n",
            msg, ligne, colonne);
}

int main(void)
{
    int r;

    r = yyparse();

    if (r == 0) {
        if (nb_err_lex == 0 && nb_err_syn == 0 && nb_err_sem == 0) {
            printf("Analyse syntaxique et semantique reussie.\n");

            afficherQuads();

            optimiserQuadruplets();

            afficherQuadsOptimises();

            /* Generation du code machine (assembleur 8086) */
            genererCode8086("output.asm", "ProLang_Prog");

        } else {
            printf("Analyse terminee avec erreurs.\n");
            afficherQuads();
        }

        afficher();
    } else {
        printf("Analyse interrompue.\n");
        afficherQuads();
        afficher();
    }

    liberer();
    return 0;
}