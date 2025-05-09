add_llvm_library(TypePrinter MODULE
  TypePrinter.cpp

  PLUGIN_TOOL
  clang
)

set_target_properties(TypePrinter PROPERTIES
  COMPILE_FLAGS "-fno-rtti"
)
