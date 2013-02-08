/****************************************************************************
 * Copyright (c) 2012-2013, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include <stdio.h>
#include <iconv.h>
#include <errno.h>
#include <konoha/konoha.h>
#include <konoha/sugar.h>
#include <konoha/klib.h>
#include <konoha/arch/minivm.h>

#define LOG_FUNCTION_NAME "echo"
#define ARGLENGTH 8

#ifdef __cplusplus
extern "C" {
#endif

typedef struct BashBuilder {
	struct KBuilderCommon common;
	kbool_t isIndentEmitted;
	kbool_t isEvalMethod;
	kbool_t isExprNode;
	kbool_t isInitCommand;
	kbool_t isOutPut;
	int Command;
	kMethod *visitingMethod;
	int indent;
	KGrowingArray buffer;
	kParam *args;
	struct kArrayVar localValList;
	KBuffer bashCodeBuffer;
	KBuffer wb;
} BashBuilder;

///* ------------------------------------------------------------------------ */
///* [Statement/Expression API] */
static kNode* Node_getFirstBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kNode* Node_getElseBlock(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_else, K_NULLBLOCK);
}

static kNode* Node_getFirstExpr(KonohaContext *kctx, kNode *stmt)
{
	return SUGAR kNode_GetNode(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kMethod* CallNode_getMethod(kNode *expr)
{
	return expr->NodeList->MethodItems[0];
}

static const char *BASHKEYWORD_LIST[] = {
	"", "$Indent", "$Symbol", "$Text", "$Number", "$Member", "$Type",
	"()", "[]", "{}", "$Expr", "$Block", "$Param", "$TypeDecl", "$MethodDecl", "$Token",
	".", "/", "%", "*", "+", "-", " -lt ", " -le ", " -gt ", " -ge ", " -eq ", " -ne ",
	"&&", "||", "!", "=", ",", "$", ":", ";", /*"@",*/
	"true", "false", "if", "else", "return", // syn
	"new", "void", "script"
};

static const char *BashSymbol_text(KonohaContext *kctx, ksymbol_t sym)
{
	size_t index = (size_t) KSymbol_Unmask(sym);
	return BASHKEYWORD_LIST[index];
}

#define MN_opNOT  KMethodName_("!")
#define MN_opADD  KMethodName_("+")
#define MN_opSUB  KMethodName_("-")
#define MN_opMUL  KMethodName_("*")
#define MN_opDIV  KMethodName_("/")
#define MN_opMOD  KMethodName_("%")
#define MN_opEQ   KMethodName_("==")
#define MN_opNEQ  KMethodName_("!=")
#define MN_opLT   KMethodName_("<")
#define MN_opLTE  KMethodName_("<=")
#define MN_opGT   KMethodName_(">")
#define MN_opGTE  KMethodName_(">=")
#define MN_opLAND KMethodName_("&")
#define MN_opLOR  KMethodName_("|")
#define MN_opLXOR KMethodName_("^")
#define MN_opLSFT KMethodName_("<<")
#define MN_opRSFT KMethodName_(">>")

enum kSymbolPrefix{
	kSymbolPrefix_NONE,
	kSymbolPrefix_SET,
	kSymbolPrefix_GET,
	kSymbolPrefix_AT,
	kSymbolPrefix_IS,
	kSymbolPrefix_UNKNOWN,
	kSymbolPrefix_TO,
	kSymbolPrefix_DOLLAR,
};

static enum kSymbolPrefix KSymbol_prefixText_ID(ksymbol_t sym)
{
	size_t mask = ((size_t)(KSymbol_Attr(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
	DBG_ASSERT(mask < 8);
	return (enum kSymbolPrefix)mask;
}

static int KMethodName_isBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opADD ) return 1;
	if(mn == MN_opSUB ) return 1;
	if(mn == MN_opMUL ) return 1;
	if(mn == MN_opDIV ) return 1;
	if(mn == MN_opMOD ) return 1;
	if(mn == MN_opEQ  ) return 1;
	if(mn == MN_opNEQ ) return 1;
	if(mn == MN_opLT  ) return 1;
	if(mn == MN_opLTE ) return 1;
	if(mn == MN_opGT  ) return 1;
	if(mn == MN_opGTE ) return 1;
	if(mn == MN_opLAND) return 1;
	if(mn == MN_opLOR ) return 1;
	if(mn == MN_opLXOR) return 1;
	if(mn == MN_opLSFT) return 1;
	if(mn == MN_opRSFT) return 1;
	return 0;
}

static int KMethodName_isUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opSUB) return 1;
	if(mn == MN_opNOT) return 1;
	return 0;
}

static void BashBuilder_EmitIndent(KonohaContext *kctx, KBuilder *builder)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	if(!bashBuilder->isIndentEmitted) {
		int i;
		for(i = 0; i < bashBuilder->indent; i++) {
			printf("    ");
		}
		bashBuilder->isIndentEmitted = true;
	}
}

