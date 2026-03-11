; ModuleID = 'examples/exp1/O2/exp1.c'
source_filename = "examples/exp1/O2/exp1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str = private unnamed_addr constant [49 x i8] c"Usage: 1 argument - exponent series len < 10000\0A\00", align 1
@.str.1 = private unnamed_addr constant [24 x i8] c"Exp (len %lu) = %.10lf\0A\00", align 1

; Function Attrs: nounwind uwtable
define dso_local double @fact(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca double, align 8
  %4 = alloca i64, align 8
  store i64 %0, ptr %2, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 8, ptr %3) #6
  store double 1.000000e+00, ptr %3, align 8, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 8, ptr %4) #6
  store i64 1, ptr %4, align 8, !tbaa !5
  br label %5

5:                                                ; preds = %15, %1
  %6 = load i64, ptr %4, align 8, !tbaa !5
  %7 = load i64, ptr %2, align 8, !tbaa !5
  %8 = icmp ule i64 %6, %7
  br i1 %8, label %10, label %9

9:                                                ; preds = %5
  call void @llvm.lifetime.end.p0(i64 8, ptr %4) #6
  br label %18

10:                                               ; preds = %5
  %11 = load i64, ptr %4, align 8, !tbaa !5
  %12 = uitofp i64 %11 to double
  %13 = load double, ptr %3, align 8, !tbaa !9
  %14 = fmul double %13, %12
  store double %14, ptr %3, align 8, !tbaa !9
  br label %15

15:                                               ; preds = %10
  %16 = load i64, ptr %4, align 8, !tbaa !5
  %17 = add i64 %16, 1
  store i64 %17, ptr %4, align 8, !tbaa !5
  br label %5, !llvm.loop !11

18:                                               ; preds = %9
  %19 = load double, ptr %3, align 8, !tbaa !9
  call void @llvm.lifetime.end.p0(i64 8, ptr %3) #6
  ret double %19
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.start.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(argmem: readwrite)
declare void @llvm.lifetime.end.p0(i64 immarg, ptr nocapture) #1

; Function Attrs: nounwind uwtable
define dso_local double @calc_exp(i64 noundef %0) #0 {
  %2 = alloca i64, align 8
  %3 = alloca double, align 8
  %4 = alloca i64, align 8
  store i64 %0, ptr %2, align 8, !tbaa !5
  call void @llvm.lifetime.start.p0(i64 8, ptr %3) #6
  store double 0.000000e+00, ptr %3, align 8, !tbaa !9
  call void @llvm.lifetime.start.p0(i64 8, ptr %4) #6
  %5 = load i64, ptr %2, align 8, !tbaa !5
  store i64 %5, ptr %4, align 8, !tbaa !5
  br label %6

6:                                                ; preds = %16, %1
  %7 = load i64, ptr %4, align 8, !tbaa !5
  %8 = icmp ugt i64 %7, 0
  br i1 %8, label %10, label %9

9:                                                ; preds = %6
  call void @llvm.lifetime.end.p0(i64 8, ptr %4) #6
  br label %19

10:                                               ; preds = %6
  %11 = load i64, ptr %4, align 8, !tbaa !5
  %12 = call double @fact(i64 noundef %11)
  %13 = fdiv double 1.000000e+00, %12
  %14 = load double, ptr %3, align 8, !tbaa !9
  %15 = fadd double %14, %13
  store double %15, ptr %3, align 8, !tbaa !9
  br label %16

16:                                               ; preds = %10
  %17 = load i64, ptr %4, align 8, !tbaa !5
  %18 = add i64 %17, -1
  store i64 %18, ptr %4, align 8, !tbaa !5
  br label %6, !llvm.loop !13

19:                                               ; preds = %9
  %20 = load double, ptr %3, align 8, !tbaa !9
  %21 = fadd double %20, 1.000000e+00
  call void @llvm.lifetime.end.p0(i64 8, ptr %3) #6
  ret double %21
}

; Function Attrs: nounwind uwtable
define dso_local i32 @main(i32 noundef %0, ptr noundef %1) #0 {
  %3 = alloca i32, align 4
  %4 = alloca i32, align 4
  %5 = alloca ptr, align 8
  %6 = alloca i64, align 8
  %7 = alloca i32, align 4
  %8 = alloca i64, align 8
  %9 = alloca double, align 8
  store i32 0, ptr %3, align 4
  store i32 %0, ptr %4, align 4, !tbaa !14
  store ptr %1, ptr %5, align 8, !tbaa !16
  call void @llvm.lifetime.start.p0(i64 8, ptr %6) #6
  store i64 0, ptr %6, align 8, !tbaa !5
  %10 = load i32, ptr %4, align 4, !tbaa !14
  %11 = icmp ne i32 %10, 2
  br i1 %11, label %23, label %12

