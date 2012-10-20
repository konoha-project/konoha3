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

static void JSVisitor_emitIndent(KonohaContext *kctx, IRBuilder *self)
{
	if(!DUMPER(self)->isIndentEmitted){
		int i;
		//printf("%d: ", DUMPER(self)->indent);
		for(i = 0; i < DUMPER(self)->indent; i++){
			printf("    ");
		}
		DUMPER(self)->isIndentEmitted = true;
	}
}

static void JSVisitor_emitNewLine(KonohaContext *kctx, IRBuilder *self, const char* endline)
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

static void JSVisitor_emitStringToUnderLevel(KonohaContext *kctx, IRBuilder *self, const char* prefix, const char* str, const char* suffix)
{
	DUMPER(self)->indent--;
	JSVisitor_emitString(kctx, self, prefix, str, suffix);
	DUMPER(self)->indent++;
}

static void JSVisitor_emitNewLineToUnderLevel(KonohaContext *kctx, IRBuilder *self, const char* endline)
{
	DUMPER(self)->indent--;
	JSVisitor_emitNewLine(kctx, self, endline);
	DUMPER(self)->indent++;
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
	if(expr->build == TEXPR_LET){
		JSVisitor_emitString(kctx, self, "var ", "", "");
	}
	handleExpr(kctx, self, expr);
	JSVisitor_emitNewLine(kctx, self, ";");
}

static void JSVisitor_visitBlockStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitNewLine(kctx, self, "{");
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	JSVisitor_emitNewLine(kctx, self, "}");
}

static void JSVisitor_visitReturnStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	if(DUMPER(self)->visitingMethod->mn != 0){
		JSVisitor_emitString(kctx, self, "return ", "", "");
	}
	kExpr* expr = Stmt_getFirstExpr(kctx, stmt);
	if(expr != NULL && IS_Expr(expr)){
		handleExpr(kctx, self, expr);
	}
	JSVisitor_emitNewLine(kctx, self, ";");
}

static void JSVisitor_visitIfStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "if(", "", "");
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	JSVisitor_emitNewLine(kctx, self, "){");
	DUMPER(self)->indent++;
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	kBlock *elseBlock = Stmt_getElseBlock(kctx, stmt);
	if(elseBlock != K_NULLBLOCK){
		JSVisitor_emitNewLineToUnderLevel(kctx, self, "}else{");
		visitBlock(kctx, self, elseBlock);
	}
	JSVisitor_emitNewLineToUnderLevel(kctx, self, "}");
	DUMPER(self)->indent--;
}

static void JSVisitor_visitLoopStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "while(", "", "");
	handleExpr(kctx, self, Stmt_getFirstExpr(kctx, stmt));
	JSVisitor_emitNewLine(kctx, self, "){");
	DUMPER(self)->indent++;
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	JSVisitor_emitNewLineToUnderLevel(kctx, self, "}");
	DUMPER(self)->indent--;
}

static void JSVisitor_visitJumpStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "Jump", "", "");
}

static void JSVisitor_visitTryStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitNewLine(kctx, self, "try{");
	DUMPER(self)->indent++;
	visitBlock(kctx, self, Stmt_getFirstBlock(kctx, stmt));
	kBlock *catchBlock   = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("catch"),   K_NULLBLOCK);
	kBlock *finallyBlock = SUGAR kStmt_getBlock(kctx, stmt, NULL, SYM_("finally"), K_NULLBLOCK);
	if(catchBlock != K_NULLBLOCK){
		JSVisitor_emitNewLineToUnderLevel(kctx, self, "}catch(e){");
		visitBlock(kctx, self, catchBlock);
	}
	if(finallyBlock != K_NULLBLOCK){
		JSVisitor_emitNewLineToUnderLevel(kctx, self, "}finally{");
		visitBlock(kctx, self, finallyBlock);
	}
	JSVisitor_emitNewLineToUnderLevel(kctx, self, "}");
	DUMPER(self)->indent--;
}

