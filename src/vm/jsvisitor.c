/****************************************************************************
 * Copyright (c) 2012, the Konoha project authors. All rights reserved.
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *	this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *	notice, this list of conditions and the following disclaimer in the
 *	documentation and/or other materials provided with the distribution.
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

#ifdef USE_JS_VISITOR
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MN_isNotNull MN_("isNotNull")
#define MN_isNull    MN_("isNull")
#define MN_get    MN_("get")
#define MN_set    MN_("set")
#define MN_opNOT  MN_("!")
#define MN_opADD  MN_("+")
#define MN_opSUB  MN_("-")
#define MN_opMUL  MN_("*")
#define MN_opDIV  MN_("/")
#define MN_opMOD  MN_("%")
#define MN_opEQ   MN_("==")
#define MN_opNEQ  MN_("!=")
#define MN_opLT   MN_("<")
#define MN_opLTE  MN_("<=")
#define MN_opGT   MN_(">")
#define MN_opGTE  MN_(">=")
#define MN_opLAND MN_("&")
#define MN_opLOR  MN_("|")
#define MN_opLXOR MN_("^")
#define MN_opLSFT MN_("<<")
#define MN_opRSFT MN_(">>")

#define DUMPER(BUILDER)  ((JSVisitorLocal*)(BUILDER)->local_fields)

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

static enum kSymbolPrefix SYM_PRE_ID(ksymbol_t sym)
{
	size_t mask = ((size_t)(SYM_HEAD(sym)) >> ((sizeof(ksymbol_t) * 8)-3));
	DBG_ASSERT(mask < 8);
	return (enum kSymbolPrefix)mask;
}

static void JSVisitor_emitIndent(KonohaContext *kctx, IRBuilder *self)
{
	if(!DUMPER(self)->isIndentEmitted) {
		int i;
		//printf("%d: ", DUMPER(self)->indent);
		for(i = 0; i < DUMPER(self)->indent; i++) {
			printf("    ");
		}
		DUMPER(self)->isIndentEmitted = true;
	}
}

static void JSVisitor_emitNewLineWith(KonohaContext *kctx, IRBuilder *self, const char* endline)
{
	JSVisitor_emitIndent(kctx, self);
	DUMPER(self)->isIndentEmitted = false;
	printf("%s\n", endline);
}

static void JSVisitor_emitString(KonohaContext *kctx, IRBuilder *self, const char* prefix, const char* str, const char* suffix)
{
	JSVisitor_emitIndent(kctx, self);
	printf("%s%s%s", prefix, str, suffix);
}

static void JSVisitor_visitBlock(KonohaContext *kctx, IRBuilder *self, kBlock *block)
{
	JSVisitor_emitNewLineWith(kctx, self, "{");
	DUMPER(self)->indent++;
	visitBlock(kctx, self, block);
	DUMPER(self)->indent--;
	JSVisitor_emitString(kctx, self, "}", "", "");
}

static void JSVisitor_visitExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr, const char* prefix, const char* suffix)
{
	JSVisitor_emitString(kctx, self, prefix, "", "");
	handleExpr(kctx, self, expr);
	JSVisitor_emitString(kctx, self, suffix, "", "");
}

static int MethodName_isBinaryOperator(KonohaContext *kctx, kmethodn_t mn)
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

static int MethodName_isUnaryOperator(KonohaContext *kctx, kmethodn_t mn)
{
	if(mn == MN_opSUB) return 1;
	if(mn == MN_opNOT) return 1;
	return 0;
}

static void JSVisitor_visitErrStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, S_text(kStmt_getObjectNULL(kctx, stmt, KW_ERR)), "", "");
}

static void JSVisitor_visitExprStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	kExpr *expr = Stmt_getFirstExpr(kctx, stmt);
	if(expr->build == TEXPR_LET) {
		if(kExpr_at(expr, 1)->build == TEXPR_FIELD){
			JSVisitor_emitString(kctx, self, "this.", "", "");
		}else{
			JSVisitor_emitString(kctx, self, "var ", "", "");
		}
	}
	handleExpr(kctx, self, expr);
	JSVisitor_emitNewLineWith(kctx, self, ";");
}

static void JSVisitor_visitBlockStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
}

static void JSVisitor_visitReturnStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	if(DUMPER(self)->visitingMethod->mn != 0) {
		JSVisitor_emitString(kctx, self, "return ", "", "");
	}
	kExpr* expr = Stmt_getFirstExpr(kctx, stmt);
	if(expr != NULL && IS_Expr(expr)) {
		handleExpr(kctx, self, expr);
	}
	JSVisitor_emitNewLineWith(kctx, self, ";");
}

static void JSVisitor_visitIfStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_visitExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt), "if(", ") ");
	JSVisitor_visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	kBlock *elseBlock = Stmt_getElseBlock(kctx, stmt);
	if(elseBlock != K_NULLBLOCK) {
		JSVisitor_emitString(kctx, self, "else", "", "");
		JSVisitor_visitBlock(kctx, self, elseBlock);
	}
	JSVisitor_emitNewLineWith(kctx, self, "");
}

static void JSVisitor_visitLoopStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_visitExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt), "while(", ") ");
	JSVisitor_visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	JSVisitor_emitNewLineWith(kctx, self, "");
}

static void JSVisitor_visitJumpStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "Jump", "", "");
}

static void JSVisitor_visitTryStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitNewLineWith(kctx, self, "try ");
	JSVisitor_visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	kBlock *catchBlock   = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
	kBlock *finallyBlock = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
	if(catchBlock != K_NULLBLOCK) {
		JSVisitor_emitString(kctx, self, "catch(e) ", "", "");
		JSVisitor_visitBlock(kctx, self, catchBlock);
	}
	if(finallyBlock != K_NULLBLOCK) {
		JSVisitor_emitString(kctx, self, "finally", "", "");
		JSVisitor_visitBlock(kctx, self, finallyBlock);
	}
	JSVisitor_emitNewLineWith(kctx, self, "");
}

static void JSVisitor_visitUndefinedStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "UNDEF", "", "");
}

static void JSVisitor_emitKonohaValue(KonohaContext *kctx, IRBuilder *self, KonohaClass* ct, KonohaStack* sfp)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	ct->p(kctx, sfp, 0, &wb);
	char *str = (char*)KLIB Kwb_top(kctx, &wb, 0);
	JSVisitor_emitString(kctx, self, str, "", "");
	KLIB Kwb_free(&wb);
}

static void JSVisitor_emitConstValue(KonohaContext *kctx, IRBuilder *self, kObject *obj)
{
	KonohaStack sfp[1];
	sfp[0].asObject = obj;
	JSVisitor_emitKonohaValue(kctx, self, O_ct(obj), sfp);
}

static void JSVisitor_emitNConstValue(KonohaContext *kctx, IRBuilder *self, KonohaClass *ct, unsigned long long unboxVal)
{
	KonohaStack sfp[1];
	sfp[0].unboxValue = unboxVal;
	JSVisitor_emitKonohaValue(kctx, self, ct, sfp);
}

static void JSVisitor_visitConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitConstValue(kctx, self, expr->objectConstValue);
}

static void JSVisitor_visitNConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitNConstValue(kctx, self, CT_(expr->ty), expr->unboxConstValue);
}

static void JSVisitor_visitNewExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitString(kctx, self, "new", "", "");
}

static void JSVisitor_visitNullExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitString(kctx, self, "null", "", "");
}

static void JSVisitor_visitLocalExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kToken *tk = (kToken*)expr->termToken;
	JSVisitor_emitString(kctx, self, S_text(tk->text), "", "");
}

static void JSVisitor_visitBlockExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_visitBlock(kctx, self, expr->block);
}

static void JSVisitor_visitFieldExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kToken *tk = (kToken*)expr->termToken;
	JSVisitor_emitString(kctx, self, S_text(tk->text), "", "");
}

static bool JSVisitor_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KImportPackage(ns, S_text(package), NULL);
	return true;
}

static void JSVisitor_visitExprParams(KonohaContext *kctx, IRBuilder *self, kExpr *expr, int beginIndex, const char* delimiter, const char* leftBracket, const char* rightBracket)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	if(leftBracket) {
		JSVisitor_emitString(kctx, self, leftBracket, "", "");
	}
	for(i = beginIndex; i < n;) {
		handleExpr(kctx, self, kExpr_at(expr, i));
		if(++i < n) {
			JSVisitor_emitString(kctx, self, delimiter, "", "");
		}
	}
	if(rightBracket) {
		JSVisitor_emitString(kctx, self, rightBracket, "", "");
	}
}

static void JSVisitor_ConvertAndEmitMethodName(KonohaContext *kctx, IRBuilder *self, kExpr *expr, kExpr *receiver, kMethod *mtd)
{
	KonohaClass *globalObjectClass = KLIB kNameSpace_GetClass(kctx, self->currentStmt->parentBlockNULL->BlockNameSpace, "GlobalObject", 12, NULL);
	kbool_t isGlobal = (CT_(receiver->ty) == globalObjectClass || receiver->ty == TY_NameSpace);
	const char *methodName = SYM_t(mtd->mn);
	if(receiver->ty == TY_NameSpace) {
		if(mtd->mn == MN_("import")) {
			kString *packageNameString = (kString*)kExpr_at(expr, 2)->objectConstValue;
			kNameSpace *ns = (kNameSpace*)receiver->objectConstValue;
			JSVisitor_importPackage(kctx, ns, packageNameString, expr->termToken->uline);
			JSVisitor_emitString(kctx, self, "//", "", "");
			return;
		}
	}
	if(receiver->ty == TY_System && methodName[0] == 'p') {
		JSVisitor_emitString(kctx, self, "console.log", "", "");
	}else if(strcmp(SYM_t(mtd->mn), "new") == 0) {
		JSVisitor_emitString(kctx, self, "new ", CT_t(CT_(receiver->ty)), "");
	}else if(strcmp(SYM_t(mtd->mn), "newList") == 0) {
	}else if(strcmp(SYM_t(mtd->mn), "newArray") == 0) {
		JSVisitor_emitString(kctx, self, "new Array", "", "");
	}else{
		if(!isGlobal) {
			if(receiver->build == TEXPR_NULL) {
				JSVisitor_emitString(kctx, self, CT_t(CT_(receiver->ty)), "", "");
			}else{
				handleExpr(kctx, self, receiver);
			}
		}
		switch(SYM_PRE_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
			if(kArray_size(expr->cons) > 2) {
				JSVisitor_visitExpr(kctx, self, kExpr_at(expr, 2), "[", "]");
			}else{
				if(!isGlobal) {
					JSVisitor_emitString(kctx, self, ".", "", "");
				}
				JSVisitor_emitString(kctx, self, methodName, "", "");
			}
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->cons) > 3) {
				JSVisitor_visitExpr(kctx, self, kExpr_at(expr, 2), "[", "]");
			}else{
				if(isGlobal) {
					JSVisitor_emitString(kctx, self, "var ", "", "");
				}else{
					JSVisitor_emitString(kctx, self, ".", "", "");
				}
			}
			JSVisitor_emitString(kctx, self, methodName, " = ", "");
			break;
		case kSymbolPrefix_TO:
			break;
		default:
			if(!isGlobal){
				JSVisitor_emitString(kctx, self, ".", "", "");
			}
			JSVisitor_emitString(kctx, self, methodName, "", "");
			break;
		}
	}
}

static void JSVisitor_visitCallExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);
	kbool_t isArray = false;

	if(kArray_size(expr->cons) == 2 && MethodName_isUnaryOperator(kctx, mtd->mn)) {
		JSVisitor_emitString(kctx, self, MethodName_t(mtd->mn), "(");
		handleExpr(kctx, self, kExpr_at(expr, 1));
		JSVisitor_emitString(kctx, self, ")", "", "");
	}
	else if(MethodName_isBinaryOperator(kctx, mtd->mn)) {
		JSVisitor_visitExprParams(kctx, self, expr, 1, SYM_t(mtd->mn),"(", ")");
	}
	else{
		kExpr *receiver = kExpr_at(expr, 1);
		if(mtd == DUMPER(self)->visitingMethod) {
			JSVisitor_emitString(kctx, self, "arguments.callee", "", "");
		}else if(strcmp(SYM_t(mtd->mn), "newList") == 0) {
			isArray = true;
		}else {
			JSVisitor_ConvertAndEmitMethodName(kctx, self, expr, receiver, mtd);
		}
		switch(SYM_PRE_ID(mtd->mn)) {
		case kSymbolPrefix_GET:
		case kSymbolPrefix_TO:
			break;
		case kSymbolPrefix_SET:
			if(kArray_size(expr->cons) > 3) {
				handleExpr(kctx, self, kExpr_at(expr, 3));
			}else{
				handleExpr(kctx, self, kExpr_at(expr, 2));
			}
			break;
		default:
			JSVisitor_visitExprParams(kctx, self, expr, 2, ", ", isArray ? "[" : "(", isArray ? "]" : ")");
			break;
		}
	}
}

static void JSVisitor_visitAndExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_visitExprParams(kctx, self, expr, 1, " && ", "(", ")");
}

static void JSVisitor_visitOrExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_visitExprParams(kctx, self, expr, 1, " || ", "(", ")");
}

static void JSVisitor_visitLetExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_visitExprParams(kctx, self, expr, 1, " = ", NULL, NULL);
}

static void JSVisitor_visitStackTopExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitString(kctx, self, "/*FIXME*/STACKTOP", "", "");
}

