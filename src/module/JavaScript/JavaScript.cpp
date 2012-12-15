/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
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
#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/klib.h>
#include <minikonoha/arch/minivm.h>
#ifdef HAVE_LIBV8
	#include <v8.h>
#endif

#define LOG_FUNCTION_NAME "p"

extern "C" {

#ifdef HAVE_LIBV8

static v8::Handle<v8::Value> JSLog(const v8::Arguments& args)
{
	if(args.Length() < 1) {
		return v8::Undefined();
	}

	v8::HandleScope scope;
	v8::Handle<v8::Value> arg = args[0];
	v8::String::Utf8Value value(arg);
	printf("%s\n", *value);

	return v8::Undefined();
}

v8::Persistent<v8::Context> createContext(){	
	v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New();
	global->Set(v8::String::New("p"), v8::FunctionTemplate::New(JSLog));
	return v8::Context::New(NULL, global);
}

class JSContext{
public:
	v8::HandleScope handleScope;
	v8::Persistent<v8::Context> context;
	v8::Context::Scope contextScope;
	JSContext():context(createContext()),contextScope(v8::Context::Scope(context)){
	}
	~JSContext(){
		context.Dispose();
	}
};

static JSContext* globalJSContext = NULL;

#endif

typedef struct JSBuilder {
	struct KBuilderCommon common;
	kbool_t isIndentEmitted;
	kMethod *visitingMethod;
	int indent;
	KGrowingArray buffer;
	KBuffer jsCodeBuffer;
} JSBuilder;

/* ------------------------------------------------------------------------ */
/* [Statement/Expression API] */
static kBlock* Stmt_getFirstBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_BlockPattern, K_NULLBLOCK);
}

static kBlock* Stmt_getElseBlock(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_else, K_NULLBLOCK);
}

static kExpr* Stmt_getFirstExpr(KonohaContext *kctx, kStmt *stmt)
{
	return SUGAR kStmt_GetExpr(kctx, stmt, KSymbol_ExprPattern, NULL);
}

static kMethod* CallExpr_getMethod(kExpr *expr)
{
	return expr->cons->MethodItems[0];
}

#define MN_opNOT  KKMethodName_("!")
#define MN_opADD  KKMethodName_("+")
#define MN_opSUB  KKMethodName_("-")
#define MN_opMUL  KKMethodName_("*")
#define MN_opDIV  KKMethodName_("/")
#define MN_opMOD  KKMethodName_("%")
#define MN_opEQ   KKMethodName_("==")
#define MN_opNEQ  KKMethodName_("!=")
#define MN_opLT   KKMethodName_("<")
#define MN_opLTE  KKMethodName_("<=")
#define MN_opGT   KKMethodName_(">")
#define MN_opGTE  KKMethodName_(">=")
#define MN_opLAND KKMethodName_("&")
#define MN_opLOR  KKMethodName_("|")
#define MN_opLXOR KKMethodName_("^")
#define MN_opLSFT KKMethodName_("<<")
#define MN_opRSFT KKMethodName_(">>")

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

static void JSBuilder_EmitIndent(KonohaContext *kctx, KBuilder *builder)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	if(!jsBuilder->isIndentEmitted) {
		int i;
		for(i = 0; i < jsBuilder->indent; i++) {
			KLIB KBuffer_printf(kctx, &jsBuilder->jsCodeBuffer, "    ");
		}
		jsBuilder->isIndentEmitted = true;
	}
}

static void JSBuilder_EmitNewLineWith(KonohaContext *kctx, KBuilder *builder, const char* endline)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	JSBuilder_EmitIndent(kctx, builder);
	jsBuilder->isIndentEmitted = false;
	KLIB KBuffer_printf(kctx, &jsBuilder->jsCodeBuffer, "%s\n", endline);
}

static void JSBuilder_EmitString(KonohaContext *kctx, KBuilder *builder, const char* prefix, const char* str, const char* suffix)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	JSBuilder_EmitIndent(kctx, builder);
	KLIB KBuffer_printf(kctx, &jsBuilder->jsCodeBuffer, "%s%s%s", prefix, str, suffix);
}

static void JSBuilder_VisitBlock(KonohaContext *kctx, KBuilder *builder, kBlock *block)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	JSBuilder_EmitNewLineWith(kctx, builder, "{");
	jsBuilder->indent++;
	SUGAR VisitBlock(kctx, builder, block);
	jsBuilder->indent--;
	JSBuilder_EmitString(kctx, builder, "}", "", "");
}

