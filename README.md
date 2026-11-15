# SensorDisSys

Clone this repository

In order to run the cluster, run these commands:

Create the cluster:
```
k3d cluster create sensornet

kubectl get nodes 
```

Build the image and import it into the cluster:
```
docker build -t sensordissys-server:latest -f Dockerfile.server .

k3d image import sensordissys-server:latest -c sensornet
```

Deploy: 
```
kubectl apply -f statefulset.yaml

kubectl get pods -l app=sensornode -w  
```

View all logs in real time: 
```
kubectl logs -l app=sensornode --prefix -f --max-log-requests=10
```

Scale (optional, default is 3 pods):
```
kubectl scale statefulset sensornode --replicas=5
```

Update after making code changes to the node: 
```
docker build -t sensordissys-server:latest -f Dockerfile.server .

k3d image import sensordissys-server:latest -c sensornet

kubectl rollout restart statefulset sensornode
```


