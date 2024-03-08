#!/bin/bash
make clean
make
rm -rf ./HttpSever
rm -rf ./wwwroot/cgi
./output/HttpSever 8080