12:                                               ; preds = %2
  %13 = load ptr, ptr %5, align 8, !tbaa !16
  %14 = getelementptr inbounds ptr, ptr %13, i64 1
  %15 = load ptr, ptr %14, align 8, !tbaa !16
  %16 = call i32 @atoi(ptr noundef %15) #7
  %17 = sext i32 %16 to i64
  store i64 %17, ptr %6, align 8, !tbaa !5
  %18 = icmp ugt i64 %17, 10000
  br i1 %18, label %23, label %19

19:                                               ; preds = %12
  %20 = call ptr @__errno_location() #8
  %21 = load i32, ptr %20, align 4, !tbaa !14
  %22 = icmp ne i32 %21, 0
  br i1 %22, label %23, label %25

23:                                               ; preds = %19, %12, %2
  %24 = call i32 (ptr, ...) @printf(ptr noundef @.str)
  store i32 1, ptr %3, align 4
  store i32 1, ptr %7, align 4
  br label %45

25:                                               ; preds = %19
  call void @llvm.lifetime.start.p0(i64 8, ptr %8) #6
  store i64 0, ptr %8, align 8, !tbaa !5
  br label %26

26:                                               ; preds = %41, %25
  %27 = load i64, ptr %8, align 8, !tbaa !5
  %28 = icmp ult i64 %27, 10000
  br i1 %28, label %30, label %29

29:                                               ; preds = %26
  store i32 2, ptr %7, align 4
  call void @llvm.lifetime.end.p0(i64 8, ptr %8) #6
  br label %44

30:                                               ; preds = %26
  call void @llvm.lifetime.start.p0(i64 8, ptr %9) #6
  %31 = load i64, ptr %6, align 8, !tbaa !5
  %32 = call double @calc_exp(i64 noundef %31)
  store double %32, ptr %9, align 8, !tbaa !9
  %33 = load i64, ptr %8, align 8, !tbaa !5
  %34 = load i64, ptr %6, align 8, !tbaa !5
  %35 = icmp eq i64 %33, %34
  br i1 %35, label %36, label %40

36:                                               ; preds = %30
  %37 = load i64, ptr %6, align 8, !tbaa !5
  %38 = load double, ptr %9, align 8, !tbaa !9
  %39 = call i32 (ptr, ...) @printf(ptr noundef @.str.1, i64 noundef %37, double noundef %38)
  br label %40

40:                                               ; preds = %36, %30
  call void @llvm.lifetime.end.p0(i64 8, ptr %9) #6
  br label %41

41:                                               ; preds = %40
  %42 = load i64, ptr %8, align 8, !tbaa !5
  %43 = add i64 %42, 1
  store i64 %43, ptr %8, align 8, !tbaa !5
  br label %26, !llvm.loop !18

44:                                               ; preds = %29
  store i32 0, ptr %3, align 4
  store i32 1, ptr %7, align 4
  br label %45

45:                                               ; preds = %44, %23
  call void @llvm.lifetime.end.p0(i64 8, ptr %6) #6
  %46 = load i32, ptr %3, align 4
  ret i32 %46
}

; Function Attrs: inlinehint nounwind willreturn memory(read) uwtable
define available_externally i32 @atoi(ptr noundef nonnull %0) #2 {
  %2 = alloca ptr, align 8
  store ptr %0, ptr %2, align 8, !tbaa !16
  %3 = load ptr, ptr %2, align 8, !tbaa !16
  %4 = call i64 @strtol(ptr noundef %3, ptr noundef null, i32 noundef 10) #6
  %5 = trunc i64 %4 to i32
  ret i32 %5
}

; Function Attrs: nounwind willreturn memory(none)
declare ptr @__errno_location() #3

declare i32 @printf(ptr noundef, ...) #4

; Function Attrs: nounwind
declare i64 @strtol(ptr noundef, ptr noundef, i32 noundef) #5

attributes #0 = { nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(argmem: readwrite) }
attributes #2 = { inlinehint nounwind willreturn memory(read) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #6 = { nounwind }
attributes #7 = { nounwind willreturn memory(read) }
attributes #8 = { nounwind willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"long", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"double", !7, i64 0}
!11 = distinct !{!11, !12}
!12 = !{!"llvm.loop.mustprogress"}
!13 = distinct !{!13, !12}
!14 = !{!15, !15, i64 0}
!15 = !{!"int", !7, i64 0}
!16 = !{!17, !17, i64 0}
!17 = !{!"any pointer", !7, i64 0}
!18 = distinct !{!18, !12}