static void BashBuilder_EmitNewLineWith(KonohaContext *kctx, KBuilder *builder, const char* endline)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	BashBuilder_EmitIndent(kctx, builder);
	bashBuilder->isIndentEmitted = false;
	printf("%s\n", endline);
}

static void BashBuilder_EmitString(KonohaContext *kctx, KBuilder *builder, const char* prefix, const char* str, const char* suffix)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	BashBuilder_EmitIndent(kctx, builder);
	KLIB KBuffer_printf(kctx, &bashBuilder->bashCodeBuffer, "%s%s%s", prefix, str, suffix);
	if(strcmp(prefix, "(") == 0 || strcmp(prefix, ")") == 0) {
		printf("%s%s%s", " ", str, "");
	} else {
		printf("%s%s%s", prefix, str, suffix);
	}
}

static kbool_t BashBuilder_VisitBlockNode(KonohaContext *kctx, KBuilder *builder, kNode *block, void *thunk)
{
	size_t i;
	kbool_t ret = true;
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	DBG_ASSERT(kNode_node(block) == KNode_Block);
	if(!IS_Array(block->NodeList)) {
		BashBuilder_EmitString(kctx, builder, "{ /* ERROR: block->NodeList is not Array. */ }", "", "");
		return true;
	}
	DBG_ASSERT(IS_Array(block->NodeList));
	if(!kNode_IsRootNode(block)) {
		BashBuilder_EmitNewLineWith(kctx, builder, "");
		bashBuilder->indent++;
	}
	for (i = 0; i < kArray_size(block->NodeList); ++i) {
		kNode *node = block->NodeList->NodeItems[i];
		if(kNode_node(node) == KNode_Block && kArray_size(node->NodeList) == 1) {
			node = node->NodeList->NodeItems[0];
		}
		if(kNode_node(node) == KNode_Assign) {
			if(kNode_node(kNode_At(node, 1)) == KNode_Field){
				BashBuilder_EmitString(kctx, builder, "this.", "", "");
			}
			else {
				BashBuilder_EmitString(kctx, builder, "", "", "");
			}
		}

		if(!SUGAR VisitNode(kctx, builder, node, thunk)) {
			ret = false;
			break;
		}
		if(IS_NameSpace(node->NodeList)) {
			BashBuilder_EmitNewLineWith(kctx, builder, "");
			goto L_RETURN;
		}
		if(kNode_At(node, 1)->attrTypeId <= KType_0) {
			BashBuilder_EmitNewLineWith(kctx, builder, "");
		}
	}
	if(!kNode_IsRootNode(block)) {
		bashBuilder->indent--;
		BashBuilder_EmitString(kctx, builder, "", "", "");
	}
L_RETURN:
	return ret;
}

static kbool_t BashBuilder_VisitNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, const char* prefix, const char* suffix)
{
	BashBuilder_EmitString(kctx, builder, prefix, "", "");
	kbool_t ret = SUGAR VisitNode(kctx, builder, expr, thunk);
	BashBuilder_EmitString(kctx, builder, suffix, "", "");
	return ret;
}

static kbool_t BashBuilder_VisitPushNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	SUGAR VisitNode(kctx, builder, expr->NodeToPush, thunk);
	return true;
}

static kbool_t BashBuilder_VisitBoxNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	SUGAR VisitNode(kctx, builder, expr->NodeToPush, thunk);
	return true;
}

static kbool_t BashBuilder_VisitErrorNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	return false;
}

static kbool_t BashBuilder_VisitReturnNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void *thunk)
{
	if(((BashBuilder *)builder)->visitingMethod->mn != 0) {
		BashBuilder_EmitString(kctx, builder, "return ", "", "");
	}
	kNode* expr = Node_getFirstExpr(kctx, stmt);
	if(expr != NULL && IS_Node(expr)) {
		SUGAR VisitNode(kctx, builder, expr, thunk);
	}
	BashBuilder_EmitString(kctx, builder, "", "", "");
	return true;
}

static kbool_t BashBuilder_VisitIfNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	BashBuilder_VisitNode(kctx, builder, Node_getFirstExpr(kctx, stmt), thunk, "if [ ", " ]; then ");
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), thunk);
	kNode *elseNode = Node_getElseBlock(kctx, stmt);
	if(elseNode != K_NULLBLOCK) {
		BashBuilder_EmitString(kctx, builder, "else", "", "");
		SUGAR VisitNode(kctx, builder, elseNode, thunk);
	}
	BashBuilder_EmitString(kctx, builder, "fi", "", "");
	return true;
}

