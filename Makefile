bin=HttpSever
cc=g++
LD_FLAGS=-std=c++11 -lpthread
src=main.cpp
cur=$(shell pwd) # 获取当前工作路径

.PHONY:ALL
ALL:$(bin) cgi

$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)

cgi:
	cd ./CGI;\
	make;\
	cd -

.PHONY:clean
clean:
	rm -rf $(bin) 
	rm -rf output

.PHONY:output # 发布软件
output:
	mkdir -p output
	mv $(bin) output
	cp -rf wwwroot/ output/
	cp -rf CGI/cgi output/wwwroot/
	cp -rf CGI/mysql_cgi output/wwwroot/