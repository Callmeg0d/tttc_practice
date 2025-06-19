; ModuleID = 'input.ll'
source_filename = "test.c"

define i32 @main() {
entry:
  call void @instrument_start()
  %a = alloca i32, align 4
  store i32 0, i32* %a, align 4
  %val = load i32, i32* %a, align 4
  %add = add nsw i32 %val, 42
  call void @instrument_end()
  ret i32 %add
}

declare void @instrument_start()

declare void @instrument_end()
