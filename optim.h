#ifndef OPTIM_H
#define OPTIM_H

#include "quad.h"

int propagationCopie(void);
int propagationExpression(void);
int eliminationRedondantes(void);
int simplificationAlgebrique(void);
int eliminationCodeInutile(void);

void optimiserQuadruplets(void);
void afficherQuadsOptimises(void);

#endif