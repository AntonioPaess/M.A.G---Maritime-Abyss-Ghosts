/**
 * listaEncadeada.h
 * Created on Nov, 6th 2024
 * Author: Antônio Paes
 * Based on "From C to C++ course - 2002"
*/

#ifndef LISTAENCADEADA_H
#define LISTAENCADEADA_H

typedef struct Node {
    void* data;
    struct Node* next;
} Node;

typedef int (*CompareFunc)(void*, void*);

Node* criarLista();
void inserirInicio(Node** head, void* data);
void inserirFim(Node** head, void* data);
void removerElemento(Node** head, void* data, CompareFunc compare);
Node* pesquisarElemento(Node* head, void* data, CompareFunc compare);
void ordenarLista(Node** head, CompareFunc compare);
void imprimirLista(Node* head, void (*printFunc)(void*));
void liberarLista(Node** head);

#endif // LISTAENCADEADA_H