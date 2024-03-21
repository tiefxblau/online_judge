oj_server:oj_server.cpp
	g++ -o $@ $^ -std=c++11 -L/usr/lib64/mysql -lpthread -lctemplate -ljsoncpp -lmysqlclient

.PHONY:clean
clean:
	rm -f oj_server