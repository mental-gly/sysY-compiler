## Clang AST and LLVM IR Examples
As we generates LLVM IR, it's necessary to see how Clang AST and LLVM works in several cases.
Here are code examples:
```bash
$ clang -Xclang -ast-dump # dump ast
$ clang -S -emit-llvm # dump LLVM IR
```
When construct LLVM IR types or constants, a LLVM Contexts module maintains ref counts.
Thus same type of Types and Constants are shared.

---
#### Function Declarations and Definition
We declare a function and later define it
```c
int f(int a, int b);

int f(int a, int b) {
    return a + b;
}

int g();

```


The root of the AST is a `TranslationUnit` (in libclang is type `CXTranslationUnit`), and Clang AST treat decl same as def exception the `CompoundStmt` function body. Note that return operator has an implicit cast on AST.
```bash
TranslationUnitDecl 0x5622ebc340e8 <<invalid sloc>> <invalid sloc>
|-TypedefDecl 0x5622ebc349b0 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'
| `-BuiltinType 0x5622ebc34690 '__int128'
|-TypedefDecl 0x5622ebc34a20 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'
| `-BuiltinType 0x5622ebc346b0 'unsigned __int128'
|-TypedefDecl 0x5622ebc34d28 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'
| `-RecordType 0x5622ebc34b00 'struct __NSConstantString_tag'
|   `-Record 0x5622ebc34a78 '__NSConstantString_tag'
|-TypedefDecl 0x5622ebc34dc0 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'
| `-PointerType 0x5622ebc34d80 'char *'
|   `-BuiltinType 0x5622ebc34190 'char'
|-TypedefDecl 0x5622ebc755a0 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag [1]'
| `-ConstantArrayType 0x5622ebc35060 'struct __va_list_tag [1]' 1 
|   `-RecordType 0x5622ebc34ea0 'struct __va_list_tag'
|     `-Record 0x5622ebc34e18 '__va_list_tag'
|-FunctionDecl 0x5622ebc75770 <<source>:1:1, col:19> col:5 f 'int (int, int)'
| |-ParmVarDecl 0x5622ebc75610 <col:7, col:11> col:11 a 'int'
| `-ParmVarDecl 0x5622ebc75690 <col:14, col:18> col:18 b 'int'
|-FunctionDecl 0x5622ebc759a0 prev 0x5622ebc75770 <line:3:1, line:5:1> line:3:5 f 'int (int, int)'
| |-ParmVarDecl 0x5622ebc75880 <col:7, col:11> col:11 used a 'int'
| |-ParmVarDecl 0x5622ebc75900 <col:14, col:18> col:18 used b 'int'
| `-CompoundStmt 0x5622ebc75af0 <col:21, line:5:1>
|   `-ReturnStmt 0x5622ebc75ae0 <line:4:5, col:16>
|     `-BinaryOperator 0x5622ebc75ac0 <col:12, col:16> 'int' '+'
|       |-ImplicitCastExpr 0x5622ebc75a90 <col:12> 'int' <LValueToRValue>
|       | `-DeclRefExpr 0x5622ebc75a50 <col:12> 'int' lvalue ParmVar 0x5622ebc75880 'a' 'int'
|       `-ImplicitCastExpr 0x5622ebc75aa8 <col:16> 'int' <LValueToRValue>
|         `-DeclRefExpr 0x5622ebc75a70 <col:16> 'int' lvalue ParmVar 0x5622ebc75900 'b' 'int'
`-FunctionDecl 0x5622ebc75b60 <line:7:1, col:7> col:5 g 'int ()'
```

But LLVM IR has explicit define and declare
```bash
efine dso_local i32 @f(i32 %0, i32 %1) #0 !dbg !9 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  call void @llvm.dbg.declare(metadata i32* %3, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !16, metadata !DIExpression()), !dbg !17
  %5 = load i32, i32* %3, align 4, !dbg !18
  %6 = load i32, i32* %4, align 4, !dbg !19
  %7 = add nsw i32 %5, %6, !dbg !20
  ret i32 %7, !dbg !21
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
```

#### Local variable definition
For local variables, there's no such declaration (because compiler need to allocate stack space).

```c
int f(int a, int b) {
    int c;
    int d = 10;
    c = 10;
    return a + b + c + d;
}
```

We can see that clang AST also treat definiation simply as `ValDecl` (except possible initialize literals)
Another important insight is, as Clang AST node class has no common ancestors but multiple hierarchies for basic node types like Decl and Stmt. So you can found DeclStmt and its child VarDecl who may really binds a Literal.
```bash
TranslationUnitDecl 0x55f1be4ba0e8 <<invalid sloc>> <invalid sloc>
|-TypedefDecl 0x55f1be4ba9b0 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'
| `-BuiltinType 0x55f1be4ba690 '__int128'
|-TypedefDecl 0x55f1be4baa20 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'
| `-BuiltinType 0x55f1be4ba6b0 'unsigned __int128'
|-TypedefDecl 0x55f1be4bad28 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'
| `-RecordType 0x55f1be4bab00 'struct __NSConstantString_tag'
|   `-Record 0x55f1be4baa78 '__NSConstantString_tag'
|-TypedefDecl 0x55f1be4badc0 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'
| `-PointerType 0x55f1be4bad80 'char *'
|   `-BuiltinType 0x55f1be4ba190 'char'
|-TypedefDecl 0x55f1be4fb5a0 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag [1]'
| `-ConstantArrayType 0x55f1be4bb060 'struct __va_list_tag [1]' 1 
|   `-RecordType 0x55f1be4baea0 'struct __va_list_tag'
|     `-Record 0x55f1be4bae18 '__va_list_tag'
`-FunctionDecl 0x55f1be4fb770 <<source>:1:1, line:6:1> line:1:5 f 'int (int, int)'
  |-ParmVarDecl 0x55f1be4fb610 <col:7, col:11> col:11 used a 'int'
  |-ParmVarDecl 0x55f1be4fb690 <col:14, col:18> col:18 used b 'int'
  `-CompoundStmt 0x55f1be4fbb68 <col:21, line:6:1>
    |-DeclStmt 0x55f1be4fb8e8 <line:2:5, col:10>
    | `-VarDecl 0x55f1be4fb880 <col:5, col:9> col:9 used c 'int'
    |-DeclStmt 0x55f1be4fb9a0 <line:3:5, col:15>
    | `-VarDecl 0x55f1be4fb918 <col:5, col:13> col:9 used d 'int' cinit
    |   `-IntegerLiteral 0x55f1be4fb980 <col:13> 'int' 10
    |-BinaryOperator 0x55f1be4fb9f8 <line:4:5, col:9> 'int' '='
    | |-DeclRefExpr 0x55f1be4fb9b8 <col:5> 'int' lvalue Var 0x55f1be4fb880 'c' 'int'
    | `-IntegerLiteral 0x55f1be4fb9d8 <col:9> 'int' 10
    `-ReturnStmt 0x55f1be4fbb58 <line:5:5, col:24>
      `-BinaryOperator 0x55f1be4fbb38 <col:12, col:24> 'int' '+'
        |-BinaryOperator 0x55f1be4fbae0 <col:12, col:20> 'int' '+'
        | |-BinaryOperator 0x55f1be4fba88 <col:12, col:16> 'int' '+'
        | | |-ImplicitCastExpr 0x55f1be4fba58 <col:12> 'int' <LValueToRValue>
        | | | `-DeclRefExpr 0x55f1be4fba18 <col:12> 'int' lvalue ParmVar 0x55f1be4fb610 'a' 'int'
        | | `-ImplicitCastExpr 0x55f1be4fba70 <col:16> 'int' <LValueToRValue>
        | |   `-DeclRefExpr 0x55f1be4fba38 <col:16> 'int' lvalue ParmVar 0x55f1be4fb690 'b' 'int'
        | `-ImplicitCastExpr 0x55f1be4fbac8 <col:20> 'int' <LValueToRValue>
        |   `-DeclRefExpr 0x55f1be4fbaa8 <col:20> 'int' lvalue Var 0x55f1be4fb880 'c' 'int'
        `-ImplicitCastExpr 0x55f1be4fbb20 <col:24> 'int' <LValueToRValue>
          `-DeclRefExpr 0x55f1be4fbb00 <col:24> 'int' lvalue Var 0x55f1be4fb918 'd' 'int'
