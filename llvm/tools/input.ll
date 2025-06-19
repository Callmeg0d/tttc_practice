; ModuleID = 'input.ll'
source_filename = "test.c"

define i32 @main() {
entry:
  %a = alloca i32, align 4
  store i32 0, i32* %a, align 4
  %val = load i32, i32* %a, align 4
  %add = add nsw i32 %val, 42
  ret i32 %add
}