static void JSVisitor_visitUndefinedStmt(KonohaContext *kctx, IRBuilder *self, kStmt *stmt)
{
	JSVisitor_emitString(kctx, self, "UNDEF", "", "");
}

static void JSVisitor_visitConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KonohaStack sfp[1];
	kObject *obj = expr->objectConstValue;
	sfp[0].asObject = obj;
	O_ct(obj)->p(kctx, sfp, 0, &wb);
	char  *str = (char *) KLIB Kwb_top(kctx, &wb, 0);
	char buf[128];
	snprintf(buf, 128, "%s", str);
	JSVisitor_emitString(kctx, self, buf, "", "");
	KLIB Kwb_free(&wb);
}

static void JSVisitor_visitNConstExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KonohaStack sfp[1];
	unsigned long unboxVal = expr->unboxConstValue;
	KonohaClass *ct = CT_(expr->ty);
	sfp[0].unboxValue = unboxVal;
	ct->p(kctx, sfp, 0, &wb);
	char  *str = (char *) KLIB Kwb_top(kctx, &wb, 0);
	JSVisitor_emitString(kctx, self, str, "", "");
	KLIB Kwb_free(&wb);
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
	JSVisitor_emitNewLine(kctx, self, "{");
	visitBlock(kctx, self, expr->block);
	JSVisitor_emitNewLine(kctx, self, "}");
}

static void JSVisitor_visitFieldExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitString(kctx, self, "FIELD", "", "");
}

static bool JSVisitor_importPackage(KonohaContext *kctx, kNameSpace *ns, kString *package, kfileline_t uline)
{
	KImportPackage(ns, S_text(package), NULL);
	return true;
}

static void JSVisitor_ConvertAndEmitMethodName(KonohaContext *kctx, IRBuilder *self, kExpr *expr, kExpr *receiver, kMethod *mtd)
{
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	KLIB Kwb_printf(kctx, &wb, "%s%s", T_mn(mtd->mn));

	const char *methodName = KLIB Kwb_top(kctx, &wb, 1);
	if(receiver->ty == TY_System && methodName[0] == 'p'){
		JSVisitor_emitString(kctx, self, "console.log", "", "");
	}else{
		if(receiver->ty == TY_NameSpace){
			if(mtd->mn == MN_("import")){
				kString *packageNameString = (kString*)kExpr_at(expr, 2)->objectConstValue;
				kNameSpace *ns = (kNameSpace*)receiver->objectConstValue;
				JSVisitor_importPackage(kctx, ns, packageNameString, expr->termToken->uline);
				JSVisitor_emitString(kctx, self, "//", "", "");
			}
		}else{
			if(receiver->build == TEXPR_NULL){
				JSVisitor_emitString(kctx, self, CT_t(CT_(receiver->ty)), "", "");
			}else{
				handleExpr(kctx, self, receiver);
			}
			JSVisitor_emitString(kctx, self, ".", "", "");
		}
		JSVisitor_emitString(kctx, self, "", T_mn(mtd->mn));
	}
}

static void JSVisitor_visitCallExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	kMethod *mtd = CallExpr_getMethod(expr);

	if(kArray_size(expr->cons) == 2 && MethodName_isUnaryOperator(kctx, mtd->mn)){
		JSVisitor_emitString(kctx, self, T_mn(mtd->mn), "(");
		handleExpr(kctx, self, kExpr_at(expr, 1));
		JSVisitor_emitString(kctx, self, ")", "", "");
	}
	else if(MethodName_isBinaryOperator(kctx, mtd->mn)){
		JSVisitor_emitString(kctx, self, "(", "", "");
		handleExpr(kctx, self, kExpr_at(expr, 1));
		JSVisitor_emitString(kctx, self, " ", SYM_t(mtd->mn), " ");
		handleExpr(kctx, self, kExpr_at(expr, 2));
		JSVisitor_emitString(kctx, self, ")", "", "");
	}
	else {
		kExpr *receiver = kExpr_at(expr, 1);
		if(mtd == DUMPER(self)->visitingMethod){
			JSVisitor_emitString(kctx, self, "arguments.callee", "", "");
		}else{
			JSVisitor_ConvertAndEmitMethodName(kctx, self, expr, receiver, mtd);
		}
		JSVisitor_emitString(kctx, self, "(", "", "");
		unsigned i;
		unsigned n = kArray_size(expr->cons);
		for(i = 2; i < n;){
			handleExpr(kctx, self, kExpr_at(expr, i));
			if(++i < n){
				JSVisitor_emitString(kctx, self, ", ", "", "");
			}
		}
		JSVisitor_emitString(kctx, self, ")", "", "");
	}
}

