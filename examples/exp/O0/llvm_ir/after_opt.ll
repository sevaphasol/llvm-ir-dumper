; ModuleID = '/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/exp1/exp1.c'
source_filename = "/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/exp1/exp1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [49 x i8] c"Usage: 1 argument - exponent series len < 10000\0A\00", align 1
@.str.1 = private unnamed_addr constant [24 x i8] c"Exp (len %lu) = %.10lf\0A\00", align 1

; Function Attrs: noinline nounwind optnone uwtable
define dso_local double @fact(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca double, align 8
  %4 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  store double 1.000000e+00, ptr %3, align 8
  store i64 1, ptr %4, align 8
  br label %5

5:                                                ; preds = %14, %1
  %6 = load i64, ptr %4, align 8
  %7 = load i64, ptr %2, align 8
  %8 = icmp ule i64 %6, %7
  br i1 %8, label %9, label %17

9:                                                ; preds = %5
  %10 = load i64, ptr %4, align 8
  %11 = uitofp i64 %10 to double
  %12 = load double, ptr %3, align 8
  %13 = fmul double %12, %11
  store double %13, ptr %3, align 8
  br label %14

14:                                               ; preds = %9
  %15 = load i64, ptr %4, align 8
  %16 = add i64 %15, 1
  store i64 %16, ptr %4, align 8
  br label %5, !llvm.loop !6

17:                                               ; preds = %5
  %18 = load double, ptr %3, align 8
  ret double %18
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local double @calc_exp(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca double, align 8
  %4 = alloca i64, align 8
  store i64 %0, ptr %2, align 8
  store double 0.000000e+00, ptr %3, align 8
  %5 = load i64, ptr %2, align 8
  store i64 %5, ptr %4, align 8
  br label %6

6:                                                ; preds = %15, %1
  %7 = load i64, ptr %4, align 8
  %8 = icmp ugt i64 %7, 0
  br i1 %8, label %9, label %18

9:                                                ; preds = %6
  %10 = load i64, ptr %4, align 8
  %11 = call double @fact(i64 noundef %10)
  %12 = fdiv double 1.000000e+00, %11
  %13 = load double, ptr %3, align 8
  %14 = fadd double %13, %12
  store double %14, ptr %3, align 8
  br label %15

15:                                               ; preds = %9
  %16 = load i64, ptr %4, align 8
  %17 = add i64 %16, -1
  store i64 %17, ptr %4, align 8
  br label %6, !llvm.loop !8

18:                                               ; preds = %6
  %19 = load double, ptr %3, align 8
  %20 = fadd double %19, 1.000000e+00
  ret double %20
}

; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i64, align 8
  %8 = alloca double, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4
  store ptr %1, ptr %5, align 8
  store i64 0, ptr %6, align 8
  %9 = load i32, ptr %4, align 4
  %10 = icmp ne i32 %9, 2
  br i1 %10, label %22, label %11

11:                                               ; preds = %2
  %12 = load ptr, ptr %5, align 8
  %13 = getelementptr inbounds ptr, ptr %12, i64 1
  %14 = load ptr, ptr %13, align 8
  %15 = call i32 @atoi(ptr noundef %14) #4
  %16 = sext i32 %15 to i64
  store i64 %16, ptr %6, align 8
  %17 = icmp ugt i64 %16, 10000
  br i1 %17, label %22, label %18

18:                                               ; preds = %11
  %19 = call ptr @__errno_location() #5
  %20 = load i32, ptr %19, align 4
  %21 = icmp ne i32 %20, 0
  br i1 %21, label %22, label %24

22:                                               ; preds = %18, %11, %2
  %23 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 1, ptr %3, align 4
  br label %43

24:                                               ; preds = %18
  store i64 0, ptr %7, align 8
  br label %25

25:                                               ; preds = %39, %24
  %26 = load i64, ptr %7, align 8
  %27 = icmp ult i64 %26, 10000
  br i1 %27, label %28, label %42

28:                                               ; preds = %25
  %29 = load i64, ptr %6, align 8
  %30 = call double @calc_exp(i64 noundef %29)
  store double %30, ptr %8, align 8
  %31 = load i64, ptr %7, align 8
  %32 = load i64, ptr %6, align 8
  %33 = icmp eq i64 %31, %32
  br i1 %33, label %34, label %38

34:                                               ; preds = %28
  %35 = load i64, ptr %6, align 8
  %36 = load double, ptr %8, align 8
  %37 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i64 noundef %35, double noundef %36)
  br label %38

38:                                               ; preds = %34, %28
  br label %39

39:                                               ; preds = %38
  %40 = load i64, ptr %7, align 8
  %41 = add i64 %40, 1
  store i64 %41, ptr %7, align 8
  br label %25, !llvm.loop !9

42:                                               ; preds = %25
  store i32 0, ptr %3, align 4
  br label %43

43:                                               ; preds = %42, %22
  %44 = load i32, ptr %3, align 4
  ret i32 %44
}

; Function Attrs: nounwind willreturn memory(read)
declare i32 @atoi(ptr noundef) #1

; Function Attrs: nounwind willreturn memory(none)
declare ptr @__errno_location() #2

declare i32 @printf(ptr noundef, ...) #3

attributes #0 = { noinline nounwind optnone uwtable "frame-pointer"="all" "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nounwind willreturn memory(read) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nounwind willreturn memory(none) "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { "frame-pointer"="all" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { nounwind willreturn memory(read) }
attributes #5 = { nounwind willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{i32 7, !"frame-pointer", i32 2}
!5 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!6 = distinct !{!6, !7}
!7 = !{!"llvm.loop.mustprogress"}
!8 = distinct !{!8, !7}
!9 = distinct !{!9, !7}