static void JSBuilder_VisitExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, const char* prefix, const char* suffix)
{
	JSBuilder_EmitString(kctx, builder, prefix, "", "");
	SUGAR VisitExpr(kctx, builder, stmt, expr);
	JSBuilder_EmitString(kctx, builder, suffix, "", "");
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

static kbool_t JSBuilder_VisitErrStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_EmitString(kctx, builder, kString_text(kStmt_GetObjectNULL(kctx, stmt, KSymbol_ERR)), "", "");
	return true;
}

static kbool_t JSBuilder_VisitExprStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	kExpr *expr = Stmt_getFirstExpr(kctx, stmt);
	if(expr->build == TEXPR_LET) {
		if(kExpr_At(expr, 1)->build == TEXPR_FIELD){
			JSBuilder_EmitString(kctx, builder, "this.", "", "");
		}else{
			JSBuilder_EmitString(kctx, builder, "var ", "", "");
		}
	}
	SUGAR VisitExpr(kctx, builder, stmt, expr);
	JSBuilder_EmitNewLineWith(kctx, builder, ";");
	return true;
}

static kbool_t JSBuilder_VisitBlockStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	return true;
}

static kbool_t JSBuilder_VisitReturnStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	if(((JSBuilder *)builder)->visitingMethod->mn != 0) {
		JSBuilder_EmitString(kctx, builder, "return ", "", "");
	}
	kExpr* expr = Stmt_getFirstExpr(kctx, stmt);
	if(expr != NULL && IS_Expr(expr)) {
		SUGAR VisitExpr(kctx, builder, stmt, expr);
	}
	JSBuilder_EmitNewLineWith(kctx, builder, ";");
	return true;
}

static kbool_t JSBuilder_VisitIfStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), "if(", ") ");
	JSBuilder_VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	kBlock *elseBlock = Stmt_getElseBlock(kctx, stmt);
	if(elseBlock != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "else", "", "");
		JSBuilder_VisitBlock(kctx, builder, elseBlock);
	}
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	return true;
}

static kbool_t JSBuilder_VisitLoopStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_VisitExpr(kctx, builder, stmt, Stmt_getFirstExpr(kctx, stmt), "while(", ") ");
	JSBuilder_VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	return true;
}

static kbool_t JSBuilder_VisitJumpStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_EmitString(kctx, builder, "Jump", "", "");
	return true;
}

static kbool_t JSBuilder_VisitTryStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_EmitNewLineWith(kctx, builder, "try ");
	JSBuilder_VisitBlock(kctx, builder, Stmt_getFirstBlock(kctx, stmt));
	kBlock *catchBlock   = SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_("catch"),   K_NULLBLOCK);
	kBlock *finallyBlock = SUGAR kStmt_GetBlock(kctx, stmt, NULL, KSymbol_("finally"), K_NULLBLOCK);
	if(catchBlock != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "catch(e) ", "", "");
		JSBuilder_VisitBlock(kctx, builder, catchBlock);
	}
	if(finallyBlock != K_NULLBLOCK) {
		JSBuilder_EmitString(kctx, builder, "finally", "", "");
		JSBuilder_VisitBlock(kctx, builder, finallyBlock);
	}
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	return true;
}

static kbool_t JSBuilder_VisitUndefinedStmt(KonohaContext *kctx, KBuilder *builder, kStmt *stmt)
{
	JSBuilder_EmitString(kctx, builder, "UNDEF", "", "");
	return false;
}

static void JSBuilder_EmitKonohaValue(KonohaContext *kctx, KBuilder *builder, KClass* ct, KonohaStack* sfp)
{
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	ct->p(kctx, sfp, 0, &wb);
	char *str = (char *)KLIB KBuffer_text(kctx, &wb, NonZero);
	JSBuilder_EmitString(kctx, builder, str, "", "");
	KLIB KBuffer_Free(&wb);
}

static void JSBuilder_EmitConstValue(KonohaContext *kctx, KBuilder *builder, kObject *obj)
{
	KonohaStack sfp[1];
	sfp[0].asObject = obj;
	JSBuilder_EmitKonohaValue(kctx, builder, kObject_class(obj), sfp);
}

static void JSBuilder_EmitNConstValue(KonohaContext *kctx, KBuilder *builder, KClass *ct, unsigned long long unboxVal)
{
	KonohaStack sfp[1];
	sfp[0].unboxValue = unboxVal;
	JSBuilder_EmitKonohaValue(kctx, builder, ct, sfp);
}

