clean:
	@rm -rf gobit

all: 
	gcc -D_GNU_SOURCE  -g gobit.c -O2 -o gobit -L /usr/lib -ltidy -lfcgi -lsodium  -ltidy -L/usr/lib/x86_64-linux-gnu -lmysqlclient -lpthread -ldl -lssl -lcrypto -lresolv -lm 

	

