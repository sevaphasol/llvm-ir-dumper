; ModuleID = '/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/fact/fact.c'
source_filename = "/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/fact/fact.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str.1 = private unnamed_addr constant [17 x i8] c"Fact(%lu) = %lu\0A\00", align 1
@str = private unnamed_addr constant [34 x i8] c"Usage: 1 argument - factorial len\00", align 1
@str.2 = private unnamed_addr constant [34 x i8] c"Usage: 1 argument - factorial len\00", align 1

; Function Attrs: nofree norecurse nosync nounwind memory(none) uwtable
define dso_local i64 @fact(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp ult i64 %0, 2
  br i1 %2, label %45, label %3

3:                                                ; preds = %1
  %4 = add i64 %0, -1
  %5 = add i64 %0, -2
  %6 = and i64 %4, 7
  %7 = icmp ult i64 %5, 7
  br i1 %7, label %32, label %8

8:                                                ; preds = %3
  %9 = and i64 %4, -8
  br label %10

10:                                               ; preds = %10, %8
  %11 = phi i64 [ %0, %8 ], [ %28, %10 ]
  %12 = phi i64 [ 1, %8 ], [ %29, %10 ]
  %13 = phi i64 [ 0, %8 ], [ %30, %10 ]
  %14 = add i64 %11, -1
  %15 = mul i64 %11, %12
  %16 = add i64 %11, -2
  %17 = mul i64 %14, %15
  %18 = add i64 %11, -3
  %19 = mul i64 %16, %17
  %20 = add i64 %11, -4
  %21 = mul i64 %18, %19
  %22 = add i64 %11, -5
  %23 = mul i64 %20, %21
  %24 = add i64 %11, -6
  %25 = mul i64 %22, %23
  %26 = add i64 %11, -7
  %27 = mul i64 %24, %25
  %28 = add i64 %11, -8
  %29 = mul i64 %26, %27
  %30 = add i64 %13, 8
  %31 = icmp eq i64 %30, %9
  br i1 %31, label %32, label %10

32:                                               ; preds = %10, %3
  %33 = phi i64 [ undef, %3 ], [ %29, %10 ]
  %34 = phi i64 [ %0, %3 ], [ %28, %10 ]
  %35 = phi i64 [ 1, %3 ], [ %29, %10 ]
  %36 = icmp eq i64 %6, 0
  br i1 %36, label %45, label %37

37:                                               ; preds = %32, %37
  %38 = phi i64 [ %41, %37 ], [ %34, %32 ]
  %39 = phi i64 [ %42, %37 ], [ %35, %32 ]
  %40 = phi i64 [ %43, %37 ], [ 0, %32 ]
  %41 = add i64 %38, -1
  %42 = mul i64 %38, %39
  %43 = add i64 %40, 1
  %44 = icmp eq i64 %43, %6
  br i1 %44, label %45, label %37, !llvm.loop !5

45:                                               ; preds = %32, %37, %1
  %46 = phi i64 [ 1, %1 ], [ %33, %32 ], [ %42, %37 ]
  ret i64 %46
}

; Function Attrs: nofree nounwind uwtable
define dso_local noundef i32 @main(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #1 {
  %3 = icmp eq i32 %0, 2
  br i1 %3, label %6, label %4

4:                                                ; preds = %2
  %5 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.2)
  br label %64

6:                                                ; preds = %2
  %7 = tail call ptr @__errno_location() #6
  %8 = load i32, ptr %7, align 4, !tbaa !7
  %9 = icmp eq i32 %8, 0
  br i1 %9, label %10, label %62

10:                                               ; preds = %6
  %11 = getelementptr inbounds ptr, ptr %1, i64 1
  %12 = load ptr, ptr %11, align 8, !tbaa !11
  %13 = tail call i64 @strtol(ptr nocapture noundef nonnull %12, ptr noundef null, i32 noundef 10) #7
  %14 = shl i64 %13, 32
  %15 = ashr exact i64 %14, 32
  %16 = icmp ult i64 %15, 2
  br i1 %16, label %59, label %17