static void JSBuilder_VisitConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_EmitConstValue(kctx, builder, expr->objectConstValue);
}

static void JSBuilder_VisitNConstExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_EmitNConstValue(kctx, builder, KClass_(expr->attrTypeId), expr->unboxConstValue);
}

static void JSBuilder_VisitNewExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_EmitString(kctx, builder, "new", "", "");
}

static void JSBuilder_VisitNullExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_EmitString(kctx, builder, "null", "", "");
}

static void JSBuilder_VisitLocalExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kToken *tk = (kToken *)expr->termToken;
	JSBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
}

static void JSBuilder_VisitBlockExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_VisitBlock(kctx, builder, expr->block);
}

static void JSBuilder_VisitFieldExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	kToken *tk = (kToken *)expr->termToken;
	JSBuilder_EmitString(kctx, builder, kString_text(tk->text), "", "");
}

static bool JSBuilder_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KBaseTrace(trace);
	KImportPackage(ns, kString_text(package), trace);
	return true;
}

static void JSBuilder_VisitExprParams(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, int beginIndex, const char* delimiter, const char* leftBracket, const char* rightBracket)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	if(leftBracket) {
		JSBuilder_EmitString(kctx, builder, leftBracket, "", "");
	}
	for(i = beginIndex; i < n;) {
		SUGAR VisitExpr(kctx, builder, stmt, kExpr_At(expr, i));
		if(++i < n) {
			JSBuilder_EmitString(kctx, builder, delimiter, "", "");
		}
	}
	if(rightBracket) {
		JSBuilder_EmitString(kctx, builder, rightBracket, "", "");
	}
}

static void JSBuilder_ConvertAndEmitMethodName(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr, kExpr *receiver, kMethod *mtd)
{
	KClass *globalObjectClass = KLIB kNameSpace_GetClassByFullName(kctx, Stmt_ns(stmt), "GlobalObject", 12, NULL);
	kbool_t isGlobal = (KClass_(receiver->attrTypeId) == globalObjectClass || receiver->attrTypeId == KType_NameSpace);
	const char *methodName = KSymbol_text(mtd->mn);
	if(receiver->attrTypeId == KType_NameSpace) {
		if(mtd->mn == KKMethodName_("import")) {
			kString *packageNameString = (kString *)kExpr_At(expr, 2)->objectConstValue;
			kNameSpace *ns = (kNameSpace *)receiver->objectConstValue;
			JSBuilder_importPackage(kctx, ns, packageNameString, expr->termToken->uline);
			JSBuilder_EmitString(kctx, builder, "//", "", "");
			return;
		}
	}
	if(receiver->attrTypeId == KType_System && methodName[0] == 'p') {
		JSBuilder_EmitString(kctx, builder, LOG_FUNCTION_NAME, "", "");
	}else if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		JSBuilder_EmitString(kctx, builder, "new ", KClass_text(KClass_(receiver->attrTypeId)), "");
	}else if(strcmp(KSymbol_text(mtd->mn), "newList") == 0) {
	}else if(strcmp(KSymbol_text(mtd->mn), "newArray") == 0) {
		JSBuilder_EmitString(kctx, builder, "new Array", "", "");
	}else{
		if(!isGlobal) {
			if(receiver->build == TEXPR_NULL) {
				JSBuilder_EmitString(kctx, builder, KClass_text(KClass_(receiver->attrTypeId)), "", "");
			}
			else{
				SUGAR VisitExpr(kctx, builder, stmt, receiver);
			}
		}
		switch(KSymbol_prefixText_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
			if(kArray_size(expr->cons) > 2) {
				JSBuilder_VisitExpr(kctx, builder, stmt, kExpr_At(expr, 2), "[", "]");
			}
			else{
				if(!isGlobal) {
					JSBuilder_EmitString(kctx, builder, ".", "", "");
				}
				JSBuilder_EmitString(kctx, builder, methodName, "", "");
			}
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->cons) > 3) {
				JSBuilder_VisitExpr(kctx, builder, stmt, kExpr_At(expr, 2), "[", "]");
			}else{
				if(isGlobal) {
					JSBuilder_EmitString(kctx, builder, "var ", "", "");
				}
				else{
					JSBuilder_EmitString(kctx, builder, ".", "", "");
				}
			}
			JSBuilder_EmitString(kctx, builder, methodName, " = ", "");
			break;
		case kSymbolPrefix_TO:
			break;
		default:
			if(!isGlobal){
				JSBuilder_EmitString(kctx, builder, ".", "", "");
			}
			JSBuilder_EmitString(kctx, builder, methodName, "", "");
			break;
		}
	}
}

