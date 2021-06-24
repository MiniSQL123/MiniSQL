main : main.o interpreter.o api.o record_manager.o buffer_manager.o catalog_manager.o index_manager.o basic.o
	g++ -o main -g main.o interpreter.o api.o record_manager.o buffer_manager.o catalog_manager.o index_manager.o basic.o
main.o: main.cpp interpreter.h catalog_manager.h basic.h buffer_manager.h exception.h const.h
	g++ -c -g main.cpp
interpreter.o: interpreter.cpp interpreter.h catalog_manager.h basic.h buffer_manager.h exception.h const.h
	g++ -c -g interpreter.cpp
api.o: api.cpp api.h buffer_manager.h record_manager.h const.h basic.h catalog_manager.h buffer_manager.h exception.h catalog_manager.h template_function.h
	g++ -c -g api.cpp
record_manager.o: record_manager.cpp record_manager.h index_manager.h catalog_manager.h buffer_manager.h bplustree.h exception.h basic.h const.h template_function.h
	g++ -c -g record_manager.cpp
buffer_manager.o: buffer_manager.cpp buffer_manager.h const.h
	g++ -c -g buffer_manager.cpp
catalog_manager.o: catalog_manager.cpp catalog_manager.h buffer_manager.h basic.h exception.h const.h
	g++ -c -g catalog_manager.cpp
index_manager.o: index_manager.cpp index_manager.h basic.h buffer_manager.h bplustree.h const.h
	g++ -c -g index_manager.cpp
basic.o: basic.cpp basic.h
	g++ -c -g basic.cpp
clear:
	rm *.o