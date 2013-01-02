#include "codegen.h"
#include "FuelVM.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

static SValue System_Print(void *Context, SValue *Args)
{
    SValue Val = {};
    return Val;
}

static void f()
{
    FuelIRBuilder builderbuf = {}, *builder = &builderbuf;
    IRBuilder_Init(builder, 0);
    /*
     * void func() {
     *   boolean cond = true;
     *   int x = 10;
     *   if(cond) {
     *     x = 100;
     *   } else {
     *     x = 200;
     *   }
     *   print(x);
     *   return void;
     * }
     */
    Block *EntryBB = CreateBlock(builder);
    Block *ThenBB  = CreateBlock(builder);
    Block *ElseBB  = CreateBlock(builder);
    Block *MergeBB = CreateBlock(builder);

    IRBuilder_setBlock(builder, EntryBB);
    INode *Field0 = CreateLocal(builder, TYPE_boolean);
    INode *Field1 = CreateLocal(builder, TYPE_int);
    {
        INode *Val0 = CreateUpdate(builder, Field0, CreateBool(builder, 1));
        CreateUpdate(builder, Field1, CreateInt(builder, 10));
        CreateBranch(builder, Val0, ThenBB, ElseBB);
    }

    IRBuilder_setBlock(builder, ThenBB);
    {
        CreateUpdate(builder, Field1, CreateInt(builder, 100));
        IRBuilder_JumpTo(builder, MergeBB);
    }

    IRBuilder_setBlock(builder, ElseBB);
    {
        CreateUpdate(builder, Field1, CreateInt(builder, 200));
        IRBuilder_JumpTo(builder, MergeBB);
    }

    IRBuilder_setBlock(builder, MergeBB);
    {
        INode *Sys = CreateObject(builder, TYPE_Any, 0);
        INode *Mtd = CreateObject(builder, TYPE_Method, (void *) System_Print);
        INode *Param[3];
        Param[0] = Mtd;
        Param[1] = Sys;
        Param[2] = Field1;
        INode *Val = CreateCall(builder, DefaultCall, Param, 3);
        CreateReturn(builder, 0);
    }

    bool JITCompiled = false;
    IRBuilder_Compile(builder, EntryBB, &JITCompiled);
    IRBuilder_Exit(builder);
}

static void g()
{
    FuelIRBuilder builderbuf = {}, *builder = &builderbuf;
    IRBuilder_Init(builder);
    /*
     * int func(int n) {
     *   int i = 0;
     *   while(i < 10) {
     *       if(i == n) {
     *         i = i * 2;
     *         break;
     *       } else {
     *       }
     *   }
     *   return i;
     * }
     * BB0:  int i = 0;
     *       goto BB1
     * BB1:  bool cond0 = i < 10
     *       Branch cond0 BB2 BB6
     * BB2:  bool cond1 = i == n
     *       Branch cond1 BB3 BB4
     * BB3:  i = i * 2
     *       goto BB6
     * BB4:  goto BB5
     * BB5:  i = i + 1;
     *       goto BB1
     * BB6:  return i;
     */
    Block *BB0 = CreateBlock(builder);
    Block *BB1 = CreateBlock(builder);
    Block *BB2 = CreateBlock(builder);
    Block *BB3 = CreateBlock(builder);
    Block *BB4 = CreateBlock(builder);
    Block *BB5 = CreateBlock(builder);
    Block *BB6 = CreateBlock(builder);

    IRBuilder_setBlock(builder, BB0);
    INode *Field0 = CreateLocal(builder, TYPE_int);
    INode *Field1 = CreateLocal(builder, TYPE_int);

    {
        CreateUpdate(builder, Field0, CreateArgument(builder, 1));
        CreateUpdate(builder, Field1, CreateInt(builder, 0));
        IRBuilder_JumpTo(builder, BB1);
    }

    IRBuilder_setBlock(builder, BB1);
    {
        INode *Val0 = CreateBinaryInst(builder, Lt, Field1, CreateInt(builder, 10));
        CreateBranch(builder, Val0, BB2, BB6);
    }

    IRBuilder_setBlock(builder, BB2);
    {
        INode *Val0 = CreateBinaryInst(builder, Eq, Field1, Field0);
        CreateBranch(builder, Val0, BB3, BB4);
    }

    IRBuilder_setBlock(builder, BB3);
    {
        INode *Val0 = CreateBinaryInst(builder, Mul, Field1, CreateInt(builder, 2));
        CreateUpdate(builder, Field1, Val0);
        IRBuilder_JumpTo(builder, BB6);
    }

    IRBuilder_setBlock(builder, BB4);
    {
        IRBuilder_JumpTo(builder, BB5);
    }

    IRBuilder_setBlock(builder, BB5);
    {
        INode *Val0 = CreateBinaryInst(builder, Add, Field1, CreateInt(builder, 1));
        CreateUpdate(builder, Field1, Val0);
        IRBuilder_JumpTo(builder, BB1);
    }

    IRBuilder_setBlock(builder, BB6);
    {
        CreateReturn(builder, Field1);
    }

    bool JITCompiled = false;
    union ByteCode *code = IRBuilder_Compile(builder, BB0, &JITCompiled);
    SValue Stack[128];
    Stack[0].ival = 6;
    SValue Ret = FuelVM_Exec(Stack+1, code+1);
    fprintf(stderr, "%lld\n", Ret.ival);
    IRBuilder_Exit(builder);
}

