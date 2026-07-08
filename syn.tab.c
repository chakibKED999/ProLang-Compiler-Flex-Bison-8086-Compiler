
/* A Bison parser, made by GNU Bison 2.4.1.  */

/* Skeleton implementation for Bison's Yacc-like parsers in C
   
      Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "2.4.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1

/* Using locations.  */
#define YYLSP_NEEDED 0



/* Copy the first part of user declarations.  */

/* Line 189 of yacc.c  */
#line 1 "syn.y"

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


/* Line 189 of yacc.c  */
#line 396 "syn.tab.c"

/* Enabling traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

/* Enabling the token table.  */
#ifndef YYTOKEN_TABLE
# define YYTOKEN_TABLE 0
#endif


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     mc_beginproject = 258,
     mc_endproject = 259,
     mc_setup = 260,
     mc_run = 261,
     mc_define = 262,
     mc_const = 263,
     mc_if = 264,
     mc_then = 265,
     mc_else = 266,
     mc_endif = 267,
     mc_loop = 268,
     mc_while = 269,
     mc_endloop = 270,
     mc_for = 271,
     mc_to = 272,
     mc_endfor = 273,
     mc_and = 274,
     mc_or = 275,
     mc_non = 276,
     mc_out = 277,
     mc_in = 278,
     err = 279,
     affect = 280,
     infeg = 281,
     supeg = 282,
     egal = 283,
     diff = 284,
     tok_pipe = 285,
     idf = 286,
     mc_integer = 287,
     mc_float = 288,
     tok_chaine = 289,
     cst_int = 290,
     cst_reel = 291,
     UMINUS = 292
   };
#endif



#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 214 of yacc.c  */
#line 323 "syn.y"

    int   entier;
    float reel;
    char *str;
    void *expr;
    int   idx;



/* Line 214 of yacc.c  */
#line 479 "syn.tab.c"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif


/* Copy the second part of user declarations.  */


/* Line 264 of yacc.c  */
#line 491 "syn.tab.c"

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#elif (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
typedef signed char yytype_int8;
#else
typedef short int yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(e) ((void) (e))
#else
# define YYUSE(e) /* empty */
#endif

/* Identity function, used to suppress warnings about constant conditions.  */
#ifndef lint
# define YYID(n) (n)
#else
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static int
YYID (int yyi)
#else
static int
YYID (yyi)
    int yyi;
#endif
{
  return yyi;
}
#endif

