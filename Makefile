shell: FTPserver.o FTPclient.o
	gcc FTPserver.o -o FTPserver
	gcc FTPclient.o -o FTPclient

FTPserver.o: FTPserver.c 
	gcc -c FTPserver.c

FTPclient.o: FTPclient.c 
	gcc -c FTPclient.c

clean:
	rm *.o shell

# shell: print.o read.o splitter.o checkpipe.o singlecommand.o pipe.o server.o client.o
# 	gcc print.o read.o splitter.o checkpipe.o singlecommand.o pipe.o server.o -o server
# 	gcc print.o read.o splitter.o checkpipe.o singlecommand.o pipe.o client.o -o client



# server.o: server.c server.h
# 	gcc -c server.c

# client.o: client.c client.h
# 	gcc -c client.c

# print.o: print.c print.h
# 	gcc -c print.c

# read.o: read.c read.h
# 	gcc -c read.c

# splitter.o: splitter.c splitter.h
# 	gcc -c splitter.c

# checkpipe.o: checkpipe.c checkpipe.h
# 	gcc -c checkpipe.c

# singlecommand.o: singlecommand.c singlecommand.h
# 	gcc -c singlecommand.c

# pipe.o: pipe.c pipe.h
# 	gcc -c pipe.c

# clean:
# 	rm *.o shell

