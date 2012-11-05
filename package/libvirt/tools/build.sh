clang++ ./header2csv.cpp `llvm-config --cxxflags` \
`llvm-config --ldflags` \
`llvm-config --libs` \
/usr/local/lib/libclang.a \
/usr/local/lib/libclangAST.a \
/usr/local/lib/libclangASTMatchers.a \
/usr/local/lib/libclangAnalysis.a \
/usr/local/lib/libclangBasic.a \
/usr/local/lib/libclangDriver.a \
/usr/local/lib/libclangEdit.a \
/usr/local/lib/libclangFrontend.a \
/usr/local/lib/libclangFrontendTool.a \
/usr/local/lib/libclangLex.a \
/usr/local/lib/libclangParse.a \
/usr/local/lib/libclangRewriteCore.a \
/usr/local/lib/libclangRewriteFrontend.a \
/usr/local/lib/libclangSema.a \
/usr/local/lib/libclangSerialization.a \
/usr/local/lib/libclangStaticAnalyzerCheckers.a \
/usr/local/lib/libclangStaticAnalyzerCore.a \
/usr/local/lib/libclangStaticAnalyzerFrontend.a \
/usr/local/lib/libclangTooling.a \
-fno-rtti -fno-exceptions