#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#     ifndef _STDLIB_H
#      define _STDLIB_H 1
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's `empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (YYID (0))
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined _STDLIB_H \
       && ! ((defined YYMALLOC || defined malloc) \
	     && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef _STDLIB_H
#    define _STDLIB_H 1
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined _STDLIB_H && (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
	 || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

/* Copy COUNT objects from FROM to TO.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(To, From, Count) \
      __builtin_memcpy (To, From, (Count) * sizeof (*(From)))
#  else
#   define YYCOPY(To, From, Count)		\
      do					\
	{					\
	  YYSIZE_T yyi;				\
	  for (yyi = 0; yyi < (Count); yyi++)	\
	    (To)[yyi] = (From)[yyi];		\
	}					\
      while (YYID (0))
#  endif
# endif

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)				\
    do									\
      {									\
	YYSIZE_T yynewbytes;						\
	YYCOPY (&yyptr->Stack_alloc, Stack, yysize);			\
	Stack = &yyptr->Stack_alloc;					\
	yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
	yyptr += yynewbytes / sizeof (*yyptr);				\
      }									\
    while (YYID (0))

#endif

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  4
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   344

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  54
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  34
/* YYNRULES -- Number of rules.  */
#define YYNRULES  72
/* YYNRULES -- Number of states.  */
#define YYNSTATES  179

/* YYTRANSLATE(YYLEX) -- Bison symbol number corresponding to YYLEX.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   292

#define YYTRANSLATE(YYX)						\
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[YYLEX] -- Bison symbol number corresponding to YYLEX.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      51,    52,    41,    39,    53,    40,     2,    42,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    45,    44,
      37,    48,    38,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    49,     2,    50,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    46,     2,    47,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    43
};

#if YYDEBUG
/* YYPRHS[YYN] -- Index of the first RHS symbol of rule number YYN in
   YYRHS.  */
static const yytype_uint16 yyprhs[] =
{
       0,     0,     3,    17,    18,    21,    25,    27,    29,    31,
      39,    45,    53,    60,    63,    67,    77,    79,    81,    82,
      85,    89,    91,    93,    95,    97,    99,   101,   102,   108,
     109,   118,   119,   127,   134,   135,   147,   148,   149,   157,
     164,   165,   173,   180,   181,   188,   194,   196,   200,   201,
     204,   206,   208,   212,   216,   220,   224,   228,   232,   236,
     240,   244,   248,   252,   256,   261,   264,   268,   270,   275,
     277,   279,   281
};

/* YYRHS -- A `-1'-separated list of the rules' RHS.  */
static const yytype_int8 yyrhs[] =
{
      55,     0,    -1,     3,    31,    44,     5,    45,    56,     6,
      45,    46,    63,    47,     4,    44,    -1,    -1,    56,    57,
      -1,    56,     1,    44,    -1,    58,    -1,    59,    -1,    61,
      -1,     8,    31,    45,    62,    48,    87,    44,    -1,     7,
      31,    45,    62,    44,    -1,     7,    31,    45,    62,    48,
      87,    44,    -1,     7,    31,    60,    45,    62,    44,    -1,
      30,    31,    -1,    60,    30,    31,    -1,     7,    31,    45,
      49,    62,    44,    35,    50,    44,    -1,    32,    -1,    33,
      -1,    -1,    63,    64,    -1,    63,     1,    44,    -1,    65,
      -1,    70,    -1,    75,    -1,    78,    -1,    79,    -1,    81,
      -1,    -1,    31,    25,    66,    86,    44,    -1,    -1,    31,
      49,    67,    86,    50,    25,    86,    44,    -1,    -1,     9,
      51,    69,    85,    52,    10,    45,    -1,    68,    46,    63,
      47,    12,    44,    -1,    -1,    68,    46,    63,    47,    11,
      71,    46,    63,    47,    12,    44,    -1,    -1,    -1,    13,
      14,    73,    51,    74,    85,    52,    -1,    72,    46,    63,
      47,    15,    44,    -1,    -1,    16,    31,    23,    77,    86,
      17,    86,    -1,    76,    46,    63,    47,    18,    44,    -1,
      -1,    22,    51,    80,    82,    52,    44,    -1,    23,    51,
      31,    52,    44,    -1,    83,    -1,    82,    53,    83,    -1,
      -1,    84,    86,    -1,    34,    -1,    86,    -1,    86,    39,
      86,    -1,    86,    40,    86,    -1,    86,    41,    86,    -1,
      86,    42,    86,    -1,    86,    37,    86,    -1,    86,    38,
      86,    -1,    86,    26,    86,    -1,    86,    27,    86,    -1,
      86,    28,    86,    -1,    86,    29,    86,    -1,    86,    19,
      86,    -1,    86,    20,    86,    -1,    21,    51,    86,    52,
      -1,    40,    86,    -1,    51,    86,    52,    -1,    31,    -1,
      31,    49,    86,    50,    -1,    35,    -1,    36,    -1,    35,
      -1,    36,    -1
};

/* YYRLINE[YYN] -- source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   357,   357,   366,   368,   369,   376,   377,   378,   382,
     404,   415,   434,   444,   450,   458,   479,   484,   491,   493,
     494,   501,   502,   503,   504,   505,   506,   511,   510,   541,
     540,   577,   576,   604,   610,   609,   625,   629,   624,   661,
     676,   675,   719,   728,   727,   738,   751,   752,   756,   756,
     766,   774,   781,   785,   789,   793,   797,   801,   805,   809,
     813,   817,   821,   825,   829,   833,   837,   841,   859,   876,
     882,   891,   897
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || YYTOKEN_TABLE
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "mc_beginproject", "mc_endproject",
  "mc_setup", "mc_run", "mc_define", "mc_const", "mc_if", "mc_then",
  "mc_else", "mc_endif", "mc_loop", "mc_while", "mc_endloop", "mc_for",
  "mc_to", "mc_endfor", "mc_and", "mc_or", "mc_non", "mc_out", "mc_in",
  "err", "affect", "infeg", "supeg", "egal", "diff", "tok_pipe", "idf",
  "mc_integer", "mc_float", "tok_chaine", "cst_int", "cst_reel", "'<'",
  "'>'", "'+'", "'-'", "'*'", "'/'", "UMINUS", "';'", "':'", "'{'", "'}'",
  "'='", "'['", "']'", "'('", "')'", "','", "$accept", "S", "DECLARATIONS",
  "DEC", "DECL_CONST", "DECL_VARIABLE", "SUITE_IDF", "DECL_TABLEAU",
  "TYPE", "INSTRUCTIONS", "INSTRUCTION", "instaff", "@1", "@2",
  "IF_PREFIX", "@3", "instif", "@4", "WHILE_PREFIX", "@5", "@6",
  "instwhile", "FOR_PREFIX", "@7", "instfor", "instout", "$@8", "instin",
  "LIST_ARGS", "arg", "@9", "CONDITION", "EXPR", "VALEUR_LITTERALE", 0
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[YYLEX-NUM] -- Internal token number corresponding to
   token YYLEX-NUM.  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,    60,    62,    43,
      45,    42,    47,   292,    59,    58,   123,   125,    61,    91,
      93,    40,    41,    44
};
# endif

/* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    54,    55,    56,    56,    56,    57,    57,    57,    58,
      59,    59,    59,    60,    60,    61,    62,    62,    63,    63,
      63,    64,    64,    64,    64,    64,    64,    66,    65,    67,
      65,    69,    68,    70,    71,    70,    73,    74,    72,    75,
      77,    76,    78,    80,    79,    81,    82,    82,    84,    83,
      83,    85,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    86,    86,
      86,    87,    87
};

/* YYR2[YYN] -- Number of symbols composing right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,    13,     0,     2,     3,     1,     1,     1,     7,
       5,     7,     6,     2,     3,     9,     1,     1,     0,     2,
       3,     1,     1,     1,     1,     1,     1,     0,     5,     0,
       8,     0,     7,     6,     0,    11,     0,     0,     7,     6,
       0,     7,     6,     0,     6,     5,     1,     3,     0,     2,
       1,     1,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     4,     2,     3,     1,     4,     1,
       1,     1,     1
};

/* YYDEFACT[STATE-NAME] -- Default rule to reduce with in state
   STATE-NUM when YYTABLE doesn't specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     0,     0,     0,     1,     0,     0,     3,     0,     0,
       0,     0,     0,     4,     6,     7,     8,     5,     0,     0,
       0,    18,     0,     0,     0,     0,     0,    13,    16,    17,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,    19,    21,     0,    22,     0,    23,     0,
      24,    25,    26,     0,    10,     0,    14,     0,     0,    20,
      31,    36,     0,    43,     0,    27,    29,     0,    18,    18,
      18,     0,    71,    72,     0,    12,     0,     0,     0,    40,
      48,     0,     0,     0,     2,     0,     0,     0,     0,    11,
       9,     0,    67,    69,    70,     0,     0,     0,    51,    37,
       0,    50,     0,    46,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    65,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,    48,    49,    45,    28,     0,    34,     0,     0,
       0,    15,     0,     0,    66,     0,    62,    63,    58,    59,
      60,    61,    56,    57,    52,    53,    54,    55,     0,     0,
      44,    47,     0,     0,    33,    39,    42,    64,    68,    32,
      38,    41,     0,    18,    30,     0,     0,     0,    35
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     8,    13,    14,    15,    24,    16,    31,    26,
      43,    44,    82,    83,    45,    77,    46,   163,    47,    78,
     129,    48,    49,   100,    50,    51,    80,    52,   102,   103,
     104,    97,    98,    74
};

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
#define YYPACT_NINF -80
static const yytype_int16 yypact[] =
{
      10,    -7,    31,    -8,   -80,    45,     7,   -80,   162,    18,
       9,    36,    51,   -80,   -80,   -80,   -80,   -80,    38,   -25,
      48,   -80,    59,   -23,   -22,   -21,     6,   -80,   -80,   -80,
     -21,   -30,    61,   -21,    58,    54,    44,    85,    72,    56,
      63,   -19,   106,   -80,   -80,    69,   -80,    70,   -80,    77,
     -80,   -80,   -80,    80,   -80,    22,   -80,    83,    22,   -80,
     -80,   -80,   105,   -80,    98,   -80,   -80,    92,   -80,   -80,
     -80,   102,   -80,   -80,    94,   -80,    97,    34,    99,   -80,
     109,   100,    34,    34,   -80,    50,    55,    78,   101,   -80,
     -80,   103,   104,   -80,   -80,    34,    34,   112,   273,   -80,
      34,   -80,     8,   -80,    34,   111,   197,   147,    64,   129,
     138,   121,    34,    34,   -80,    93,   161,    34,    34,    34,
      34,    34,    34,    34,    34,    34,    34,    34,    34,    34,
     223,   133,   109,   273,   -80,   -80,   153,   -80,   135,   136,
     137,   -80,   120,   172,   -80,   145,   296,   279,   302,   302,
     302,   302,   302,   302,    47,    47,   -80,   -80,   130,    34,
     -80,   -80,    34,   148,   -80,   -80,   -80,   -80,   -80,   -80,
     -80,   273,   247,   -80,   -80,    95,   171,   149,   -80
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
     -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,     2,   -68,
     -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,
     -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,   -80,    71,
     -80,    66,   -79,   144
};

/* YYTABLE[YYPACT[STATE-NUM]].  What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule which
   number is the opposite.  If zero, do what YYDEFACT says.
   If YYTABLE_NINF, syntax error.  */
#define YYTABLE_NINF -1
static const yytype_int16 yytable[] =
{
      85,    86,    87,   106,   107,    22,    65,    35,    32,    28,
      29,    28,    29,     1,    54,    36,   114,   115,    55,    37,
      23,   130,    38,    33,     3,   133,    30,    34,    39,    40,
      66,     4,    53,   142,   143,    57,     5,    41,   146,   147,
     148,   149,   150,   151,   152,   153,   154,   155,   156,   157,
       6,    35,     7,    42,    18,    91,    35,    72,    73,    36,
     131,   132,    17,    37,    36,    92,    38,    19,    37,    93,
      94,    38,    39,    40,    95,   137,   138,    39,    40,    35,
     171,    41,    20,   172,    21,    96,    41,    36,   127,   128,
      27,    37,    56,    25,    38,    60,    35,   108,    59,    61,
      39,    40,   109,    62,    36,   175,    58,    63,    37,    41,
      67,    38,   117,   118,    64,    68,    69,    39,    40,   119,
     120,   121,   122,    70,    71,   110,    41,    75,    79,    81,
     123,   124,   125,   126,   127,   128,    84,    88,    89,   117,
     118,    90,   176,   101,   139,   144,   119,   120,   121,   122,
      99,   111,   105,   113,   112,   134,   140,   123,   124,   125,
     126,   127,   128,     9,   116,   141,   117,   118,    10,    11,
      12,   145,   167,   119,   120,   121,   122,   160,   162,   164,
     165,   166,   170,   177,   123,   124,   125,   126,   127,   128,
     169,   117,   118,   178,   173,   158,     0,   136,   119,   120,
     121,   122,    76,   161,     0,     0,     0,     0,     0,   123,
     124,   125,   126,   127,   128,     0,   117,   118,     0,     0,
       0,     0,   168,   119,   120,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,   124,   125,   126,   127,   128,
     159,   135,   117,   118,     0,     0,     0,     0,     0,   119,
     120,   121,   122,     0,     0,     0,     0,     0,     0,     0,
     123,   124,   125,   126,   127,   128,   117,   118,     0,     0,
       0,     0,     0,   119,   120,   121,   122,     0,     0,     0,
       0,     0,     0,     0,   123,   124,   125,   126,   127,   128,
       0,   174,   117,   118,     0,     0,     0,     0,   117,   119,
     120,   121,   122,     0,     0,   119,   120,   121,   122,     0,
     123,   124,   125,   126,   127,   128,   123,   124,   125,   126,
     127,   128,   119,   120,   121,   122,     0,     0,    -1,    -1,
      -1,    -1,     0,   123,   124,   125,   126,   127,   128,    -1,
      -1,   125,   126,   127,   128
};

static const yytype_int16 yycheck[] =
{
      68,    69,    70,    82,    83,    30,    25,     1,    30,    32,
      33,    32,    33,     3,    44,     9,    95,    96,    48,    13,
      45,   100,    16,    45,    31,   104,    49,    25,    22,    23,
      49,     0,    30,   112,   113,    33,    44,    31,   117,   118,
     119,   120,   121,   122,   123,   124,   125,   126,   127,   128,
       5,     1,    45,    47,    45,    21,     1,    35,    36,     9,
      52,    53,    44,    13,     9,    31,    16,    31,    13,    35,
      36,    16,    22,    23,    40,    11,    12,    22,    23,     1,
     159,    31,    31,   162,    46,    51,    31,     9,    41,    42,
      31,    13,    31,    45,    16,    51,     1,    47,    44,    14,
      22,    23,    47,    31,     9,   173,    48,    51,    13,    31,
       4,    16,    19,    20,    51,    46,    46,    22,    23,    26,
      27,    28,    29,    46,    44,    47,    31,    44,    23,    31,
      37,    38,    39,    40,    41,    42,    44,    35,    44,    19,
      20,    44,    47,    34,    15,    52,    26,    27,    28,    29,
      51,    50,    52,    49,    51,    44,    18,    37,    38,    39,
      40,    41,    42,     1,    52,    44,    19,    20,     6,     7,
       8,    10,    52,    26,    27,    28,    29,    44,    25,    44,
      44,    44,    52,    12,    37,    38,    39,    40,    41,    42,
      45,    19,    20,    44,    46,   129,    -1,    50,    26,    27,
      28,    29,    58,   132,    -1,    -1,    -1,    -1,    -1,    37,
      38,    39,    40,    41,    42,    -1,    19,    20,    -1,    -1,
      -1,    -1,    50,    26,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      17,    44,    19,    20,    -1,    -1,    -1,    -1,    -1,    26,
      27,    28,    29,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      37,    38,    39,    40,    41,    42,    19,    20,    -1,    -1,
      -1,    -1,    -1,    26,    27,    28,    29,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    37,    38,    39,    40,    41,    42,
      -1,    44,    19,    20,    -1,    -1,    -1,    -1,    19,    26,
      27,    28,    29,    -1,    -1,    26,    27,    28,    29,    -1,
      37,    38,    39,    40,    41,    42,    37,    38,    39,    40,
      41,    42,    26,    27,    28,    29,    -1,    -1,    26,    27,
      28,    29,    -1,    37,    38,    39,    40,    41,    42,    37,
      38,    39,    40,    41,    42
};

/* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
   symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,    55,    31,     0,    44,     5,    45,    56,     1,
       6,     7,     8,    57,    58,    59,    61,    44,    45,    31,
      31,    46,    30,    45,    60,    45,    63,    31,    32,    33,
      49,    62,    30,    45,    62,     1,     9,    13,    16,    22,
      23,    31,    47,    64,    65,    68,    70,    72,    75,    76,
      78,    79,    81,    62,    44,    48,    31,    62,    48,    44,
      51,    14,    31,    51,    51,    25,    49,     4,    46,    46,
      46,    44,    35,    36,    87,    44,    87,    69,    73,    23,
      80,    31,    66,    67,    44,    63,    63,    63,    35,    44,
      44,    21,    31,    35,    36,    40,    51,    85,    86,    51,
      77,    34,    82,    83,    84,    52,    86,    86,    47,    47,
      47,    50,    51,    49,    86,    86,    52,    19,    20,    26,
      27,    28,    29,    37,    38,    39,    40,    41,    42,    74,
      86,    52,    53,    86,    44,    44,    50,    11,    12,    15,
      18,    44,    86,    86,    52,    10,    86,    86,    86,    86,
      86,    86,    86,    86,    86,    86,    86,    86,    85,    17,
      44,    83,    25,    71,    44,    44,    44,    52,    50,    45,
      52,    86,    86,    46,    44,    63,    47,    12,    44
};

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		(-2)
#define YYEOF		0

#define YYACCEPT	goto yyacceptlab
#define YYABORT		goto yyabortlab
#define YYERROR		goto yyerrorlab


/* Like YYERROR except do call yyerror.  This remains here temporarily
   to ease the transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */

#define YYFAIL		goto yyerrlab

#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)					\
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    {								\
      yychar = (Token);						\
      yylval = (Value);						\
      yytoken = YYTRANSLATE (yychar);				\
      YYPOPSTACK (1);						\
      goto yybackup;						\
    }								\
  else								\
    {								\
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;							\
    }								\
while (YYID (0))


#define YYTERROR	1
#define YYERRCODE	256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#define YYRHSLOC(Rhs, K) ((Rhs)[K])
#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)				\
    do									\
      if (YYID (N))                                                    \
	{								\
	  (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;	\
	  (Current).first_column = YYRHSLOC (Rhs, 1).first_column;	\
	  (Current).last_line    = YYRHSLOC (Rhs, N).last_line;		\
	  (Current).last_column  = YYRHSLOC (Rhs, N).last_column;	\
	}								\
      else								\
	{								\
	  (Current).first_line   = (Current).last_line   =		\
	    YYRHSLOC (Rhs, 0).last_line;				\
	  (Current).first_column = (Current).last_column =		\
	    YYRHSLOC (Rhs, 0).last_column;				\
	}								\
    while (YYID (0))
#endif


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if YYLTYPE_IS_TRIVIAL
#  define YY_LOCATION_PRINT(File, Loc)			\
     fprintf (File, "%d.%d-%d.%d",			\
	      (Loc).first_line, (Loc).first_column,	\
	      (Loc).last_line,  (Loc).last_column)
# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


/* YYLEX -- calling `yylex' with the right arguments.  */

#ifdef YYLEX_PARAM
# define YYLEX yylex (YYLEX_PARAM)
#else
# define YYLEX yylex ()
#endif

/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)			\
do {						\
  if (yydebug)					\
    YYFPRINTF Args;				\
} while (YYID (0))

# define YY_SYMBOL_PRINT(Title, Type, Value, Location)			  \
do {									  \
  if (yydebug)								  \
    {									  \
      YYFPRINTF (stderr, "%s ", Title);					  \
      yy_symbol_print (stderr,						  \
		  Type, Value); \
      YYFPRINTF (stderr, "\n");						  \
    }									  \
} while (YYID (0))


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_value_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# else
  YYUSE (yyoutput);
# endif
  switch (yytype)
    {
      default:
	break;
    }
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
#else
static void
yy_symbol_print (yyoutput, yytype, yyvaluep)
    FILE *yyoutput;
    int yytype;
    YYSTYPE const * const yyvaluep;
#endif
{
  if (yytype < YYNTOKENS)
    YYFPRINTF (yyoutput, "token %s (", yytname[yytype]);
  else
    YYFPRINTF (yyoutput, "nterm %s (", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
#else
static void
yy_stack_print (yybottom, yytop)
    yytype_int16 *yybottom;
    yytype_int16 *yytop;
#endif
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)				\
do {								\
  if (yydebug)							\
    yy_stack_print ((Bottom), (Top));				\
} while (YYID (0))


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yy_reduce_print (YYSTYPE *yyvsp, int yyrule)
#else
static void
yy_reduce_print (yyvsp, yyrule)
    YYSTYPE *yyvsp;
    int yyrule;
#endif
{
  int yynrhs = yyr2[yyrule];
  int yyi;
  unsigned long int yylno = yyrline[yyrule];
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
	     yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr, yyrhs[yyprhs[yyrule] + yyi],
		       &(yyvsp[(yyi + 1) - (yynrhs)])
		       		       );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)		\
do {					\
  if (yydebug)				\
    yy_reduce_print (yyvsp, Rule); \
} while (YYID (0))

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef	YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif



#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static YYSIZE_T
yystrlen (const char *yystr)
#else
static YYSIZE_T
yystrlen (yystr)
    const char *yystr;
#endif
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static char *
yystpcpy (char *yydest, const char *yysrc)
#else
static char *
yystpcpy (yydest, yysrc)
    char *yydest;
    const char *yysrc;
#endif
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
	switch (*++yyp)
	  {
	  case '\'':
	  case ',':
	    goto do_not_strip_quotes;

	  case '\\':
	    if (*++yyp != '\\')
	      goto do_not_strip_quotes;
	    /* Fall through.  */
	  default:
	    if (yyres)
	      yyres[yyn] = *yyp;
	    yyn++;
	    break;

	  case '"':
	    if (yyres)
	      yyres[yyn] = '\0';
	    return yyn;
	  }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into YYRESULT an error message about the unexpected token
   YYCHAR while in state YYSTATE.  Return the number of bytes copied,
   including the terminating null byte.  If YYRESULT is null, do not
   copy anything; just return the number of bytes that would be
   copied.  As a special case, return 0 if an ordinary "syntax error"
   message will do.  Return YYSIZE_MAXIMUM if overflow occurs during
   size calculation.  */
static YYSIZE_T
yysyntax_error (char *yyresult, int yystate, int yychar)
{
  int yyn = yypact[yystate];

  if (! (YYPACT_NINF < yyn && yyn <= YYLAST))
    return 0;
  else
    {
      int yytype = YYTRANSLATE (yychar);
      YYSIZE_T yysize0 = yytnamerr (0, yytname[yytype]);
      YYSIZE_T yysize = yysize0;
      YYSIZE_T yysize1;
      int yysize_overflow = 0;
      enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
      char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
      int yyx;

# if 0
      /* This is so xgettext sees the translatable formats that are
	 constructed on the fly.  */
      YY_("syntax error, unexpected %s");
      YY_("syntax error, unexpected %s, expecting %s");
      YY_("syntax error, unexpected %s, expecting %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s");
      YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s");
# endif
      char *yyfmt;
      char const *yyf;
      static char const yyunexpected[] = "syntax error, unexpected %s";
      static char const yyexpecting[] = ", expecting %s";
      static char const yyor[] = " or %s";
      char yyformat[sizeof yyunexpected
		    + sizeof yyexpecting - 1
		    + ((YYERROR_VERBOSE_ARGS_MAXIMUM - 2)
		       * (sizeof yyor - 1))];
      char const *yyprefix = yyexpecting;

      /* Start YYX at -YYN if negative to avoid negative indexes in
	 YYCHECK.  */
      int yyxbegin = yyn < 0 ? -yyn : 0;

      /* Stay within bounds of both yycheck and yytname.  */
      int yychecklim = YYLAST - yyn + 1;
      int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
      int yycount = 1;

      yyarg[0] = yytname[yytype];
      yyfmt = yystpcpy (yyformat, yyunexpected);

      for (yyx = yyxbegin; yyx < yyxend; ++yyx)
	if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR)
	  {
	    if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
	      {
		yycount = 1;
		yysize = yysize0;
		yyformat[sizeof yyunexpected - 1] = '\0';
		break;
	      }
	    yyarg[yycount++] = yytname[yyx];
	    yysize1 = yysize + yytnamerr (0, yytname[yyx]);
	    yysize_overflow |= (yysize1 < yysize);
	    yysize = yysize1;
	    yyfmt = yystpcpy (yyfmt, yyprefix);
	    yyprefix = yyor;
	  }

      yyf = YY_(yyformat);
      yysize1 = yysize + yystrlen (yyf);
      yysize_overflow |= (yysize1 < yysize);
      yysize = yysize1;

      if (yysize_overflow)
	return YYSIZE_MAXIMUM;

      if (yyresult)
	{
	  /* Avoid sprintf, as that infringes on the user's name space.
	     Don't have undefined behavior even if the translation
	     produced a string with the wrong number of "%s"s.  */
	  char *yyp = yyresult;
	  int yyi = 0;
	  while ((*yyp = *yyf) != '\0')
	    {
	      if (*yyp == '%' && yyf[1] == 's' && yyi < yycount)
		{
		  yyp += yytnamerr (yyp, yyarg[yyi++]);
		  yyf += 2;
		}
	      else
		{
		  yyp++;
		  yyf++;
		}
	    }
	}
      return yysize;
    }
}
#endif /* YYERROR_VERBOSE */


/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

/*ARGSUSED*/
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
#else
static void
yydestruct (yymsg, yytype, yyvaluep)
    const char *yymsg;
    int yytype;
    YYSTYPE *yyvaluep;
#endif
{
  YYUSE (yyvaluep);

  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  switch (yytype)
    {

      default:
	break;
    }
}

/* Prevent warnings from -Wmissing-prototypes.  */
#ifdef YYPARSE_PARAM
#if defined __STDC__ || defined __cplusplus
int yyparse (void *YYPARSE_PARAM);
#else
int yyparse ();
#endif
#else /* ! YYPARSE_PARAM */
#if defined __STDC__ || defined __cplusplus
int yyparse (void);
#else
int yyparse ();
#endif
#endif /* ! YYPARSE_PARAM */


/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;

/* Number of syntax errors so far.  */
int yynerrs;



/*-------------------------.
| yyparse or yypush_parse.  |
`-------------------------*/

#ifdef YYPARSE_PARAM
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void *YYPARSE_PARAM)
#else
int
yyparse (YYPARSE_PARAM)
    void *YYPARSE_PARAM;
