#!/bin/bash

export DOCKER_DEFAULT_PLATFORM=linux/amd64
buildnumber=`cat ../buildnumber`
curl -LO http://dev.shushik.kiev.ua/files/Dockerfile
curl -LO http://dev.shushik.kiev.ua/init-files.sh
curl -LO http://dev.shushik.kiev.ua/gobit.yaml
url=`cat ../github`
sed -i 's+_GITHUB_+'"$url"'+g' Dockerfile
docker build . -t stormstack/bshcms:$buildnumber 
docker --config ../.docker push stormstack/bshcms:$buildnumber 
docker rmi stormstack/bshcms:$buildnumber
sed -i 's+buildnumber+'"$buildnumber"'+g' gobit.yaml
TOKEN=`cat ../token`
curl -k -H "Authorization: Bearer $TOKEN" https://185.151.244.169:6443
kubectl apply -f gobit.yaml
