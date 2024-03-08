#!/bin/bash
make clean
make
rm -rf ./HttpSever
rm -rf ./wwwroot/cgi
cd output
./HttpSever 8080