; ModuleID = '/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/exp1/exp1.c'
source_filename = "/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/exp1/exp1.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str.1 = private unnamed_addr constant [24 x i8] c"Exp (len %lu) = %.10lf\0A\00", align 1
@str = private unnamed_addr constant [48 x i8] c"Usage: 1 argument - exponent series len < 10000\00", align 1

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @fact(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp eq i64 %0, 0
  br i1 %2, label %6, label %3

3:                                                ; preds = %1
  %4 = add i64 %0, 1
  %5 = tail call i64 @llvm.umax.i64(i64 %4, i64 2)
  br label %8

6:                                                ; preds = %8, %1
  %7 = phi double [ 1.000000e+00, %1 ], [ %12, %8 ]
  ret double %7

8:                                                ; preds = %3, %8
  %9 = phi i64 [ %13, %8 ], [ 1, %3 ]
  %10 = phi double [ %12, %8 ], [ 1.000000e+00, %3 ]
  %11 = uitofp i64 %9 to double
  %12 = fmul double %10, %11
  %13 = add nuw i64 %9, 1
  %14 = icmp eq i64 %13, %5
  br i1 %14, label %6, label %8, !llvm.loop !5
}

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local double @calc_exp(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp eq i64 %0, 0
  br i1 %2, label %5, label %7

3:                                                ; preds = %19
  %4 = fadd double %21, 1.000000e+00
  br label %5

5:                                                ; preds = %3, %1
  %6 = phi double [ 1.000000e+00, %1 ], [ %4, %3 ]
  ret double %6

7:                                                ; preds = %1, %19
  %8 = phi i64 [ %22, %19 ], [ %0, %1 ]
  %9 = phi double [ %21, %19 ], [ 0.000000e+00, %1 ]
  %10 = add i64 %8, 1
  %11 = tail call i64 @llvm.umax.i64(i64 %10, i64 2)
  br label %12

12:                                               ; preds = %12, %7
  %13 = phi i64 [ %17, %12 ], [ 1, %7 ]
  %14 = phi double [ %16, %12 ], [ 1.000000e+00, %7 ]
  %15 = uitofp i64 %13 to double
  %16 = fmul double %14, %15
  %17 = add nuw i64 %13, 1
  %18 = icmp eq i64 %17, %11
  br i1 %18, label %19, label %12, !llvm.loop !5

19:                                               ; preds = %12
  %20 = fdiv double 1.000000e+00, %16
  %21 = fadd double %9, %20
  %22 = add i64 %8, -1
  %23 = icmp eq i64 %22, 0
  br i1 %23, label %3, label %7, !llvm.loop !8
}

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #1 {
  %3 = icmp eq i32 %0, 2
  br i1 %3, label %4, label %18

4:                                                ; preds = %2
  %5 = getelementptr inbounds ptr, ptr %1, i64 1
  %6 = load ptr, ptr %5, align 8, !tbaa !9
  %7 = tail call i64 @strtol(ptr nocapture noundef nonnull %6, ptr noundef null, i32 noundef 10) #7
  %8 = trunc i64 %7 to i32
  %9 = shl i64 %7, 32
  %10 = ashr exact i64 %9, 32
  %11 = icmp ugt i32 %8, 10000
  br i1 %11, label %18, label %12

12:                                               ; preds = %4
  %13 = tail call ptr @__errno_location() #8
  %14 = load i32, ptr %13, align 4, !tbaa !13
  %15 = icmp eq i32 %14, 0
  br i1 %15, label %16, label %18

16:                                               ; preds = %12
  %17 = icmp eq i64 %9, 0
  br label %20

18:                                               ; preds = %12, %4, %2
  %19 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %49

20:                                               ; preds = %16, %46
  %21 = phi i64 [ 0, %16 ], [ %47, %46 ]
  br i1 %17, label %41, label %24

22:                                               ; preds = %36
  %23 = fadd double %38, 1.000000e+00
  br label %41

24:                                               ; preds = %20, %36
  %25 = phi i64 [ %39, %36 ], [ %10, %20 ]
  %26 = phi double [ %38, %36 ], [ 0.000000e+00, %20 ]
  %27 = add i64 %25, 1
  %28 = tail call i64 @llvm.umax.i64(i64 %27, i64 2)
  br label %29

29:                                               ; preds = %29, %24
  %30 = phi i64 [ %34, %29 ], [ 1, %24 ]
  %31 = phi double [ %33, %29 ], [ 1.000000e+00, %24 ]
  %32 = uitofp i64 %30 to double
  %33 = fmul double %31, %32
  %34 = add nuw i64 %30, 1
  %35 = icmp eq i64 %34, %28
  br i1 %35, label %36, label %29, !llvm.loop !5

36:                                               ; preds = %29
  %37 = fdiv double 1.000000e+00, %33
  %38 = fadd double %26, %37
  %39 = add i64 %25, -1
  %40 = icmp eq i64 %39, 0
  br i1 %40, label %22, label %24, !llvm.loop !8

41:                                               ; preds = %20, %22
  %42 = phi double [ 1.000000e+00, %20 ], [ %23, %22 ]
  %43 = icmp eq i64 %21, %10
  br i1 %43, label %44, label %46

44:                                               ; preds = %41
  %45 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i64 noundef %10, double noundef %42)
  br label %46

46:                                               ; preds = %44, %41
  %47 = add nuw nsw i64 %21, 1
  %48 = icmp eq i64 %47, 10000
  br i1 %48, label %49, label %20, !llvm.loop !15

49:                                               ; preds = %46, %18
  %50 = phi i32 [ 1, %18 ], [ 0, %46 ]
  ret i32 %50
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
!5 = distinct !{!5, !6, !7}
!6 = !{!"llvm.loop.mustprogress"}
!7 = !{!"llvm.loop.unroll.disable"}
!8 = distinct !{!8, !6, !7}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !11, i64 0}
!11 = !{!"omnipotent char", !12, i64 0}
!12 = !{!"Simple C/C++ TBAA"}
!13 = !{!14, !14, i64 0}
!14 = !{!"int", !11, i64 0}
!15 = distinct !{!15, !6, !7}
