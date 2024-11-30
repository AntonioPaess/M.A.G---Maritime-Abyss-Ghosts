#ifndef DROPS_H
#define DROPS_H

#include <stdbool.h>
#include "player.h"

// Configurações do drop
#define MAX_DROPS 5
#define CHANCE_DROP 100 // Chance em porcentagem de dropar um coração
#define TEMPO_VIDA_DROP 25.0f // Tempo que o drop permanece ativo

// Estrutura do drop
typedef struct {
    int x;
    int y;
    bool ativo;
    float tempoVida;
} Drop;

// Declaração externa do array drops
extern Drop drops[MAX_DROPS];

// Funções do drop
void inicializarDrops(void);
void tentarCriarDrop(int x, int y);
void atualizarDrops(float deltaTime);
void verificarColetaDrops(Objeto *jogador);
void desenharDrops(void);

#endif