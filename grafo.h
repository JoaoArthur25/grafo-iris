#ifndef GRAFO_H
#define GRAFO_H

#include <stdio.h>

typedef struct grafo Grafo;

Grafo *grafo_carregar_base(const char *arquivo);

Grafo *grafo_carregar_salvo(const char *arquivo);

int grafo_salvar(Grafo *g, const char *arquivo);
void grafo_liberar(Grafo *g);

int grafo_grau(Grafo *g, int v);
int grafo_num_arestas(Grafo *g);
int grafo_componentes(Grafo *g, int *tamanhos);
void grafo_imprimir_cabecalho(Grafo *g, FILE *saida);

#endif
