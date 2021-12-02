# PW6 实验报告

学号1 姓名1 学号2 姓名2 学号3 姓名3 (不要上传到外网)



## 问题回答

1-1 请给出while语句对应的LLVM IR的代码布局特点，重点解释其中涉及的几个`br`指令的含义（包含各个参数的含义）

while布局特点：

while的条件为一个基本块，后面附带两个基本块，第一个是循环体，第二个是循环后面的部分。

例如

``` {.cpp}
while(i<n+1){
    // dp[i] = dp[i-1] + dp[i-2];
    i = i + 1;
}
```

对应的是

``` {.c}
14:                                               ; preds = %19, %10
  %15 = load i32, i32* %5, align 4;i
  %16 = load i32, i32* %3, align 4;n
  %17 = add nsw i32 %16, 1;n+1
  %18 = icmp slt i32 %15, %17;i<n+1
  br i1 %18, label %19, label %36;true 跳转到 %19, false 跳转到 %36

19:                                               ; preds = %14
  ...;省略无关部分
  %34 = load i32, i32* %5, align 4;i
  %35 = add nsw i32 %34, 1;i+1
  store i32 %35, i32* %5, align 4;i=i+1
  br label %14, !llvm.loop !3;继续下一次循环条件判断

36:                                               ; preds = %14
```

1-2 请简述函数调用语句对应的LLVM IR的代码特点

函数调用代码特点：

先把参数都加载到寄存器，再`call <类型> @函数名(参数列表)`。

例如`climbStairs(n + tmp)` 对应的是

``` {.c}
  %8 = load i32, i32* @n, align 4
  %9 = load i32, i32* @tmp, align 4
  %10 = add nsw i32 %8, %9
  %11 = call i32 @climbStairs(i32 %10)
```

2-1. 请给出SysYFIR.md中提到的两种getelementptr用法的区别, 并解释原因:

``` {.c}
%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0
%2 = getelementptr i32, i32* %1, i32 %0
```

第一种的%1是pointer(array(10,i32))类型，因此`*(%1+0)`是array(10,i32)类型，后面的%0就是取数组的第%0个元素。

第二种的%1是pointer(i32)类型，因此就是取`*(%1+0)`这个i32值。

3-1. 在`scope`内单独处理`func`的好处有哪些。

1.  区分函数和变量，方便调用时处理
2.  可以扩展实现函数嵌套等
3.  便于检查函数与变量是否存在同名

## 实验设计

第一关意义在于熟悉llvm语言语法；第二关意义在于，对一特定.c文件，用cpp语言构造出c语言到llvm语言的映射；第三关意义在于，用cpp语言编写出一个将AST语法树映射为llvm的自动编译器。 由于第一关，第二关难度较低，在此不予讨论。仅讨论第三关的设计思路。

默认测试样例没有错误，已经通过了语法分析和语义检查。该实验目标即为将.c文件形成的AST语法树映射成为llvm语言的.ll文件。即对AST树中每个结点，编写相应的映射规则。

先考虑普通语句：

1.  BlockStmt：在一个scope中操作；
2.  EmptyStmt：无操作；
3.  ExprStmt：访问该表达式；

考虑表达式：

1. UnaryCondExpr：处理非的情况，具体放在难点中讨论；

2. BinaryCondExpr：分为&&与\|\|进行讨论。亮点短路计算的实现在难点中讨论；

3. BinaryExpr：取出左右指针所指的字面值进行运算，考虑其的类型不相同情况，需要更改类型；

4. UnaryExpr：取出指针所指的字面值，方便后续操作

   

再考虑控制流语句： 控制流语句主要是进行BasicBlock的跳转。考虑到有嵌套情况，所以定义了四个容器，用于储存每种情况下的分支；

``` {.cpp}
std::vector<BasicBlock *> tmp_condbb_while;
std::vector<BasicBlock *> tmp_falsebb_while;
std::vector<BasicBlock *> tmp_truebb;
std::vector<BasicBlock *> tmp_falsebb;
```

1. if语句：

   1.  若没有else语句，则整个语句即可分为true 部分 (if(true)) 与next部分。先将创造出的true压入相应容器。 进入判断条件语句：判断条件表达式是否大于与其类型相同的常数0；若大于，则跳转到true 部分。 之后，运行到next部分；
   2.  若有else语句，则添加false 部分 (if(false))。将创造出的false 部分压入相应容器。进入判断条件语句：判断条件表达式是否大于与其类型相同的常数0；若大于，则跳转到true 部分；小于，则跳转的false 部分。 之后，运行到next 部分；

   结束后，弹出相应容器中的值。

2. while语句：

   创造cond，true，false三个部分，压入相应容器，与if相同，判断条件表达式是否大于与其类型相同的常数0，后跳转到相应部分，true结束后跳转到cond。

   结束后，弹出相应容器中的值。

3. break语句：

   跳转到while false部分。

4. continue语句：

   跳转到while cond部分。

再考虑变量定义：

​	VarDef ：分为数组，非数组，初始化，非初始化等多种情况考虑，具体在实验难点中讨论。

左值表达式：

​	LVal：分为是否为数组进行讨论；若不是数组，则可以直接在符号表中找到相应的值进行赋值。若为数组，则利用gep对数组进行寻址赋值。

函数定义：

​	FuncDef：根据函数返回值类型与paramlist形参类型创建函数，函数体中根据是否有“return”，构建“ret”。

​	FuncFParamList：遍历参数表，记录名字，分配空间，并赋实参值；



## 实验难点及解决方案

1.  左值、右值转换

2.  短路计算

## 实验总结

## 实验反馈

## 组间交流
