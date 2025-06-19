#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct InstrumentPass : PassInfoMixin<InstrumentPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &) {
        errs() << "Running InstrumentPass on function: " << F.getName() << "\n";

        if (F.isDeclaration())
            return PreservedAnalyses::all();

        Module *M = F.getParent();
        LLVMContext &Ctx = F.getContext();

        FunctionCallee Start = M->getOrInsertFunction(
            "instrument_start", FunctionType::get(Type::getVoidTy(Ctx), false));
        FunctionCallee End = M->getOrInsertFunction(
            "instrument_end", FunctionType::get(Type::getVoidTy(Ctx), false));

        // Вставка вызова instrument_start в начало функции
        BasicBlock &Entry = F.getEntryBlock();
        IRBuilder<> BuilderStart(&*Entry.getFirstInsertionPt());
        BuilderStart.CreateCall(Start);

        // Вставка вызова instrument_end перед каждым return
        for (BasicBlock &BB : F) {
            if (ReturnInst *Ret = dyn_cast<ReturnInst>(BB.getTerminator())) {
                IRBuilder<> BuilderEnd(Ret);
                BuilderEnd.CreateCall(End);
            }
        }

        return PreservedAnalyses::none();
    }
};

extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    puts(">>> [C] Plugin InstrumentPass loaded <<<");
    errs() << ">>> [C++] Plugin InstrumentPass loaded <<<\n";

    return {
        LLVM_PLUGIN_API_VERSION, "InstrumentPass", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, ModulePassManager &MPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "instrument") {
                        FunctionPassManager FPM;
                        FPM.addPass(InstrumentPass());
                        MPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPM)));
                        return true;
                    }
                    return false;
                });
        }
    };
}
