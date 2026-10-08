#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "grafo.h"

#define LIMIAR 0.3
#define TAM_LINHA 1024

typedef struct {
    double m[4];
} Flor;

typedef struct {
    double valor;
    int i, j;
} Extremo;

struct grafo {
    int n;
    int **adj;
    Flor *flores;
    Extremo maior_de, menor_de, maior_den, menor_den;
};

static int **nova_matriz(int n)
{
    int **m = malloc(n * sizeof(int *));
    for (int i = 0; i < n; i++)
        m[i] = calloc(n, sizeof(int));
    return m;
}

static double distancia(Flor a, Flor b)
{
    double soma = 0;
    for (int k = 0; k < 4; k++)
        soma += (a.m[k] - b.m[k]) * (a.m[k] - b.m[k]);
    return sqrt(soma);
}

static void procurar_extremos(double **t, int n, Extremo *maior, Extremo *menor)
{
    *maior = (Extremo){t[0][1], 0, 1};
    *menor = *maior;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            if (t[i][j] > maior->valor)
                *maior = (Extremo){t[i][j], i, j};
            if (t[i][j] < menor->valor)
                *menor = (Extremo){t[i][j], i, j};
        }
}

Grafo *grafo_carregar_base(const char *arquivo)
{
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) {
        printf("Erro: nao foi possivel abrir %s\n", arquivo);
        return NULL;
    }

    Grafo *g = calloc(1, sizeof(Grafo));
    char linha[TAM_LINHA];
    int num_linha = 0;
    while (fgets(linha, TAM_LINHA, f) != NULL) {
        num_linha++;
        if (num_linha == 1 || linha[0] == '\n' || linha[0] == '\r')
            continue;
        Flor fl;
        if (sscanf(linha, "%*[^,],%lf,%lf,%lf,%lf",
                   &fl.m[0], &fl.m[1], &fl.m[2], &fl.m[3]) != 4) {
            printf("Erro: linha %d da base invalida\n", num_linha);
            fclose(f);
            grafo_liberar(g);
            return NULL;
        }
        g->flores = realloc(g->flores, (g->n + 1) * sizeof(Flor));
        g->flores[g->n++] = fl;
    }
    fclose(f);

    int n = g->n;
    if (n < 2) {
        printf("Erro: a base precisa de pelo menos 2 observacoes\n");
        grafo_liberar(g);
        return NULL;
    }

    double **t = malloc(n * sizeof(double *));
    for (int i = 0; i < n; i++)
        t[i] = malloc(n * sizeof(double));
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            t[i][j] = distancia(g->flores[i], g->flores[j]);
    procurar_extremos(t, n, &g->maior_de, &g->menor_de);

    double min = g->menor_de.valor;
    double amplitude = g->maior_de.valor - min;
    if (amplitude == 0)
        amplitude = 1;
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            t[i][j] = (t[i][j] - min) / amplitude;
    procurar_extremos(t, n, &g->maior_den, &g->menor_den);

    g->adj = nova_matriz(n);
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (t[i][j] <= LIMIAR)
                g->adj[i][j] = g->adj[j][i] = 1;

    for (int i = 0; i < n; i++)
        free(t[i]);
    free(t);
    return g;
}

static void imprimir_extremo(FILE *saida, const char *nome, Extremo e)
{
    fprintf(saida, "%s,%.6f,c%d,c%d\n", nome, e.valor, e.i + 1, e.j + 1);
}

void grafo_imprimir_cabecalho(Grafo *g, FILE *saida)
{
    int lacos = 0, multiplas = 0;
    for (int i = 0; i < g->n; i++) {
        lacos += g->adj[i][i];
        for (int j = i + 1; j < g->n; j++)
            if (g->adj[i][j] > 1)
                multiplas += g->adj[i][j];
    }

    int grau_max = grafo_grau(g, 0), grau_min = grau_max;
    for (int v = 1; v < g->n; v++) {
        int grau = grafo_grau(g, v);
        if (grau > grau_max)
            grau_max = grau;
        if (grau < grau_min)
            grau_min = grau;
    }

    int *tamanhos = malloc(g->n * sizeof(int));
    int num_comp = grafo_componentes(g, tamanhos);

    fprintf(saida, "total_vertices,%d\n", g->n);
    imprimir_extremo(saida, "maior_DE", g->maior_de);
    imprimir_extremo(saida, "menor_DE", g->menor_de);
    imprimir_extremo(saida, "maior_DEN", g->maior_den);
    imprimir_extremo(saida, "menor_DEN", g->menor_den);
    fprintf(saida, "grau_maximo,%d\n", grau_max);
    fprintf(saida, "grau_minimo,%d\n", grau_min);
    fprintf(saida, "tipo_grafo,%s,lacos,%d,arestas_multiplas,%d\n",
            (lacos > 0 || multiplas > 0) ? "multigrafo" : "simples", lacos, multiplas);
    fprintf(saida, "componentes_conexos,%d\n", num_comp);
    fprintf(saida, "tamanhos_componentes");
    for (int c = 0; c < num_comp; c++)
        fprintf(saida, ",%d", tamanhos[c]);
    fprintf(saida, "\n");
    fprintf(saida, "total_arestas,%d\n", grafo_num_arestas(g));
    free(tamanhos);
}

