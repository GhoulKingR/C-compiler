SOURCE_FILES = main.cpp
PROJECT_NAME = comp

CXX = g++
CXXFLAGS = -std=c++17

build:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(PROJECT_NAME)

run:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -fsanitize=address,undefined -o $(PROJECT_NAME)
	./$(PROJECT_NAME) ./sample/main.c

debug:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -g -o $(PROJECT_NAME)
	lldb $(PROJECT_NAME) -- ./sample/main.c

leaks:
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -g -o $(PROJECT_NAME)
	leaks --atExit -- ./$(PROJECT_NAME) ./sample/main.c

BUILD_FILES = comp comp.dSYM main main.dSYM
clean:
	rm -rf $(BUILD_FILES)
