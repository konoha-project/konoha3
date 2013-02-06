// written by imasahiro
import("Type.Float");
import("Lib.LLVM");

void f() {
    Type.getVoidTy().dump();
    Type.getLabelTy().dump();
    Type.getFloatTy().dump();
    Type.getDoubleTy().dump();
    Type.getMetadataTy().dump();
    Type.getX86_FP80Ty().dump();
    Type.getFP128Ty().dump();
    Type.getPPC_FP128Ty().dump();
    Type.getX86_MMXTy().dump();
    Type.getInt1Ty().dump();
    Type.getInt8Ty().dump();
    Type.getInt16Ty().dump();
    Type.getInt32Ty().dump();
    Type.getInt64Ty().dump();
    Type.getFloatPtrTy().dump();
    Type.getDoublePtrTy().dump();
    Type.getX86_FP80PtrTy().dump();
    Type.getFP128PtrTy().dump();
    Type.getPPC_FP128PtrTy().dump();
    Type.getX86_MMXPtrTy().dump();
    Type.getInt1PtrTy().dump();
    Type.getInt8PtrTy().dump();
    Type.getInt16PtrTy().dump();
    Type.getInt32PtrTy().dump();
    Type.getInt64PtrTy().dump();
}
f();
