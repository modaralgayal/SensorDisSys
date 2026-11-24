import json
import sys

rings, size = int(sys.argv[1]), int(sys.argv[2])
neighbors = {i: set() for i in range(rings * size)}

for ring in range(rings):
    for k in range(size):
        i = ring * size + k
        right = ring * size + (k + 1) % size
        neighbors[i].add(right)
        neighbors[right].add(i)
        if ring + 1 < rings:
            for j in range(size):
                down = (ring + 1) * size + j
                neighbors[i].add(down)
                neighbors[down].add(i)

for i in sorted(neighbors):
    peers = [
        f"ws://sensornode-{j}.sensornode:5000"
        for j in sorted(neighbors[i])
        if j < i
    ]

    print(i, *peers)