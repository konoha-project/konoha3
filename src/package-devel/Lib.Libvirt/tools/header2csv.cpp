#include <clang/Basic/TargetInfo.h>
#include <clang/AST/ASTConsumer.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclGroup.h>

#include <string.h>
#include <clang/Frontend/CompilerInstance.h>
#include <clang/Frontend/CompilerInvocation.h>
#include <clang/Parse/ParseAST.h>
#include <iostream>

struct extractor : public clang::ASTConsumer {
    bool HandleTopLevelDecl(clang::DeclGroupRef decls) {
        for (auto& decl : decls) {
            if(auto const* fd = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
                handle_functiondecl(fd);
            }
        }
        return true;
    }

private:
    void handle_functiondecl(clang::FunctionDecl const* fd) const {
        std::string name = fd->getName();
        if(strncmp(name.c_str(), "vir", 3) == 0) {
            handle_mpi_functiondecl(fd);
        }
    }

    void handle_mpi_functiondecl(clang::FunctionDecl const* fd) const {
        std::vector<std::string> data;

        data.push_back(fd->getName());
        data.push_back(fd->getResultType().getAsString());

        for(auto I = fd->param_begin(), E = fd->param_end(); I != E; ++I) {
            data.push_back((*I)->getType().getAsString());
        }

        for(std::vector<std::string>::iterator I = data.begin(), E = data.end();
                I != E; ++I) {
            llvm::outs() << *I << ", ";
        }
        llvm::outs() << "\n";
    }
};

int main(int argc, char** argv) {
    clang::CompilerInstance compiler;
    compiler.createDiagnostics(argc, argv);

    auto& diag = compiler.getDiagnostics();
    auto& invocation = compiler.getInvocation();

    clang::CompilerInvocation::CreateFromArgs(invocation, argv + 1, argv + argc, diag);
    compiler.setTarget(clang::TargetInfo::CreateTargetInfo(diag, compiler.getTargetOpts()));

    compiler.createFileManager();
    compiler.createSourceManager(compiler.getFileManager());
    compiler.createPreprocessor();
    compiler.createASTContext();
    compiler.setASTConsumer(new extractor);
    //compiler.createSema(false, nullptr);

    auto& inputs = compiler.getFrontendOpts().Inputs;
    if(inputs.size() > 0) {
        compiler.InitializeSourceManager(inputs[0].File);
        clang::ParseAST(
            compiler.getPreprocessor(),
            &compiler.getASTConsumer(),
            compiler.getASTContext()
        );
    }

    return 0;
}
