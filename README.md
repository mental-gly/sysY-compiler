# sysY-compiler

系统环境：Ubuntu 18.04

sysY-Lexer测试方法：

```
//1.产生lex.yy.c
## lex lexer.l

//2.编译lex.yy.c
## gcc lex.yy.c

//3.运行测试程序 testing中是一段用于测试的sysY代码
## ./a.out <testing
```

## Lauguage Standard
Compared to `C99` Standard, we cut off:
- wide char string literal and C++11 unicode string literals
- unnecessary keywords somehow like volatile, register, restrict and auto
- GCC extension like `__attribute__` and builtin types and functions like SIMD `__mm256d`
- VLA, it's a C99 standard (removed in C++ standard)


## Roadmaps
- [ ] Basic numeric type support
- [ ] Linear control flow
- [ ] Non-Linear control flow like for, if
- [ ] Arrays, struct, typedef
- [ ] Preprocessor macros
- [ ] Better front end diagnose output (may need hand written parser)