static void JSVisitor_emitExtendFunctionCode(KonohaContext *kctx, IRBuilder *builder)
{
	JSVisitor_emitNewLineWith(kctx, builder, "var __extends = this.__extends || function (d, b) {");
	DUMPER(builder)->indent++;
	JSVisitor_emitNewLineWith(kctx, builder, "function __() { this.constructor = d; }");
	JSVisitor_emitNewLineWith(kctx, builder, "__.prototype = b.prototype;");
	JSVisitor_emitNewLineWith(kctx, builder, "d.prototype = new __();");
	DUMPER(builder)->indent--;
	JSVisitor_emitNewLineWith(kctx, builder, "}");
}

static void JSVisitor_visitClassFields(KonohaContext *kctx, IRBuilder *builder, KonohaClass *class)
{
	kushort_t i;
	KonohaClassField *field = class->fieldItems;
	kObject *constList = class->defaultNullValue_OnGlobalConstList;
	for(i = 0; i < class->fieldsize; ++i) {
		JSVisitor_emitString(kctx, builder, "this.", SYM_t(field[i].fn), " = ");
		if(TY_isUnbox(field[i].ty)) {
			JSVisitor_emitNConstValue(kctx, builder, CT_(field[i].ty), constList->fieldUnboxItems[i]);
		}else{
			JSVisitor_emitConstValue(kctx, builder, constList->fieldObjectItems[i]);
		}
		JSVisitor_emitNewLineWith(kctx, builder, ";");
	}
}