static kbool_t BashBuilder_VisitWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	BashBuilder_VisitNode(kctx, builder, Node_getFirstExpr(kctx, stmt), thunk, "while [ ", " ]\n");
	bashBuilder->isIndentEmitted = false;
	BashBuilder_EmitString(kctx, builder, "do", "", "");
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), thunk);
	BashBuilder_EmitString(kctx, builder, "done", "", "");
	return true;
}

static kbool_t BashBuilder_VisitDoWhileNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	BashBuilder_EmitString(kctx, builder, "do", "", "");
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), thunk);
	BashBuilder_VisitNode(kctx, builder, Node_getFirstExpr(kctx, stmt), thunk, "while(", ");");
	return true;
}

static kbool_t BashBuilder_VisitForNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	abort();
	return true;
}

static kbool_t BashBuilder_VisitBreakNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void* thunk)
{
	BashBuilder_EmitString(kctx, builder, "break", "", "");
	return true;
}

static kbool_t BashBuilder_VisitContinueNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void* thunk)
{
	BashBuilder_EmitString(kctx, builder, "continue", "", "");
	return true;
}

static kbool_t BashBuilder_VisitThrowNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void* thunk)
{
	BashBuilder_VisitNode(kctx, builder, Node_getFirstExpr(kctx, node), thunk, "throw ", ";");
	return true;
}

static kbool_t BashBuilder_VisitTryNode(KonohaContext *kctx, KBuilder *builder, kNode *stmt, void* thunk)
{
	BashBuilder_EmitNewLineWith(kctx, builder, "try ");
	SUGAR VisitNode(kctx, builder, Node_getFirstBlock(kctx, stmt), thunk);
	kNode *catchNode   = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("catch"),   K_NULLBLOCK);
	kNode *finallyNode = SUGAR kNode_GetNode(kctx, stmt, KSymbol_("finally"), K_NULLBLOCK);
	if(catchNode != K_NULLBLOCK) {
		BashBuilder_EmitString(kctx, builder, "catch(e) ", "", "");
		SUGAR VisitNode(kctx, builder, catchNode, thunk);
	}
	if(finallyNode != K_NULLBLOCK) {
		BashBuilder_EmitString(kctx, builder, "finally", "", "");
		SUGAR VisitNode(kctx, builder, finallyNode, thunk);
	}
	return true;
}

static void BashBuilder_EmitKonohaValue(KonohaContext *kctx, KBuilder *builder, KClass* ct, KonohaStack* sfp)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	ct->p(kctx, sfp, 0, &wb);
	char *str = (char *)KLIB KBuffer_text(kctx, &wb, NonZero);
	BashBuilder_EmitString(kctx, builder, str, "", "");
	KLIB KBuffer_Free(&wb);
}

static void BashBuilder_EmitConstValue(KonohaContext *kctx, KBuilder *builder, kObject *obj)
{
	KonohaStack sfp[1];
	sfp[0].asObject = obj;
	BashBuilder_EmitKonohaValue(kctx, builder, kObject_class(obj), sfp);
}

static void BashBuilder_EmitUnboxConstValue(KonohaContext *kctx, KBuilder *builder, KClass *ct, unsigned long long unboxVal)
{
	KonohaStack sfp[1];
	sfp[0].unboxValue = unboxVal;
	BashBuilder_EmitKonohaValue(kctx, builder, ct, sfp);
}

static kbool_t BashBuilder_VisitConstNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{

	BashBuilder_EmitConstValue(kctx, builder, node->ObjectConstValue);

	return true;
}

static kbool_t BashBuilder_VisitUnboxConstNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder_EmitUnboxConstValue(kctx, builder, KClass_(node->attrTypeId), node->unboxConstValue);
	return true;
}

static kbool_t BashBuilder_VisitNewNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder_EmitString(kctx, builder, "new", "", "");
	return true;
}

static kbool_t BashBuilder_VisitNullNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder_EmitString(kctx, builder, "", "", "");
	return true;
}

static kbool_t BashBuilder_VisitLocalNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	size_t i;
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	kToken *tk = (kToken *)node->TermToken;

	if(node->attrTypeId > KType_System) {
		BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
		bashBuilder->indent--;
		BashBuilder_EmitNewLineWith(kctx, builder, "");
	}

	if(bashBuilder->args->psize == 0 && bashBuilder->isEvalMethod) {
		BashBuilder_EmitString(kctx, builder, "${", "", "");
		BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "}", "");
	}
	else {
		for(i = 0; i < bashBuilder->args->psize; ++i) {
			const char *argname = KSymbol_text(bashBuilder->args->paramtypeItems[i].name);
			const char *localname = kString_text(tk->text);
			if(strcmp(argname, localname) == 0) {
				char argref[ARGLENGTH];
				PLATAPI snprintf_i(argref, ARGLENGTH, "$%d", i+1);
				BashBuilder_EmitString(kctx, builder, argref, "", "");
			}else{
				BashBuilder_EmitString(kctx, builder, "$", "", "");
				BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
			}
		}
	}
	return true;
}

