FLAGS_GCC = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -g -lzip -lcrypto -lssl
FLAGS_CPP = --enable=performance,portability,information,unusedFunction -q

all: check cliente server productores

clean:
	rm -r bin/*
	rm -r html/ latex/
	make clean_logs
	rm archivos/logs/
	
clean_logs:
	echo " " > archivos/logs/productor_1.log
	echo " " > archivos/logs/productor_2.log
	echo " " > archivos/logs/productor_3.log
	echo " " > archivos/logs/cli.log
	
check:
	cppcheck $(FLAGS_CPP) ./
	
cliente:
	gcc  src/sock_client.c src/funciones1.c -o bin/client $(FLAGS_GCC)
	
server:
	#gcc  src/sock_server.c src/funciones1.c src/funciones2.c -o bin/DeliveryManager $(FLAGS_GCC)
	gcc  src/cli.c src/funciones1.c -o bin/cli $(FLAGS_GCC)
	gcc  src/sock_server_02.c src/funciones1.c src/funciones2.c  -o bin/DeliveryManager $(FLAGS_GCC)
	
productores:
	gcc src/productor1.c src/funciones1.c -o bin/prod1 $(FLAGS_GCC)
	gcc src/productor2.c src/funciones1.c -o bin/prod2 $(FLAGS_GCC)
	gcc src/productor3.c src/funciones1.c -o bin/prod3 $(FLAGS_GCC)
	gcc src/file.c	src/funciones1.c -o bin/file $(FLAGS_GCC)
	gcc src/downloader.c	src/funciones1.c -o bin/downloader $(FLAGS_GCC)
