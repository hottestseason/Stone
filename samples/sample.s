; ModuleID = 'top'

@.str = private unnamed_addr constant [3 x i8] c"%f\00", align 1

define i32 @main() {
entry:
  %i = call double @fib(double 1000.0)
  call i32 (i8*, ...)* @printf(i8* getelementptr inbounds ([3 x i8]* @.str, i32 0, i32 0), double %i)
  ret i32 0
}

define double @fib(double %n) {
entry:
  %0 = fcmp ugt double %n, 1.000000e+00
  %1 = uitofp i1 %0 to double
  %2 = fcmp one double %1, 0.000000e+00
  br i1 %2, label %then, label %else

then:                                             ; preds = %entry
  %3 = fsub double %n, 1.000000e+00
  %4 = call double @fib(double %3)
  %5 = fsub double %n, 2.000000e+00
  %6 = call double @fib(double %5)
  %7 = fadd double %4, %6
  br label %merge

else:                                             ; preds = %entry
  br label %merge

merge:                                            ; preds = %else, %then
  %8 = phi double [ %7, %then ], [ %n, %else ]
  ret double %8
}

declare i32 @printf(i8*, ...)