static kbool_t BashBuilder_VisitLocalAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	size_t i, arraySize;
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	kToken *tk = (kToken *)node->TermToken;

	if(node->attrTypeId > KType_System) {
		BashBuilder_EmitString(kctx, builder, KClass_text(KClass_(node->attrTypeId)), " \'", "");
		BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "\'", "");
	}
	else {
		if(bashBuilder->localValList.bytesize == 0) {
			KLIB KArray_Init(kctx, &(bashBuilder->localValList), sizeof(kArray));
			KLIB kArray_Add(kctx, &(bashBuilder->localValList), tk->text);
			BashBuilder_EmitString(kctx, builder, "local ", "", "");
		}
		arraySize = kArray_size(&bashBuilder->localValList);
		for(i = 0; i < arraySize; i++) {
			const char *valname = kString_text(tk->text);
			const char *localvalname = bashBuilder->localValList.stringItems[i]->text;
			if(strcmp(valname, localvalname) == 0) {
				break;
			}
			if(i == arraySize-1) {
				KLIB kArray_Add(kctx, &(bashBuilder->localValList), tk->text);
				BashBuilder_EmitString(kctx, builder, "local ", "", "");
			}
		}
		BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
	}
	return true;
}

static kbool_t BashBuilder_VisitFieldNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{ 
	kToken *tk = (kToken *)node->TermToken;
	BashBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
	return true;
}

kbool_t LoadBashModule(KonohaFactory *factory, ModuleType type);
static void compileAllDefinedMethodsInNameSpace(KonohaContext *kctx, kNameSpace *ns);
static void SetUpBashShebang(void);

static bool BashBuilder_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	KonohaFactory *factory = (KonohaFactory *)kctx->platApi;

	SUGAR kNameSpace_UseDefaultVirtualMachine(kctx, ns);

	KImportPackage(ns, kString_text(package), trace);
	compileAllDefinedMethodsInNameSpace(kctx, ns);


	SetUpBashShebang();
	LoadBashModule(factory, ReleaseModule);
	ns->builderApi = factory->ExecutionEngineModule.GetDefaultBuilderAPI();
	return true;
}

static kbool_t BashBuilder_VisitNodeParams(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, int beginIndex, const char* delimiter, const char* leftBracket, const char* rightBracket)
{
	unsigned n = kArray_size(expr->NodeList);
	unsigned i;

	if(leftBracket) {
		BashBuilder_EmitString(kctx, builder, leftBracket, "", "");
	}
	for(i = beginIndex; i < n;) {
		if(strcmp(KClass_text(KClass_(expr->attrTypeId)), "SubProc") == 0 || strcmp(KClass_text(KClass_(expr->attrTypeId)), "Array[String]") == 0) {
		}
		else {
			SUGAR VisitNode(kctx, builder, kNode_At(expr, i), thunk);
		}
		if(++i < n) {
			BashBuilder_EmitString(kctx, builder, delimiter, " ", "");
		}
	}
	if(rightBracket) {
		BashBuilder_EmitString(kctx, builder, rightBracket, "", "");
	}
	return true;
}

