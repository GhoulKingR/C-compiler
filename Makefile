SOURCE_FILES = main.cpp
PROJECT_NAME = comp

CXX = g++
CXXFLAGS = -g -std=c++17 -O3 -fsanitize=address,undefined

$(PROJECT_NAME): $(SOURCE_FILES)
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(PROJECT_NAME)

run: $(PROJECT_NAME)
	./$(PROJECT_NAME) ./sample/main.c

debug: $(PROJECT_NAME)
	lldb $(PROJECT_NAME) -- ./sample/main.c

BUILD_FILES = comp comp.dSYM main main.dSYM
clean:
	rm -rf $(BUILD_FILES)
