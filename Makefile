SOURCE_FILES = main.c
PROJECT_NAME = comp

CC = gcc
CCFLAGS = -o $(PROJECT_NAME) $(SOURCE_FILES)

build:
	$(CC) $(CCFLAGS) 

run:
	$(CC) $(CCFLAGS) -fsanitize=address,undefined -g
	./$(PROJECT_NAME) ./sample/main.c

debug:
	$(CC) $(CCFLAGS) -g
	lldb $(PROJECT_NAME) -- ./sample/main.c

leaks:
	$(CC) $(CCFLAGS) -g
	leaks --atExit -- ./$(PROJECT_NAME) ./sample/main.c

BUILD_FILES = comp comp.dSYM main main.dSYM
clean:
	rm -rf $(BUILD_FILES)