static void BashBuilder_ConvertAndEmitMethodName(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk, kNode *receiver, kMethod *mtd)
{
	KClass *globalObjectClass = KLIB kNameSpace_GetClassByFullName(kctx, kNode_ns(expr), "GlobalObject", 12, NULL);
	kbool_t isGlobal = (KClass_(receiver->attrTypeId) == globalObjectClass || receiver->attrTypeId == KType_NameSpace);
	const char *methodName = KSymbol_text(mtd->mn);
	const char *receiverClassName = KClass_text(KClass_(receiver->attrTypeId));

	if(receiver->attrTypeId == KType_NameSpace) {
		if(mtd->mn == KMethodName_("import")) {
			kString *packageNameString = (kString *)kNode_At(expr, 2)->ObjectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->ObjectConstValue;
			BashBuilder_importPackage(kctx, ns, packageNameString, expr->TermToken->uline);
			BashBuilder_EmitString(kctx, builder, "#import", "", "");
			return;
		}
	}
	if(receiver->attrTypeId == KType_System && methodName[0] == 'p') {
		// System.p
		BashBuilder_EmitString(kctx, builder, LOG_FUNCTION_NAME, "", "");
	}
	else if(strcmp(methodName, "new") == 0) {
	}
	else if(strcmp(methodName, "newList") == 0) {
	}
	else if(strcmp(methodName, "eval") == 0) {
		BashBuilder_EmitString(kctx, builder, "eval.method", "", "");
	}
	else if(strcmp(methodName, "[]") == 0) {
		// [1, 2, 3];
//		BashBuilder_EmitString(kctx, builder, "new Array", "", "");
	}
	else {
		// Normal functions
		if(!isGlobal) {
			if(kNode_node(receiver) == KNode_Null) {
				// Static methods
				BashBuilder_EmitString(kctx, builder, KClass_text(KClass_(receiver->attrTypeId)), "", "");
			}
			else {
				// Instance methods
				if(receiver->attrTypeId <= KType_System) {
					SUGAR VisitNode(kctx, builder, receiver, thunk);
				}
			}
		}

		switch(KSymbol_prefixText_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
			if(kArray_size(expr->NodeList) > 2) {
				BashBuilder_VisitNode(kctx, builder, kNode_At(expr, 2), thunk, "[", "]");
			}
			else {
				if(!isGlobal) {
					if(strcmp(receiverClassName, "SubProc") == 0) {
						goto L_EXIT;
					}
					BashBuilder_EmitString(kctx, builder, "$", "", "");
					BashBuilder_EmitString(kctx, builder, kString_text(receiver->TermToken->text), "", "");
					BashBuilder_EmitString(kctx, builder, "_", "", "");
				}
				BashBuilder_EmitString(kctx, builder, methodName, "", "");
			}
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->NodeList) > 3) {
				BashBuilder_VisitNode(kctx, builder, kNode_At(expr, 2), thunk, "[", "]");
			}
			if(receiver->attrTypeId <= KType_System) {
				BashBuilder_EmitString(kctx, builder, methodName, " = ", "");
			}
			break;
		case kSymbolPrefix_TO:
			// TODO
			break;
		default:
			if(!isGlobal){
				if(strcmp(receiverClassName, "SubProc") == 0) {
					goto L_EXIT;
				}
				BashBuilder_EmitString(kctx, builder, "$", "", "");
				BashBuilder_EmitString(kctx, builder, kString_text(receiver->TermToken->text), "_", "");
			}
			BashBuilder_EmitString(kctx, builder, methodName, "", "");
		L_EXIT:
			break;
		}
	}
}


static kbool_t BashBuilder_VisitMethodCallNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	kMethod *mtd = CallNode_getMethod(node);
	kbool_t isArray = false;

	if(strcmp(KClass_text(KClass_(node->attrTypeId)), "SubProc") == 0 && strcmp(KSymbol_text(mtd->mn), "new") == 0 && !bashBuilder->isInitCommand) {
		KLIB KBuffer_Init(&(kctx->stack->cwb), &(bashBuilder->wb));
		bashBuilder->isInitCommand = true;
	}
	if(strcmp(KClass_text(KClass_(node->attrTypeId)), "SubProc") == 0 && strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		bashBuilder->Command++;
	}

	if(bashBuilder->Command > 1) {
		KLIB KBuffer_printf(kctx, &(bashBuilder->wb), "| ");
		bashBuilder->Command--;
	}
	if(strcmp(KSymbol_text(mtd->mn), "OutputStream") == 0) {
		bashBuilder->isOutPut = true;
	}

	if(strcmp(KSymbol_text(mtd->mn), "lastExitStatus") == 0 && bashBuilder->isOutPut) {
		char *str = (char *)KLIB KBuffer_text(kctx, &(bashBuilder->wb), NonZero);
		PLATAPI printf_i("%s\n", str);
		KLIB KBuffer_Free(&(bashBuilder->wb));
		return true;
	}

	if(strcmp(KClass_text(KClass_(node->attrTypeId)), "File") == 0 && strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		KLIB KBuffer_printf(kctx, &(bashBuilder->wb), "> ");
		KLIB KBuffer_printf(kctx, &(bashBuilder->wb), "%s", ((kString *)kNode_At(node, 2)->ObjectConstValue)->text);
		return true;
	}

	if(kArray_size(node->NodeList) == 2 && KMethodName_isUnaryOperator(kctx, mtd->mn)) {
		BashBuilder_EmitString(kctx, builder, KMethodName_Fmt2(mtd->mn), "(");
		SUGAR VisitNode(kctx, builder, kNode_At(node, 1), thunk);
		BashBuilder_EmitString(kctx, builder, ")", "", "");
	}
	else if(KMethodName_isBinaryOperator(kctx, mtd->mn)) {
		BashBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, BashSymbol_text(kctx, mtd->mn),"", "");
	}
	else {
		kNode *receiver = kNode_At(node, 1);
		const char *classname = KClass_text(KClass_(receiver->attrTypeId));
		if(strcmp(KSymbol_text(mtd->mn), "newList") == 0) {
			isArray = true;
		}
		else {
			BashBuilder_ConvertAndEmitMethodName(kctx, builder, node, thunk, receiver, mtd);
		}
		if(strcmp(KSymbol_text(mtd->mn), "eval") == 0) {
			((BashBuilder *)builder)->isEvalMethod = true;
		}
		switch(KSymbol_prefixText_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
		case kSymbolPrefix_TO:
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(node->NodeList) > 3) {
				SUGAR VisitNode(kctx, builder, kNode_At(node, 3), thunk);
			}
			else {
				if(receiver->attrTypeId <= KType_System || strcmp(classname, "GlobalObject") == 0) {
					SUGAR VisitNode(kctx, builder, kNode_At(node, 2), thunk);
				}
			}
			if(strcmp(KSymbol_text(mtd->mn), "ArgumentList") == 0) {
				BashBuilder_VisitNodeParams(kctx, builder, node, thunk, 2, " ", isArray ? "[" : "(", isArray ? "]" : ")");
			}
			break;
		default:
/*command*/
			if(strcmp(classname, "SubProc") == 0 && strcmp(KSymbol_text(mtd->mn), "new") == 0) {
				KLIB KBuffer_printf(kctx, &(bashBuilder->wb), "%s", ((kString *)kNode_At(node, 2)->ObjectConstValue)->text);
				KLIB KBuffer_printf(kctx, &(bashBuilder->wb), " ");
			}
/*option*/
			if(strcmp(classname, "Array[String]") == 0 && strcmp(KSymbol_text(mtd->mn), "[]") == 0) {
				unsigned n = kArray_size(node->NodeList);
				unsigned i;
				for(i = 2; i < n; i++) {
					KLIB KBuffer_printf(kctx, &(bashBuilder->wb), "%s", ((kString *)kNode_At(node, i)->ObjectConstValue)->text);
					KLIB KBuffer_printf(kctx, &(bashBuilder->wb), " ");
				}
			}
				BashBuilder_VisitNodeParams(kctx, builder, node, thunk, 2, " ", isArray ? "[" : "(", isArray ? "]" : ")");
			if(node->attrTypeId > KType_System && strcmp(classname, "SubProc") != 0) {
				BashBuilder_EmitNewLineWith(kctx, builder, "");
			}
			break;
		}
		if(mtd->mn == KMethodName_("import")) {
			BashBuilder_EmitNewLineWith(kctx, builder, ";");
		}
	}
	return true;
}

