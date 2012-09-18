#include "llcache.h"

#include <iostream>
#include <llvm/Module.h>
#include <llvm/LLVMContext.h>
#include <llvm/Support/IRBuilder.h>
#include <llvm/ADT/SmallVector.h>

using namespace std;
using namespace llvm;
using namespace logpool;
/*
 * float t1(float p) { return (p>0)?p:-p; }
 */
Function *createAbs(Module *m, LLVMContext &Context) {
    Type *floatTy = Type::getFloatTy(Context);
    Type *ArgTypes[] = { floatTy };
    FunctionType *Ty = FunctionType::get(floatTy, ArgTypes, false);
    Function *func = Function::Create(Ty,
            GlobalValue::ExternalLinkage, "fabs", m);

    BasicBlock *bb = BasicBlock::Create(Context, "EntryBlock", func);
    IRBuilder<> *builder = new IRBuilder<>(bb);

    Value *arg0 = func->arg_begin();arg0->setName("arg0");
    Value *v    = builder->CreateFCmpOGE(arg0, ConstantFP::get(floatTy, 0.0));
    Value *v0   = builder->CreateFNeg(arg0);
    Value *ret  = builder->CreateSelect(v, arg0, v0);
    builder->CreateRet(ret);
    return func;
}

int main(int argc, char **argv)
{
    llmc llmc("0.0.0.0", 11211);
    LLVMContext &Context = getGlobalContext();
    Module *m = new Module("test", Context);
    Function *F = createAbs(m, Context);
    cout << "initial state" << endl;
    (*m).dump();
    llmc.set("fabs", F);
    F->eraseFromParent();
    cout << "before set" << endl;
    (*m).dump();
    F = llmc.get("fabs", m);
    cout << "after set" << endl;
    (*m).dump();
    return 0;
}
