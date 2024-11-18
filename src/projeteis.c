#include "projeteis.h"
#include "player.h"
#include "inimigo.h"
#include "boss.h"
#include <stdlib.h>

Machado machado = {0, 0, 0, ' ', 0, 0};

void iniciarMovimentoMachado()
{
    char dir;
    switch (lastDir)
    {
    case 'w':
        dir = 'w';
        break;
    case 's':
        dir = 's';
        break;
    case 'a':
        dir = 'a';
        break;
    case 'd':
        dir = 'd';
        break;
    default:
        dir = 'd';
    }
    machado.ativo = 1;
    machado.direcao = dir;
    machado.distancia = 8;
    machado.moveCounter = 0;
    machado.x = obj.x;
    machado.y = obj.y;
}

void moverMachado()
{
    if (machado.ativo)
    {
        machado.moveCounter++;
        if (machado.moveCounter % 3 == 0)
        {
            switch (machado.direcao)
            {
            case 'w':
                machado.y--;
                break;
            case 's':
                machado.y++;
                break;
            case 'a':
                machado.x--;
                break;
            case 'd':
                machado.x++;
                break;
            }
            machado.distancia--;
            if (machado.distancia <= 0 || machado.x < MINX || machado.x > MAXX || machado.y < MINY || machado.y > MAXY)
            {
                machado.ativo = 0;
            }
        }
    }
}

void moverMachadoEAtacar()
{
    if (machado.ativo)
    {
        moverMachado();

        if (boss.ativo && machado.x == boss.x && machado.y == boss.y)
        {
            machado.ativo = 0;
            boss.vida -= 100;
            if (boss.vida <= 0)
            {
                boss.ativo = 0;
                pontuacao += 500;
                youWin = 1;
            }
            return;
        }

        Node *temp = inimigos;
        while (temp != NULL)
        {
            Inimigo *inimigo = (Inimigo *)temp->data;
            if (machado.ativo && inimigo->ativo && machado.x == inimigo->x && machado.y == inimigo->y)
            {
                machado.ativo = 0;
                inimigo->vida -= 100;
                if (inimigo->vida <= 0)
                {
                    inimigo->ativo = 0;
                    pontuacao += 100;
                }
                break;
            }
            temp = temp->next;
        }
    }
}