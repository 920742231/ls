all : ls

ls : ls.o ugname.o

ls.o : ls.c

ugname.o : ugname.c

clean :
	rm *.o