; ModuleID = 'assign_test.ll'

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca float, align 4
  %3 = alloca [2 x i32], align 4
  store i32 0, i32* %1, align 4
  store float 0x3FFCCCCCC0000000, float* %2, align 4
  %4 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0
  store i32 2, i32* %4, align 4
  %5 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 0
  %6 = load i32, i32* %5, align 4
  %7 = sitofp i32 %6 to float
  %8 = load float, float* %2, align 4
  %9 = fmul float %7, %8
  %10 = fptosi float %9 to i32
  %11 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1
  store i32 %10, i32* %11, align 4
  %12 = getelementptr inbounds [2 x i32], [2 x i32]* %3, i64 0, i64 1
  %13 = load i32, i32* %12, align 4
  ret i32 %13
}