static kbool_t BashBuilder_VisitAndNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, " && ", "(", ")");
	return true;
}

static kbool_t BashBuilder_VisitOrNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	BashBuilder_VisitNodeParams(kctx, builder, node, thunk, 1, " || ", "(", ")");
	return true;
}

static kbool_t BashBuilder_VisitAssignNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	unsigned n = kArray_size(node->NodeList);
	unsigned i;
	BashBuilder_VisitLocalAssignNode(kctx, builder, kNode_At(node, 1), thunk);
	for(i = 2; i < n;) {
		kNode *tmpNode = kNode_At(node, i);
		switch(kNode_node(tmpNode)) {
		case KNode_Const :
		case KNode_UnboxConst :
			BashBuilder_EmitString(kctx, builder, "=", "", "");
		case KNode_Null :
			SUGAR VisitNode(kctx, builder, kNode_At(node, i), thunk);
			break;
		default:
			if(tmpNode->attrTypeId > KType_System) {
				SUGAR VisitNode(kctx, builder, kNode_At(node, i), thunk);
			}
			else {
				BashBuilder_EmitString(kctx, builder, "=", "", "");
				BashBuilder_EmitString(kctx, builder, "$((", "", "");
				SUGAR VisitNode(kctx, builder, kNode_At(node, i), thunk);
				PLATAPI printf_i("))");
			}
			break;
		}
		i++;
	}

	return true;
}

static kbool_t BashBuilder_VisitDoneNode(KonohaContext *kctx, KBuilder *builder, kNode *node, void *thunk)
{
	return true;
}

static kbool_t BashBuilder_VisitFunctionNode(KonohaContext *kctx, KBuilder *builder, kNode *expr, void *thunk)
{
	abort();/*FIXME*/
	return true;
}

static void compileAllDefinedMethodsInNameSpace(KonohaContext *kctx, kNameSpace *ns)
{
	size_t i;
	for(i = 0; i < kArray_size(ns->methodList_OnList); i++) {
		kMethod *mtd = ns->methodList_OnList->MethodItems[i];
		if(IS_NameSpace(mtd->LazyCompileNameSpace)) {
			KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, HatedLazyCompile|CrossCompile);
		}
	}
}

static void compileAllDefinedMethods(KonohaContext *kctx)
{
	KRuntime *share = kctx->share;
	size_t i;
	for(i = 0; i < kArray_size(share->GlobalConstList); i++) {
		kObject *object = share->GlobalConstList->ObjectItems[i];
		if(kObject_class(object) == KClass_NameSpace) {
			compileAllDefinedMethodsInNameSpace(kctx, (kNameSpace *)object);
		}
	}
}

