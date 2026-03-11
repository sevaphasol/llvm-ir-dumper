; ModuleID = 'examples/exp2/O2/exp2.c'
source_filename = "examples/exp2/O2/exp2.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str.1 = private unnamed_addr constant [14 x i8] c"Res = %.10lf\0A\00", align 1
@str = private unnamed_addr constant [48 x i8] c"Usage: 1 argument - exponent series len < 10000\00", align 1

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @fact(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp eq i64 %0, 0
  br i1 %2, label %26, label %3

3:                                                ; preds = %1
  %4 = add i64 %0, 1
  %5 = tail call i64 @llvm.umax.i64(i64 %4, i64 2)
  %6 = add i64 %5, -1
  %7 = add i64 %5, -2
  %8 = and i64 %6, 7
  %9 = icmp ult i64 %7, 7
  br i1 %9, label %12, label %10

10:                                               ; preds = %3
  %11 = and i64 %6, -8
  br label %28

12:                                               ; preds = %28, %3
  %13 = phi double [ undef, %3 ], [ %54, %28 ]
  %14 = phi i64 [ 1, %3 ], [ %55, %28 ]
  %15 = phi double [ 1.000000e+00, %3 ], [ %54, %28 ]
  %16 = icmp eq i64 %8, 0
  br i1 %16, label %26, label %17

17:                                               ; preds = %12, %17
  %18 = phi i64 [ %23, %17 ], [ %14, %12 ]
  %19 = phi double [ %22, %17 ], [ %15, %12 ]
  %20 = phi i64 [ %24, %17 ], [ 0, %12 ]
  %21 = uitofp i64 %18 to double
  %22 = fmul double %19, %21
  %23 = add nuw i64 %18, 1
  %24 = add i64 %20, 1
  %25 = icmp eq i64 %24, %8
  br i1 %25, label %26, label %17, !llvm.loop !5

26:                                               ; preds = %12, %17, %1
  %27 = phi double [ 1.000000e+00, %1 ], [ %13, %12 ], [ %22, %17 ]
  ret double %27

28:                                               ; preds = %28, %10
  %29 = phi i64 [ 1, %10 ], [ %55, %28 ]
  %30 = phi double [ 1.000000e+00, %10 ], [ %54, %28 ]
  %31 = phi i64 [ 0, %10 ], [ %56, %28 ]
  %32 = uitofp i64 %29 to double
  %33 = fmul double %30, %32
  %34 = add nuw nsw i64 %29, 1
  %35 = uitofp i64 %34 to double
  %36 = fmul double %33, %35
  %37 = add nuw nsw i64 %29, 2
  %38 = uitofp i64 %37 to double
  %39 = fmul double %36, %38
  %40 = add nuw nsw i64 %29, 3
  %41 = uitofp i64 %40 to double
  %42 = fmul double %39, %41
  %43 = add nuw nsw i64 %29, 4
  %44 = uitofp i64 %43 to double
  %45 = fmul double %42, %44
  %46 = add nuw nsw i64 %29, 5
  %47 = uitofp i64 %46 to double
  %48 = fmul double %45, %47
  %49 = add nuw nsw i64 %29, 6
  %50 = uitofp i64 %49 to double
  %51 = fmul double %48, %50
  %52 = add nuw i64 %29, 7
  %53 = uitofp i64 %52 to double
  %54 = fmul double %51, %53
  %55 = add nuw i64 %29, 8
  %56 = add i64 %31, 8
  %57 = icmp eq i64 %56, %11
  br i1 %57, label %12, label %28, !llvm.loop !7
}

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @calc_exp(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp eq i64 %0, 0
  br i1 %2, label %7, label %3

3:                                                ; preds = %1
  %4 = add i64 %0, 1
  br label %9

5:                                                ; preds = %65
  %6 = fadd double %68, 1.000000e+00
  br label %7

7:                                                ; preds = %5, %1
  %8 = phi double [ 1.000000e+00, %1 ], [ %6, %5 ]
  ret double %8

9:                                                ; preds = %3, %65
  %10 = phi i64 [ 0, %3 ], [ %71, %65 ]
  %11 = phi i64 [ %0, %3 ], [ %69, %65 ]
  %12 = phi double [ 0.000000e+00, %3 ], [ %68, %65 ]
  %13 = sub i64 %4, %10
  %14 = tail call i64 @llvm.umax.i64(i64 %13, i64 2)
  %15 = add i64 %14, -1
  %16 = add i64 %14, -2
  %17 = and i64 %15, 7
  %18 = icmp ult i64 %16, 7
  br i1 %18, label %51, label %19

19:                                               ; preds = %9
  %20 = and i64 %15, -8
  br label %21

21:                                               ; preds = %21, %19
  %22 = phi i64 [ 1, %19 ], [ %48, %21 ]
  %23 = phi double [ 1.000000e+00, %19 ], [ %47, %21 ]
  %24 = phi i64 [ 0, %19 ], [ %49, %21 ]
  %25 = uitofp i64 %22 to double
  %26 = fmul double %23, %25
  %27 = add nuw nsw i64 %22, 1
  %28 = uitofp i64 %27 to double
  %29 = fmul double %26, %28
  %30 = add nuw nsw i64 %22, 2
  %31 = uitofp i64 %30 to double
  %32 = fmul double %29, %31
  %33 = add nuw nsw i64 %22, 3
  %34 = uitofp i64 %33 to double
  %35 = fmul double %32, %34
  %36 = add nuw nsw i64 %22, 4
  %37 = uitofp i64 %36 to double
  %38 = fmul double %35, %37
  %39 = add nuw nsw i64 %22, 5
  %40 = uitofp i64 %39 to double
  %41 = fmul double %38, %40
  %42 = add nuw nsw i64 %22, 6
  %43 = uitofp i64 %42 to double
  %44 = fmul double %41, %43
  %45 = add nuw i64 %22, 7
  %46 = uitofp i64 %45 to double
  %47 = fmul double %44, %46
  %48 = add nuw i64 %22, 8
  %49 = add i64 %24, 8
  %50 = icmp eq i64 %49, %20
  br i1 %50, label %51, label %21, !llvm.loop !7

51:                                               ; preds = %21, %9
  %52 = phi double [ undef, %9 ], [ %47, %21 ]
  %53 = phi i64 [ 1, %9 ], [ %48, %21 ]
  %54 = phi double [ 1.000000e+00, %9 ], [ %47, %21 ]
  %55 = icmp eq i64 %17, 0
  br i1 %55, label %65, label %56

56:                                               ; preds = %51, %56
  %57 = phi i64 [ %62, %56 ], [ %53, %51 ]
  %58 = phi double [ %61, %56 ], [ %54, %51 ]
  %59 = phi i64 [ %63, %56 ], [ 0, %51 ]
  %60 = uitofp i64 %57 to double
  %61 = fmul double %58, %60
  %62 = add nuw i64 %57, 1
  %63 = add i64 %59, 1
  %64 = icmp eq i64 %63, %17
  br i1 %64, label %65, label %56, !llvm.loop !9

65:                                               ; preds = %56, %51
  %66 = phi double [ %52, %51 ], [ %61, %56 ]
  %67 = fdiv double 1.000000e+00, %66
  %68 = fadd double %12, %67
  %69 = add i64 %11, -1
  %70 = icmp eq i64 %69, 0
  %71 = add i64 %10, 1
  br i1 %70, label %5, label %9, !llvm.loop !10
}

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #1 {
  %3 = icmp eq i32 %0, 2
  br i1 %3, label %4, label %16

