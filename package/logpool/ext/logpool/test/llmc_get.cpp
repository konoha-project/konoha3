#include "llcache.h"
#include <iostream>
#include <llvm/LLVMContext.h>
using namespace std;
using namespace llvm;
using namespace logpool;

int main(int argc, char **argv)
{
    llmc llmc("0.0.0.0", 11211);
    LLVMContext &Context = getGlobalContext();
    Module *m = new Module("test", Context);
    cout << "initial state" << endl;
    (*m).dump();
    cout << "before get" << endl;
    (*m).dump();
    Function *F = llmc.get("fabs", m);
    cout << "after get" << endl;
    (*m).dump();
    (void)F;
    return 0;
}
