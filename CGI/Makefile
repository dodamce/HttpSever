# 在CGI下形成CGI程序
.PHONY:all
all:cgi mysql_cgi

cgi:cgi.cpp
	g++ -o $@ $^ -std=c++11

mysql_cgi:connect.cpp
	g++ -o $@ $^ -std=c++11 -I include -L lib -lmysqlclient -lpthread -ldl -static

.PHONY:clean

clean:
	rm -rf cgi mysql_cgi