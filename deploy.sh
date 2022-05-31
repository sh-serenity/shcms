#!/bin/bash

buildnumber=`cat ../buildnumber`
cutl -LO http://dev.shushik.kiev.ua/files/Dockerfile 
url=`cat ../github`
sed -i 's+_GIT_HUB_+$url+g' Dockerfile
docker build . -t stormstack/bshcms:$buildnumber 
docker push stormstack/bshcms:$buildnumber 
docer rmi stormstack/bshcms:$buildnumber
