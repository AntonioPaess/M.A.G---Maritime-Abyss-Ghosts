#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>
#include "player.h"   
#include "projeteis.h" 
#include "globals.h"

void obterDiretorioExecutavel(char *diretorio, size_t tamanho);
void salvarPontuacao(char *nome, double tempo, int pontuacao);
void reiniciarJogo();
void atualizarTela(Objeto *obj, Machado *machado, double tempoDecorrido);

#endif // UTIL_H