static void JSBuilder_VisitCallExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	kMethod *mtd = CallExpr_getMethod(expr);
	kbool_t isArray = false;

	if(kArray_size(expr->cons) == 2 && KMethodName_isUnaryOperator(kctx, mtd->mn)) {
		JSBuilder_EmitString(kctx, builder, KMethodName_Fmt2(mtd->mn), "(");
		SUGAR VisitExpr(kctx, builder, stmt, kExpr_At(expr, 1));
		JSBuilder_EmitString(kctx, builder, ")", "", "");
	}
	else if(KMethodName_isBinaryOperator(kctx, mtd->mn)) {
		JSBuilder_VisitExprParams(kctx, builder, stmt, expr, 1, KSymbol_text(mtd->mn),"(", ")");
	}
	else{
		kExpr *receiver = kExpr_At(expr, 1);
		/*if(mtd == jsBuilder->visitingMethod) {
			JSBuilder_EmitString(kctx, builder, "arguments.callee", "", "");
		}
		else*/ if(strcmp(KSymbol_text(mtd->mn), "newList") == 0) {
			isArray = true;
		}
		else {
			JSBuilder_ConvertAndEmitMethodName(kctx, builder, stmt, expr, receiver, mtd);
		}
		switch(KSymbol_prefixText_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
		case kSymbolPrefix_TO:
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->cons) > 3) {
				SUGAR VisitExpr(kctx, builder, stmt, kExpr_At(expr, 3));
			}else{
				SUGAR VisitExpr(kctx, builder, stmt, kExpr_At(expr, 2));
			}
			break;
		default:
			JSBuilder_VisitExprParams(kctx, builder, stmt, expr, 2, ", ", isArray ? "[" : "(", isArray ? "]" : ")");
			break;
		}
	}
}

static void JSBuilder_VisitAndExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_VisitExprParams(kctx, builder, stmt, expr, 1, " && ", "(", ")");
}

static void JSBuilder_VisitOrExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_VisitExprParams(kctx, builder, stmt, expr, 1, " || ", "(", ")");
}

static void JSBuilder_VisitLetExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_VisitExprParams(kctx, builder, stmt, expr, 1, " = ", NULL, NULL);
}

static void JSBuilder_VisitStackTopExpr(KonohaContext *kctx, KBuilder *builder, kStmt *stmt, kExpr *expr)
{
	JSBuilder_EmitString(kctx, builder, "/*FIXME*/STACKTOP", "", "");
}

static void compileAllDefinedMethods(KonohaContext *kctx)
{
	KRuntime *share = kctx->share;
	size_t i;
	for(i = 0; i < kArray_size(share->GlobalConstList); i++) {
		kObject *o = share->GlobalConstList->ObjectItems[i];
		if(kObject_class(o) == KClass_NameSpace) {
			kNameSpace *ns = (kNameSpace  *) o;
			size_t j;
			for(j = 0; j < kArray_size(ns->methodList_OnList); j++) {
				kMethod *mtd = ns->methodList_OnList->MethodItems[j];
				if(IS_NameSpace(mtd->LazyCompileNameSpace)) {
					KLIB kMethod_DoLazyCompilation(kctx, mtd, NULL, HatedLazyCompile|CrossCompile);
				}
			}
		}
	}
}

static void JSBuilder_EmitExtendFunctionCode(KonohaContext *kctx, KBuilder *builder)
{
	JSBuilder *self = (JSBuilder *)builder;
	JSBuilder_EmitNewLineWith(kctx, builder, "var __extends = this.__extends || function (d, b) {");
	self->indent++;
	JSBuilder_EmitNewLineWith(kctx, builder, "function __() { this.constructor = d; }");
	JSBuilder_EmitNewLineWith(kctx, builder, "__.prototype = b.prototype;");
	JSBuilder_EmitNewLineWith(kctx, builder, "d.prototype = new __();");
	self->indent--;
	JSBuilder_EmitNewLineWith(kctx, builder, "}");
}

