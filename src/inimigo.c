#include "inimigo.h"
#include "listaEncadeada.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>


// Variáveis globais
Node *spawnPositions = NULL;
Node *inimigos = NULL;
int spawnInimigosAtivo = 1;

// Implementação das funções

void initSpawnPositions()
{
    spawnPositions = criarLista();
    for (int x = MINX; x <= MAXX; x++)
    {
        for (int y = MINY; y <= MAXY; y++)
        {
            int *pos = (int *)malloc(2 * sizeof(int));
            pos[0] = x;
            pos[1] = y;
            inserirFim(&spawnPositions, pos);
        }
    }
}

void freeSpawnPositions()
{
    liberarLista(&spawnPositions);
}

void getRandomSpawnPosition(int *x, int *y)
{
    int count = 0;
    Node *temp = spawnPositions;
    while (temp != NULL)
    {
        count++;
        temp = temp->next;
    }
    if (count == 0)
        return;
    int randomIndex = rand() % count;
    temp = spawnPositions;
    for (int i = 0; i < randomIndex; i++)
    {
        temp = temp->next;
    }
    int *pos = (int *)temp->data;
    *x = pos[0];
    *y = pos[1];
}

Inimigo *criarInimigo()
{
    Inimigo *novoInimigo = (Inimigo *)malloc(sizeof(Inimigo));
    getRandomSpawnPosition(&novoInimigo->x, &novoInimigo->y);
    novoInimigo->vida = 100;
    novoInimigo->ativo = 1;

    const char *formasInimigos[] = {"🦈", "🐙", "🐡"};
    int numFormasInimigos = sizeof(formasInimigos) / sizeof(formasInimigos[0]);
    novoInimigo->forma = formasInimigos[rand() % numFormasInimigos];

    return novoInimigo;
}

void moverInimigo(Inimigo *inimigo, Objeto *obj)
{
    int deltaX = obj->x - inimigo->x;
    int deltaY = obj->y - inimigo->y;

    if (abs(deltaX) > abs(deltaY))
    {
        if (deltaX > 0)
        {
            inimigo->x += 1;
        }
        else if (deltaX < 0)
        {
            inimigo->x -= 1;
        }
    }
    else
    {
        if (deltaY > 0)
        {
            inimigo->y += 1;
        }
        else if (deltaY < 0)
        {
            inimigo->y -= 1;
        }
    }
}

void adicionarInimigo(Node **lista, Inimigo *inimigo)
{
    inserirFim(lista, inimigo);
}

int contarInimigosAtivos(Node *lista)
{
    int count = 0;
    Node *temp = lista;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        if (inimigo->ativo && inimigo->vida > 0)
        {
            count++;
        }
        temp = temp->next;
    }
    return count;
}

void liberarInimigos(Node **lista)
{
    Node *temp = *lista;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        free(inimigo);
        temp = temp->next;
    }
    liberarLista(lista);
}

void duplicarInimigos(double tempoAtual)
{
    if (!spawnInimigosAtivo)
        return;

    int numInimigosDesejados;

    if (tempoAtual >= 100.0)
    {
        numInimigosDesejados = 60; // Dobra o limite de inimigos para 120
    }
    else if (tempoAtual >= 20.0)
    {
        numInimigosDesejados = 16;
    }
    else if (tempoAtual >= 12.0)
    {
        numInimigosDesejados = 16;
    }
    else if (tempoAtual >= 6.5)
    {
        numInimigosDesejados = 8;
    }
    else if (tempoAtual >= 4.0)
    {
        numInimigosDesejados = 4;
    }
    else if (tempoAtual >= 2.5)
    {
        numInimigosDesejados = 2;
    }
    else
    {
        numInimigosDesejados = 1;
    }

    int numInimigosAtivos = contarInimigosAtivos(inimigos);

    // Limitar o número máximo de inimigos a 42, exceto se o tempo for maior ou igual a 100 segundos
    if (tempoAtual < 100.0 && numInimigosDesejados > 60)
    {
        numInimigosDesejados = 60;
    }

    if (numInimigosAtivos < numInimigosDesejados && spawnInimigosAtivo)
    {
        int inimigosParaAdicionar = numInimigosDesejados - numInimigosAtivos;

        for (int i = 0; i < inimigosParaAdicionar; i++)
        {
            if (!spawnInimigosAtivo)
                break;

            Inimigo *novoInimigo = criarInimigo();
            adicionarInimigo(&inimigos, novoInimigo);
        }
    }
}