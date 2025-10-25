# adding expressions

int main () {
    int digit = 10 * 23 + 5 * 34 * (3 - 4 - 2);     // digit    -> -280
    int num = 500 + digit;                          // num      ->  220
    return num;
}


# grammar
```
expression     → equality ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | "(" expression ")" ;
```

# intermediate representation

int main() {
    int t1 = 10 * 23        // 10 * 23
    int t2 = 5 * 34         // 5 * 34       // 5 * 34 * (3 - 4 - 2) ====|
    int t3 = 3 - 4          // (3 - 4                                   |
    int t3 = $t2 - 2        // - 2)                                     |
    int t2 = t2 * t3        //  ----------------------------------------|
    int digit = t1 + t2        
}

# Assembly

```asm
.globl _main
.p2align 2
_main:
    mov x0, #10
    mul x0, x0, #23
    mov x1, #5
    mul x1, x1, #34
```

# Target

Program (
    declarations = [
        Function(                               # Statement type
            type = "int"
            name = "main"
            body = [
                VariableDecl(                   # Statement type
                    type    = "int",
                    name    = "digit",
                    value   = "0"
                ),
                Return(                         # Statement type
                    Var("digit")                    # Expression type
                )
            ]
        )
    ]
)

# Grammar

```
program                 :: <declaration>*                               :: Program( declarations = [ <declaration>* ] )
declaration             :: <function_declaration>                       :: // container class for detecting type of declaration //
function_declaration    :: <type> IDENTIFIER "(void){" <statement>* "}" :: Function ( type = <type>, name = IDENTIFIER, body = [ <statement>* ] )
type                    :: "void" | "int" | "char"
statement               :: <return> | <variable_decl>                   :: // container class for detecting type of statement //
return                  :: "return " (CONSTANT | IDENTIFIER) ";"        :: Return ( value = CONSTANT )
variable_decl           :: <type> IDENTIFIER "=" CONSTANT ";"           :: VariableDecl ( type = <type>, name = IDENTIFIER, value = CONSTANT )
```


# Notes

- Implement these expressions digit by digit first, then combine them (with nesting) at the end
- To implement multiple-operation expressions you'll also have to build a mechanism that decays complex instructions to two-operation IRs
- The decay mechanism will happen between the parser and converter


# TODO

- Implement an expression type
- For now only able to hold constants and then swap it in place of the current way return holds values

# Unary

unary   ->  ("-" | "!" | "~") primary
primary -> char | num

char -- 'a'
num  -- 123