Compiler returned: 0
```


As LLVM IR is SSA type, which utilize a `alloca` trick to maintain SSA feature
```bash
define dso_local i32 @f(i32 %0, i32 %1) #0 !dbg !9 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  call void @llvm.dbg.declare(metadata i32* %3, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.declare(metadata i32* %5, metadata !18, metadata !DIExpression()), !dbg !19
  call void @llvm.dbg.declare(metadata i32* %6, metadata !20, metadata !DIExpression()), !dbg !21
  store i32 10, i32* %6, align 4, !dbg !21
  store i32 10, i32* %5, align 4, !dbg !22
  %7 = load i32, i32* %3, align 4, !dbg !23
  %8 = load i32, i32* %4, align 4, !dbg !24
  %9 = add nsw i32 %7, %8, !dbg !25
  %10 = load i32, i32* %5, align 4, !dbg !26
  %11 = add nsw i32 %9, %10, !dbg !27
  %12 = load i32, i32* %6, align 4, !dbg !28
  %13 = add nsw i32 %11, %12, !dbg !29
  ret i32 %13, !dbg !30
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
```
Another good example is 
```c
int f() {
    int c, d, *p;
}
```
and corresponding part of Clang AST:
```bash
`-FunctionDecl 0x55cc612d96b0 <<source>:1:1, line:3:1> line:1:5 f 'int ()'
  `-CompoundStmt 0x55cc612d9980 <col:9, line:3:1>
    `-DeclStmt 0x55cc612d9968 <line:2:5, col:17>
      |-VarDecl 0x55cc612d97b0 <col:5, col:9> col:9 c 'int'
      |-VarDecl 0x55cc612d9830 <col:5, col:12> col:12 d 'int'
      `-VarDecl 0x55cc612d98e0 <col:5, col:16> col:16 p 'int *'
