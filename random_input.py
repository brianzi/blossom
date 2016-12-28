"""
Make a random input file for blossom_alt.c
"""
import numpy as np


def random_bipartite(n=10):

    pairs = [(i, i + n, np.random.randint(20)) for i in range(n)]
    pairs += [(i, np.random.randint(n) + n, np.random.randint(20))
              for i in range(n) for j in range(5)]

    return pairs


def random_non_bipartite(n=10):
    pairs = {} 
    for i in range(n):
        for _ in range(5):
            j = i
            while j == i:
                j = np.random.randint(n)
            pairs[frozenset([i, j])] =  np.random.randint(20)
    return pairs.items()


# pairs = random_bipartite()
pairs = random_non_bipartite()


for s, w in pairs:
    print(*tuple(s), w)
