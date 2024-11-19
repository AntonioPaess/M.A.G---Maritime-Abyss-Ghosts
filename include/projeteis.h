#ifndef PROJETEIS_H
#define PROJETEIS_H

#include "globals.h"

typedef struct
{
    int x;
    int y;
    int ativo;
    char direcao;
    int distancia;
    int moveCounter;
} ProjetilBoss;

typedef struct
{
    int x;
    int y;
    int ativo;
    char direcao;
    int distancia;
    int moveCounter;
} Machado;

// Variáveis globais
extern Machado machado;

// Funções
void iniciarMovimentoMachado();
void moverMachado();
void moverMachadoEAtacar();

#endif // PROJETEIS_H