static void JSBuilder_VisitClassFields(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	kushort_t i;
	KClassField *field = kclass->fieldItems;
	kObject *constList = kclass->defaultNullValue;
	for(i = 0; i < kclass->fieldsize; ++i) {
		JSBuilder_EmitString(kctx, builder, "this.", KSymbol_text(field[i].name), " = ");
		if(KType_Is(UnboxType, field[i].attrTypeId)) {
			JSBuilder_EmitNConstValue(kctx, builder, KClass_(field[i].attrTypeId), constList->fieldUnboxItems[i]);
		}else{
			JSBuilder_EmitConstValue(kctx, builder, constList->fieldObjectItems[i]);
		}
		JSBuilder_EmitNewLineWith(kctx, builder, ";");
	}
}

static void JSBuilder_EmitMethodHeader(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	KClass *kclass = KClass_(mtd->typeId);
	KBuffer wb;
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kParam *params = kMethod_GetParam(mtd);
	unsigned i;
	if(mtd->typeId == KType_NameSpace) {
		KLIB KBuffer_printf(kctx, &wb, "var %s%s = function(", KMethodName_Fmt2(mtd->mn));
	}else if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
		KLIB KBuffer_printf(kctx, &wb, "function %s(", KClass_text(kclass));
	}else{
		compileAllDefinedMethods(kctx);
		if(kMethod_Is(Static, mtd)) {
			KLIB KBuffer_printf(kctx, &wb, "%s.%s%s = function(", KClass_text(KClass_(mtd->typeId)), KMethodName_Fmt2(mtd->mn));
		}else{
			KLIB KBuffer_printf(kctx, &wb, "%s.prototype.%s%s = function(", KClass_text(KClass_(mtd->typeId)), KMethodName_Fmt2(mtd->mn));
		}
	}
	for(i = 0; i < params->psize; ++i) {
		if(i != 0) {
			KLIB KBuffer_printf(kctx, &wb, ", ");
		}
		KLIB KBuffer_printf(kctx, &wb, "%s", KSymbol_text(params->paramtypeItems[i].name));
	}
	KLIB KBuffer_printf(kctx, &wb, ")");
	JSBuilder_EmitString(kctx, builder, KLIB KBuffer_text(kctx, &wb, EnsureZero), "", "");
	KLIB KBuffer_Free(&wb);
}

static void JSBuilder_EmitClassHeader(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	if(base->typeId != KType_Object) {
		JSBuilder_EmitExtendFunctionCode(kctx, builder);
		JSBuilder_EmitString(kctx, builder, "var ", KClass_text(kclass), " = (function (_super) {");
	}else{
		JSBuilder_EmitString(kctx, builder, "var ", KClass_text(kclass), " = (function () {");
	}
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	jsBuilder->indent++;
	if(base->typeId != KType_Object){
		JSBuilder_EmitString(kctx, builder, "__extends(", KClass_text(kclass), ", _super);");
		JSBuilder_EmitNewLineWith(kctx, builder, "");
	}
}

static void JSBuilder_EmitClassFooter(KonohaContext *kctx, KBuilder *builder, KClass *kclass)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	KClass *base = KClass_(kclass->superTypeId);
	JSBuilder_EmitString(kctx, builder, "return ", KClass_text(kclass), ";");
	JSBuilder_EmitNewLineWith(kctx, builder, "");
	jsBuilder->indent--;
	if(base->typeId != KType_Object) {
		JSBuilder_EmitString(kctx, builder, "})(", KClass_text(base), ");");
		JSBuilder_EmitNewLineWith(kctx, builder, "");
	}else{
		JSBuilder_EmitNewLineWith(kctx, builder, "})();");
	}
}



static void JSBuilder_Init(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	kbool_t isConstractor = false;
	jsBuilder->visitingMethod = mtd;
	jsBuilder->isIndentEmitted = false;
	jsBuilder->indent = 0;
	KLIB KBuffer_Init(&jsBuilder->buffer, &jsBuilder->jsCodeBuffer);
	
	KClass *kclass = KClass_(mtd->typeId);
	KClass *base  = KClass_(kclass->superTypeId);
	
	if(mtd->mn != 0) {
		KLIB kMethod_SetFunc(kctx, mtd, NULL);
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			isConstractor = true;
		}
		if(isConstractor) {
			JSBuilder_EmitClassHeader(kctx, builder, kclass);
		}
		JSBuilder_EmitMethodHeader(kctx, builder, mtd);
		JSBuilder_EmitNewLineWith(kctx, builder, " {");
		jsBuilder->indent++;
		if(isConstractor) {
			if(base->typeId != KType_Object) {
				JSBuilder_EmitNewLineWith(kctx, builder, "_super.call(this);");
			}
			JSBuilder_VisitClassFields(kctx, builder, kclass);
		}
	}else{
		compileAllDefinedMethods(kctx);
	}
}