static void JSVisitor_emitMethodHeader(KonohaContext *kctx, IRBuilder *builder, kMethod *mtd)
{
	KonohaClass *class = CT_(mtd->typeId);
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kParam *params = Method_param(mtd);
	unsigned i;
	if(mtd->typeId == TY_NameSpace) {
		KLIB Kwb_printf(kctx, &wb, "%s%s = function(", MethodName_t(mtd->mn));
	}else if(strcmp(SYM_t(mtd->mn), "new") == 0) {
		KLIB Kwb_printf(kctx, &wb, "function %s(", CT_t(class));
	}else{
		KLIB kMethod_DoLazyCompilation(kctx, mtd);
		if(kMethod_is(Static, mtd)) {
			KLIB Kwb_printf(kctx, &wb, "%s.%s%s = function(", CT_t(CT_(mtd->typeId)), MethodName_t(mtd->mn));
		}else{
			KLIB Kwb_printf(kctx, &wb, "%s.prototype.%s%s = function(", CT_t(CT_(mtd->typeId)), MethodName_t(mtd->mn));
		}
	}
	for(i = 0; i < params->psize; ++i) {
		if(i != 0) {
			KLIB Kwb_printf(kctx, &wb, ", ");
		}
		KLIB Kwb_printf(kctx, &wb, "%s", SYM_t(params->paramtypeItems[i].fn));
	}
	KLIB Kwb_printf(kctx, &wb, ")");
	JSVisitor_emitString(kctx, builder, KLIB Kwb_top(kctx, &wb, 1), "", "");
	KLIB Kwb_free(&wb);
}

