#!/bin/bash

export DOCKER_DEFAULT_PLATFORM=linux/amd64
buildnumber=`cat ../buildnumber`
curl -LO http://dev.shushik.kiev.ua/files/Dockerfile
curl -LO http://dev.shushik.kiev.ua/init-files.sh
url=`cat ../github`
sed -i 's+_GITHUB_+'"$url"'+g' Dockerfile
docker build . -t stormstack/bshcms:$buildnumber 
docker docker --config ../.docker/config.json push stormstack/bshcms:$buildnumber 
docer rmi stormstack/bshcms:$buildnumber
