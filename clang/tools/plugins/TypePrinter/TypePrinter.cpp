#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/AST/Decl.h"
#include "clang/AST/DeclCXX.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;

namespace {

std::string accessToStr(AccessSpecifier AS) {
  switch (AS) {
    case AS_public: return "public";
    case AS_protected: return "protected";
    case AS_private: return "private";
    default: return "unspecified";
  }
}

class TypePrinterConsumer : public ASTConsumer {
public:
  void HandleTranslationUnit(ASTContext &Context) override {
    for (auto D : Context.getTranslationUnitDecl()->decls()) {
      if (const auto *RD = dyn_cast<CXXRecordDecl>(D)) {
        if (!RD->isThisDeclarationADefinition()  RD->isImplicit()) continue;

        llvm::outs() << RD->getNameAsString();

        if (RD->getNumBases() > 0) {
          for (const auto &Base : RD->bases()) {
            llvm::outs() << " -> " << Base.getType().getAsString();
          }
        }

        llvm::outs() << "\n";

        printFields(RD);
        printMethods(RD);
      }
    }
  }

private:
  void printFields(const CXXRecordDecl *RD) {
    bool hasFields = false;
    for (auto *field : RD->fields()) {
      if (!hasFields) {
        llvm::outs() << "|_Fields\n";
        hasFields = true;
      }
      llvm::outs() << "| |_ " << field->getNameAsString()
                   << " (" << field->getType().getAsString()
                   << "|" << accessToStr(field->getAccess()) << ")\n";
    }
  }

  void printMethods(const CXXRecordDecl *RD) {
    llvm::outs() << "\n|_Methods\n";
    for (const auto *method : RD->methods()) {
        if (isa<CXXConstructorDecl>(method)  isa<CXXDestructorDecl>(method)  method->isCopyAssignmentOperator()  method->isMoveAssignmentOperator())
            continue;

        std::string methodSig;
        llvm::raw_string_ostream sigStream(methodSig);

        method->getReturnType().print(sigStream, RD->getASTContext().getPrintingPolicy());
        sigStream << "(";
        for (unsigned i = 0; i < method->getNumParams(); ++i) {
            if (i > 0)
                sigStream << ", ";
            method->getParamDecl(i)->getType().print(sigStream, RD->getASTContext().getPrintingPolicy());
        }
        sigStream << ")";

        llvm::outs() << "| |_ " << method->getNameAsString()
                     << " (" << sigStream.str() << "|";

        switch (method->getAccess()) {
            case AS_public: llvm::outs() << "public"; break;
            case AS_protected: llvm::outs() << "protected"; break;
            case AS_private: llvm::outs() << "private"; break;
            default: llvm::outs() << "unspecified"; break;
        }

        if (method->isVirtual())
            llvm::outs() << "|virtual";
        if (method->isPureVirtual())
            llvm::outs() << "|pure";
        if (method->size_overridden_methods() > 0)
            llvm::outs() << "|override";

        llvm::outs() << ")\n";
    }
  }
};

class TypePrinterAction : public PluginASTAction {
protected:
  std::unique_ptr<ASTConsumer> CreateASTConsumer(CompilerInstance &, llvm::StringRef) override {
    return std::make_unique<TypePrinterConsumer>();
  }

  bool ParseArgs(const CompilerInstance &, const std::vector<std::string> &) override {
    return true;
  }
};

}

static FrontendPluginRegistry::Add<TypePrinterAction>
X("type-printer", "Prints information about user-defined types");