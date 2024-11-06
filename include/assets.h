/**
 * assets.h
 * Created on Nov, 6th 2024
 * Author: Antônio Paes
 * Based on "From C to C++ course - 2002"
*/


#ifndef ASSETS_H
#define ASSETS_H

typedef struct {
    char nome[50];
    char tipo[20]; // Exemplo: "imagem", "som", etc.
} Asset;

typedef struct {
    int id;
    char nome[50];
    Asset* assets;
    int assetCount;
    int assetCapacity;
} Entidade;

Entidade* criarEntidade(int id, const char* nome);
void adicionarAsset(Entidade* entidade, const char* nome, const char* tipo);
void removerAsset(Entidade* entidade, const char* nome);
void listarAssets(Entidade* entidade);
void liberarEntidade(Entidade* entidade);

#endif // ASSETS_H

