# C compiler

This is a WIP c compiler.

## What is supported currently
This can run:
```c
int main(void) {
    int digit = 20 * 10 + 5 + 10;     /* digits */
    char name = 'a';
    int a = digit || name;
    return digit--;
}
```

## Running the project
Running the project:
```bash
$ make run
```

Debugging with lldb:
```bash
$ make debug
```
