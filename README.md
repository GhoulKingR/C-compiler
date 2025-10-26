# C compiler

This is a WIP c compiler.

## What is supported currently
This can run:
```c
int main(void) {
    int digit = (20 + 10) + 5;     /* digits */
    char name = 'a';
    return digit;
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
