#!/usr/bin/env bash
# usage: utils/shutdown.sh

set -eo pipefail
kubectl delete statefulset sensornode --ignore-not-found=true
k3d cluster stop sensornet
docker image prune -f
docker builder prune -f

