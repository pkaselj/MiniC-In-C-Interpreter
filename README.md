# MiniC Interpreter in C
## General
This project implements an interpreter for a simple toy language inspired by C - written in C without external libraries other than the standard library. The language is dynamically typed LL(1)*-ish* grammar interpreted by a recursive descent parser written in C.

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

### Lexing and Parsing output

```
Program input: 'function a(b, c, d){ b + c * d; } if(x) { y  = 4; } else { y = 5; }'

[Token type=TT_K_FN length=8 data=]
[Token type=TT_ID length=1 data=a]
[Token type=TT_O_PAREN length=1 data=]
[Token type=TT_ID length=1 data=b]
[Token type=TT_K_COMMA length=1 data=]
[Token type=TT_ID length=1 data=c]
[Token type=TT_K_COMMA length=1 data=]
[Token type=TT_ID length=1 data=d]
[Token type=TT_C_PAREN length=1 data=]
[Token type=TT_O_BRACE length=1 data=]
[Token type=TT_ID length=1 data=b]
[Token type=TT_OP_ADD length=1 data=]
[Token type=TT_ID length=1 data=c]
[Token type=TT_OP_MUL length=1 data=]
[Token type=TT_ID length=1 data=d]
[Token type=TT_DELIM length=1 data=]
[Token type=TT_C_BRACE length=1 data=]
[Token type=TT_K_IF length=2 data=]
[Token type=TT_O_PAREN length=1 data=]
[Token type=TT_ID length=1 data=x]
[Token type=TT_C_PAREN length=1 data=]
[Token type=TT_O_BRACE length=1 data=]
[Token type=TT_ID length=1 data=y]
[Token type=TT_ASSIGN length=1 data=]
[Token type=TT_NUMBER length=1 data=4.000000]
[Token type=TT_DELIM length=1 data=]
[Token type=TT_C_BRACE length=1 data=]
[Token type=TT_K_ELSE length=4 data=]
[Token type=TT_O_BRACE length=1 data=]
[Token type=TT_ID length=1 data=y]
[Token type=TT_ASSIGN length=1 data=]
[Token type=TT_NUMBER length=1 data=5.000000]
[Token type=TT_DELIM length=1 data=]
[Token type=TT_C_BRACE length=1 data=]

AST_S
 AST_FN_DEF_STMT
.>AST_ID_EXPR
.. a
.>AST_ID_EXPR
.. b
.>AST_ID_EXPR
.. c
.>AST_ID_EXPR
.. d
.>AST_BLOCK_STMT
.. AST_BINARY_EXPR
...>AST_ID_EXPR
.... b
...>AST_BINARY_EXPR
.... AST_ID_EXPR
.....>c
.... AST_ID_EXPR
.....>d
.... TT_OP_MUL
...>TT_OP_ADD
 AST_IF_STMT
.>AST_ID_EXPR
.. x
.>AST_BLOCK_STMT
.. AST_ASSIGN_EXPR
...>AST_ID_EXPR
.... y
...>AST_NUM_EXPR
.... 4.000000
.>AST_BLOCK_STMT
.. AST_ASSIGN_EXPR
...>AST_ID_EXPR
.... y
...>AST_NUM_EXPR
.... 5.000000
```

```
Program input: '3*5+15-2;'
[Token type=TT_NUMBER length=1 data=3.000000]
[Token type=TT_OP_MUL length=1 data=]
[Token type=TT_NUMBER length=1 data=5.000000]
[Token type=TT_OP_ADD length=1 data=]
[Token type=TT_NUMBER length=2 data=15.000000]
[Token type=TT_OP_SUB length=1 data=]
[Token type=TT_NUMBER length=1 data=2.000000]
[Token type=TT_DELIM length=1 data=]
AST_S
 AST_BINARY_EXPR
.>AST_BINARY_EXPR
.. AST_BINARY_EXPR
...>AST_NUM_EXPR
.... 3.000000
...>AST_NUM_EXPR
.... 5.000000
...>TT_OP_MUL
.. AST_NUM_EXPR
...>15.000000
.. TT_OP_ADD
.>AST_NUM_EXPR
.. 2.000000
.>TT_OP_SUB
[VT_NUMBER]=28.000000
```

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

### Valgrind Results

