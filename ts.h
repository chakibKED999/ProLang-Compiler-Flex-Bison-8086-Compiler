#ifndef TS_H
#define TS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 101

typedef struct Noeud {
    char NomEntite[15];
    char CodeEntite[10];
    char TypeEntite[10];
    char ValeurEntite[20];
    int  TailleTableau;
    int  EstConst;
    int  EstTableau;
    struct Noeud *suivant;
} Noeud;

#ifndef TS_DEFINE
extern Noeud *table[HASH_SIZE];
extern int CpTS;
#endif

unsigned int hacher(const char *s);
void inserer(const char *entite, const char *code);
void supprimerEntite(const char *entite);
Noeud *trouver(const char *entite);

int rechercheType(const char *entite);
const char *getType(const char *entite);
const char *getCode(const char *entite);
const char *getValeur(const char *entite);

void insererType(const char *entite, const char *type);
void insererValeur(const char *entite, const char *valeur);
void insererTaille(const char *entite, int taille);
void mettreAjourCode(const char *entite, const char *code);

void marquerConstante(const char *entite, int v);
void marquerTableau(const char *entite, int v);

int estConstante(const char *entite);
int estTableau(const char *entite);
int getTaille(const char *entite);

void afficher(void);
void liberer(void);

#ifdef TS_DEFINE

Noeud *table[HASH_SIZE] = { NULL };
int CpTS = 0;

unsigned int hacher(const char *s)
{
    unsigned int h = 5381;

    if (s == NULL)
        return 0;

    while (*s) {
        h = (h * 33) + (unsigned char)(*s);
        s++;
    }

    return h % HASH_SIZE;
}

Noeud *trouver(const char *entite)
{
    unsigned int idx;
    Noeud *n;

    if (entite == NULL)
        return NULL;

    idx = hacher(entite);
    n = table[idx];

    while (n != NULL) {
        if (strcmp(n->NomEntite, entite) == 0)
            return n;
        n = n->suivant;
    }

    return NULL;
}

void inserer(const char *entite, const char *code)
{
    unsigned int idx;
    Noeud *nouveau;

    if (entite == NULL || code == NULL)
        return;

    if (trouver(entite) != NULL)
        return;

    idx = hacher(entite);

    nouveau = (Noeud *)malloc(sizeof(Noeud));
    if (nouveau == NULL) {
        fprintf(stderr, "Erreur memoire TS.\n");
        exit(1);
    }

    strncpy(nouveau->NomEntite, entite, 14);
    nouveau->NomEntite[14] = '\0';

    strncpy(nouveau->CodeEntite, code, 9);
    nouveau->CodeEntite[9] = '\0';

    strcpy(nouveau->TypeEntite, "");
    strcpy(nouveau->ValeurEntite, "");
    nouveau->TailleTableau = 0;
    nouveau->EstConst = 0;
    nouveau->EstTableau = 0;

    nouveau->suivant = table[idx];
    table[idx] = nouveau;
    CpTS++;
}

void supprimerEntite(const char *entite)
{
    unsigned int idx;
    Noeud *n;
    Noeud *prec;

    if (entite == NULL)
        return;

    idx = hacher(entite);
    n = table[idx];
    prec = NULL;

    while (n != NULL) {
        if (strcmp(n->NomEntite, entite) == 0) {
            if (prec == NULL)
                table[idx] = n->suivant;
            else
                prec->suivant = n->suivant;

            free(n);
            CpTS--;
            return;
        }

        prec = n;
        n = n->suivant;
    }
}

int rechercheType(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    return (n != NULL && strlen(n->TypeEntite) > 0);
}

const char *getType(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->TypeEntite;

    return "";
}

const char *getCode(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->CodeEntite;

    return "";
}

const char *getValeur(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->ValeurEntite;

    return "";
}

void insererType(const char *entite, const char *type)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL && type != NULL) {
        strncpy(n->TypeEntite, type, 9);
        n->TypeEntite[9] = '\0';
    }
}

void insererValeur(const char *entite, const char *valeur)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL && valeur != NULL) {
        strncpy(n->ValeurEntite, valeur, 19);
        n->ValeurEntite[19] = '\0';
    }
}

void insererTaille(const char *entite, int taille)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        n->TailleTableau = taille;
}

void mettreAjourCode(const char *entite, const char *code)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL && code != NULL) {
        strncpy(n->CodeEntite, code, 9);
        n->CodeEntite[9] = '\0';
    }
}

void marquerConstante(const char *entite, int v)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        n->EstConst = v;
}

void marquerTableau(const char *entite, int v)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        n->EstTableau = v;
}

int estConstante(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->EstConst;

    return 0;
}

int estTableau(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->EstTableau;

    return 0;
}

int getTaille(const char *entite)
{
    Noeud *n;

    n = trouver(entite);
    if (n != NULL)
        return n->TailleTableau;

    return 0;
}

void afficher(void)
{
    int i;
    Noeud *n;

    printf("\n/*************** Table des symboles ***************/\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("| %-14s | %-8s | %-8s | %-12s | %-6s | %-7s | %-6s |\n",
           "NomEntite", "Code", "Type", "Valeur", "Const", "Tableau", "Taille");
    printf("-------------------------------------------------------------------------------------------------\n");

    for (i = 0; i < HASH_SIZE; i++) {
        n = table[i];
        while (n != NULL) {
            printf("| %-14s | %-8s | %-8s | %-12s | %-6s | %-7s | %-6d |\n",
                   n->NomEntite,
                   n->CodeEntite,
                   n->TypeEntite,
                   n->ValeurEntite,
                   n->EstConst ? "Oui" : "Non",
                   n->EstTableau ? "Oui" : "Non",
                   n->EstTableau ? n->TailleTableau : 0);
            n = n->suivant;
        }
    }

    printf("-------------------------------------------------------------------------------------------------\n");
}

void liberer(void)
{
    int i;
    Noeud *n;
    Noeud *tmp;

    for (i = 0; i < HASH_SIZE; i++) {
        n = table[i];
        while (n != NULL) {
            tmp = n->suivant;
            free(n);
            n = tmp;
        }
        table[i] = NULL;
    }

    CpTS = 0;
}

#endif
#endif