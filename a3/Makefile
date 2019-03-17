FLAGS= -Wall -g -std=gnu99

psort : psort.o helper.o
	gcc ${FLAGS} -o $@ $^

%.o : %.c helper.h
	gcc -c -o $@ $< 

.PHONY: clean

clean: 
	rm psort *.o