static void h()
{
    FuelIRBuilder builderbuf = {}, *builder = &builderbuf;
    IRBuilder_Init(builder);
    /*
     * int fibo(int n) {
     * if(n < 3) {
     *     return 1;
     * } else {
     *   return fibo(n-1) + fibo(n-2);
     * }
     * BB0:  bool cond0 = i < 3
     *       Branch cond0 BB1 BB2
     * BB1:  return 1;
     * BB2:  int sub0 = i - 1;
     *       int ret0 = fibo(sub0);
     *       int sub1 = i - 2;
     *       int ret1 = fibo(sub1);
     *       return ret0 + ret1;
     */
    Block *BB0 = CreateBlock(builder);
    Block *BB1 = CreateBlock(builder);
    Block *BB2 = CreateBlock(builder);

    IRBuilder_setBlock(builder, BB0);
    {
        INode *Val0 = CreateBinaryInst(builder, Lt,
                CreateArgument(builder, 1), CreateInt(builder, 10));
        CreateBranch(builder, Val0, BB1, BB2);
    }

    IRBuilder_setBlock(builder, BB1);
    {
        CreateReturn(builder, CreateInt(builder, 1));
    }

    IRBuilder_setBlock(builder, BB2);
    {
        INode *Sys = CreateObject(builder, TYPE_Any, 0);
        INode *Mtd = CreateObject(builder, TYPE_Method, (void *) 0);

        INode *Val0 = CreateBinaryInst(builder, Sub,
                CreateArgument(builder, 1), CreateInt(builder, 1));
        INode *Val1 = CreateCall(builder, DefaultCall);
        CallInst_addParam((ICall *)Val1, Mtd);
        CallInst_addParam((ICall *)Val1, Sys);
        CallInst_addParam((ICall *)Val1, Val0);

        INode *Val2 = CreateBinaryInst(builder, Sub,
                CreateArgument(builder, 1), CreateInt(builder, 2));
        INode *Val3 = CreateCall(builder, DefaultCall);
        CallInst_addParam((ICall *)Val3, Mtd);
        CallInst_addParam((ICall *)Val3, Sys);
        CallInst_addParam((ICall *)Val3, Val2);

        INode *Val4 = CreateBinaryInst(builder, Add, Val0, Val1);
        CreateReturn(builder, Val4);
    }

    bool JITCompiled = false;
    //IRBuilder_Compile(builder, BB0, &JITCompiled);
    IRBuilder_Exit(builder);
}

int main(int argc, char const* argv[])
{
    f();
    g();
    //h();
    return 0;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
