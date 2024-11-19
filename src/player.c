#include "player.h"
#include <stdlib.h>

char lastDir = 'd';
Objeto obj = {40, 12, 3, 0};

void moverObjeto(Objeto *obj, char direcao)
{
    switch (direcao)
    {
    case 'w':
        obj->y -= 1;
        break;
    case 'a':
        obj->x -= 1;
        break;
    case 's':
        obj->y += 1;
        break;
    case 'd':
        obj->x += 1;
        break;
    }
    lastDir = direcao;
}

void aplicarDano(Objeto *obj, int danoRecebido)
{
    obj->dano += danoRecebido;

    int vidasPerdidas = obj->dano / DANO_POR_VIDA;
    int vidasRestantes = obj->vidas - vidasPerdidas;

    if (vidasRestantes <= 0)
    {
        // O jogador perdeu todas as vidas
        gameOver = 1; // Sinaliza que o jogo acabou
    }
}