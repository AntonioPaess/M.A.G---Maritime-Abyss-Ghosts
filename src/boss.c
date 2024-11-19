#include "boss.h"
#include "inimigo.h"
#include <stdlib.h>

Boss boss = {0, 0, 1500, 0, "🐉", 0.0, 0, 0, {{0}}};
int youWin = 0;

void verificarSpawnBoss(double tempoAtual, int pontuacao)
{
    if (!boss.ativo && tempoAtual >= 60.0 && pontuacao >= 1700)
    {
        boss.ativo = 1;
        boss.x = 40;
        boss.y = 12;
        boss.vida = 1500;
        boss.ultimoAtaque = tempoAtual;

        // Teleporta o jogador
        obj.x = 2;
        obj.y = 2;

        // Desativa spawn de novos inimigos
        spawnInimigosAtivo = 0;

        // Destroi todos os inimigos existentes
        Node *temp = inimigos;
        while (temp != NULL)
        {
            Inimigo *inimigo = (Inimigo *)temp->data;
            inimigo->ativo = 0;
            temp = temp->next;
        }

        // Spawna inimigos
        spawnInimigosBoss();
    }
}

void atacarBoss(double tempoAtual)
{
    if (!boss.ativo)
        return;

    if (tempoAtual - boss.ultimoAtaque >= 2.0)
    {
        char direcoes[4] = {'w', 's', 'a', 'd'};

        for (int i = 0; i < 4; i++)
        {
            boss.projeteis[i].ativo = 1;
            boss.projeteis[i].x = boss.x;
            boss.projeteis[i].y = boss.y;
            boss.projeteis[i].direcao = direcoes[i];
            boss.projeteis[i].distancia = 70;
            boss.projeteis[i].moveCounter = 0;
        }

        boss.ultimoAtaque = tempoAtual;
    }
}

void moverProjeteisBoss()
{
    for (int i = 0; i < 4; i++)
    {
        if (boss.projeteis[i].ativo)
        {
            boss.projeteis[i].moveCounter++;
            if (boss.projeteis[i].moveCounter % 2 == 0)
            {
                switch (boss.projeteis[i].direcao)
                {
                case 'w':
                    boss.projeteis[i].y--;
                    break;
                case 's':
                    boss.projeteis[i].y++;
                    break;
                case 'a':
                    boss.projeteis[i].x--;
                    break;
                case 'd':
                    boss.projeteis[i].x++;
                    break;
                }
                boss.projeteis[i].distancia--;

                if (boss.projeteis[i].x == obj.x && boss.projeteis[i].y == obj.y)
                {
                    aplicarDano(&obj, 10);
                    boss.projeteis[i].ativo = 0;
                }

                if (boss.projeteis[i].distancia <= 0)
                {
                    boss.projeteis[i].ativo = 0;
                }
            }
        }
    }
}

void moverBossQuadrado()
{
    if (!boss.ativo)
        return;

    boss.frameCounter++;
    if (boss.frameCounter % 8 != 0)
        return;

    int centroX = 40;
    int centroY = 12;
    int larguraRetangulo = 20;
    int alturaRetangulo = 6;

    switch (boss.estadoMovimento)
    {
    case 0:
        boss.x++;
        if (boss.x >= centroX + larguraRetangulo)
        {
            boss.estadoMovimento = 1;
        }
        break;
    case 1:
        boss.y++;
        if (boss.y >= centroY + alturaRetangulo)
        {
            boss.estadoMovimento = 2;
        }
        break;
    case 2:
        boss.x--;
        if (boss.x <= centroX - larguraRetangulo)
        {
            boss.estadoMovimento = 3;
        }
        break;
    case 3:
        boss.y--;
        if (boss.y <= centroY - alturaRetangulo)
        {
            boss.estadoMovimento = 0;
        }
        break;
    }
}

void spawnInimigosBoss()
{
    for (int i = 0; i < 4; i++)
    {
        Inimigo *novoInimigo = criarInimigo();
        adicionarInimigo(&inimigos, novoInimigo);
    }
}