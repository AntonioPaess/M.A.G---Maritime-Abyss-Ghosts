// globals.h
#ifndef GLOBALS_H
#define GLOBALS_H

#include "listaEncadeada.h"
#include "screen.h"

// Definições de constantes do jogo
#define FRAME_TIME 16667      // 16.67ms para 60 FPS
#define MAP_WIDTH (MAXX - MINX + 1)
#define MAP_HEIGHT (MAXY - MINY + 1)
#define DANO_POR_VIDA 10
#define BOSS_SPAWN_TIME 60.0
#define BOSS_SPAWN_SCORE 1700
#define BOSS_VIDA 1500
#define BOSS_ATTACK_INTERVAL 2.0

// Declaração de variáveis globais
extern int pontuacao;
extern int gameOver;
extern double tempoDecorrido;
extern char nomeJogador[50];
extern int y;
extern int youWin;
extern int spawnInimigosAtivo;
extern char lastDir;

// Outros externs necessários

#endif // GLOBALS_H