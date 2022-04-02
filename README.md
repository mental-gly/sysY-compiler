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

