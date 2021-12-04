; ModuleID = 'if_test.ll'

@a = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 10, i32* @a, align 4
  %2 = load i32, i32* @a, align 4
  %3 = icmp sgt i32 %2, 0
  br i1 %3, label %4, label %6

4:                                                ; preds = %0
  %5 = load i32, i32* @a, align 4
  ret i32 %5

6:                                                ; preds = %0
  br label %7

7:                                                ; preds = %6, %4
  ret i32 0
}
