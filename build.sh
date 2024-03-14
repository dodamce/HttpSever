#!/bin/bash
make clean
make 
make output
rm -rf ./CGI/cgi
rm -rf ./CGI/mysql_cgi
cd output
./HttpSever 8080