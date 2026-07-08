#ifndef CODEGEN_H
#define CODEGEN_H

#include "quad.h"
#include "ts.h"

/*
    Génération du code assembleur 8086.

    Retour :
    1 -> génération réussie
    0 -> erreur pendant la génération
*/
int genererCode8086(const char *nomFichier, const char *nomProg);

#endif