4:                                                ; preds = %2
  %5 = getelementptr inbounds ptr, ptr %1, i64 1
  %6 = load ptr, ptr %5, align 8, !tbaa !11
  %7 = tail call i64 @strtol(ptr nocapture noundef nonnull %6, ptr noundef null, i32 noundef 10) #7
  %8 = trunc i64 %7 to i32
  %9 = shl i64 %7, 32
  %10 = ashr exact i64 %9, 32
  %11 = icmp ugt i32 %8, 10000
  br i1 %11, label %16, label %12

12:                                               ; preds = %4
  %13 = tail call ptr @__errno_location() #8
  %14 = load i32, ptr %13, align 4, !tbaa !15
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %18, label %16

16:                                               ; preds = %12, %4, %2
  %17 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %106

18:                                               ; preds = %12
  %19 = icmp eq i64 %9, 0
  br i1 %19, label %87, label %20

20:                                               ; preds = %18
  %21 = add nsw i64 %10, 1
  br label %24

22:                                               ; preds = %80
  %23 = fadd double %83, 1.000000e+00
  br label %87

24:                                               ; preds = %20, %80
  %25 = phi i64 [ 0, %20 ], [ %86, %80 ]
  %26 = phi i64 [ %10, %20 ], [ %84, %80 ]
  %27 = phi double [ 0.000000e+00, %20 ], [ %83, %80 ]
  %28 = sub i64 %21, %25
  %29 = tail call i64 @llvm.umax.i64(i64 %28, i64 2)
  %30 = add i64 %29, -1
  %31 = add i64 %29, -2
  %32 = and i64 %30, 7
  %33 = icmp ult i64 %31, 7
  br i1 %33, label %66, label %34

34:                                               ; preds = %24
  %35 = and i64 %30, -8
  br label %36

