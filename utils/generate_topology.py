from abc import ABC, abstractmethod
import argparse
from functools import cached_property
import json
import sys
from itertools import combinations


class Topology(ABC):

    kind = "abstract"

    def __init__(self, f: int) -> None:
        if f < 0:
            raise ValueError("f must be non-negative")
        self.f = f


    @property
    @abstractmethod
    def size(self) -> int:
        """# of nodes in the topology"""

    @abstractmethod
    def edges(self) -> dict:
        """All undirected edges (u, v), including duplicates"""

    def params(self) -> dict:
        """Parameters recorded in the JSON"""
        return {"f": self.f, "kind": self.kind}

    @cached_property
    def adjacency(self) -> dict[int, set[int]]:
        """Adjacency list representation of the topology"""
        adj = {i: set() for i in range(self.size)}
        for u, v in self.edges().keys():
            adj[u].add(v)
            adj[v].add(u)
        return adj
        