#####
##	$(CC) -Iinclude -Llib src/kpsoftServiceRun.c -o kpsoftServiceRun.exe -llibmariadb

CC = gcc 
RM = rm -rf

all:
	$(CC) -Iinclude -Llib src/KpenCoder.c -o bin/KpenCoder.exe
	
clean:
	$(RM) bin/KpenCoder.exe