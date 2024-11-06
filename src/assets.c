/**
 * assets.c
 * Created on Nov, 6th 2024
 * Author: Antônio Paes
 * Based on "From C to C++ course - 2002"
*/


#include "assets.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 10

Entidade* criarEntidade(int id, const char* nome) {
    Entidade* entidade = (Entidade*)malloc(sizeof(Entidade));
    entidade->id = id;
    strcpy(entidade->nome, nome);
    entidade->assets = (Asset*)malloc(INITIAL_CAPACITY * sizeof(Asset));
    entidade->assetCount = 0;
    entidade->assetCapacity = INITIAL_CAPACITY;
    return entidade;
}

void adicionarAsset(Entidade* entidade, const char* nome, const char* tipo) {
    if (entidade->assetCount == entidade->assetCapacity) {
        entidade->assetCapacity *= 2;
        entidade->assets = (Asset*)realloc(entidade->assets, entidade->assetCapacity * sizeof(Asset));
    }
    strcpy(entidade->assets[entidade->assetCount].nome, nome);
    strcpy(entidade->assets[entidade->assetCount].tipo, tipo);
    entidade->assetCount++;
}

void removerAsset(Entidade* entidade, const char* nome) {
    for (int i = 0; i < entidade->assetCount; i++) {
        if (strcmp(entidade->assets[i].nome, nome) == 0) {
            for (int j = i; j < entidade->assetCount - 1; j++) {
                entidade->assets[j] = entidade->assets[j + 1];
            }
            entidade->assetCount--;
            return;
        }
    }
}

void listarAssets(Entidade* entidade) {
    printf("Assets de %s:\n", entidade->nome);
    for (int i = 0; i < entidade->assetCount; i++) {
        printf("Nome: %s, Tipo: %s\n", entidade->assets[i].nome, entidade->assets[i].tipo);
    }
}

void liberarEntidade(Entidade* entidade) {
    free(entidade->assets);
    free(entidade);
}