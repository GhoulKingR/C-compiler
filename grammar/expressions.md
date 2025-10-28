# adding expressions

int main () {
    int digit = 10 * 23 + 5 * 34 * (3 - 4 - 2);     // digit    -> -280
    int num = 500 + digit;                          // num      ->  220
    return num;
}


# grammar
old
```
expression     → bit_opers ;
bit_opers      → equality ( ( "&" | "|" | "^" ) equality )* ;
equality       → comparison ( ( "!=" | "==" ) comparison )* ;
comparison     → term ( ( ">" | ">=" | "<" | "<=" ) term )* ;
term           → factor ( ( "-" | "+" ) factor )* ;
factor         → unary ( ( "/" | "*" ) unary )* ;
unary          → ( "!" | "-" ) unary
               | primary ;
primary        → NUMBER | STRING | "true" | "false" | "nil"
               | IDENTIFIER | "(" expression ")" ;
```

New
```
expression      → precedence14 ;
precedence14    → IDENTIFIER ( "=" | "+=" | "-=" | "*=" | "/=" | "%=" | "<<=" | ">>=" | "&=" | "^=" | "|=" ) precedence13
                | precedence13 ;
precedence13    → precedence12 ( "?" precedence12 ":" precedence12 )* ;
precedence12    → precedence11 ( "||" precedence11 )* ;
precedence11    → precedence10 ( "&&" precedence10 )* ;
precedence10    → precedence9 ( "|" precedence9 )* ;
precedence9     → precedence8 ( "^" precedence8 )* ;
precedence8     → precedence7 ( "&" precedence7 )* ;
precedence7     → precedence6 ( ( "!=" | "==" ) precedence6 )* ;
precedence6     → precedence5 ( ( ">" | ">=" | "<" | "<=" ) precedence5 )* ;
precedence5     → precedence4 ( ( "<<" | ">>" ) precedence4 )* ;
precedence4     → precedence3 ( ( "+" | "-" ) precedence3 )* ;
precedence3     → precedence2 ( ( "/" | "*" | "%" ) precedence2 )* ;
precedence2     → ("++" | "--" | "+" | "-" | "!" | "~" ) precedence2
                | precedence1 ;
precedence1     → primary ("--" | "++")* ;
primary         → NUMBER | CHAR | STRING | IDENTIFIER | "(" expression ")" ;
```

x1 && x0

    cmp x1, #0
    beq l1
    cmp x0, #0
    beq l1
    mov x0, #1
    b l2
l1:
    mov x0, #0
l2:

x1 || x0

    cmp x1, #0
    bne l1
    cmp x0, #0
    bne l1
    mov x0, #0
    b l2
l1:
    mov x0, #1
l2:


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
