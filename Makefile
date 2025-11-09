SOURCE_FILES = 						\
				src/main.c 			\
				src/target_arm.c	\
				src/helpers.c		\
				src/lexer/lexer.c	\
				src/lexer/token.c	\
				src/parser/parser.c	\
				src/parser/nodes.c	\
				src/parser/expr.c
PROJECT_NAME = comp

CC = gcc
CCFLAGS = -o $(PROJECT_NAME) $(SOURCE_FILES)

build:
	$(CC) $(CCFLAGS)

run:
	$(CC) $(CCFLAGS) -fsanitize=address,undefined -g -DDEBUG
	./$(PROJECT_NAME) ./sample/main.c

debug:
	$(CC) $(CCFLAGS) -g -DDEBUG
	lldb $(PROJECT_NAME) -- ./sample/main.c

leaks:
	$(CC) $(CCFLAGS) -g -DDEBUG
	leaks --atExit -- ./$(PROJECT_NAME) ./sample/main.c

BUILD_FILES = comp comp.dSYM main main.dSYM
clean:
	rm -rf $(BUILD_FILES)