```


#### Inner variable shadows outer variable
```c
int f(int a, int b) {
    int c = 10;
    {
        int c = 2;
    }
    return c;
}
```  
Here we observe that extra `{}` would introduce `CompoundStmt`
```bash
TranslationUnitDecl 0x55c9610ed0e8 <<invalid sloc>> <invalid sloc>
|-TypedefDecl 0x55c9610ed9b0 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'
| `-BuiltinType 0x55c9610ed690 '__int128'
|-TypedefDecl 0x55c9610eda20 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'
| `-BuiltinType 0x55c9610ed6b0 'unsigned __int128'
|-TypedefDecl 0x55c9610edd28 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'
| `-RecordType 0x55c9610edb00 'struct __NSConstantString_tag'
|   `-Record 0x55c9610eda78 '__NSConstantString_tag'
|-TypedefDecl 0x55c9610eddc0 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'
| `-PointerType 0x55c9610edd80 'char *'
|   `-BuiltinType 0x55c9610ed190 'char'
|-TypedefDecl 0x55c96112e5a0 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag [1]'
| `-ConstantArrayType 0x55c9610ee060 'struct __va_list_tag [1]' 1 
|   `-RecordType 0x55c9610edea0 'struct __va_list_tag'
|     `-Record 0x55c9610ede18 '__va_list_tag'
`-FunctionDecl 0x55c96112e770 <<source>:1:1, line:7:1> line:1:5 f 'int (int, int)'
  |-ParmVarDecl 0x55c96112e610 <col:7, col:11> col:11 a 'int'
  |-ParmVarDecl 0x55c96112e690 <col:14, col:18> col:18 b 'int'
  `-CompoundStmt 0x55c96112ea38 <col:21, line:7:1>
    |-DeclStmt 0x55c96112e908 <line:2:5, col:15>
    | `-VarDecl 0x55c96112e880 <col:5, col:13> col:9 used c 'int' cinit
    |   `-IntegerLiteral 0x55c96112e8e8 <col:13> 'int' 10
    |-CompoundStmt 0x55c96112e9d8 <line:3:5, line:5:5>
    | `-DeclStmt 0x55c96112e9c0 <line:4:9, col:18>
    |   `-VarDecl 0x55c96112e938 <col:9, col:17> col:13 c 'int' cinit
    |     `-IntegerLiteral 0x55c96112e9a0 <col:17> 'int' 2
    `-ReturnStmt 0x55c96112ea28 <line:6:5, col:12>
      `-ImplicitCastExpr 0x55c96112ea10 <col:12> 'int' <LValueToRValue>
        `-DeclRefExpr 0x55c96112e9f0 <col:12> 'int' lvalue Var 0x55c96112e880 'c' 'int'
```