17:                                               ; preds = %10
  %18 = add nsw i64 %15, -1
  %19 = add nsw i64 %15, -2
  %20 = and i64 %18, 7
  %21 = icmp ult i64 %19, 7
  br i1 %21, label %46, label %22

22:                                               ; preds = %17
  %23 = and i64 %18, -8
  br label %24

24:                                               ; preds = %24, %22
  %25 = phi i64 [ %15, %22 ], [ %42, %24 ]
  %26 = phi i64 [ 1, %22 ], [ %43, %24 ]
  %27 = phi i64 [ 0, %22 ], [ %44, %24 ]
  %28 = add i64 %25, -1
  %29 = mul i64 %26, %25
  %30 = add i64 %25, -2
  %31 = mul i64 %29, %28
  %32 = add i64 %25, -3
  %33 = mul i64 %31, %30
  %34 = add i64 %25, -4
  %35 = mul i64 %33, %32
  %36 = add i64 %25, -5
  %37 = mul i64 %35, %34
  %38 = add i64 %25, -6
  %39 = mul i64 %37, %36
  %40 = add i64 %25, -7
  %41 = mul i64 %39, %38
  %42 = add i64 %25, -8
  %43 = mul i64 %41, %40
  %44 = add i64 %27, 8
  %45 = icmp eq i64 %44, %23
  br i1 %45, label %46, label %24

46:                                               ; preds = %24, %17
  %47 = phi i64 [ undef, %17 ], [ %43, %24 ]
  %48 = phi i64 [ %15, %17 ], [ %42, %24 ]
  %49 = phi i64 [ 1, %17 ], [ %43, %24 ]
  %50 = icmp eq i64 %20, 0
  br i1 %50, label %59, label %51

51:                                               ; preds = %46, %51
  %52 = phi i64 [ %55, %51 ], [ %48, %46 ]
  %53 = phi i64 [ %56, %51 ], [ %49, %46 ]
  %54 = phi i64 [ %57, %51 ], [ 0, %46 ]
  %55 = add i64 %52, -1
  %56 = mul i64 %53, %52
  %57 = add i64 %54, 1
  %58 = icmp eq i64 %57, %20
  br i1 %58, label %59, label %51, !llvm.loop !13

59:                                               ; preds = %46, %51, %10
  %60 = phi i64 [ 1, %10 ], [ %47, %46 ], [ %56, %51 ]
  %61 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i64 noundef %15, i64 noundef %60)
  br label %64

62:                                               ; preds = %6
  %63 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %64

64:                                               ; preds = %62, %59, %4
  %65 = phi i32 [ 1, %4 ], [ 0, %59 ], [ 1, %62 ]
  ret i32 %65
}

; Function Attrs: nofree nounwind
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nosync nounwind willreturn memory(none)
declare ptr @__errno_location() local_unnamed_addr #3

; Function Attrs: mustprogress nofree nounwind willreturn
declare i64 @strtol(ptr noundef readonly, ptr nocapture noundef, i32 noundef) local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #5

attributes #0 = { nofree norecurse nosync nounwind memory(none) uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #1 = { nofree nounwind uwtable "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "min-legal-vector-width"="0" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #2 = { nofree nounwind "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #3 = { mustprogress nofree nosync nounwind willreturn memory(none) "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #4 = { mustprogress nofree nounwind willreturn "approx-func-fp-math"="true" "denormal-fp-math"="preserve-sign,preserve-sign" "no-infs-fp-math"="true" "no-nans-fp-math"="true" "no-signed-zeros-fp-math"="true" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" "unsafe-fp-math"="true" }
attributes #5 = { nofree nounwind }
attributes #6 = { nounwind willreturn memory(none) }
attributes #7 = { nounwind }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = distinct !{!5, !6}
!6 = !{!"llvm.loop.unroll.disable"}
!7 = !{!8, !8, i64 0}
!8 = !{!"int", !9, i64 0}
!9 = !{!"omnipotent char", !10, i64 0}
!10 = !{!"Simple C/C++ TBAA"}
!11 = !{!12, !12, i64 0}
!12 = !{!"any pointer", !9, i64 0}
!13 = distinct !{!13, !6}