static void BashBuilder_EmitExtendFunctionCode(KonohaContext *kctx, KBuilder *builder)
{
	BashBuilder *self = (BashBuilder *)builder;
	BashBuilder_EmitNewLineWith(kctx, builder, "var __extends = this.__extends || function (d, b) {");
	self->indent++;
	BashBuilder_EmitNewLineWith(kctx, builder, "function __() { this.constructor = d; }");
	BashBuilder_EmitNewLineWith(kctx, builder, "__.prototype = b.prototype;");
	BashBuilder_EmitNewLineWith(kctx, builder, "d.prototype = new __();");
	self->indent--;
	BashBuilder_EmitNewLineWith(kctx, builder, "}");
}

static kbool_t BashBuilder_VisitClassFields(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	kushort_t i;
	KClassField *field = kclass->fieldItems;
	kObject *constList = kclass->defaultNullValue;
	BashBuilder_EmitString(kctx, builder, "base=$FUNCNAME", "", "");
	BashBuilder_EmitNewLineWith(kctx, builder, "");
	BashBuilder_EmitString(kctx, builder, "this=$1", "", "");
	BashBuilder_EmitNewLineWith(kctx, builder, "");

	for(i = 0; i < kclass->fieldsize; ++i) {
		BashBuilder_EmitString(kctx, builder, "export ${this}_", KSymbol_text(field[i].name), "=");
		if(KType_Is(UnboxType, field[i].attrTypeId)) {
			BashBuilder_EmitString(kctx, builder, "$", "", "");
			PLATAPI printf_i("%d", i+2);
		}else {
			BashBuilder_EmitConstValue(kctx, builder, constList->fieldObjectItems[i]);
		}
		BashBuilder_EmitNewLineWith(kctx, builder, "");
	}
	BashBuilder_EmitString(kctx, builder, "for method in $(compgen -A function)", "", "");
	BashBuilder_EmitNewLineWith(kctx, builder, "");
	BashBuilder_EmitString(kctx, builder, "do", "", "");
	BashBuilder_EmitNewLineWith(kctx, builder, "");
	BashBuilder_EmitString(kctx, builder, "    ", "export ${method/#$base\\_/$this\\_}=\"${method} ${this}\"", "");
	BashBuilder_EmitNewLineWith(kctx, builder, "");
	BashBuilder_EmitString(kctx, builder, "done", "", "");

	return true;
}

static void BashBuilder_EmitMethodHeader(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	KClass *kclass = KClass_(mtd->typeId);
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kParam *params = kMethod_GetParam(mtd);
	bashBuilder->args = params;
	if(mtd->typeId == KType_NameSpace) {
		// Top level functions
		KLIB KBuffer_printf(kctx, &wb, "function %s%s", KMethodName_Fmt2(mtd->mn));
	}
	else if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		// Constractor
//		KLIB KBuffer_printf(kctx, &wb, "function %s(", KClass_text(kclass));
	}
	else {
		// Methods
		compileAllDefinedMethods(kctx);
		if(kMethod_Is(Static, mtd)) {
			KLIB KBuffer_printf(kctx, &wb, "%s.%s%s = function(", KClass_text(kclass), KMethodName_Fmt2(mtd->mn));
		}
		else {
			KLIB KBuffer_printf(kctx, &wb, "function %s_%s%s", KClass_text(kclass), KMethodName_Fmt2(mtd->mn));
		}
	}
	// Emit all paramators
	BashBuilder_EmitString(kctx, builder, KLIB KBuffer_text(kctx, &wb, EnsureZero), "", "");
	KLIB KBuffer_Free(&wb);
}

static void BashBuilder_EmitClassHeader(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	if(base->typeId != KType_Object) {
		BashBuilder_EmitExtendFunctionCode(kctx, builder);
		BashBuilder_EmitString(kctx, builder, "var ", KClass_text(kclass), " = (function(_super) {");
	}else {
		BashBuilder_EmitString(kctx, builder, "function ", KClass_text(kclass), " () {");
	}
	BashBuilder_EmitNewLineWith(kctx, builder, "");
	bashBuilder->indent++;
	if(base->typeId != KType_Object){
		BashBuilder_EmitString(kctx, builder, "__extends(", KClass_text(kclass), ", _super);");
		BashBuilder_EmitNewLineWith(kctx, builder, "");
	}
}

static void BashBuilder_EmitClassFooter(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	bashBuilder->indent--;

	if(base->typeId != KType_Object) {
		BashBuilder_EmitString(kctx, builder, "})(", KClass_text(base), ");");
		BashBuilder_EmitNewLineWith(kctx, builder, "");
	}
}

