#include <stdlib.h>
#include <time.h>
#include "drops.h"
#include "globals.h"
#include "screen.h"

// Definição global do array drops
Drop drops[MAX_DROPS];
Shield playerShield = {false, 0, {false, false, false, false}};

void inicializarDrops(void) {
    for (int i = 0; i < MAX_DROPS; i++) {
        drops[i].ativo = false;
    }
    srand(time(NULL)); // Inicializa a semente do rand()
}

void tentarCriarDrop(int x, int y) {
    if (rand() % 100 < CHANCE_DROP) {
        for (int i = 0; i < MAX_DROPS; i++) {
            if (!drops[i].ativo) {
                drops[i].x = x;
                drops[i].y = y;
                drops[i].ativo = true;
                drops[i].tempoVida = TEMPO_VIDA_DROP;
                drops[i].tipo = (rand() % 100 < 30) ? DROP_SHIELD : DROP_VIDA;
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
                if (drops[i].tipo == DROP_SHIELD && !playerShield.ativo) {
                    ativarShield();
                } else if (drops[i].tipo == DROP_VIDA) {
                    // Verifica se o jogador não está com vida máxima
                    int vidasAtuais = jogador->vidas - (jogador->dano / DANO_POR_VIDA);
                    if (vidasAtuais < jogador->vidas) {
                        // Recupera uma vida (reduz o dano)
                        jogador->dano -= DANO_POR_VIDA;
                        if (jogador->dano < 0) {
                            jogador->dano = 0;
                        }
                    }
                }
                drops[i].ativo = false; // Remove o drop
            }
        }
    }
}

void ativarShield(void) {
    playerShield.ativo = true;
    playerShield.durabilidade = SHIELD_DURABILITY;
    for (int i = 0; i < 4; i++) {
        playerShield.shields[i] = true;
    }
}

void reduzirShield(void) {
    if (playerShield.ativo) {
        playerShield.durabilidade--;
        if (playerShield.durabilidade <= 0) {
            playerShield.ativo = false;
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