```
valgrind --leak-check=full --show-leak-kinds=all \
                --track-origins=yes --verbose ./program
==890== Memcheck, a memory error detector
==890== Copyright (C) 2002-2022, and GNU GPL'd, by Julian Seward et al.
==890== Using Valgrind-3.22.0-bd4db67b1d-20231031 and LibVEX; rerun with -h for copyright info
==890== Command: ./program
==890==
--890-- Valgrind options:
--890--    --leak-check=full
--890--    --show-leak-kinds=all
--890--    --track-origins=yes
--890--    --verbose
--890-- Contents of /proc/version:
--890--   Linux version 6.6.87.2-microsoft-standard-WSL2 (root@439a258ad544) (gcc (GCC) 11.2.0, GNU ld (GNU Binutils) 2.37) #1 SMP PREEMPT_DYNAMIC Thu Jun  5 18:30:46 UTC 2025
--890--
--890-- Arch and hwcaps: AMD64, LittleEndian, amd64-cx16-lzcnt-rdtscp-sse3-ssse3-avx-avx2-bmi-f16c-rdrand-rdseed
--890-- Page sizes: currently 4096, max supported 4096
--890-- Valgrind library directory: /usr/libexec/valgrind
--890-- Reading syms from /mnt/s/TEMP/MiniC-Interpreter-C/program
--890-- Reading syms from /usr/lib/x86_64-linux-gnu/ld-linux-x86-64.so.2
--890--   Considering /usr/lib/debug/.build-id/52/0e05878220fb2fc6d28ff46b63b3fd5d48e763.debug ..
--890--   .. build-id is valid
--890-- Reading syms from /usr/libexec/valgrind/memcheck-amd64-linux
--890--    object doesn't have a dynamic symbol table
--890-- Scheduler: using generic scheduler lock implementation.
--890-- Reading suppressions file: /usr/libexec/valgrind/default.supp
==890== embedded gdbserver: reading from /tmp/vgdb-pipe-from-vgdb-to-890-by-kaso-on-???
==890== embedded gdbserver: writing to   /tmp/vgdb-pipe-to-vgdb-from-890-by-kaso-on-???
==890== embedded gdbserver: shared mem   /tmp/vgdb-pipe-shared-mem-vgdb-890-by-kaso-on-???
==890==
==890== TO CONTROL THIS PROCESS USING vgdb (which you probably
==890== don't want to do, unless you know exactly what you're doing,
==890== or are doing some strange experiment):
==890==   /usr/bin/vgdb --pid=890 ...command...
==890==
==890== TO DEBUG THIS PROCESS USING GDB: start GDB like this
==890==   /path/to/gdb ./program
==890== and then give GDB the following command
==890==   target remote | /usr/bin/vgdb --pid=890
==890== --pid is optional if only one valgrind process is running
==890==
--890-- REDIR: 0x4028b00 (ld-linux-x86-64.so.2:strlen) redirected to 0x580c2e1a (???)
--890-- REDIR: 0x40272b0 (ld-linux-x86-64.so.2:index) redirected to 0x580c2e34 (???)
--890-- Reading syms from /usr/libexec/valgrind/vgpreload_core-amd64-linux.so
--890-- Reading syms from /usr/libexec/valgrind/vgpreload_memcheck-amd64-linux.so
==890== WARNING: new redirection conflicts with existing -- ignoring it
--890--     old: 0x04028b00 (strlen              ) R-> (0000.0) 0x580c2e1a ???
--890--     new: 0x04028b00 (strlen              ) R-> (2007.0) 0x0484f340 strlen
--890-- REDIR: 0x40274e0 (ld-linux-x86-64.so.2:strcmp) redirected to 0x4850460 (strcmp)
--890-- REDIR: 0x4026910 (ld-linux-x86-64.so.2:mempcpy) redirected to 0x4853cd0 (mempcpy)
--890-- Reading syms from /usr/lib/x86_64-linux-gnu/libc.so.6
--890--   Considering /usr/lib/debug/.build-id/27/4eec488d230825a136fa9c4d85370fed7a0a5e.debug ..
--890--   .. build-id is valid
--890-- REDIR: 0x4028ca0 (ld-linux-x86-64.so.2:strncmp) redirected to 0x484fc90 (strncmp)
--890-- REDIR: 0x4917050 (libc.so.6:strnlen) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49170e0 (libc.so.6:strpbrk) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49151a0 (libc.so.6:strcmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492e3b0 (libc.so.6:wcsnlen) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4914290 (libc.so.6:memset) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492db20 (libc.so.6:wcslen) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49993f0 (libc.so.6:__memcpy_chk) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4914200 (libc.so.6:memrchr) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492e350 (libc.so.6:wcsncpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4913720 (libc.so.6:memcpy@@GLIBC_2.14) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492c8e0 (libc.so.6:wcschr) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4915090 (libc.so.6:index) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4917110 (libc.so.6:rindex) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492c990 (libc.so.6:wcscmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49144b0 (libc.so.6:stpncpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x493aeb0 (libc.so.6:wmemchr) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916ef0 (libc.so.6:strncmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4914510 (libc.so.6:strcasecmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916310 (libc.so.6:strcspn) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x492d8f0 (libc.so.6:wcscpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4915020 (libc.so.6:strcat) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916df0 (libc.so.6:strncasecmp_l) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4915110 (libc.so.6:strchrnul) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4913630 (libc.so.6:bcmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49162a0 (libc.so.6:strcpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49145b0 (libc.so.6:strcasecmp_l) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916cc0 (libc.so.6:strlen) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916f90 (libc.so.6:strncpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x493af30 (libc.so.6:wmemcmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4999510 (libc.so.6:__memmove_chk) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
==890== WARNING: new redirection conflicts with existing -- ignoring it
--890--     old: 0x049eaa10 (__memcpy_chk_avx_una) R-> (2030.0) 0x04853dd0 __memcpy_chk
--890--     new: 0x049eaa10 (__memcpy_chk_avx_una) R-> (2024.0) 0x04853740 __memmove_chk
--890-- REDIR: 0x4914440 (libc.so.6:stpcpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4913fc0 (libc.so.6:memmove) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
==890== Preferring higher priority redirection:
--890--     old: 0x049eaa40 (__memcpy_avx_unalign) R-> (2018.0) 0x04851580 __memcpy_avx_unaligned_erms
--890--     new: 0x049eaa40 (__memcpy_avx_unalign) R-> (2018.1) 0x04852d60 memmove
--890-- REDIR: 0x49135b0 (libc.so.6:memchr) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49172e0 (libc.so.6:strspn) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49140e0 (libc.so.6:mempcpy) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x4916d50 (libc.so.6:strncasecmp) redirected to 0x483d1c0 (_vgnU_ifunc_wrapper)
--890-- REDIR: 0x49ef630 (libc.so.6:__strrchr_avx2) redirected to 0x484ed20 (rindex)
--890-- REDIR: 0x49ed780 (libc.so.6:__strlen_avx2) redirected to 0x484f220 (strlen)
--890-- REDIR: 0x49ecde0 (libc.so.6:__strchrnul_avx2) redirected to 0x48537b0 (strchrnul)
--890-- REDIR: 0x49eaa40 (libc.so.6:__memcpy_avx_unaligned_erms) redirected to 0x4852d60 (memmove)
--890-- REDIR: 0x490f650 (libc.so.6:malloc) redirected to 0x48467b0 (malloc)
Program input: '3*5+15-2;'
--890-- REDIR: 0x49ee860 (libc.so.6:__strncmp_avx2) redirected to 0x484fab0 (strncmp)
--890-- REDIR: 0x49eb440 (libc.so.6:__memset_avx2_unaligned_erms) redirected to 0x4852c50 (memset)
--890-- REDIR: 0x49eaa00 (libc.so.6:__mempcpy_avx_unaligned_erms) redirected to 0x48538d0 (mempcpy)
[Token type=TT_NUMBER length=1 data=3.000000]
[Token type=TT_OP_MUL length=1 data=]
[Token type=TT_NUMBER length=1 data=5.000000]
[Token type=TT_OP_ADD length=1 data=]
[Token type=TT_NUMBER length=2 data=15.000000]
[Token type=TT_OP_SUB length=1 data=]
[Token type=TT_NUMBER length=1 data=2.000000]
[Token type=TT_DELIM length=1 data=]
--890-- REDIR: 0x490fd30 (libc.so.6:free) redirected to 0x4849820 (free)
AST_S
 AST_BINARY_EXPR
.>AST_BINARY_EXPR
.. AST_BINARY_EXPR
...>AST_NUM_EXPR
.... 3.000000
...>AST_NUM_EXPR
.... 5.000000
...>TT_OP_MUL
.. AST_NUM_EXPR
...>15.000000
.. TT_OP_ADD
.>AST_NUM_EXPR
.. 2.000000
.>TT_OP_SUB
[VT_NUMBER]=28.000000
==890==
==890== HEAP SUMMARY:
==890==     in use at exit: 0 bytes in 0 blocks
==890==   total heap usage: 35 allocs, 35 frees, 2,032 bytes allocated
==890==
==890== All heap blocks were freed -- no leaks are possible
==890==
==890== ERROR SUMMARY: 0 errors from 0 contexts (suppressed: 0 from 0)
```

### Code Examples

Code examples can be found in [./examples](./examples).

- [Examples](./examples/)
  - [Fibonacci Sum](./examples/fibonacci_sum.mc) - Counts the sum of first `n` Fibonacci numbers

## Using the Interpreter

~To use the interpreter in interactive mode/REPL, run it as `python main.py --interactive` or `python main.py -i`. To interpret code from a file,
run `python main.py --file <file>` or `python main.py -f <file>` where `<file>` is path to a file that contains *Mini-C* code e.g. `python main.py --file .\examples\fibonacci_sum.mc`.~

### Windows

Compile with Visual Studio or MinGW compiler and run.

### Linux

Build with make `MODE=release make` or `MODE=debug make`. Run valgrind checks with `MODE=debug make valgrind`. Clean with `make clean`

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
- [ ] owned String destructor