36:                                               ; preds = %36, %34
  %37 = phi i64 [ 1, %34 ], [ %63, %36 ]
  %38 = phi double [ 1.000000e+00, %34 ], [ %62, %36 ]
  %39 = phi i64 [ 0, %34 ], [ %64, %36 ]
  %40 = uitofp i64 %37 to double
  %41 = fmul double %38, %40
  %42 = add nuw nsw i64 %37, 1
  %43 = uitofp i64 %42 to double
  %44 = fmul double %41, %43
  %45 = add nuw nsw i64 %37, 2
  %46 = uitofp i64 %45 to double
  %47 = fmul double %44, %46
  %48 = add nuw nsw i64 %37, 3
  %49 = uitofp i64 %48 to double
  %50 = fmul double %47, %49
  %51 = add nuw nsw i64 %37, 4
  %52 = uitofp i64 %51 to double
  %53 = fmul double %50, %52
  %54 = add nuw nsw i64 %37, 5
  %55 = uitofp i64 %54 to double
  %56 = fmul double %53, %55
  %57 = add nuw nsw i64 %37, 6
  %58 = uitofp i64 %57 to double
  %59 = fmul double %56, %58
  %60 = add nuw i64 %37, 7
  %61 = uitofp i64 %60 to double
  %62 = fmul double %59, %61
  %63 = add nuw i64 %37, 8
  %64 = add i64 %39, 8
  %65 = icmp eq i64 %64, %35
  br i1 %65, label %66, label %36, !llvm.loop !7

66:                                               ; preds = %36, %24
  %67 = phi double [ undef, %24 ], [ %62, %36 ]
  %68 = phi i64 [ 1, %24 ], [ %63, %36 ]
  %69 = phi double [ 1.000000e+00, %24 ], [ %62, %36 ]
  %70 = icmp eq i64 %32, 0
  br i1 %70, label %80, label %71

71:                                               ; preds = %66, %71
  %72 = phi i64 [ %77, %71 ], [ %68, %66 ]
  %73 = phi double [ %76, %71 ], [ %69, %66 ]
  %74 = phi i64 [ %78, %71 ], [ 0, %66 ]
  %75 = uitofp i64 %72 to double
  %76 = fmul double %73, %75
  %77 = add nuw i64 %72, 1
  %78 = add i64 %74, 1
  %79 = icmp eq i64 %78, %32
  br i1 %79, label %80, label %71, !llvm.loop !17

80:                                               ; preds = %71, %66
  %81 = phi double [ %67, %66 ], [ %76, %71 ]
  %82 = fdiv double 1.000000e+00, %81
  %83 = fadd double %27, %82
  %84 = add i64 %26, -1
  %85 = icmp eq i64 %84, 0
  %86 = add i64 %25, 1
  br i1 %85, label %22, label %24, !llvm.loop !10

87:                                               ; preds = %18, %22
  %88 = phi double [ 1.000000e+00, %18 ], [ %23, %22 ]
  br label %91

89:                                               ; preds = %91
  %90 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, double noundef %103)
  br label %106

91:                                               ; preds = %91, %87
  %92 = phi i64 [ 0, %87 ], [ %104, %91 ]
  %93 = phi double [ 0.000000e+00, %87 ], [ %103, %91 ]
  %94 = fadd double %88, %93
  %95 = fadd double %88, %94
  %96 = fadd double %88, %95
  %97 = fadd double %88, %96
  %98 = fadd double %88, %97
  %99 = fadd double %88, %98
  %100 = fadd double %88, %99
  %101 = fadd double %88, %100
  %102 = fadd double %88, %101
  %103 = fadd double %88, %102
  %104 = add nuw nsw i64 %92, 10
  %105 = icmp eq i64 %104, 10000
  br i1 %105, label %89, label %91, !llvm.loop !18

106:                                              ; preds = %89, %16
  %107 = phi i32 [ 1, %16 ], [ 0, %89 ]
  ret i32 %107
}

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare ptr @__errno_location() local_unnamed_addr #2

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #5

; Function Attrs: nocallback nofree nosync nounwind speculatable willreturn memory(none)
declare i64 @llvm.umax.i64(i64, i64) #6

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { mustprogress nofree nosync nounwind willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { nofree nounwind "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nounwind willreturn "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind }
attributes #6 = { nocallback nofree nosync nounwind speculatable willreturn memory(none) }
attributes #7 = { nounwind }
attributes #8 = { nounwind willreturn memory(none) }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = distinct !{!7, !8}
!8 = !{!"llvm.loop.mustprogress"}
!9 = distinct !{!9, !6}
!10 = distinct !{!10, !8}
!11 = !{!12, !12, i64 0}
!12 = !{!"any pointer", !13, i64 0}
!13 = !{!"omnipotent char", !14, i64 0}
!14 = !{!"Simple C/C++ TBAA"}
!15 = !{!16, !16, i64 0}
!16 = !{!"int", !13, i64 0}
!17 = distinct !{!17, !6}
!18 = distinct !{!18, !8}
