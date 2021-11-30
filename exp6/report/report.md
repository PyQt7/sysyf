# PW6 实验报告

学号1 姓名1 学号2 姓名2 学号3 姓名3 (不要上传到外网)

## 问题回答

1-1 请给出while语句对应的LLVM IR的代码布局特点，重点解释其中涉及的几个`br`指令的含义（包含各个参数的含义）

while布局特点：

while的条件为一个基本块，后面附带两个基本块，第一个是循环体，第二个是循环后面的部分。

例如

```c++
while(i<n+1){
    // dp[i] = dp[i-1] + dp[i-2];
    i = i + 1;
}
```

对应的是

```c
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

```c
  %8 = load i32, i32* @n, align 4
  %9 = load i32, i32* @tmp, align 4
  %10 = add nsw i32 %8, %9
  %11 = call i32 @climbStairs(i32 %10)
```

2-1. 请给出SysYFIR.md中提到的两种getelementptr用法的区别, 并解释原因:

```c
%2 = getelementptr [10 x i32], [10 x i32]* %1, i32 0, i32 %0
%2 = getelementptr i32, i32* %1, i32 %0
```

第一种的%1是pointer(array(10,i32))类型，因此`*(%1+0)`是array(10,i32)类型，后面的%0就是取数组的第%0个元素。

第二种的%1是pointer(i32)类型，因此就是取`*(%1+0)`这个i32值。

3-1. 在`scope`内单独处理`func`的好处有哪些。

1. 区分函数和变量，方便调用时处理
2. 可以扩展实现函数嵌套等
3. 便于检查函数与变量是否存在同名

## 实验设计

## 实验难点及解决方案

1. 左值、右值转换
2. 短路计算

## 实验总结

## 实验反馈

## 组间交流
