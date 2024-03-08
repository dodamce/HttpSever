bin=HttpSever
cgi=cgi
cc=g++
LD_FLAGS=-std=c++11 -lpthread
src=main.cpp
cur=$(shell pwd) # 获取当前工作路径

BIN:$(bin) $(cgi)
.PHONY:BIN
BIN:
$(bin):$(src)
	$(cc) -o $@ $^ $(LD_FLAGS)
$(cgi):CGI/cgi.cpp
	$(cc) -o $@ $^
	mv $(cgi) wwwroot/
	make output

.PHONY:clean
clean:
	rm -rf $(bin) 
	rm -rf wwwroot/$(cgi)
	rm -rf output

.PHONY:output # 发布软件
output:
	mkdir -p output
	cp $(bin) output
	cp -rf wwwroot/ output/
	cp wwwroot/$(cgi) output/wwwroot/