static void JSVisitor_emitClassHeader(KonohaContext *kctx, IRBuilder *builder, KonohaClass *class)
{
	KonohaClass *base = CT_(class->superTypeId);
	if(base->typeId != TY_Object) {
		JSVisitor_emitExtendFunctionCode(kctx, builder);
		JSVisitor_emitString(kctx, builder, "var ", CT_t(class), " = (function (_super) {");
	}else{
		JSVisitor_emitString(kctx, builder, "var ", CT_t(class), " = (function () {");
	}
	JSVisitor_emitNewLineWith(kctx, builder, "");
	DUMPER(builder)->indent++;
	if(base->typeId != TY_Object){
		JSVisitor_emitString(kctx, builder, "__extends(", CT_t(class), ", _super);");
		JSVisitor_emitNewLineWith(kctx, builder, "");
	}
}

static void JSVisitor_emitClassFooter(KonohaContext *kctx, IRBuilder *builder, KonohaClass *class)
{
	KonohaClass *base = CT_(class->superTypeId);
	JSVisitor_emitString(kctx, builder, "return ", CT_t(class), ";");
	JSVisitor_emitNewLineWith(kctx, builder, "");
	DUMPER(builder)->indent--;
	if(base->typeId != TY_Object) {
		JSVisitor_emitString(kctx, builder, "})(", CT_t(base), ");");
		JSVisitor_emitNewLineWith(kctx, builder, "");
	}else{
		JSVisitor_emitNewLineWith(kctx, builder, "})();");
	}
}

