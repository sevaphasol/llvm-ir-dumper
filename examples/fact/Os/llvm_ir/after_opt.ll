; ModuleID = '/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/fact/fact.c'
source_filename = "/home/sevsol/Projects/4sem/lisitsyn/llvm-ir-dumper/wt/graph_serialization_v2/examples/fact/fact.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-f80:128-n8:16:32:64-S128"
target triple = "x86_64-pc-linux-gnu"

@.str.1 = private unnamed_addr constant [17 x i8] c"Fact(%lu) = %lu\0A\00", align 1
@str = private unnamed_addr constant [34 x i8] c"Usage: 1 argument - factorial len\00", align 1
@str.2 = private unnamed_addr constant [34 x i8] c"Usage: 1 argument - factorial len\00", align 1

; Function Attrs: nofree norecurse nosync nounwind optsize memory(none) uwtable
define dso_local i64 @fact(i64 noundef %0) local_unnamed_addr #0 {
  %2 = icmp ult i64 %0, 2
  br i1 %2, label %9, label %3

3:                                                ; preds = %1, %3
  %4 = phi i64 [ %6, %3 ], [ %0, %1 ]
  %5 = phi i64 [ %7, %3 ], [ 1, %1 ]
  %6 = add i64 %4, -1
  %7 = mul i64 %4, %5
  %8 = icmp ult i64 %4, 3
  br i1 %8, label %9, label %3

9:                                                ; preds = %3, %1
  %10 = phi i64 [ 1, %1 ], [ %7, %3 ]
  ret i64 %10
}

; Function Attrs: nofree nounwind optsize uwtable
define dso_local noundef i32 @main(i32 noundef %0, ptr nocapture noundef readonly %1) local_unnamed_addr #1 {
  %3 = icmp eq i32 %0, 2
  br i1 %3, label %6, label %4

4:                                                ; preds = %2
  %5 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str.2)
  br label %27

6:                                                ; preds = %2
  %7 = tail call ptr @__errno_location() #6
  %8 = load i32, ptr %7, align 4, !tbaa !5
  %9 = icmp eq i32 %8, 0
  br i1 %9, label %10, label %25

10:                                               ; preds = %6
  %11 = getelementptr inbounds ptr, ptr %1, i64 1
  %12 = load ptr, ptr %11, align 8, !tbaa !9
  %13 = tail call i32 @atoi(ptr nocapture noundef %12) #7
  %14 = sext i32 %13 to i64
  %15 = icmp ult i32 %13, 2
  br i1 %15, label %22, label %16

16:                                               ; preds = %10, %16
  %17 = phi i64 [ %19, %16 ], [ %14, %10 ]
  %18 = phi i64 [ %20, %16 ], [ 1, %10 ]
  %19 = add i64 %17, -1
  %20 = mul i64 %18, %17
  %21 = icmp ult i64 %17, 3
  br i1 %21, label %22, label %16

22:                                               ; preds = %16, %10
  %23 = phi i64 [ 1, %10 ], [ %20, %16 ]
  %24 = tail call i32 (ptr, ...) @printf(ptr noundef nonnull dereferenceable(1) @.str.1, i64 noundef %14, i64 noundef %23) #8
  br label %27

25:                                               ; preds = %6
  %26 = tail call i32 @puts(ptr nonnull dereferenceable(1) @str)
  br label %27

27:                                               ; preds = %25, %22, %4
  %28 = phi i32 [ 1, %4 ], [ 0, %22 ], [ 1, %25 ]
  ret i32 %28
}

; Function Attrs: nofree nounwind optsize
declare noundef i32 @printf(ptr nocapture noundef readonly, ...) local_unnamed_addr #2

; Function Attrs: mustprogress nofree nounwind optsize willreturn memory(read)
declare i32 @atoi(ptr nocapture noundef) local_unnamed_addr #3

; Function Attrs: mustprogress nofree nosync nounwind optsize willreturn memory(none)
declare ptr @__errno_location() local_unnamed_addr #4

; Function Attrs: nofree nounwind
declare noundef i32 @puts(ptr nocapture noundef readonly) local_unnamed_addr #5

attributes #0 = { nofree norecurse nosync nounwind optsize memory(none) uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #1 = { nofree nounwind optsize uwtable "min-legal-vector-width"="0" "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #2 = { nofree nounwind optsize "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #3 = { mustprogress nofree nounwind optsize willreturn memory(read) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #4 = { mustprogress nofree nosync nounwind optsize willreturn memory(none) "no-trapping-math"="true" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cmov,+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "tune-cpu"="generic" }
attributes #5 = { nofree nounwind }
attributes #6 = { nounwind optsize willreturn memory(none) }
attributes #7 = { nounwind optsize willreturn memory(read) }
attributes #8 = { optsize }

!llvm.module.flags = !{!0, !1, !2, !3}
!llvm.ident = !{!4}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{i32 8, !"PIC Level", i32 2}
!2 = !{i32 7, !"PIE Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 2}
!4 = !{!"Ubuntu clang version 18.1.3 (1ubuntu1)"}
!5 = !{!6, !6, i64 0}
!6 = !{!"int", !7, i64 0}
!7 = !{!"omnipotent char", !8, i64 0}
!8 = !{!"Simple C/C++ TBAA"}
!9 = !{!10, !10, i64 0}
!10 = !{!"any pointer", !7, i64 0}
