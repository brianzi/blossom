"""
Make a random input file for blossom2.c, run and check return value
"""
import numpy as np

import subprocess


def random_bipartite(n=10):

    pairs = {}
    for i in range(n):
        pairs[frozenset([i, i + n])] = np.random.randint(20)
    for i in range(n):
        for _ in range(5):
            j = np.random.randint(n)
            pairs[frozenset([i, j + n])] = np.random.randint(20)

    return pairs.items()


def random_non_bipartite(n=10):
    pairs = {}
    for i in range(n):
        for _ in range(5):
            j = i
            while j == i:
                j = np.random.randint(n)
            pairs[frozenset([i, j])] = np.random.randint(10)
    return pairs.items()


def make_file(fn="rnd.dat", n=8):
    with open(fn, "w") as f:
        pairs = random_non_bipartite(n)
        for s, w in pairs:
            print(*tuple(s), w, file=f)


if __name__ == "__main__":
    for i in range(200):
        
        make_file(n=10)
        p = subprocess.run(["./a.out", "rnd.dat"], timeout=2, stdout=subprocess.DEVNULL)

        if p.returncode != 0:
            print("failed!")
            break
        print(i)
