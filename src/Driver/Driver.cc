#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Vectorize.h"
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
#include <cstdio>
#include <system_error>

extern FILE *yyin;
extern int yyparse(CompileUnitDecl &comp_unit);

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
cl::opt<bool> EmitAssembly("S", cl::desc("emit LLVM IR"), cl::init(false));
// cl::opt<bool> EmitLLVMFile("emit-llvm", cl::desc("emit LLVM IR '.ll' or '.bc' file"), cl::init(false));
cl::opt<bool> EmitObject("c", cl::desc("emit object file"), cl::init(false));
cl::opt<std::string> RuntimeHeaderDir("I", cl::desc("directory of builtin runtime function header llvm .ll files"),
                                      cl::value_desc("directory"),
                                      cl::init(RUNTIME_HEADER));

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

void EmitObjectFile(const std::string &ObjectFileName,
                    Module *module, TargetMachine *target) {
    // Configuring output filesystem.
    std::error_code ErrorCode;
    raw_fd_ostream dest(ObjectFileName, ErrorCode, sys::fs::OF_None);
    if (ErrorCode) {
        errs() << "Could not open file: " << ErrorCode.message() << "\n";
        exit(1);
    }

    // Emit object code.
    auto FileType = CGFT_ObjectFile;
    legacy::PassManager pass;

    if (target->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
        errs() << "TargetMachine can't emit object file" << "\n";
        exit(1);
    }

    pass.run(*module);
    dest.flush();
}

void EmitLLVMIR(const std::string &IRFileName,
                Module *module) {
    // Configuring output filesystem.

    std::error_code ErrorCode;
    raw_fd_ostream dest(IRFileName, ErrorCode, sys::fs::OF_None);
    if (ErrorCode) {
        errs() << "Could not open file: " << ErrorCode.message() << "\n";
        exit(1);
    }
    module->print(dest, nullptr);
    // write to .S
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
        MPM = PB.buildPerModuleDefaultPipeline(OptLevel);

    std::cout << "opt level: " << OptimizationLevel << "\n";
    MPM.run(*module, MAM);
}

int main(int argc, char **argv, char **envp) {
    cl::ParseCommandLineOptions(argc, argv);

    yyin = fopen(InputFileName.c_str(), "r");
    auto Unit = new CompileUnitDecl(InputFileName);
    TypeContext::Init(Unit->getContext());
    Unit->setRuntimeHeader(RuntimeHeaderDir);
    auto status = yyparse(*Unit);
    if (status != 0) {
        // do something dealing with parsing error.
    }

    Module *module = GenLLVMIR(Unit);
    TargetMachine *target = GetTargetMachine(module);
    RunOptPasses(module, target);
    if (EmitAssembly) {
        std::string IRFileName;
        if (!OutputFileName.empty()) {
            IRFileName = OutputFileName;
        } else {
            IRFileName = InputFileName + ".ll";
        }
        EmitLLVMIR(IRFileName, module);
    }
    else if (EmitObject) {
        std::string ObjectFileName;
        if (!OutputFileName.empty()) {
            ObjectFileName = OutputFileName;
        } else {
            ObjectFileName = InputFileName + ".o";
        }
        EmitObjectFile(ObjectFileName, module, target);
    }
    else {
        std::string ObjectFileName;
        ObjectFileName = InputFileName + ".o";
        EmitObjectFile(ObjectFileName, module, target);
        // GNU gold linker command line.
        // link mode. -no-pie (generate executable).
        char *ld_argv[] = {"gcc", "example.c.o", "-no-pie", "-o", "a.out", NULL};
        ld_argv[1] = const_cast<char *>(ObjectFileName.c_str());
        if (!OutputFileName.empty()) {
            ld_argv[4] = const_cast<char *>(OutputFileName.c_str());
        }
        CHECK_EQ(execve("/usr/bin/gcc", ld_argv, envp), 0);
    }
    return 0;
}
