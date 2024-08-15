FLAGS_GCC = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion 
FLAGS_OTHER = -g -lnetcdf -lm -fopenmp
FLAGS_CPP = --enable=performance,portability,information,unusedFunction -q --suppress=missingIncludeSystem

all: check  paralelo

clean:
	rm bin/*
	rm -r html/ latex/
	
check:
	cppcheck $(FLAGS_CPP) ./  
	
serial:
	gcc  src/secuencial.c src/simple_bmp.c -o bin/secuencial $(FLAGS_GCC) $(FLAGS_OTHER)
	
paralelo:
	gcc  src/paralelo.c src/simple_bmp.c -o bin/paralelo $(FLAGS_GCC) $(FLAGS_OTHER)
	
	
docs:
	doxygen Doxyfile
	
install:
	mkdir bin
	mkdir /archivos/destino

unninstall:
	rm -r /archivos/destino
	rm -r bin/*
	rm -r html/ latex/
	
restart:
	make unninstall && make install
