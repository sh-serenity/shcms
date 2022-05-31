#!/bin/bash

export DOCKER_DEFAULT_PLATFORM=linux/amd64
buildnumber=`cat ../buildnumber`
curl -LO http://dev.shushik.kiev.ua/files/Dockerfile
curl -LO http://dev.shushik.kiev.ua/init-files.sh
url=`cat ../github`
sed -i 's+_GITHUB_+'"$url"'+g' Dockerfile
docker build . -t stormstack/bshcms:$buildnumber 
docker --config ../.docker push stormstack/bshcms:$buildnumber 
docker rmi stormstack/bshcms:$buildnumber