static void JSVisitor_visitAndExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	JSVisitor_emitString(kctx, self, "(", "", "");
	for(i = 1; i < kArray_size(expr->cons);){
		handleExpr(kctx, self, kExpr_at(expr, i));
		if(++i < n){
			JSVisitor_emitString(kctx, self, " && ", "", "");
		}
	}
	JSVisitor_emitString(kctx, self, ")", "", "");
}

static void JSVisitor_visitOrExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	unsigned n = kArray_size(expr->cons);
	unsigned i;
	JSVisitor_emitString(kctx, self, "(", "", "");
	for(i = 1; i < kArray_size(expr->cons);){
		handleExpr(kctx, self, kExpr_at(expr, i));
		if(++i < n){
			JSVisitor_emitString(kctx, self, " || ", "", "");
		}
	}
	JSVisitor_emitString(kctx, self, ")", "", "");
}

static void JSVisitor_visitLetExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	DUMPER(self)->indent++;
	handleExpr(kctx, self, kExpr_at(expr, 1));
	JSVisitor_emitString(kctx, self, " = ", "", "");
	handleExpr(kctx, self, kExpr_at(expr, 2));
	DUMPER(self)->indent--;
}

static void JSVisitor_visitStackTopExpr(KonohaContext *kctx, IRBuilder *self, kExpr *expr)
{
	JSVisitor_emitString(kctx, self, "/*FIXME*/STACKTOP", "", "");
}

static void JSVisitor_init(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	unsigned i;
	builder->local_fields = (void *) KMalloc_UNTRACE(sizeof(JSVisitorLocal));
	DUMPER(builder)->visitingMethod = mtd;
	DUMPER(builder)->isIndentEmitted = false;
	DUMPER(builder)->indent = 0;
	
	KGrowingBuffer wb;
	KLIB Kwb_init(&(kctx->stack->cwb), &wb);
	kParam *pa = Method_param(mtd);
	if(mtd->mn != 0){
		kMethod_setFunc(kctx, mtd, NULL);
		if(mtd->typeId == TY_NameSpace){
			KLIB Kwb_printf(kctx, &wb, "%s%s = function(", T_mn(mtd->mn));
		}else{
			KLIB Kwb_printf(kctx, &wb, "%s.%s%s = function(", CT_t(CT_(mtd->typeId)), T_mn(mtd->mn));
		}
		for(i = 0; i < pa->psize; i++){
			if(i != 0){
				KLIB Kwb_printf(kctx, &wb, ", %s", SYM_t(pa->paramtypeItems[i].fn));
			}else{
				KLIB Kwb_printf(kctx, &wb, "%s", SYM_t(pa->paramtypeItems[i].fn));
			}
		}
		JSVisitor_emitString(kctx, builder, KLIB Kwb_top(kctx, &wb, 1), "", "");
		JSVisitor_emitNewLine(kctx, builder, "){");
		DUMPER(builder)->indent++;
	}else{
		KLIB kNameSpace_compileAllDefinedMethods(kctx);
		//KLIB Kwb_printf(kctx, &wb, "(function(", CT_t(CT_(mtd->typeId)), T_mn(mtd->mn));
	}

	KLIB Kwb_free(&wb);
}

static void JSVisitor_free(KonohaContext *kctx, struct IRBuilder *builder, kMethod *mtd)
{
	if(mtd->mn != 0){
		DUMPER(builder)->indent--;
		JSVisitor_emitNewLine(kctx, builder, "}");
	}else{
		//JSVisitor_emitNewLine(kctx, builder, "})();");
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