int grafo_salvar(Grafo *g, const char *arquivo)
{
    FILE *f = fopen(arquivo, "w");
    if (f == NULL) {
        printf("Erro: nao foi possivel criar %s\n", arquivo);
        return 0;
    }

    grafo_imprimir_cabecalho(g, f);

    fprintf(f, "vertices,sepal.length,sepal.width,petal.length,petal.width\n");
    for (int i = 0; i < g->n; i++) {
        Flor fl = g->flores[i];
        fprintf(f, "c%d,%g,%g,%g,%g\n", i + 1, fl.m[0], fl.m[1], fl.m[2], fl.m[3]);
    }

    fprintf(f, "matriz_adjacencia");
    for (int j = 0; j < g->n; j++)
        fprintf(f, ",c%d", j + 1);
    fprintf(f, "\n");
    for (int i = 0; i < g->n; i++) {
        fprintf(f, "c%d", i + 1);
        for (int j = 0; j < g->n; j++)
            fprintf(f, ",%d", g->adj[i][j]);
        fprintf(f, "\n");
    }

    fclose(f);
    return 1;
}

static int ler_extremo(FILE *f, Extremo *e, int n)
{
    if (fscanf(f, " %*[^,],%lf,c%d,c%d", &e->valor, &e->i, &e->j) != 3)
        return 0;
    e->i--;
    e->j--;
    return e->i >= 0 && e->i < n && e->j >= 0 && e->j < n;
}

Grafo *grafo_carregar_salvo(const char *arquivo)
{
    FILE *f = fopen(arquivo, "r");
    if (f == NULL) {
        printf("Erro: nao foi possivel abrir %s\n", arquivo);
        return NULL;
    }

    Grafo *g = calloc(1, sizeof(Grafo));
    int ok = fscanf(f, " total_vertices,%d", &g->n) == 1 && g->n > 0;
    if (ok) {
        g->flores = malloc(g->n * sizeof(Flor));
        g->adj = nova_matriz(g->n);
        ok = ler_extremo(f, &g->maior_de, g->n) && ler_extremo(f, &g->menor_de, g->n)
          && ler_extremo(f, &g->maior_den, g->n) && ler_extremo(f, &g->menor_den, g->n);
    }

    char linha[TAM_LINHA];
    while (ok && fgets(linha, TAM_LINHA, f) != NULL && strncmp(linha, "vertices", 8) != 0)
        ;

    for (int i = 0; ok && i < g->n; i++) {
        Flor *fl = &g->flores[i];
        ok = fscanf(f, " c%*d,%lf,%lf,%lf,%lf", &fl->m[0], &fl->m[1], &fl->m[2], &fl->m[3]) == 4;
    }

    if (ok && fscanf(f, " matriz_adjacencia%*[^\n]") == EOF)
        ok = 0;
    for (int i = 0; ok && i < g->n; i++) {
        ok = fscanf(f, " c%*d") != EOF;
        for (int j = 0; ok && j < g->n; j++)
            ok = fscanf(f, ",%d", &g->adj[i][j]) == 1;
    }

    fclose(f);
    if (!ok) {
        printf("Erro: %s nao e um grafo salvo valido\n", arquivo);
        grafo_liberar(g);
        return NULL;
    }
    return g;
}

void grafo_liberar(Grafo *g)
{
    if (g == NULL)
        return;
    if (g->adj != NULL)
        for (int i = 0; i < g->n; i++)
            free(g->adj[i]);
    free(g->adj);
    free(g->flores);
    free(g);
}

int grafo_grau(Grafo *g, int v)
{
    int grau = g->adj[v][v];
    for (int j = 0; j < g->n; j++)
        grau += g->adj[v][j];
    return grau;
}

int grafo_num_arestas(Grafo *g)
{
    int total = 0;
    for (int i = 0; i < g->n; i++)
        for (int j = i; j < g->n; j++)
            total += g->adj[i][j];
    return total;
}

static int visitar(Grafo *g, int v, int *visitado)
{
    int tamanho = 1;
    visitado[v] = 1;
    for (int w = 0; w < g->n; w++)
        if (g->adj[v][w] > 0 && !visitado[w])
            tamanho += visitar(g, w, visitado);
    return tamanho;
}

int grafo_componentes(Grafo *g, int *tamanhos)
{
    int *visitado = calloc(g->n, sizeof(int));
    int total = 0;
    for (int v = 0; v < g->n; v++)
        if (!visitado[v])
            tamanhos[total++] = visitar(g, v, visitado);
    free(visitado);
    return total;
}