static void JSVisitor_init(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	kbool_t isConstractor = false;
	builder->local_fields = (void*)KMalloc_UNTRACE(sizeof(JSVisitorLocal));
	DUMPER(builder)->visitingMethod = mtd;
	DUMPER(builder)->isIndentEmitted = false;
	DUMPER(builder)->indent = 0;
	
	KonohaClass *class = CT_(mtd->typeId);
	KonohaClass *base  = CT_(class->superTypeId);
	
	if(mtd->mn != 0) {
		kMethod_setFunc(kctx, mtd, NULL);
		if(strcmp(SYM_t(mtd->mn), "new") == 0) {
			isConstractor = true;
		}
		if(isConstractor) {
			JSVisitor_emitClassHeader(kctx, builder, class);
		}
		JSVisitor_emitMethodHeader(kctx, builder, mtd);
		JSVisitor_emitNewLineWith(kctx, builder, " {");
		DUMPER(builder)->indent++;
		if(isConstractor) {
			if(base->typeId != TY_Object) {
				JSVisitor_emitNewLineWith(kctx, builder, "_super.call(this);");
			}
			JSVisitor_visitClassFields(kctx, builder, class);
		}
	}else{
		KLIB kMethod_DoLazyCompilation(kctx, mtd);
	}
}

static void JSVisitor_free(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	if(mtd->mn != 0) {
		DUMPER(builder)->indent--;
		JSVisitor_emitNewLineWith(kctx, builder, "}");
		if(strcmp(SYM_t(mtd->mn), "new") == 0) {
			KonohaClass *class = CT_(mtd->typeId);
			JSVisitor_emitClassFooter(kctx, builder, class);
		}
	}
	KFree(builder->local_fields, sizeof(JSVisitorLocal));
	builder->local_fields = NULL;
}

static IRBuilder *createJSVisitor(IRBuilder *builder)
{
#define DEFINE_BUILDER_API(NAME) builder->api.visit##NAME = JSVisitor_visit##NAME;
	VISITOR_LIST(DEFINE_BUILDER_API);
#undef DEFINE_BUILDER_API
	builder->api.fn_init = JSVisitor_init;
	builder->api.fn_free = JSVisitor_free;
	return builder;
}

#ifdef __cplusplus
}
#endif

#undef DUMPER
#endif /* USE_JS_VISITOR */
