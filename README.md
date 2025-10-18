# C compiler

Testing setup:
```tree
$ tree ..
..
├── compiler
│   ├── Makefile
│   ├── exceptions.hpp
│   ├── grammar
│   │   ├── grammar.gram
│   │   └── variables.txt
│   ├── main.cpp
│   ├── nodes.hpp
│   ├── parser.hpp
│   ├── targets
│   │   ├── arm.hpp
│   │   └── interface.hpp
│   └── token.hpp
├── main.c
...
```

With the contents of main.c being:
```c
int main(void) {
    return 2;
}
```