static void BashBuilder_Init(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	kbool_t isConstractor = false;
	bashBuilder->visitingMethod = mtd;
	bashBuilder->isIndentEmitted = false;
	bashBuilder->indent = 0;
	KLIB KBuffer_Init(&bashBuilder->buffer, &bashBuilder->bashCodeBuffer);

	KClass *kclass = KClass_(mtd->typeId);
	KClass *base   = KClass_(kclass->superTypeId);

	if(mtd->mn != 0) {
		// Functions
		KLIB kMethod_SetFunc(kctx, mtd, NULL);
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			isConstractor = true;
		}

		if(isConstractor) {
			BashBuilder_EmitClassHeader(kctx, builder, kclass);
			BashBuilder_EmitMethodHeader(kctx, builder, mtd);
			if(base->typeId != KType_Object) {
				BashBuilder_EmitNewLineWith(kctx, builder, "_super.call(this);");
			}
			BashBuilder_VisitClassFields(kctx, builder, kclass);
		}else {
			BashBuilder_EmitMethodHeader(kctx, builder, mtd);
			BashBuilder_EmitNewLineWith(kctx, builder, " {");
			bashBuilder->indent++;
		}
	}else {
		// TopLevel
		compileAllDefinedMethods(kctx);
	}
}

static void BashBuilder_Free(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	BashBuilder *bashBuilder = (BashBuilder *)builder;
	if(mtd->mn != 0) {
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			BashBuilder_EmitNewLineWith(kctx, builder, "");
		}
		bashBuilder->indent--;
		BashBuilder_EmitNewLineWith(kctx, builder, "}");
		if(bashBuilder->isEvalMethod) {
			BashBuilder_EmitString(kctx, builder, "function eval.method {\n", "", "");
			BashBuilder_EmitString(kctx, builder, "    echo $1 > evaltext.k\n", "", "");
			BashBuilder_EmitString(kctx, builder, "    minikonoha evaltext.k\n", "}\n", "");

		}
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			KClass *kclass = KClass_(mtd->typeId);
			BashBuilder_EmitClassFooter(kctx, builder, kclass);
		}
	}
	KLIB KBuffer_Free(&bashBuilder->bashCodeBuffer);
}

static struct KVirtualCode* Bash_GenerateVirtualCode(KonohaContext *kctx, kMethod *mtd, kNode *block, int option)
{
	INIT_GCSTACK();
	BashBuilder builderbuf = {}, *builder = &builderbuf;
	kNameSpace *ns = kNode_ns(block);
	builder->common.api = ns->builderApi;
	builder->visitingMethod = mtd;
	BashBuilder_Init(kctx, (KBuilder *)builder, mtd);
	SUGAR VisitNode(kctx, (KBuilder *)builder, block, NULL);
	BashBuilder_Free(kctx, (KBuilder *)builder, mtd);
	RESET_GCSTACK();
	return NULL;
}

// -------------------------------------------------------------------------

static void SetUpBashShebang(void)
{
	fprintf(stdout, "#!/bin/bash\n");
}

static KMETHOD KMethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
}

static struct KVirtualCode *Bash_RunExecutionEngine(KonohaContext *kctx, struct KonohaValueVar *sfp, struct KVirtualCode *pc)
{
	/* NotSupportedAPI(); */
	return NULL;
}


static void Bash_SetMethodCode(KonohaContext *kctx, kMethodVar *mtd, KVirtualCode *vcode, KMethodFunc func)
{
	KLIB kMethod_SetFunc(kctx, mtd, func);
	mtd->vcode_start = vcode;
}

static KMethodFunc Bash_GenerateMethodFunc(KonohaContext *kctx, KVirtualCode *vcode)
{
	return KMethodFunc_RunVirtualMachine;
}

static struct KVirtualCode* GetDefaultBootCode(void)
{
	return NULL;
}

static void Bash_DeleteVirtualMachine(KonohaContext *kctx)
{
}

static const KModuleInfo ModuleInfo = {
	"Bash", K_VERSION, 0, "Bash",
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void);

static const struct ExecutionEngineModule Bash_Module = {
	&ModuleInfo,
	Bash_DeleteVirtualMachine,
	GetDefaultBuilderAPI,
	GetDefaultBootCode,
	Bash_GenerateVirtualCode,
	Bash_GenerateMethodFunc,
	Bash_SetMethodCode,
	Bash_RunExecutionEngine
};

static const struct KBuilderAPI Bash_BuilderAPI = {
	"Bash",
	&Bash_Module,
#define DEFINE_BUILDER_API(NAME) BashBuilder_Visit##NAME##Node,
	KNodeList(DEFINE_BUILDER_API)
#undef DEFINE_BUILDER_API
};

static const struct KBuilderAPI *GetDefaultBuilderAPI(void)
{
	return &Bash_BuilderAPI;
}

// -------------------------------------------------------------------------

kbool_t LoadBashModule(KonohaFactory *factory, ModuleType type)
{
	memcpy(&factory->ExecutionEngineModule, &Bash_Module, sizeof(Bash_Module));
	return true;
}

#ifdef __cplusplus
} /* extern "C" */
#endif
