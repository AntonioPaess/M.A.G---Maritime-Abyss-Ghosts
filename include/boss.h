#ifndef BOSS_H
#define BOSS_H

#include "player.h"
#include "projeteis.h"
#include "listaEncadeada.h"
#include "globals.h"
#include "stdbool.h"

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

typedef struct {
    bool ativo;
    int x;
    int y;
} Porta;

// Variáveis globais
extern Boss boss;
extern Porta portaBoss;
extern bool spawnInimigosPermitido;
extern bool portaJaUsada;

// Funções
void iniciarBossFight(void);
void verificarSpawnBoss(double tempoAtual, int pontuacao);
void atacarBoss(double tempoAtual);
void moverProjeteisBoss();
void moverBossQuadrado();
void spawnInimigosBoss();
void verificarSpawnPorta(double tempoDecorrido, int pontuacao);
bool mostrarDialogoBoss(void);

#endif // BOSS_H