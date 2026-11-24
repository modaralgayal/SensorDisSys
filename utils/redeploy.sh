#!/usr/bin/env bash
# usage: utils/redeploy.sh <rings> <nodes per ring> [--no-logs]
set -eo pipefail
cd "$(dirname "$0")/.."

docker build -t sensordissys-server:latest -f Dockerfile.server .
k3d image import sensordissys-server:latest -c sensornet

python3 utils/generate_topology.py "$1" "$2" | 
kubectl create configmap swarm-topology --from-file=topology.txt=/dev/stdin \
 -o yaml --dry-run=client | kubectl apply -f -

kubectl delete statefulset sensornode --ignore-not-found=true --cascade=foreground
kubectl apply -f statefulset.yaml
kubectl scale statefulset sensornode --replicas=$(( $1 * $2 ))
kubectl rollout status statefulset sensornode
[[ "${3:-}" == "--no-logs" ]] || kubectl logs -l app=sensornode --prefix -f --max-log-requests=100

# remember to chmod +x utils/redeploy.sh