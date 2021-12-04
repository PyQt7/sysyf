; ModuleID = 'while_test.ll'

@a = dso_local global i32 0, align 4
@b = dso_local global i32 0, align 4

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store i32 0, i32* @b, align 4
  store i32 3, i32* @a, align 4
  br label %2

2:                                                ; preds = %5, %0
  %3 = load i32, i32* @a, align 4
  %4 = icmp sgt i32 %3, 0
  br i1 %4, label %5, label %11

5:                                                ; preds = %2
  %6 = load i32, i32* @b, align 4
  %7 = load i32, i32* @a, align 4
  %8 = add nsw i32 %6, %7
  store i32 %8, i32* @b, align 4
  %9 = load i32, i32* @a, align 4
  %10 = sub nsw i32 %9, 1
  store i32 %10, i32* @a, align 4
  br label %2

11:                                               ; preds = %2
  %12 = load i32, i32* @b, align 4
  ret i32 %12
}
