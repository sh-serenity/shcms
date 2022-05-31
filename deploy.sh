#!/bin/bash

buildnumber=`cat ../buildnumber`
wget -O ./Dockerfile http://dev.shushik.kiev.ua/d.sh 2>&1>/build/log
url=`cat ../github`
sed -i 's+_GIT_HUB_+$url+g' Dockerfile
docker build . -t stormstack/bshcms:$buildnumber 2>&1>/build/log
docker push stormstack/bshcms:$buildnumber 2>&1>/build/log
docer rmi stormstack/bshcms:$buildnumber 2>&1>/build/log
