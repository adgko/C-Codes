FLAGS_GCC = -std=gnu11 -Wall -Werror -pedantic -Wextra -Wconversion -g
FLAGS_OTHER = -lulfius -ljansson
FLAGS_CPP = --enable=performance,portability,information,unusedFunction -q

make:
	gcc src/users.c src/funciones.c -o bin/users $(FLAGS_GCC) $(FLAGS_OTHER)
	gcc src/goes.c src/funciones.c -o bin/goes $(FLAGS_GCC) $(FLAGS_OTHER)

check:
	cppcheck $(FLAGS_CPP) ./
	
clean:
	rm bin/*
	rm -r html/ latex/	
	
docs:
	doxygen Doxyfile

install:
	sudo useradd so-users && sudo useradd so-goes
	sudo cp ./bin/users /usr/bin/server_users && sudo cp ./bin/goes /usr/bin/server_goes
	sudo cp ./install/*.service /lib/systemd/system/ && sudo cp ./install/*.service /etc/systemd/system/
	sudo chown so-users /usr/bin/server_users && sudo chown so-goes /usr/bin/server_goes
	sudo touch /var/log/users.log && sudo touch /var/log/goes.log
	sudo mkdir /var/archivos
	sudo chown so-users /var/log/users.log && sudo chown so-goes /var/log/goes.log && chown so-goes /var/archivos
	systemctl daemon-reload 
	sudo cp ./install/so2-tp3.conf /etc/nginx/sites-available/
	sudo nginx -s reload
	sudo systemctl start server_users && sudo systemctl start server_goes
	sudo systemctl enable server_users && sudo systemctl enable server_goes
	sudo echo "so-users ALL = NOPASSWD:/usr/sbin/useradd" >> /etc/sudoers
