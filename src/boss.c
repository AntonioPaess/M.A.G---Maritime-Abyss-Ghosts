#include "boss.h"
#include "inimigo.h"
#include "screen.h"
#include <stdlib.h>
#include <stdio.h>
#include "keyboard.h"

Boss boss = {0, 0, 1500, 0, "🐉", 0.0, 0, 0, {{0}}};
int youWin = 0;
Porta portaBoss = {false, 0, 0};
bool portaJaUsada = false;

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

void verificarSpawnPorta(double tempoDecorrido, int pontuacao) {
    if (!portaBoss.ativo && !portaJaUsada && tempoDecorrido >= 10.0 && pontuacao >= 100) {
        portaBoss.ativo = true;
        portaBoss.x = MAP_WIDTH / 2;
        portaBoss.y = MAP_HEIGHT / 2;
        
        // Desativa spawn de inimigos
        spawnInimigosPermitido = false;
        
        // Remove todos inimigos existentes
        while (inimigos != NULL) {
            Node *temp = inimigos;
            inimigos = inimigos->next;
            free(temp->data);
            free(temp);
        }
        inimigos = NULL;
    }
}

bool mostrarDialogoBoss(void) {
    int escolha = 0;
    bool dialogoAtivo = true;

    while (dialogoAtivo) {
        // Desenha caixa de diálogo
        screenClear();
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 - 2);
        printf("╔═════════════════════════════╗");
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 - 1);
        printf("║      Portal do Leviatã      ║");
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2);
        printf("║                             ║");
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 1);
        printf("║  %s Enfrentar Leviatã        ║", escolha == 0 ? ">" : " ");
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 2);
        printf("║  %s Continuar Sobrevivendo   ║", escolha == 1 ? ">" : " ");
        screenGotoxy(MAP_WIDTH / 2 - 15, MAP_HEIGHT / 2 + 3);
        printf("╚═════════════════════════════╝");
        fflush(stdout); // Certifica que o diálogo é exibido

        // Aguarda entrada do usuário
        char input = getchar();

        switch (input) {
            case '\033': // Tecla especial (setas)
                getchar(); // Ignora '['
                switch (getchar()) {
                    case 'A': // Seta para cima
                        if (escolha > 0)
                            escolha--;
                        break;
                    case 'B': // Seta para baixo
                        if (escolha < 1)
                            escolha++;
                        break;
                }
                break;
            case '\n': // Tecla Enter
                portaBoss.ativo = false; // Porta desaparece
                portaJaUsada = true;     // Marca que a porta já foi usada
                dialogoAtivo = false;    // Fecha o diálogo

                if (escolha == 0) {
                    return true; // Enfrentar o Leviatã
                } else {
                    spawnInimigosPermitido = true; // Reativa o spawn dos inimigos
                    // Move o jogador para fora da porta
                    if (obj.x > 0) obj.x -= 1;
                    return false; // Continua sobrevivendo
                }
                break;
            default:
                // Ignora outras entradas
                break;
        }
    }
    return false;
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

void iniciarBossFight(void) {
    boss.ativo = 1;
    boss.x = 40;
    boss.y = 12;
    boss.vida = 1500;
    boss.ultimoAtaque = 0.0;

    // Teleporta o jogador
    obj.x = 2;
    obj.y = 2;
    
    // Limpa inimigos existentes
    while (inimigos != NULL) {
        Node *temp = inimigos;
        inimigos = inimigos->next;
        free(temp->data);
        free(temp);
    }

    // Desativa a porta
    portaBoss.ativo = false;
}