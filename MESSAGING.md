# Node Communication Overview

Based on Algorithm 1 (Resilient Average Consensus) from Yuan & Ishii.
This covers only the messages and updates exchanged between nodes. Detection logic is left out for now.

## 1. Big picture

Nodes run in synchronous rounds. In every round each node:

1. **Sends** one message to all its out-neighbors (the same message to everyone)
2. **Receives** one message from each in-neighbor
3. **Updates** its own state from what it received
4. Prepares the message for the next round

```
   Node 1 ──── Φ1 ────▶ Node 2
      │                   │
      └───── Φ1 ────▶ Node 3
                          │
   Node 2 ──── Φ2 ────▶ Node 3
```

A node sends the **same** message Φ to every out-neighbor (the malicious model assumes this too).

## 2. Data a node keeps (local state)

| Name | Meaning |
|------|---------|
| `y`, `z` | current push-sum numerator and denominator |
| `λ` (lambda) | running sum of all y-shares this node has sent so far |
| `γ` (gamma) | running sum of all z-shares this node has sent so far |
| `δ[j]` | latest λ received from in-neighbor `j` (also `δ[i]` for itself) |
| `ω[j]` | latest γ received from in-neighbor `j` (also `ω[i]` for itself) |
| `A` | set of node IDs detected as malicious (empty without attackers) |
| `d+` | number of non-faulty out-neighbors |

Estimate of the average: `y / z`

## 3. Round 1 message (simple)

In round 1 a node only sends its first running sums:

```
Node i  ──▶  { λ_i[1], γ_i[1] }

λ_i[1] = x_i[0] / (1 + d+)
γ_i[1] = 1       / (1 + d+)
```

## 4. Message Φ from round 2 onward

```
Φ_i = {
  A_i,                          // detected malicious IDs
  (i, λ_i[next], γ_i[next]),    // own new running sums
  [ (j, δ_ij, ω_ij) ... ]       // what node i last received from each
                                //   in-neighbor j (and from itself)
}
```

| Field | Used for |
|-------|----------|
| `A_i` | sharing detection results with neighbors |
| `(i, λ, γ)` | **averaging**: the receiver takes the difference to get i's latest share |
| `(j, δ_ij, ω_ij)` list | **two-hop information**, so out-neighbors can cross-check i (detection only) |

For plain averaging, only the second field matters. The third field is only needed once detection is added.

## 5. What the receiver does with a message

Say node 2 receives `λ_1` from node 1.

```
share from node 1 = λ_1[k] − (previous λ_1 stored in δ_21)
```

That difference is node 1's latest share. Then node 2 updates:

```
y_2 = sum over trusted in-neighbors j (and itself) of ( δ_2j[k] − δ_2j[k−1] )
z_2 = same, using ω
```

If node `j` is detected as malicious, `δ_2j` is set to 0. That subtracts everything node 2 ever got from j.

After updating, node 2 builds its next running sums:

```
λ_2[k+1] = λ_2[k] + y_2 / (1 + d+)
γ_2[k+1] = γ_2[k] + z_2 / (1 + d+)
```

## 6. Tiny example

Three nodes, edges 1→2, 1→3, 2→3. Initial values: `x1 = 9`, `x2 = 7`, `x3 = 1`.

**Round 1 (sent):**

| From | To | λ | γ |
|------|----|---|---|
| Node 1 (d+ = 2) | 2 and 3 | 9/3 = 3 | 1/3 |
| Node 2 (d+ = 1) | 3 | 7/2 = 3.5 | 1/2 |
| Node 3 (d+ = 0) | none | none | none |

**Round 1 update, node 2:**
- own kept share: `λ_2[1] = 3.5`
- received from node 1: `δ_21 = 3`
- `y_2 = 3.5 + 3 = 6.5`, and similarly `z_2 = 1/2 + 1/3`
- next message carries `λ_2[2] = 3.5 + 6.5/2 = 6.75`

Every round repeats this, and `y/z` at each node moves toward the common average.

## 7. When a malicious neighbor is detected

| Case | What the node does |
|------|--------------------|
| **In-neighbor j detected** | set `δ_ij = ω_ij = 0`, which removes everything j ever sent |
| **Out-neighbor q detected** | `d+` decreases by 1, and `y += λ_i`, `z += γ_i` (takes back what q received) |

## 8. Per-round timeline

```
round k
  ├─ send    Φ_i[k−1]      → all out-neighbors
  ├─ receive Φ_j[k−1]      ← all in-neighbors
  ├─ detect  update A_i
  ├─ update  y_i, z_i      (remove / compensate if A_i changed)
  └─ compute λ_i[k+1], γ_i[k+1]  → goes into the next Φ
```