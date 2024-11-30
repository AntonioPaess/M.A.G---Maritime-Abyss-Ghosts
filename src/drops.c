#include <stdlib.h>
#include <time.h>
#include "drops.h"
#include "globals.h"
#include "screen.h"

// Definição global do array drops
Drop drops[MAX_DROPS];

void inicializarDrops(void) {
    for (int i = 0; i < MAX_DROPS; i++) {
        drops[i].ativo = false;
    }
    srand(time(NULL)); // Inicializa a semente do rand()
}

void tentarCriarDrop(int x, int y) {
    // Determina se um drop será criado com base na chance
    if (rand() % 100 < CHANCE_DROP) {
        // Procura por um slot disponível
        for (int i = 0; i < MAX_DROPS; i++) {
            if (!drops[i].ativo) {
                drops[i].x = x;
                drops[i].y = y;
                drops[i].ativo = true;
                drops[i].tempoVida = TEMPO_VIDA_DROP;
                break;
            }
        }
    }
}

void atualizarDrops(float deltaTime) {
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].ativo) {
            drops[i].tempoVida -= deltaTime;
            if (drops[i].tempoVida <= 0) {
                drops[i].ativo = false;
            }
        }
    }
}

void verificarColetaDrops(Objeto *jogador) {
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].ativo) {
            // Verifica colisão simples (ajuste conforme necessário)
            if (abs(jogador->x - drops[i].x) <= 1 && abs(jogador->y - drops[i].y) <= 1) {
                // Verifica se o jogador não está com vida máxima
                int vidasAtuais = jogador->vidas - (jogador->dano / DANO_POR_VIDA);
                if (vidasAtuais < jogador->vidas) {
                    // Recupera uma vida (reduz o dano)
                    jogador->dano -= DANO_POR_VIDA;
                    if (jogador->dano < 0) {
                        jogador->dano = 0;
                    }
                }
                drops[i].ativo = false; // Remove o drop
            }
        }
    }
}

void desenharDrops(void) {
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].ativo) {
            screenGotoxy(drops[i].x, drops[i].y);
            printf("\033[91m♥\033[0m"); // Coração vermelho
        }
    }
}