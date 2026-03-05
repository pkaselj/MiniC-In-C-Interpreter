# MiniC Interpreter in C
## General
This project implements an interpreter for a simple toy language inspired by C - written in C without external libraries other than the standard library. The language is dynamically typed LL(1)*-ish* grammar interpreted by a recursive descent parser written in C.

--- TODO: update readme below after implementing interpreter, for more information see MiniC in Python

It is still WIP and it currently implements the following features:
- Arithmetic operations (`+ - * /`) (with proper operator precedence i.e. `2+3*4` parses as `2+(3*4)`)
- Parentheses `3*(2+3)`
- Variables (create on assign)
- Compound assignment (right-associative i.e. `x = y = 1` parses as `x = (y = 1)`)
- Assignment as top-level expression i.e. assignment produces its own value - `x + (y = 4)` parses as `x + 4; y = 4`.
- `;`-delimited statements
- Semantic analysis of left hand side assignability
- Flow control with `if/else`. `else` is associated with the nearest `if`. Condition is `true` if not equal to `0`.
- `while` and `for` loops. Condition is `true` if not equal to `0`.
- `for` loop has optional arguments e.g. valid forms are:
  - `for(x = 1; x < 5; x = x + 1) { x; }` but also,
  - `x = 1; for(; x < 5; x = x + 1) { x; }` and,
  - `x = 1; for(; x < 5; ) { x = x + 1; }`
  - or even an infinite loop: `for(;;){}` so you can shoot yourself in the foot - just like in C *but mini*
- `if/else` and `while` statements are expressions that return value of last interpreted expression i.e. `if(x) { y = 3; z = 4; }` would return `4` assuming `x` was not equal t o `0` 
- Left-associative comparison operators (`== != <= >= < >`) i.e. `x < y < z` is parsed as `((x < y) < z)`
- Unary operators `- + !`, where `!x` is the logical/boolean `not(x)` operator and binary logical operators `&& ||`
- Implemented functions. Functions are defined **only at the beginning of the file (before statements)** as follows: `function fname(arg1, arg2, arg3) { expr1; expr2; }` and are called as follows: `fname(1, 2, 3)`. Return value of the function is its last executed statemened - in this case evaluated value of `expr2`.
- Implemented variables and variable local aliasing - local variables alias global ones. Each function can access its caller's symbols. (for now, maybe remove or keep *closures*).

## Grammar EBNF

Currently implemented grammar EBNF:

```bnf

<S> ::= <func_def>* <stmt>*

<arg_list> ::= <expr> ("," <expr>)*
<param_list> ::= <id> ("," <id>)*

<func_def> ::= "function" <id> "(" <param_list>? ")" <block>

<stmt> ::= <expr> ";" | <if_stmt> | <while_stmt> | <for_stmt>
<if_stmt> ::= "if" "(" <expr> ")" <block> ( "else" "(" <block> ")" )?
<while_stmt> ::= "while" "(" <expr> ")" <block>
<for_stmt> ::= "for" "(" <expr>? ";" <expr>? ";" <expr>? ")" <block>
<block> ::= "{" <stmt>* "}"

<expr> ::= <assignee> ("=" <assignee>)?
<assignee> ::= <logic_or>
<logic_or> ::= <logic_and> ("||" <logic_and>)*
<logic_and> ::= <comparee> ("&&" <comparee>)*
<comparee> ::= <additive> (("==" | "!=" | ">" | "<" | ">=" | "<=") <additive>)*
<additive> ::= <term> (("+" | "-") <term>)*
<term> ::= <unary> (("*" | "/") <unary>)*
<unary> ::= ("!" | "-" | "+") <unary> | <func_call>
<func_call> ::= <primary> ("(" <arg_list>? ")")*
<primary> ::= <number> | <id> | "(" <expr> ")"
```

## Examples

### Interactive / REPL Example (from Python example)

```
> x;
Error: Symbol [x] not defined!
> x = 3;
3.0
> 2 * x;
6.0
> x;
3.0
> x = 2*x;
6.0
> x;
6.0
> x = y = 5;
5.0
> x; y;
5.0
5.0
> x = 2 * y = 5;
Error: Could not assign to node of type: [<class 'lib.common_defs.BinaryExpressionNode'>]
> x; y = 3 * (2 + x);
5.0
21.0
> if(x) { y  = 4; } else { y = 5; }
4.0
> y;
4.0
> x = 0;
0.0
> if(x) { y  = 4; } else { y = 5; }
5.0
> y;
5.0
> x = 3; y = 20;
3.0
20.0
> while(x) { x = x - 1; y = y + 1; }
23.0
> y;
23.0
> x;
0.0
> for(x = 1; x < 5; x = x + 1){ x; }
4.0
> x = 1; y = 0;
1.0
0.0
> x && y || x && y;
0.0
> function a(b, c, d){ b + c * d; }
> a(2, 3, 4); 
14.0
> function a(arg1) { arg1 + arg2; }
> function b(arg2) { a(arg2); }
> b(1);
2.0
```

### Code Examples

Code examples can be found in [./examples](./examples).

- [Examples](./examples/)
  - [Fibonacci Sum](./examples/fibonacci_sum.mc) - Counts the sum of first `n` Fibonacci numbers

## Using the Interpreter

To use the interpreter in interactive mode/REPL, run it as `python main.py --interactive` or `python main.py -i`. To interpret code from a file,
run `python main.py --file <file>` or `python main.py -f <file>` where `<file>` is path to a file that contains *Mini-C* code e.g. `python main.py --file .\examples\fibonacci_sum.mc`.

## Extending the Language
Implement in future:
- [ ] `NULL` value - remove var from env?
- [x] Flow control and loops - `if else while`
- [x] `for` loop (implemented as `while` internally)
- [ ] Implement `elseif` keyword
- [x] Implement comparison operators (`== > < >= <= !=`)
- [ ] Implement booleans
- [x] Implement logical operators `|| &&`
- [x] Unary `- + !`
- [x] Functions
- [ ] Decide on closures or not
- [ ] Implement function overloading
- [ ] Structures and classes
- [ ] Noop expressions (extra delimiters `;`)
- [ ] `||` and `&&` _shortcircuiting_
- [ ] Comments
- [x] File execution
- [ ] Function validation pass - referencing non-existing symbols
- [ ] Scientific notation numbers