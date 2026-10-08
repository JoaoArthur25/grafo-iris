#include <stdio.h>
#include <string.h>

#include "grafo.h"

int main(int argc, char *argv[])
{
    Grafo *g;
    char *saida = NULL;

    if (argc == 4 && strcmp(argv[1], "carregar") == 0) {
        g = grafo_carregar_base(argv[2]);
        saida = argv[3];
    } else if ((argc == 3 || argc == 4) && strcmp(argv[1], "recarregar") == 0) {
        g = grafo_carregar_salvo(argv[2]);
        if (argc == 4)
            saida = argv[3];
    } else {
        printf("Uso:\n");
        printf("  %s carregar <base.csv> <grafo.csv>\n", argv[0]);
        printf("  %s recarregar <grafo.csv> [copia.csv]\n", argv[0]);
        return 1;
    }

    if (g == NULL)
        return 1;

    grafo_imprimir_cabecalho(g, stdout);

    int ok = 1;
    if (saida != NULL) {
        ok = grafo_salvar(g, saida);
        if (ok)
            printf("Grafo salvo em %s\n", saida);
    }

    grafo_liberar(g);
    return ok ? 0 : 1;
}
