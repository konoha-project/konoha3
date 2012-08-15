#include <vector>
#include <string>
#include <iostream>
#include <cstdio>
#include "llvm/LLVMContext.h"
#include "llvm/Linker.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/system_error.h"

#include "llcache.h"

using namespace std;
using namespace llvm;
using namespace logpool;

void llmc::init(const std::string host, long port)
{
    static int once = 1;
    if (once) {
        once = 0;
        InitializeNativeTarget();
    }
    memcached_return_t rc;
    memcached_server_list_st servers;

    st = memcached_create(NULL);
    if (st == NULL) {
        /* TODO Error */
        abort();
    }
    servers = memcached_server_list_append(NULL, host.c_str(), port, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        /* TODO Error */
        cerr <<"Error!! " << memcached_strerror(st, rc) << endl;
        abort();
    }
    rc = memcached_server_push(st, servers);
    if (rc != MEMCACHED_SUCCESS) {
        /* TODO Error */
        cerr <<"Error!! " << memcached_strerror(st, rc) << endl;
        abort();
    }
    memcached_server_list_free(servers);
}

static Function *Cloning(Module *M, Function *F)
{
    Function *NewF = CloneFunction(F);
    M->getFunctionList().push_back(NewF);
    return NewF;
}

void llmc::set(const std::string key, Function *F)
{
    LLVMContext &Context = getGlobalContext();
    Module *m = new Module("_", Context);
    Cloning(m, F);
    SmallString<1024> Mem;
    raw_svector_ostream OS(Mem);
    WriteBitcodeToFile(m, OS);

    std::cerr << "llmc::set do; '" << key << "'" << std::endl;
    memcached_return_t rc = memcached_set(st, key.c_str(), key.size(),
            (char *) &Mem[0], Mem.size(), 0, 0);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "llmc::set failed; '" << key << "'" << std::endl;
    }
}

void llmc::set(const std::string key, Module *m)
{
    std::string Mem;
    raw_string_ostream OS(Mem);
    WriteBitcodeToFile(m, OS);

    std::string Res = OS.str();
    std::cerr << "llmc::set do; '" << key << "'" << std::endl;
    memcached_return_t rc = memcached_set(st, key.c_str(), key.size(),
            (char *) Res.c_str(), Res.size(), 0, 0);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "llmc::set failed; '" << key << "'" << std::endl;
    }
}


Function *llmc::get(const std::string key, Module *m)
{
    memcached_return_t rc;
    LLVMContext &Context = getGlobalContext();
    size_t vlen;
    uint32_t flags;
    char *value = memcached_get(st, key.c_str(), key.size(), &vlen, &flags, &rc);
    if (rc != MEMCACHED_SUCCESS) {
        std::cerr << "llmc::get failed; '" << key << "'" << std::endl;
        return NULL;
    }
    StringRef Input(value, vlen);
    MemoryBuffer *Buffer;
    Buffer = MemoryBuffer::getMemBuffer(Input, "<llmc>", false);
    std::string ErrMsg;
    Module *newm = ParseBitcodeFile(Buffer, Context, &ErrMsg);
    if (!newm) {
        std::cout << "Error" << ErrMsg << std::endl;
        return NULL;
    }
    if (Linker::LinkModules(m, newm, Linker::DestroySource, &ErrMsg)) {
        std::cout << "error" << ErrMsg << std::endl;
        return NULL;
    }
    return m->getFunction(key);
}
#ifdef __cplusplus
extern "C" {
#endif

llcache_t *llcache_new(const char *host, long port)
{
    return (llcache_t*) new llmc(host, port);
}

void llcache_set(llcache_t *llmc, const char *key, const char *filename)
{
    logpool::llmc *c = (logpool::llmc*) llmc;
    LLVMContext &Context = getGlobalContext();
    OwningPtr<MemoryBuffer> Buffer;
    if (error_code ec = MemoryBuffer::getFileOrSTDIN((char*)filename, Buffer)) {
        fprintf(stderr, "ParseError\n");
        return;
    }
    std::string ErrMsg;
    Module *M = ParseBitcodeFile(Buffer.get(), Context, &ErrMsg);
    if (M) {
        //Function *F = M->getFunction(key);
        c->set(key, M);
    }
}

void *llcache_get(llcache_t *llmc, const char *key)
{
    logpool::llmc *c = (logpool::llmc*) llmc;
    LLVMContext &Context = getGlobalContext();
    Module *m = new Module("tmp", Context);
    Function *f = c->get(key, m);
    ExecutionEngine *ee = EngineBuilder(m).setEngineKind(EngineKind::JIT).create();
    void *ptr = ee->getPointerToFunction(f);
    return ptr;
}

void llcache_delete(llcache_t * llmc)
{
    logpool::llmc *c = (logpool::llmc*) llmc;
    delete c;
}

#ifdef __cplusplus
}
#endif
