#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Object/ObjectFile.h"
// Legacy Pass Manager
#include "llvm/IR/LegacyPassManager.h"
#include <unistd.h>
#include "AST/Decl.h"
#include <string>
#include <system_error>
using namespace llvm;

enum OptLevel {
    O0, O1, O2, O3
};
// Command line options.
cl::opt<std::string> InputFileName(cl::Positional,cl::desc("<input file>"), cl::Required);
cl::opt<std::string> OutputFileName("o", cl::desc("output filename"), cl::value_desc("filename"));
cl::opt<OptLevel> OptimizationLevel(cl::desc("Optimization level:"),
                                    cl::values(
                                            clEnumVal(O0, "default level, no optimizations"),
                                            clEnumVal(O1, "enable less optimizations"),
                                            clEnumVal(O2, "enable default optimizations"),
                                            clEnumVal(O3, "enable aggressive optimizations")
                                            ),
                                    cl::init(O0));
cl::opt<bool> EmitAssembly("S", cl::desc("emit LLVM IR"));
cl::opt<bool> EmitLLVMFile("emit-llvm", cl::desc("emit LLVM IR '.ll' or '.bc' file"))
cl::opt<bool> EmitObject("c", cl::desc("emit object file"));

CompileUnitDecl *ParseAST();
Module *GenLLVMIR(CompileUnitDecl *Unit) {
    Unit->CodeGen();
    return Unit->getModule();
}

TargetMachine *GetTargetMachine(Module *module) {
    // TargetTriples.
    // link to functionality with current machine.
    auto TargetTriple = sys::getDefaultTargetTriple();
    InitializeAllTargetInfos();
    InitializeAllTargets();
    InitializeAllTargetMCs();
    InitializeAllAsmParsers();
    InitializeAllAsmPrinters();
    std::string Error;
    auto Target = TargetRegistry::lookupTarget(TargetTriple, Error);
    // Error if we could not find requested targets.
    if (!Target) {
        errs() << Error;
        exit(1);
    }

    // TargetMachine.
    auto CPU = "generic";
    auto Features = "";
    TargetOptions TOpt;
    auto RM = Optional<Reloc::Model>();
    auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features, TOpt, RM);

    // Configuring the module.
    module->setDataLayout(TargetMachine->createDataLayout());
    module->setTargetTriple(TargetTriple);
    return TargetMachine;
}

void EmitObjectFile(Module *module, TargetMachine *target) {
    // Configuring output filesystem.
    std::string ObjectFileName;
    if (OutputFileName.c_str()) {
        ObjectFileName = OutputFileName;
    } else {
        ObjectFileName = InputFileName + ".o";
    }
    std::error_code ErrorCode;
    raw_fd_ostream dest(ObjectFileName, ErrorCode, sys::fs::OF_None);
    if (ErrorCode) {
        errs() << "Could not open file: " << ErrorCode.message();
        exit(1);
    }

    // Emit object code.
    auto FileType = CGFT_ObjectFile;
    legacy::PassManager pass;

    if (target->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TargetMachine can't emit object file";
        exit(1);
    }

    pass.run(*module);
    dest.flush();
}

void EmitLLVMIR(Module *module) {
    // Configuring output filesystem.
    std::string IRFileName;
    if (IRFileName.c_str()) {
        IRFileName = OutputFileName;
    } else {
        IRFileName = InputFileName + ".ll";
    }
    std::error_code ErrorCode;
    raw_fd_ostream dest(IRFileName, ErrorCode, sys::fs::OF_None);
    if (ErrorCode) {
        errs() << "Could not open file: " << ErrorCode.message();
        exit(1);
    }
    // write to .S
    module->print(dest, nullptr);
    dest.flush();
}

void RunOptPasses(Module *module, TargetMachine *target) {
    // Analysis managers.
    LoopAnalysisManager LAM;
    FunctionAnalysisManager FAM;
    CGSCCAnalysisManager CGAM;
    ModuleAnalysisManager MAM;
    // LLVM new pass builder.
    PassBuilder PB(target);
    // Register analysis.
    PB.registerModuleAnalyses(MAM);
    PB.registerCGSCCAnalyses(CGAM);
    PB.registerFunctionAnalyses(FAM);
    PB.registerLoopAnalyses(LAM);
    PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    // Create passmanager and run optimization.
    PassBuilder::OptimizationLevel OptLevel;
    ModulePassManager MPM;
    switch (OptimizationLevel) {
        case O0 : OptLevel = PassBuilder::OptimizationLevel::O0;
        case O1 : OptLevel = PassBuilder::OptimizationLevel::O1;
        case O2 : OptLevel = PassBuilder::OptimizationLevel::O2;
        case O3 : OptLevel = PassBuilder::OptimizationLevel::O3;
    }
    if (OptimizationLevel == O0)
        MPM = PB.buildO0DefaultPipeline(OptLevel);
    else
        MPM = PB.buildModuleOptimizationPipeline(OptLevel);
    MPM.run(*module, MAM);
}

int main(int argc, char **argv, char **envp) {
    cl::ParseCommandLineOptions(argc, argv);

    CompileUnitDecl *Unit = ParseAST();
    Module *module = GenLLVMIR(Unit);
    TargetMachine *target = GetTargetMachine(module);
    RunOptPasses(module, target);
    if (EmitAssembly) {
        EmitLLVMIR(module);
    }
    else if (EmitObject) {
        EmitObjectFile(module, target);
    }
    else {
        // GNU gold linker command line.
        char *ld_argv[] = {"ld", "-e", "main", "-o", "a.out"};
        if (!OutputFileName.empty()) {
            ld_argv[4] = const_cast<char *>(OutputFileName.c_str());
        }
        execve("/usr/bin/ld", ld_argv, envp);
    }
    return 0;
}
