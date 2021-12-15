TAG=./bin/chat
SRC= $(wildcard source/*.c)
OBJ=$(SRC:%.c=%.o)
CC=gcc

override CONFIG += -I./include  -L./library -lpthread 
$(TAG):$(OBJ)
	$(CC)  $(^) -o $(@) $(CONFIG)

%.o:%.c
	$(CC) $< -o $(@) $(CONFIG) -c 

clean:
	$(RM) ./bin/*  ./source/*.o

.PHONY:clean