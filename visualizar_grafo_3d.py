# Fonte: adaptado dos exemplos oficiais do Matplotlib
#   https://matplotlib.org/stable/gallery/mplot3d/scatter3d.html
#   https://matplotlib.org/stable/api/_as_gen/mpl_toolkits.mplot3d.art3d.Line3DCollection.html
import sys

import matplotlib.pyplot as plt
import numpy as np
from mpl_toolkits.mplot3d.art3d import Line3DCollection


def ler_grafo(arquivo):
    with open(arquivo) as f:
        linhas = [linha.strip().split(",") for linha in f if linha.strip()]
    n = int(linhas[0][1])
    v = next(k for k, linha in enumerate(linhas) if linha[0] == "vertices")
    medidas = np.array([linha[1:] for linha in linhas[v + 1:v + 1 + n]], dtype=float)
    m = v + 1 + n
    adj = np.array([linha[1:] for linha in linhas[m + 1:m + 1 + n]], dtype=int)
    return medidas, adj


def desenhar(medidas, adj):
    pos = medidas[:, [2, 3, 0]]
    grau = adj.sum(axis=1) + adj.diagonal()
    i, j = np.nonzero(np.triu(adj, 1))

    fig = plt.figure(figsize=(9, 7))
    ax = fig.add_subplot(projection="3d")
    if len(i) > 0:
        arestas = np.stack([pos[i], pos[j]], axis=1)
        ax.add_collection3d(Line3DCollection(arestas, colors="gray", linewidths=0.3, alpha=0.3))
    pontos = ax.scatter(pos[:, 0], pos[:, 1], pos[:, 2], c=grau, cmap="viridis", s=25)
    fig.colorbar(pontos, ax=ax, shrink=0.6, label="grau do vértice")

    ax.set_xlabel("petal.length (cm)")
    ax.set_ylabel("petal.width (cm)")
    ax.set_zlabel("sepal.length (cm)")
    ax.set_title(f"Grafo Iris: {len(adj)} vértices e {np.triu(adj).sum()} arestas (DEN ≤ 0,3)")
    return fig, ax


if __name__ == "__main__":
    arquivo = sys.argv[1] if len(sys.argv) > 1 else "grafo_iris.csv"
    desenhar(*ler_grafo(arquivo))
    plt.show()
