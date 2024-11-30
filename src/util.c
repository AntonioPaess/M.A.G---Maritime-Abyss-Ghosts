#include "util.h"
#include "inimigo.h"
#include "boss.h"
#include "projeteis.h"
#include "player.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <mach-o/dyld.h>
#include <libgen.h>
#include "drops.h"

void obterDiretorioExecutavel(char *diretorio, size_t tamanho)
{
    char path[PATH_MAX];
    uint32_t size = sizeof(path);

    if (_NSGetExecutablePath(path, &size) != 0)
    {
        fprintf(stderr, "Buffer muito pequeno. Necessário tamanho: %u\n", size);
        exit(EXIT_FAILURE);
    }

    char *dir = dirname(path);
    strncpy(diretorio, dir, tamanho - 1);
    diretorio[tamanho - 1] = '\0';
}

void salvarPontuacao(char *nome, double tempo, int pontuacao)
{
    char diretorio[PATH_MAX];
    obterDiretorioExecutavel(diretorio, sizeof(diretorio));

    char caminhoScores[PATH_MAX];
    snprintf(caminhoScores, sizeof(caminhoScores), "%s/scores.txt", diretorio);

    FILE *arquivo = fopen(caminhoScores, "a");
    if (arquivo != NULL)
    {
        fprintf(arquivo, "%s,%.1f,%d\n", nome, tempo, pontuacao);
        fclose(arquivo);
    }
    else
    {
        perror("Erro ao abrir o arquivo para salvar a pontuação");
    }
}

void reiniciarJogo(void)
{
    obj.x = 40;
    obj.y = 12;
    obj.vidas = 3;
    obj.dano = 0;

    pontuacao = 0;
    tempoDecorrido = 0.0;

    spawnInimigosAtivo = 1;

    liberarInimigos(&inimigos);
    inimigos = NULL;

    machado.ativo = 0;
    machado.x = 0;
    machado.y = 0;
    machado.distancia = 0;
    machado.moveCounter = 0;

    boss.ativo = 0;
    boss.x = 0;
    boss.y = 0;
    boss.vida = 1500;
    boss.ultimoAtaque = 0.0;

    for (int i = 0; i < 4; i++)
    {
        boss.projeteis[i].ativo = 0;
    }

    freeSpawnPositions();
    initSpawnPositions();
    
    inicializarDrops(); // Reseta todos os drops
    screenClear();      // Limpa a tela
}

void atualizarTela(Objeto *obj, Machado *machado, double tempoDecorrido)
{
    screenClear();

    // Desenha o contorno do mapa
    for (int x = 0; x <= MAP_WIDTH + 1; x++)
    {
        screenGotoxy(x, 0);
        printf("═");
        screenGotoxy(x, MAP_HEIGHT + 1);
        printf("═");
    }
    for (int y = 0; y <= MAP_HEIGHT + 1; y++)
    {
        screenGotoxy(0, y);
        printf("║");
        screenGotoxy(MAP_WIDTH + 1, y);
        printf("║");
    }
    screenGotoxy(0, 0);
    printf("╔");
    screenGotoxy(MAP_WIDTH + 1, 0);
    printf("╗");
    screenGotoxy(0, MAP_HEIGHT + 1);
    printf("╚");
    screenGotoxy(MAP_WIDTH + 1, MAP_HEIGHT + 1);
    printf("╝");

    // Desenha o jogador
    screenGotoxy(obj->x + 1, obj->y + 1);
    printf("🐟");

    // Desenha o shield ao redor do jogador se estiver ativo
    if (playerShield.ativo) {
        // Norte
        screenGotoxy(obj->x + 1, obj->y);
        printf(" 🛡️");
        // Sul
        screenGotoxy(obj->x + 1, obj->y + 2);
        printf(" 🛡️");
        // Leste
        screenGotoxy(obj->x + 2, obj->y + 1);
        printf("🛡️");
        // Oeste
        screenGotoxy(obj->x, obj->y + 1);
        printf("🛡️ 🐟🛡️");
    }

    // Desenha os drops
    for (int i = 0; i < MAX_DROPS; i++) {
        if (drops[i].ativo) {
            screenGotoxy(drops[i].x + 1, drops[i].y + 1);
            // Desenha emoji diferente baseado no tipo do drop
            if (drops[i].tipo == DROP_SHIELD) {
                printf("🛡️");
            } else {
                printf("❤️");
            }
        }
    }

    // Desenha os inimigos
    Node *temp = inimigos;
    while (temp != NULL)
    {
        Inimigo *inimigo = (Inimigo *)temp->data;
        if (inimigo->ativo && inimigo->vida > 0)
        {
            screenGotoxy(inimigo->x + 1, inimigo->y + 1);
            printf("%s", inimigo->forma);
        }
        temp = temp->next;
    }

    // Após desenhar inimigos e antes do boss
    if (portaBoss.ativo) {
        screenGotoxy(portaBoss.x + 1, portaBoss.y + 1);
        printf("🚪");
    }

    // Desenha o boss e seus projéteis
    if (boss.ativo)
    {
        screenGotoxy(boss.x + 1, boss.y + 1);
        printf("%s", boss.forma);

        // Desenha os projéteis do boss
        for (int i = 0; i < 4; i++)
        {
            if (boss.projeteis[i].ativo)
            {
                screenGotoxy(boss.projeteis[i].x + 1, boss.projeteis[i].y + 1);
                printf("(🔥)");
            }
        }

        // Exibe vida do boss
        screenGotoxy(MAP_WIDTH - 50, 1);
        printf("Leviatã: %d/500", boss.vida);
    }

    // Desenha o machado se estiver ativo
    if (machado->ativo)
    {
        screenGotoxy(machado->x + 1, machado->y + 1);
        printf("💦");
    }

    // Exibe a pontuação no canto superior esquerdo
    screenGotoxy(2, 1);
    printf(" Pontuação: %d ", pontuacao);

    // Exibe o cronômetro no canto superior direito
    screenGotoxy(MAP_WIDTH - 20, 1);
    printf(" Tempo: %.1fs ", tempoDecorrido);

    // Exibe a barra de vidas no canto inferior esquerdo
    screenGotoxy(2, MAP_HEIGHT + 2);
    printf(" Vidas:  ");

    int maxVidas = obj->vidas;
    int vidasPerdidas = obj->dano / DANO_POR_VIDA;
    int vidasAtuais = maxVidas - vidasPerdidas;

    for (int i = 0; i < maxVidas; i++)
    {
        if (i < vidasAtuais)
        {
            printf("❤️ ");
        }
        else
        {
            printf("   ");
        }
    }

    fflush(stdout);
}