static void JSBuilder_Free(KonohaContext *kctx, KBuilder *builder, kMethod *mtd)
{
	JSBuilder *jsBuilder = (JSBuilder *)builder;
	if(mtd->mn != 0) {
		jsBuilder->indent--;
		JSBuilder_EmitNewLineWith(kctx, builder, "}");
		if(strcmp(KSymbol_text(mtd->mn), "new") == 0) {
			KClass *kclass = KClass_(mtd->typeId);
			JSBuilder_EmitClassFooter(kctx, builder, kclass);
		}
	}
	const char* jsSrc = KLIB KBuffer_text(kctx, &jsBuilder->jsCodeBuffer, EnsureZero);
#ifdef HAVE_LIBV8
	kbool_t isCompileOnly = KonohaContext_Is(CompileOnly, kctx);
#else
#define isCompileOnly (1)
#endif
	if(isCompileOnly) {
		printf("%s\n", jsSrc);
	}
#ifdef HAVE_LIBV8
	else {
		v8::Handle<v8::String> v8Src = v8::String::New(jsSrc);
		v8::Handle<v8::Script> v8Script = v8::Script::Compile(v8Src);
		v8::Handle<v8::Value> result = v8Script->Run();
		v8::String::AsciiValue resultStr(result);
	}
#endif
	KLIB KBuffer_Free(&jsBuilder->jsCodeBuffer);
}

static struct KVirtualCode* V8_GenerateKVirtualCode(KonohaContext *kctx, kMethod *mtd, kBlock *block, int option)
{
	INIT_GCSTACK();
	JSBuilder builderbuf = {}, *builder = &builderbuf;
	kNameSpace *ns = block->BlockNameSpace;
	builder->common.api = ns->builderApi;
	builder->common.espidx = 0;
	builder->common.a = 0;
	builder->common.shift = 0;
	builder->visitingMethod = mtd;
	JSBuilder_Init(kctx, (KBuilder *)builder, mtd);

	SUGAR VisitBlock(kctx, (KBuilder *)builder, block);

	builder->common.shift = 0;
	JSBuilder_Free(kctx, (KBuilder *)builder, mtd);
	RESET_GCSTACK();
	return NULL;
}

static KMETHOD KMethodFunc_RunVirtualMachine(KonohaContext *kctx, KonohaStack *sfp)
{
}

static KMethodFunc V8_GenerateKMethodFunc(KonohaContext *kctx, KVirtualCode *vcode)
{
	return KMethodFunc_RunVirtualMachine;
}

static struct KVirtualCode* GetDefaultBootCode(void)
{
	return NULL;
}

static void InitStaticBuilderApi(struct KBuilderAPI2 *builderApi)
{
	builderApi->target = "JavaScript";
#define DEFINE_BUILDER_API(NAME) builderApi->visit##NAME = JSBuilder_Visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builderApi->GenerateKVirtualCode = V8_GenerateKVirtualCode;
	builderApi->GenerateKMethodFunc  = V8_GenerateKMethodFunc;
	//builderApi->RunVirtualMachine   = KonohaVirtualMachine_Run;
}

static struct KBuilderAPI2* GetDefaultBuilderAPI(void)
{
	static struct KBuilderAPI2 builderApi = {};
	if(builderApi.target == NULL) {
		InitStaticBuilderApi(&builderApi);
	}
	return &builderApi;
}
// -------------------------------------------------------------------------

kbool_t LoadJavaScriptModule(KonohaFactory *factory, ModuleType type)
{
	static KModuleInfo ModuleInfo = {
		"JavaScript", K_VERSION, 0, "JavaScript",
	};
	factory->VirtualMachineInfo            = &ModuleInfo;
	//factory->IsSupportedKVirtualCode        = IsSupportedKVirtualCode;
	factory->GetDefaultBootCode            = GetDefaultBootCode;
	factory->GetDefaultBuilderAPI          = GetDefaultBuilderAPI;
#ifdef HAVE_LIBV8
	globalJSContext = new JSContext();
#endif

	return true;
}

} /* extern "C" */

