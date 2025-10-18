SOURCE_FILES = main.cpp
PROJECT_NAME = comp

CXX = g++
CXXFLAGS = -g -std=c++17 -O0

$(PROJECT_NAME): $(SOURCE_FILES)
	$(CXX) $(CXXFLAGS) $(SOURCE_FILES) -o $(PROJECT_NAME)

run: $(PROJECT_NAME)
	./$(PROJECT_NAME) ../main.c

debug: $(PROJECT_NAME)
	lldb $(PROJECT_NAME) -- ../main.c

clean:
	rm -rf comp comp.dSYM
