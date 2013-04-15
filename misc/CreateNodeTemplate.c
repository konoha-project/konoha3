#include "konoha3.h"
#include "konoha3/node2.h"
#include <stdio.h>

#define PRINT(T) \
    fprintf(stderr, "static void k" #T "Node_init(KonohaContext *kctx, kObject *o, void *conf)\n");\
    fprintf(stderr, "{\n");\
    fprintf(stderr, "\tk" #T "Node *Node = (k" #T "Node *) o;\n");\
    fprintf(stderr, "\t//KFieldInit(Node, Node->, K_NULL);\n");\
    fprintf(stderr, "}\n");\
    fprintf(stderr, "\n");\
    fprintf(stderr, "static void k" #T "Node_reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)\n");\
    fprintf(stderr, "{\n");\
    fprintf(stderr, "\tk" #T "Node *Node = (k" #T "Node *) o;\n");\
    fprintf(stderr, "\t//KRefTrace(Node->);\n");\
    fprintf(stderr, "}\n");\
    fprintf(stderr, "\n");\
    fprintf(stderr, "static void k" #T "Node_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)\n");\
    fprintf(stderr, "{\n");\
    fprintf(stderr, "\tk" #T "Node *Node = (k" #T "Node *) v[pos].asObject;\n");\
    fprintf(stderr, "\tKLIB KBuffer_printf(kctx, wb, \"(" #T " %%s)\", KType_text(Node->typeAttr));\n");\
    fprintf(stderr, "}\n");\
    fprintf(stderr, "\n");\

#define PRINT2(T) \
    fprintf(stderr, "\tKDEFINE_CLASS def" #T " = {0};\n");\
    fprintf(stderr, "\tSETSTRUCTNAME(def" #T ", " #T "Node);\n");\
    fprintf(stderr, "\tdef" #T ".init     = k" #T "Node_init;\n");\
    fprintf(stderr, "\tdef" #T ".reftrace = k" #T "Node_reftrace;\n");\
    fprintf(stderr, "\tdef" #T ".format   = k" #T "Node_format;\n");\
    fprintf(stderr, "\tmod->c" #T "Node = KLIB KClass_define(kctx, PackageId_sugar, NULL, &def" #T ", 0);\n");\
    fprintf(stderr, "\n");\

int main(int argc, char const* argv[])
{
    fprintf(stderr, "#include \"konoha3.h\"\n");
    fprintf(stderr, "#include \"konoha3/node2.h\"\n");
    fprintf(stderr, "\n");
    NODE_LIST_OP(PRINT);
    fprintf(stderr, "\n");
    fprintf(stderr, "static void InitNodeClass(KonohaContext *kctx, KParserModel *mod)\n");
    fprintf(stderr, "{\n");
    NODE_LIST_OP(PRINT2);
    fprintf(stderr, "}\n");
    fprintf(stderr, "\n");
    return 0;
}