#endif
#else /* ! YYPARSE_PARAM */
#if (defined __STDC__ || defined __C99__FUNC__ \
     || defined __cplusplus || defined _MSC_VER)
int
yyparse (void)
#else
int
yyparse ()

#endif
#endif
{


    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       `yyss': related to states.
       `yyvs': related to semantic values.

       Refer to the stacks thru separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yytoken = 0;
  yyss = yyssa;
  yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */
  yyssp = yyss;
  yyvsp = yyvs;

  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
	/* Give user a chance to reallocate the stack.  Use copies of
	   these so that the &'s don't force the real ones into
	   memory.  */
	YYSTYPE *yyvs1 = yyvs;
	yytype_int16 *yyss1 = yyss;

	/* Each stack pointer address is followed by the size of the
	   data in use in that stack, in bytes.  This used to be a
	   conditional around just the two extra args, but that might
	   be undefined if yyoverflow is a macro.  */
	yyoverflow (YY_("memory exhausted"),
		    &yyss1, yysize * sizeof (*yyssp),
		    &yyvs1, yysize * sizeof (*yyvsp),
		    &yystacksize);

	yyss = yyss1;
	yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
	goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
	yystacksize = YYMAXDEPTH;

      {
	yytype_int16 *yyss1 = yyss;
	union yyalloc *yyptr =
	  (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
	if (! yyptr)
	  goto yyexhaustedlab;
	YYSTACK_RELOCATE (yyss_alloc, yyss);
	YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
	if (yyss1 != yyssa)
	  YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
		  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
	YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yyn == YYPACT_NINF)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = YYLEX;
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yyn == 0 || yyn == YYTABLE_NINF)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  *++yyvsp = yylval;

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     `$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:

/* Line 1455 of yacc.c  */
#line 358 "syn.y"
    {
          ajouterQuad("FIN", "vide", "vide", "vide");
          supprimerEntite((yyvsp[(2) - (13)].str));
          free((yyvsp[(2) - (13)].str));
          YYACCEPT;
      ;}
    break;

  case 5:

/* Line 1455 of yacc.c  */
#line 370 "syn.y"
    {
          yyerrok;
      ;}
    break;

  case 9:

/* Line 1455 of yacc.c  */
#line 383 "syn.y"
    {
          if (rechercheType((yyvsp[(2) - (7)].str))) {
              erreur_semantique("double declaration", (yyvsp[(2) - (7)].str));
          } else {
              if (!type_compatible(SauvType, ((ExprInfo *)(yyvsp[(6) - (7)].expr))->type)) {
                  erreur_semantique("incompatibilite de types dans declaration constante", (yyvsp[(2) - (7)].str));
                  supprimerEntite((yyvsp[(2) - (7)].str));
              } else {
                  mettreAjourCode((yyvsp[(2) - (7)].str), "const");
                  insererType((yyvsp[(2) - (7)].str), SauvType);
                  marquerConstante((yyvsp[(2) - (7)].str), 1);
                  sauvegarder_valeur_expr((yyvsp[(2) - (7)].str), (ExprInfo *)(yyvsp[(6) - (7)].expr));
              }
          }

          free((yyvsp[(2) - (7)].str));
          libererExpr((ExprInfo *)(yyvsp[(6) - (7)].expr));
      ;}
    break;

  case 10:

/* Line 1455 of yacc.c  */
#line 405 "syn.y"
    {
          if (rechercheType((yyvsp[(2) - (5)].str))) {
              erreur_semantique("double declaration", (yyvsp[(2) - (5)].str));
          } else {
              insererType((yyvsp[(2) - (5)].str), SauvType);
              mettreAjourCode((yyvsp[(2) - (5)].str), "var");
          }

          free((yyvsp[(2) - (5)].str));
      ;}
    break;

  case 11:

/* Line 1455 of yacc.c  */
#line 416 "syn.y"
    {
          if (rechercheType((yyvsp[(2) - (7)].str))) {
              erreur_semantique("double declaration", (yyvsp[(2) - (7)].str));
          } else {
              if (!type_compatible(SauvType, ((ExprInfo *)(yyvsp[(6) - (7)].expr))->type)) {
                  erreur_semantique("incompatibilite de types dans initialisation", (yyvsp[(2) - (7)].str));
                  supprimerEntite((yyvsp[(2) - (7)].str));
              } else {
                  insererType((yyvsp[(2) - (7)].str), SauvType);
                  mettreAjourCode((yyvsp[(2) - (7)].str), "var");
                  sauvegarder_valeur_expr((yyvsp[(2) - (7)].str), (ExprInfo *)(yyvsp[(6) - (7)].expr));
                  ajouterQuad(":=", ((ExprInfo *)(yyvsp[(6) - (7)].expr))->place, "vide", (yyvsp[(2) - (7)].str));
              }
          }

          free((yyvsp[(2) - (7)].str));
          libererExpr((ExprInfo *)(yyvsp[(6) - (7)].expr));
      ;}
    break;

  case 12:

/* Line 1455 of yacc.c  */
#line 435 "syn.y"
    {
          ajouterAttente((yyvsp[(2) - (6)].str));
          appliquerTypeVar();
          viderAttente();
          free((yyvsp[(2) - (6)].str));
      ;}
    break;

  case 13:

/* Line 1455 of yacc.c  */
#line 445 "syn.y"
    {
          viderAttente();
          ajouterAttente((yyvsp[(2) - (2)].str));
          free((yyvsp[(2) - (2)].str));
      ;}
    break;

  case 14:

/* Line 1455 of yacc.c  */
#line 451 "syn.y"
    {
          ajouterAttente((yyvsp[(3) - (3)].str));
          free((yyvsp[(3) - (3)].str));
      ;}
    break;

  case 15:

/* Line 1455 of yacc.c  */
#line 459 "syn.y"
    {
          if (rechercheType((yyvsp[(2) - (9)].str))) {
              erreur_semantique("double declaration", (yyvsp[(2) - (9)].str));
          } else {
              if ((yyvsp[(7) - (9)].entier) <= 0) {
                  erreur_semantique("taille tableau positive requise", (yyvsp[(2) - (9)].str));
                  supprimerEntite((yyvsp[(2) - (9)].str));
              } else {
                  insererType((yyvsp[(2) - (9)].str), SauvType);
                  insererTaille((yyvsp[(2) - (9)].str), (yyvsp[(7) - (9)].entier));
                  mettreAjourCode((yyvsp[(2) - (9)].str), "tab");
                  marquerTableau((yyvsp[(2) - (9)].str), 1);
              }
          }

          free((yyvsp[(2) - (9)].str));
      ;}
    break;

  case 16:

/* Line 1455 of yacc.c  */
#line 480 "syn.y"
    {
          strcpy(SauvType, "integer");
          free((yyvsp[(1) - (1)].str));
      ;}
    break;

  case 17:

/* Line 1455 of yacc.c  */
#line 485 "syn.y"
    {
          strcpy(SauvType, "float");
          free((yyvsp[(1) - (1)].str));
      ;}
    break;

  case 20:

/* Line 1455 of yacc.c  */
#line 495 "syn.y"
    {
          yyerrok;
      ;}
    break;

  case 27:

/* Line 1455 of yacc.c  */
#line 511 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 28:

/* Line 1455 of yacc.c  */
#line 515 "syn.y"
    {
          int ok;

          ok = (nb_err_sem == (yyvsp[(3) - (5)].idx));

          if (!verifierVariableSimple((yyvsp[(1) - (5)].str)))
              ok = 0;

          if (!verifierConstanteModifiable((yyvsp[(1) - (5)].str)))
              ok = 0;

          if (!verifierTypeAffectation((yyvsp[(1) - (5)].str), (ExprInfo *)(yyvsp[(4) - (5)].expr)))
              ok = 0;

          if (!expr_valide((ExprInfo *)(yyvsp[(4) - (5)].expr)))
              ok = 0;

          if (ok) {
              ajouterQuad(":=", ((ExprInfo *)(yyvsp[(4) - (5)].expr))->place, "vide", (yyvsp[(1) - (5)].str));
              sauvegarder_valeur_expr((yyvsp[(1) - (5)].str), (ExprInfo *)(yyvsp[(4) - (5)].expr));
          }

          free((yyvsp[(1) - (5)].str));
          libererExpr((ExprInfo *)(yyvsp[(4) - (5)].expr));
      ;}
    break;

  case 29:

/* Line 1455 of yacc.c  */
#line 541 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 30:

/* Line 1455 of yacc.c  */
#line 545 "syn.y"
    {
          int ok;

          ok = (nb_err_sem == (yyvsp[(3) - (8)].idx));

          if (!verifierTableauIndice((yyvsp[(1) - (8)].str), (ExprInfo *)(yyvsp[(4) - (8)].expr)))
              ok = 0;

          if (!verifierConstanteModifiable((yyvsp[(1) - (8)].str)))
              ok = 0;

          if (!verifierTypeAffectation((yyvsp[(1) - (8)].str), (ExprInfo *)(yyvsp[(7) - (8)].expr)))
              ok = 0;

          if (!expr_valide((ExprInfo *)(yyvsp[(4) - (8)].expr)) || !expr_valide((ExprInfo *)(yyvsp[(7) - (8)].expr)))
              ok = 0;

          if (ok) {
              ajouterQuad("WRITETAB",
                          ((ExprInfo *)(yyvsp[(7) - (8)].expr))->place,
                          ((ExprInfo *)(yyvsp[(4) - (8)].expr))->place,
                          (yyvsp[(1) - (8)].str));
          }

          free((yyvsp[(1) - (8)].str));
          libererExpr((ExprInfo *)(yyvsp[(4) - (8)].expr));
          libererExpr((ExprInfo *)(yyvsp[(7) - (8)].expr));
      ;}
    break;

  case 31:

/* Line 1455 of yacc.c  */
#line 577 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 32:

/* Line 1455 of yacc.c  */
#line 581 "syn.y"
    {
          int ok;

          ok = (nb_err_sem == (yyvsp[(3) - (7)].idx));

          if (strcmp(((ExprInfo *)(yyvsp[(4) - (7)].expr))->type, "integer") != 0) {
              erreur_semantique("condition if invalide", "if");
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)(yyvsp[(4) - (7)].expr)))
              ok = 0;

          if (ok)
              (yyval.idx) = debut_if((ExprInfo *)(yyvsp[(4) - (7)].expr));
          else
              (yyval.idx) = -1;

          libererExpr((ExprInfo *)(yyvsp[(4) - (7)].expr));
      ;}
    break;

  case 33:

/* Line 1455 of yacc.c  */
#line 605 "syn.y"
    {
          if ((yyvsp[(1) - (6)].idx) >= 0)
              fin_if_sans_else((yyvsp[(1) - (6)].idx));
      ;}
    break;

  case 34:

/* Line 1455 of yacc.c  */
#line 610 "syn.y"
    {
          if ((yyvsp[(1) - (5)].idx) >= 0)
              (yyval.idx) = partie_else((yyvsp[(1) - (5)].idx));
          else
              (yyval.idx) = -1;
      ;}
    break;

  case 35:

/* Line 1455 of yacc.c  */
#line 617 "syn.y"
    {
          if ((yyvsp[(6) - (11)].idx) >= 0)
              fin_if((yyvsp[(6) - (11)].idx));
      ;}
    break;

  case 36:

/* Line 1455 of yacc.c  */
#line 625 "syn.y"
    {
          (yyval.idx) = debut_while();
      ;}
    break;

  case 37:

/* Line 1455 of yacc.c  */
#line 629 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 38:

/* Line 1455 of yacc.c  */
#line 633 "syn.y"
    {
          int idx_debut;
          int idx_bz;
          int ok;

          idx_debut = (yyvsp[(3) - (7)].idx);
          ok = (nb_err_sem == (yyvsp[(5) - (7)].idx));

          if (strcmp(((ExprInfo *)(yyvsp[(6) - (7)].expr))->type, "integer") != 0) {
              erreur_semantique("condition while invalide", "while");
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)(yyvsp[(6) - (7)].expr)))
              ok = 0;

          if (ok) {
              idx_bz = conditionWhile((ExprInfo *)(yyvsp[(6) - (7)].expr));
              (yyval.idx) = idx_debut * 10000 + idx_bz;
          } else {
              (yyval.idx) = -1;
          }

          libererExpr((ExprInfo *)(yyvsp[(6) - (7)].expr));
      ;}
    break;

  case 39:

/* Line 1455 of yacc.c  */
#line 662 "syn.y"
    {
          int idx_debut;
          int idx_bz;

          if ((yyvsp[(1) - (6)].idx) >= 0) {
              idx_debut = (yyvsp[(1) - (6)].idx) / 10000;
              idx_bz    = (yyvsp[(1) - (6)].idx) % 10000;
              fin_while(idx_debut, idx_bz);
          }
      ;}
    break;

  case 40:

/* Line 1455 of yacc.c  */
#line 676 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 41:

/* Line 1455 of yacc.c  */
#line 680 "syn.y"
    {
          int ok;

          ok = (nb_err_sem == (yyvsp[(4) - (7)].idx));

          if (!verifierVariableSimple((yyvsp[(2) - (7)].str)))
              ok = 0;

          if (!verifierConstanteModifiable((yyvsp[(2) - (7)].str)))
              ok = 0;

          if (strcmp(getType((yyvsp[(2) - (7)].str)), "integer") != 0) {
              erreur_semantique("variable du for non entiere", (yyvsp[(2) - (7)].str));
              ok = 0;
          }

          if (strcmp(((ExprInfo *)(yyvsp[(5) - (7)].expr))->type, "integer") != 0 ||
              strcmp(((ExprInfo *)(yyvsp[(7) - (7)].expr))->type, "integer") != 0) {
              erreur_semantique("bornes du for non entieres", (yyvsp[(2) - (7)].str));
              ok = 0;
          }

          if (!expr_valide((ExprInfo *)(yyvsp[(5) - (7)].expr)) || !expr_valide((ExprInfo *)(yyvsp[(7) - (7)].expr)))
              ok = 0;

          if (ok) {
              (yyval.idx) = debut_for((yyvsp[(2) - (7)].str), (ExprInfo *)(yyvsp[(5) - (7)].expr), (ExprInfo *)(yyvsp[(7) - (7)].expr));
              insererValeur((yyvsp[(2) - (7)].str), "");
          } else {
              (yyval.idx) = -1;
          }

          free((yyvsp[(2) - (7)].str));
          libererExpr((ExprInfo *)(yyvsp[(5) - (7)].expr));
          libererExpr((ExprInfo *)(yyvsp[(7) - (7)].expr));
      ;}
    break;

  case 42:

/* Line 1455 of yacc.c  */
#line 720 "syn.y"
    {
          if ((yyvsp[(1) - (6)].idx) >= 0 && (yyvsp[(1) - (6)].idx) < qc)
              fin_for((yyvsp[(1) - (6)].idx), quad[(yyvsp[(1) - (6)].idx)].op2);
      ;}
    break;

  case 43:

/* Line 1455 of yacc.c  */
#line 728 "syn.y"
    {
          ajouterQuad("BEGIN_OUT", "vide", "vide", "vide");
      ;}
    break;

  case 44:

/* Line 1455 of yacc.c  */
#line 732 "syn.y"
    {
          ajouterQuad("END_OUT", "vide", "vide", "vide");
      ;}
    break;

  case 45:

/* Line 1455 of yacc.c  */
#line 739 "syn.y"
    {
          if (verifierVariableSimple((yyvsp[(3) - (5)].str)) &&
              verifierConstanteModifiable((yyvsp[(3) - (5)].str))) {
              ajouterQuad("IN", "vide", "vide", (yyvsp[(3) - (5)].str));
              insererValeur((yyvsp[(3) - (5)].str), "");
          }

          free((yyvsp[(3) - (5)].str));
      ;}
    break;

  case 48:

/* Line 1455 of yacc.c  */
#line 756 "syn.y"
    {
          (yyval.idx) = nb_err_sem;
      ;}
    break;

  case 49:

/* Line 1455 of yacc.c  */
#line 760 "syn.y"
    {
          if (nb_err_sem == (yyvsp[(1) - (2)].idx) && expr_valide((ExprInfo *)(yyvsp[(2) - (2)].expr)))
              ajouterQuad("OUT", ((ExprInfo *)(yyvsp[(2) - (2)].expr))->place, "vide", "vide");

          libererExpr((ExprInfo *)(yyvsp[(2) - (2)].expr));
      ;}
    break;

  case 50:

/* Line 1455 of yacc.c  */
#line 767 "syn.y"
    {
          ajouterQuad("OUT", (yyvsp[(1) - (1)].str), "vide", "vide");
          free((yyvsp[(1) - (1)].str));
      ;}
    break;

  case 51:

/* Line 1455 of yacc.c  */
#line 775 "syn.y"
    {
          (yyval.expr) = (yyvsp[(1) - (1)].expr);
      ;}
    break;

  case 52:

/* Line 1455 of yacc.c  */
#line 782 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_arith((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), '+');
      ;}
    break;

  case 53:

/* Line 1455 of yacc.c  */
#line 786 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_arith((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), '-');
      ;}
    break;

  case 54:

/* Line 1455 of yacc.c  */
#line 790 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_arith((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), '*');
      ;}
    break;

  case 55:

/* Line 1455 of yacc.c  */
#line 794 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_arith((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), '/');
      ;}
    break;

  case 56:

/* Line 1455 of yacc.c  */
#line 798 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "<", 1);
      ;}
    break;

  case 57:

/* Line 1455 of yacc.c  */
#line 802 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), ">", 2);
      ;}
    break;

  case 58:

/* Line 1455 of yacc.c  */
#line 806 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "<=", 3);
      ;}
    break;

  case 59:

/* Line 1455 of yacc.c  */
#line 810 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), ">=", 4);
      ;}
    break;

  case 60:

/* Line 1455 of yacc.c  */
#line 814 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "==", 5);
      ;}
    break;

  case 61:

/* Line 1455 of yacc.c  */
#line 818 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_comp((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "!=", 6);
      ;}
    break;

  case 62:

/* Line 1455 of yacc.c  */
#line 822 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_logique((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "AND", 1);
      ;}
    break;

  case 63:

/* Line 1455 of yacc.c  */
#line 826 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_logique((ExprInfo *)(yyvsp[(1) - (3)].expr), (ExprInfo *)(yyvsp[(3) - (3)].expr), "OR", 2);
      ;}
    break;

  case 64:

/* Line 1455 of yacc.c  */
#line 830 "syn.y"
    {
          (yyval.expr) = (void *)traiter_expr_non((ExprInfo *)(yyvsp[(3) - (4)].expr));
      ;}
    break;

  case 65:

/* Line 1455 of yacc.c  */
#line 834 "syn.y"
    {
          (yyval.expr) = (void *)traiter_moins_unaire((ExprInfo *)(yyvsp[(2) - (2)].expr));
      ;}
    break;

  case 66:

/* Line 1455 of yacc.c  */
#line 838 "syn.y"
    {
          (yyval.expr) = (yyvsp[(2) - (3)].expr);
      ;}
    break;

  case 67:

/* Line 1455 of yacc.c  */
#line 842 "syn.y"
    {
          char place[64];
          ExprInfo *e;

          if (verifierVariableSimple((yyvsp[(1) - (1)].str))) {
              strncpy(place, (yyvsp[(1) - (1)].str), 63);
              place[63] = '\0';

              e = creerExpr(getType((yyvsp[(1) - (1)].str)), place, "BZ", 0, 0, 0, 0.0f);
              lire_valeur_ts((yyvsp[(1) - (1)].str), e);
          } else {
              e = creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
          }

          (yyval.expr) = (void *)e;
          free((yyvsp[(1) - (1)].str));
      ;}
    break;

  case 68:

/* Line 1455 of yacc.c  */
#line 860 "syn.y"
    {
          char temp[32];
          ExprInfo *e;

          if (verifierTableauIndice((yyvsp[(1) - (4)].str), (ExprInfo *)(yyvsp[(3) - (4)].expr))) {
              nouveauTemp(temp);
              ajouterQuad("READTAB", (yyvsp[(1) - (4)].str), ((ExprInfo *)(yyvsp[(3) - (4)].expr))->place, temp);
              e = creerExpr(getType((yyvsp[(1) - (4)].str)), temp, "BZ", 0, 0, 0, 0.0f);
          } else {
              e = creerExpr("", "", "BZ", 0, 0, 0, 0.0f);
          }

          (yyval.expr) = (void *)e;
          free((yyvsp[(1) - (4)].str));
          libererExpr((ExprInfo *)(yyvsp[(3) - (4)].expr));
      ;}
    break;

  case 69:

/* Line 1455 of yacc.c  */
#line 877 "syn.y"
    {
          char place[64];
          sprintf(place, "%d", (yyvsp[(1) - (1)].entier));
          (yyval.expr) = (void *)creerExpr("integer", place, "BZ", 1, 1, (yyvsp[(1) - (1)].entier), (float)(yyvsp[(1) - (1)].entier));
      ;}
    break;

  case 70:

/* Line 1455 of yacc.c  */
#line 883 "syn.y"
    {
          char place[64];
          sprintf(place, "%g", (double)(yyvsp[(1) - (1)].reel));
          (yyval.expr) = (void *)creerExpr("float", place, "BZ", 1, 0, 0, (yyvsp[(1) - (1)].reel));
      ;}
    break;

  case 71:

/* Line 1455 of yacc.c  */
#line 892 "syn.y"
    {
          char place[64];
          sprintf(place, "%d", (yyvsp[(1) - (1)].entier));
          (yyval.expr) = (void *)creerExpr("integer", place, "BZ", 1, 1, (yyvsp[(1) - (1)].entier), (float)(yyvsp[(1) - (1)].entier));
      ;}
    break;

  case 72:

/* Line 1455 of yacc.c  */
#line 898 "syn.y"
    {
          char place[64];
          sprintf(place, "%g", (double)(yyvsp[(1) - (1)].reel));
          (yyval.expr) = (void *)creerExpr("float", place, "BZ", 1, 0, 0, (yyvsp[(1) - (1)].reel));
      ;}
    break;



/* Line 1455 of yacc.c  */
#line 2612 "syn.tab.c"
      default: break;
    }
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now `shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*------------------------------------.
| yyerrlab -- here on detecting error |
`------------------------------------*/
yyerrlab:
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
      {
	YYSIZE_T yysize = yysyntax_error (0, yystate, yychar);
	if (yymsg_alloc < yysize && yymsg_alloc < YYSTACK_ALLOC_MAXIMUM)
	  {
	    YYSIZE_T yyalloc = 2 * yysize;
	    if (! (yysize <= yyalloc && yyalloc <= YYSTACK_ALLOC_MAXIMUM))
	      yyalloc = YYSTACK_ALLOC_MAXIMUM;
	    if (yymsg != yymsgbuf)
	      YYSTACK_FREE (yymsg);
	    yymsg = (char *) YYSTACK_ALLOC (yyalloc);
	    if (yymsg)
	      yymsg_alloc = yyalloc;
	    else
	      {
		yymsg = yymsgbuf;
		yymsg_alloc = sizeof yymsgbuf;
	      }
	  }

	if (0 < yysize && yysize <= yymsg_alloc)
	  {
	    (void) yysyntax_error (yymsg, yystate, yychar);
	    yyerror (yymsg);
	  }
	else
	  {
	    yyerror (YY_("syntax error"));
	    if (yysize != 0)
	      goto yyexhaustedlab;
	  }
      }
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
	 error, discard it.  */

      if (yychar <= YYEOF)
	{
	  /* Return failure if at end of input.  */
	  if (yychar == YYEOF)
	    YYABORT;
	}
      else
	{
	  yydestruct ("Error: discarding",
		      yytoken, &yylval);
	  yychar = YYEMPTY;
	}
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule which action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;	/* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (yyn != YYPACT_NINF)
	{
	  yyn += YYTERROR;
	  if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
	    {
	      yyn = yytable[yyn];
	      if (0 < yyn)
		break;
	    }
	}

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
	YYABORT;


      yydestruct ("Error: popping",
		  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  *++yyvsp = yylval;


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined(yyoverflow) || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
     yydestruct ("Cleanup: discarding lookahead",
		 yytoken, &yylval);
  /* Do not reclaim the symbols of the rule which action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
		  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  /* Make sure YYID is used.  */
  return YYID (yyresult);
}



/* Line 1675 of yacc.c  */
#line 905 "syn.y"


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
