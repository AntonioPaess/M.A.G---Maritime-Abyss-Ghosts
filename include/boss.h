#ifndef BOSS_H
#define BOSS_H

#include "player.h"
#include "projeteis.h"
#include "listaEncadeada.h"
#include "globals.h"

typedef struct
{
    int x;
    int y;
    int vida;
    int ativo;
    const char *forma;
    double ultimoAtaque;
    int estadoMovimento;
    int frameCounter;
    ProjetilBoss projeteis[4];
} Boss;

// Variáveis globais
extern Boss boss;


// Funções
void verificarSpawnBoss(double tempoAtual, int pontuacao);
void atacarBoss(double tempoAtual);
void moverProjeteisBoss();
void moverBossQuadrado();
void spawnInimigosBoss();

#endif // BOSS_H