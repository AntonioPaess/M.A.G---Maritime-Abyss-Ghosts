#ifndef DROPS_H
#define DROPS_H

#include <stdbool.h>
#include "player.h"

// Configurações do drop
#define MAX_DROPS 8
#define CHANCE_DROP 100 // Chance em porcentagem de dropar um coração
#define TEMPO_VIDA_DROP 15.0f // Tempo que o drop permanece ativo
#define DROP_VIDA 1
#define DROP_SHIELD 2
#define SHIELD_DURABILITY 4

// Estrutura do drop
typedef struct {
    int x;
    int y;
    bool ativo;
    float tempoVida;
    int tipo;        // Tipo do drop (vida ou shield)
} Drop;

// Adicionar controle do shield
typedef struct {
    bool ativo;
    int durabilidade;    // Número de hits que pode aguentar
    bool shields[4];     // Estado de cada shield (N,S,L,O)
} Shield;

// Declaração externa do array drops
extern Drop drops[MAX_DROPS];
extern Shield playerShield;

// Funções do drop
void inicializarDrops(void);
void tentarCriarDrop(int x, int y);
void atualizarDrops(float deltaTime);
void verificarColetaDrops(Objeto *jogador);
void desenharDrops(void);

// Adicionar nova função
void ativarShield(void);
void reduzirShield(void);

#endif