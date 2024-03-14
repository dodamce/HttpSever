bin=HttpSever
cgi=cgi
cc=g++
LD_FLAGS=-std=c++11 -lpthread
src=main.cpp
cur=$(shell pwd) # 获取当前工作路径

BIN:$(bin) CGI
.PHONY:BIN
BIN:
$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)
CGI:
	cd $(curr)/CGI
	make
	cd -

.PHONY:clean
clean:
	rm -rf $(bin) 
	rm -rf output

.PHONY:output # 发布软件
output:
	mkdir -p output
	cp $(bin) output
	cp -rf wwwroot/ output/
	cp CGI/ output/wwwroot/