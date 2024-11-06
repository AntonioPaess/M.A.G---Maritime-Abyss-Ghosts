/**
 * listaEncadeada.c
 * Created on Nov, 6th 2024
 * Author: Antônio Paes
 * Based on "From C to C++ course - 2002"
*/

#include "listaEncadeada.h"
#include <stdio.h>
#include <stdlib.h>

Node* criarLista() {
    return NULL;
}

void inserirInicio(Node** head, void* data) {
    Node* novoNode = (Node*)malloc(sizeof(Node));
    novoNode->data = data;
    novoNode->next = *head;
    *head = novoNode;
}

void inserirFim(Node** head, void* data) {
    Node* novoNode = (Node*)malloc(sizeof(Node));
    novoNode->data = data;
    novoNode->next = NULL;
    if (*head == NULL) {
        *head = novoNode;
    } else {
        Node* temp = *head;
        while (temp->next != NULL) {
            temp = temp->next;
        }
        temp->next = novoNode;
    }
}

void removerElemento(Node** head, void* data, CompareFunc compare) {
    Node* temp = *head;
    Node* prev = NULL;
    while (temp != NULL && compare(temp->data, data) != 0) {
        prev = temp;
        temp = temp->next;
    }
    if (temp == NULL) return;
    if (prev == NULL) {
        *head = temp->next;
    } else {
        prev->next = temp->next;
    }
    free(temp);
}

Node* pesquisarElemento(Node* head, void* data, CompareFunc compare) {
    Node* temp = head;
    while (temp != NULL) {
        if (compare(temp->data, data) == 0) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

void ordenarLista(Node** head, CompareFunc compare) {
    if (*head == NULL) return;
    Node* i = *head;
    while (i->next != NULL) {
        Node* j = i->next;
        while (j != NULL) {
            if (compare(i->data, j->data) > 0) {
                void* temp = i->data;
                i->data = j->data;
                j->data = temp;
            }
            j = j->next;
        }
        i = i->next;
    }
}

void imprimirLista(Node* head, void (*printFunc)(void*)) {
    Node* temp = head;
    while (temp != NULL) {
        printFunc(temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}

void liberarLista(Node** head) {
    Node* temp = *head;
    while (temp != NULL) {
        Node* next = temp->next;
        free(temp);
        temp = next;
    }
    *head = NULL;
}