#ifndef PLAYER_H
#define PLAYER_H

#include "globals.h"

typedef struct
{
    int x;
    int y;
    int vidas;
    int dano;
} Objeto;

// Variáveis globais
extern Objeto obj;


// Funções
void moverObjeto(Objeto *obj, char direcao);
void aplicarDano(Objeto *obj, int danoRecebido);

#endif // PLAYER_H