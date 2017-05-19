//===-- examples/clang-interpreter/main.cpp - Clang C Interpreter Example -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include <cstdio>
#include "clang/CodeGen/CodeGenAction.h"
#include "clang/Basic/DiagnosticOptions.h"
#include "clang/Driver/Compilation.h"
#include "clang/Driver/Driver.h"
#include "clang/Driver/Tool.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/CompilerInvocation.h"
#include "clang/Frontend/FrontendDiagnostic.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include <memory>
using namespace clang;
using namespace clang::driver;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
std::string GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::fs::getMainExecutable(Argv0, MainAddr);
}

static llvm::ExecutionEngine *
createExecutionEngine(std::unique_ptr<llvm::Module> M, std::string *ErrorStr) {
  return llvm::EngineBuilder(std::move(M))
      .setEngineKind(llvm::EngineKind::Either)
      .setErrorStr(ErrorStr)
      .create();
}

static int Execute(std::unique_ptr<llvm::Module> Mod, char *const *envp) {
  printf("Execute\n");

  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  llvm::Module &M = *Mod;
  std::string Error;
  std::unique_ptr<llvm::ExecutionEngine> EE(
      createExecutionEngine(std::move(Mod), &Error));
  if (!EE) {
    llvm::errs() << "unable to make execution engine: " << Error << "\n";
    return 255;
  }

  printf("Created engine\n");

  llvm::Function *EntryFn = M.getFunction("main");
  if (!EntryFn) {
    llvm::errs() << "'main' function not found in module.\n";
    return 255;
  }

  // FIXME: Support passing arguments.
  std::vector<std::string> Args;
  Args.push_back(M.getModuleIdentifier());

  EE->finalizeObject();
  return EE->runFunctionAsMain(EntryFn, Args, envp);
}

int main(int argc, const char **argv, char * const *envp) {
  printf("main!!\n");
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  std::string Path = GetExecutablePath(argv[0]);
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter *DiagClient =
    new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);

  printf("DiagnosticsEngine\n");

  // Use ELF on windows for now.
  std::string TripleStr = llvm::sys::getProcessTriple();
  llvm::Triple T(TripleStr);
  if (T.isOSBinFormatCOFF())
    T.setObjectFormat(llvm::Triple::ELF);

  Driver TheDriver(Path, T.str(), Diags);
  TheDriver.setTitle("clang interpreter");
  TheDriver.setCheckInputsExist(false);

  printf("TheDriver is here\n");

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  SmallVector<const char *, 16> Args = {};
  for (int c = 1; c < argc; c++) {
  	Args.push_back(argv[c]);
  }

  std::unique_ptr<Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C) {
  	printf("No compilation!\n");
    return 0;
  }

  printf("BuildCompilation\n");

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const driver::JobList &Jobs = C->getJobs();
  printf("getJobs!");
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin())) {
  	printf("Jobs.size %d", Jobs.size());
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    Jobs.Print(OS, "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    return 1;
  }

  printf("Jobs\n");

  const driver::Command &Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd.getCreator().getName()) != "clang") {
  	printf("Problem with jobs!\n");
    Diags.Report(diag::err_fe_expected_clang_command);
    return 1;
  }

  // Initialize a compiler invocation object from the clang (-cc1) arguments.
  const driver::ArgStringList &CCArgs = Cmd.getArguments();
  std::unique_ptr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) +
                                       CCArgs.size(),
                                     Diags);

  printf("Invocation\n");

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
  	printf("Problem with invocation!\n");
    llvm::errs() << "clang invocation:\n";
    Jobs.Print(llvm::errs(), "\n", true);
    llvm::errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  CompilerInstance Clang;
  Clang.setInvocation(std::move(CI));

  // Create the compilers actual diagnostics engine.
  Clang.createDiagnostics();
  if (!Clang.hasDiagnostics()) {
  	printf("Diagonostics????\n");
    return 1;
  }

  printf("Created Diagonosticsg\n");

  // Infer the builtin include path if unspecified.
  if (Clang.getHeaderSearchOpts().UseBuiltinIncludes &&
      Clang.getHeaderSearchOpts().ResourceDir.empty())
    Clang.getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(argv[0], MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  std::unique_ptr<CodeGenAction> Act(new EmitLLVMOnlyAction());
  if (!Clang.ExecuteAction(*Act)) {
  	printf("Wtf happened with ExecuteAction?\n");
    return 1;
  }

  printf("ExecutedAction\n");

  int Res = 255;
  if (std::unique_ptr<llvm::Module> Module = Act->takeModule())
    Res = Execute(std::move(Module), envp);

  // Shutdown.

  printf("Shutdown\n");

  llvm::llvm_shutdown();

  return Res;
}