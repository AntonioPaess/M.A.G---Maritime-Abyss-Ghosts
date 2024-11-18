#ifndef INIMIGO_H
#define INIMIGO_H

#include "listaEncadeada.h"
#include "player.h"
#include "globals.h"

typedef struct
{
    int x;
    int y;
    int vida;
    int ativo;         // Indica se o inimigo está ativo
    const char *forma; // Forma do inimigo
} Inimigo;

// Variáveis globais relacionadas aos inimigos
extern Node *inimigos;


// Funções
void initSpawnPositions();
void freeSpawnPositions();
void getRandomSpawnPosition(int *x, int *y);
Inimigo *criarInimigo();
void moverInimigo(Inimigo *inimigo, Objeto *obj);
void adicionarInimigo(Node **lista, Inimigo *inimigo);
int contarInimigosAtivos(Node *lista);
void liberarInimigos(Node **lista);
void duplicarInimigos(double tempoAtual);

#endif // INIMIGO_H