; ModuleID = 'examples/exp1/O2/exp1.c'
source_filename = "examples/exp1/O2/exp1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str.1 = private unnamed_addr constant [24 x i8] c"Exp (len %lu) = %.10lf\0A\00", align 1
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
  br i1 %3, label %4, label %19

4:                                                ; preds = %2
  %5 = getelementptr inbounds ptr, ptr %1, i64 1
  %6 = load ptr, ptr %5, align 8, !tbaa !11
  %7 = tail call i64 @strtol(ptr nocapture noundef nonnull %6, ptr noundef null, i32 noundef 10) #7
  %8 = trunc i64 %7 to i32
  %9 = shl i64 %7, 32
  %10 = ashr exact i64 %9, 32
  %11 = icmp ugt i32 %8, 10000
  br i1 %11, label %19, label %12

12:                                               ; preds = %4
  %13 = tail call ptr @__errno_location() #8
  %14 = load i32, ptr %13, align 4, !tbaa !15
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %16, label %19

16:                                               ; preds = %12
  %17 = icmp eq i64 %9, 0
  %18 = add nsw i64 %10, 1
  br label %21

19:                                               ; preds = %12, %4, %2
  %20 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %96

21:                                               ; preds = %16, %93
  %22 = phi i64 [ 0, %16 ], [ %94, %93 ]
  br i1 %17, label %88, label %25

23:                                               ; preds = %81
  %24 = fadd double %84, 1.000000e+00
  br label %88

25:                                               ; preds = %21, %81
  %26 = phi i64 [ %87, %81 ], [ 0, %21 ]
  %27 = phi i64 [ %85, %81 ], [ %10, %21 ]
  %28 = phi double [ %84, %81 ], [ 0.000000e+00, %21 ]
  %29 = sub i64 %18, %26
  %30 = tail call i64 @llvm.umax.i64(i64 %29, i64 2)
  %31 = add i64 %30, -1
  %32 = add i64 %30, -2
  %33 = and i64 %31, 7
  %34 = icmp ult i64 %32, 7
  br i1 %34, label %67, label %35

35:                                               ; preds = %25
  %36 = and i64 %31, -8
  br label %37

37:                                               ; preds = %37, %35
  %38 = phi i64 [ 1, %35 ], [ %64, %37 ]
  %39 = phi double [ 1.000000e+00, %35 ], [ %63, %37 ]
  %40 = phi i64 [ 0, %35 ], [ %65, %37 ]
  %41 = uitofp i64 %38 to double
  %42 = fmul double %39, %41
  %43 = add nuw nsw i64 %38, 1
  %44 = uitofp i64 %43 to double
  %45 = fmul double %42, %44
  %46 = add nuw nsw i64 %38, 2
  %47 = uitofp i64 %46 to double
  %48 = fmul double %45, %47
  %49 = add nuw nsw i64 %38, 3
  %50 = uitofp i64 %49 to double
  %51 = fmul double %48, %50
  %52 = add nuw nsw i64 %38, 4
  %53 = uitofp i64 %52 to double
  %54 = fmul double %51, %53
  %55 = add nuw nsw i64 %38, 5
  %56 = uitofp i64 %55 to double
  %57 = fmul double %54, %56
  %58 = add nuw nsw i64 %38, 6
  %59 = uitofp i64 %58 to double
  %60 = fmul double %57, %59
  %61 = add nuw i64 %38, 7
  %62 = uitofp i64 %61 to double
  %63 = fmul double %60, %62
  %64 = add nuw i64 %38, 8
  %65 = add i64 %40, 8
  %66 = icmp eq i64 %65, %36
  br i1 %66, label %67, label %37, !llvm.loop !7

67:                                               ; preds = %37, %25
  %68 = phi double [ undef, %25 ], [ %63, %37 ]
  %69 = phi i64 [ 1, %25 ], [ %64, %37 ]
  %70 = phi double [ 1.000000e+00, %25 ], [ %63, %37 ]
  %71 = icmp eq i64 %33, 0
  br i1 %71, label %81, label %72

72:                                               ; preds = %67, %72
  %73 = phi i64 [ %78, %72 ], [ %69, %67 ]
  %74 = phi double [ %77, %72 ], [ %70, %67 ]
  %75 = phi i64 [ %79, %72 ], [ 0, %67 ]
  %76 = uitofp i64 %73 to double
  %77 = fmul double %74, %76
  %78 = add nuw i64 %73, 1
  %79 = add i64 %75, 1
  %80 = icmp eq i64 %79, %33
  br i1 %80, label %81, label %72, !llvm.loop !17

81:                                               ; preds = %72, %67
  %82 = phi double [ %68, %67 ], [ %77, %72 ]
  %83 = fdiv double 1.000000e+00, %82
  %84 = fadd double %28, %83
  %85 = add i64 %27, -1
  %86 = icmp eq i64 %85, 0
  %87 = add i64 %26, 1
  br i1 %86, label %23, label %25, !llvm.loop !10

88:                                               ; preds = %21, %23
  %89 = phi double [ 1.000000e+00, %21 ], [ %24, %23 ]
  %90 = icmp eq i64 %22, %10
  br i1 %90, label %91, label %93

91:                                               ; preds = %88
  %92 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i64 noundef %10, double noundef %89)
  br label %93

93:                                               ; preds = %91, %88
  %94 = add nuw nsw i64 %22, 1
  %95 = icmp eq i64 %94, 10000
  br i1 %95, label %96, label %21, !llvm.loop !18

96:                                               ; preds = %93, %19
  %97 = phi i32 [ 1, %19 ], [ 0, %93 ]
  ret i32 %97
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
