@.str = private unnamed_addr constant [25 x i8] c"\E8\AF\B7\E8\BE\93\E5\85\A5\E4\B8\80\E4\B8\AA\E6\95\B4\E6\95\B0\EF\BC\9A\00", align 1
@.str.1 = private unnamed_addr constant [3 x i8] c"%d\00", align 1
@.str.2 = private unnamed_addr constant [22 x i8] c"\E9\98\B6\E4\B9\98\E7\BB\93\E6\9E\9C\E6\98\AF\EF\BC\9A%g\0A\00", align 1

define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i32, align 4
  %3 = alloca double, align 8
  %4 = alloca [1000 x double], align 16
  %5 = alloca i32, align 4
  store i32 0, i32* %1, align 4
  store double 1.000000e+00, double* %3, align 8
  %6 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([25 x i8], ptr @.str, i64 0, i64 0))
  %7 = call i32 (i8*, ...) @__isoc99_scanf(i8* noundef getelementptr inbounds ([3 x i8], ptr @.str.1, i64 0, i64 0), i32* noundef %2)
  store i32 0, i32* %5, align 4
  br label %8

8:                                                ; preds = %19, %0
  %9 = load i32, i32* %5, align 4
  %10 = load i32, i32* %2, align 4
  %11 = icmp slt i32 %9, %10
  br i1 %11, label %12, label %22

12:                                               ; preds = %8
  %13 = load i32, i32* %5, align 4
  %14 = add nsw i32 %13, 1
  %15 = sitofp i32 %14 to double
  %16 = load i32, i32* %5, align 4
  %17 = sext i32 %16 to i64
  %18 = getelementptr inbounds [1000 x double], ptr %4, i64 0, i64 %17
  store double %15, double* %18, align 8
  br label %19

19:                                               ; preds = %12
  %20 = load i32, i32* %5, align 4
  %21 = add nsw i32 %20, 1
  store i32 %21, i32* %5, align 4
  br label %8, !llvm.loop !6

22:                                               ; preds = %8
  store i32 0, i32* %5, align 4
  br label %23

23:                                               ; preds = %34, %22
  %24 = load i32, i32* %5, align 4
  %25 = load i32, i32* %2, align 4
  %26 = icmp slt i32 %24, %25
  br i1 %26, label %27, label %37

27:                                               ; preds = %23
  %28 = load i32, i32* %5, align 4
  %29 = sext i32 %28 to i64
  %30 = getelementptr inbounds [1000 x double], ptr %4, i64 0, i64 %29
  %31 = load double, double* %30, align 8
  %32 = load double, double* %3, align 8
  %33 = fmul double %32, %31
  store double %33, double* %3, align 8
  br label %34

34:                                               ; preds = %27
  %35 = load i32, i32* %5, align 4
  %36 = add nsw i32 %35, 1
  store i32 %36, i32* %5, align 4
  br label %23, !llvm.loop !8

37:                                               ; preds = %23
  %38 = load double, double* %3, align 8
  %39 = call i32 (i8*, ...) @printf(i8* noundef getelementptr inbounds ([22 x i8], ptr @.str.2, i64 0, i64 0), double noundef %38)
  ret i32 0
}

declare i32 @printf(i8* noundef, ...) #1

declare i32 @__isoc99_scanf(i8* noundef, ...) #1

!6 = distinct !{!6}
!8 = distinct !{!8}