LLVM IR use alloca and store to operates stack temp variables
```bash
define dso_local i32 @f(i32 %0, i32 %1) #0 !dbg !9 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca i32, align 4
  %6 = alloca i32, align 4
  store i32 %0, i32* %3, align 4
  call void @llvm.dbg.declare(metadata i32* %3, metadata !14, metadata !DIExpression()), !dbg !15
  store i32 %1, i32* %4, align 4
  call void @llvm.dbg.declare(metadata i32* %4, metadata !16, metadata !DIExpression()), !dbg !17
  call void @llvm.dbg.declare(metadata i32* %5, metadata !18, metadata !DIExpression()), !dbg !19
  store i32 10, i32* %5, align 4, !dbg !19
  call void @llvm.dbg.declare(metadata i32* %6, metadata !20, metadata !DIExpression()), !dbg !22
  store i32 2, i32* %6, align 4, !dbg !22
  %7 = load i32, i32* %5, align 4, !dbg !23
  ret i32 %7, !dbg !24
}

declare void @llvm.dbg.declare(metadata, metadata, metadata) #1

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nosync nounwind readnone speculatable willreturn }
```

#### Get variable address
Unary operator `&`
```c
int f(int a, int b) {
    int *p = &a;
}
```

It's worthwhile to mention `cannot overflow` metadata.
```bash
TranslationUnitDecl 0x55a4c43840e8 <<invalid sloc>> <invalid sloc>
|-TypedefDecl 0x55a4c43849b0 <<invalid sloc>> <invalid sloc> implicit __int128_t '__int128'
| `-BuiltinType 0x55a4c4384690 '__int128'
|-TypedefDecl 0x55a4c4384a20 <<invalid sloc>> <invalid sloc> implicit __uint128_t 'unsigned __int128'
| `-BuiltinType 0x55a4c43846b0 'unsigned __int128'
|-TypedefDecl 0x55a4c4384d28 <<invalid sloc>> <invalid sloc> implicit __NSConstantString 'struct __NSConstantString_tag'
| `-RecordType 0x55a4c4384b00 'struct __NSConstantString_tag'
|   `-Record 0x55a4c4384a78 '__NSConstantString_tag'
|-TypedefDecl 0x55a4c4384dc0 <<invalid sloc>> <invalid sloc> implicit __builtin_ms_va_list 'char *'
| `-PointerType 0x55a4c4384d80 'char *'
|   `-BuiltinType 0x55a4c4384190 'char'
|-TypedefDecl 0x55a4c43c55a0 <<invalid sloc>> <invalid sloc> implicit __builtin_va_list 'struct __va_list_tag [1]'
| `-ConstantArrayType 0x55a4c4385060 'struct __va_list_tag [1]' 1 
|   `-RecordType 0x55a4c4384ea0 'struct __va_list_tag'
|     `-Record 0x55a4c4384e18 '__va_list_tag'
`-FunctionDecl 0x55a4c43c5770 <<source>:3:1, line:5:1> line:3:5 f 'int (int, int)'
  |-ParmVarDecl 0x55a4c43c5610 <col:7, col:11> col:11 used a 'int'
  |-ParmVarDecl 0x55a4c43c5690 <col:14, col:18> col:18 b 'int'
  `-CompoundStmt 0x55a4c43c5968 <col:21, line:5:1>
    `-DeclStmt 0x55a4c43c5950 <line:4:5, col:16>
      `-VarDecl 0x55a4c43c58b0 <col:5, col:15> col:10 p 'int *' cinit
        `-UnaryOperator 0x55a4c43c5938 <col:14, col:15> 'int *' prefix '&' cannot overflow
          `-DeclRefExpr 0x55a4c43c5918 <col:15> 'int' lvalue ParmVar 0x55a4c43c5610 'a' 'int'
```

## Special Thanks
Thanks to [Compiler Explorer](https://www.godbolt.org/). They offer such a convenient online compiler.