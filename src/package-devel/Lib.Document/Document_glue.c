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

#include <minikonoha/minikonoha.h>
#include <minikonoha/sugar.h>
#include <minikonoha/import/methoddecl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Dummy {
	kObjectHeader h;
} kDummy;

static void Dummy_Init(KonohaContext *kctx, kObject *o, void *conf)
{
}

static void Dummy_Free(KonohaContext *kctx, kObject *o)
{
}

static void Dummy_p(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
}

static void Dummy_Reftrace(KonohaContext *kctx, kObject *o, KObjectVisitor *visitor)
{
}

static KMETHOD Dummy_dummy(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturn(K_NULL);
}

#define FN(x) (KFieldName_(#x))

struct domClasses{
	KClass *NodeClass;
	KClass *ElementClass;
	KClass *AttrClass;
	KClass *TextClass;
	KClass *CDataSectionClass;
	KClass *EntityReferenceClass;
	KClass *EntityClass;
	KClass *ProcessingInstractionClass;
	KClass *CommentClass;
	KClass *DocumentClass;
	KClass *DocumentTypeClass;
	KClass *DocumentFragmentClass;
	KClass *NotationClass;
	KClass *NodeListClass;
};

#define DEFINE_DOMNODE_(N) do{\
	KDEFINE_CLASS defNode = {0};\
	defNode.structname = #N;\
	defNode.typeId = KTypeAttr_NewId;\
	defNode.cstruct_size = sizeof(kDummy);\
	defNode.init      = Dummy_Init;\
	defNode.p         = Dummy_p;\
	defNode.reftrace  = Dummy_Reftrace;\
	defNode.free      = Dummy_Free;\
	defNode.superTypeId = KType_DomNode;\
	classes->N##Class = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defNode, trace);\
}while(0)


static kbool_t defineDomClasses(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace, struct domClasses *classes)
{
	KDEFINE_CLASS defNode = {0};
	defNode.structname = "Node";
	defNode.typeId    = KTypeAttr_NewId;
	defNode.cstruct_size = sizeof(kDummy);
	defNode.init      = Dummy_Init;
	defNode.p         = Dummy_p;
	defNode.reftrace  = Dummy_Reftrace;
	defNode.free      = Dummy_Free;
	KClass *NodeClass = classes->NodeClass = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defNode, trace);
	int KType_DomNode = NodeClass->typeId;

	DEFINE_DOMNODE_(Element);
	DEFINE_DOMNODE_(Attr);
	DEFINE_DOMNODE_(Text);
	DEFINE_DOMNODE_(CDataSection);
	DEFINE_DOMNODE_(EntityReference);
	DEFINE_DOMNODE_(Entity);
	DEFINE_DOMNODE_(ProcessingInstraction);
	DEFINE_DOMNODE_(Comment);
	DEFINE_DOMNODE_(Document);
	DEFINE_DOMNODE_(DocumentType);
	DEFINE_DOMNODE_(DocumentFragment);
	DEFINE_DOMNODE_(Notation);

	classes->NodeListClass = KClass_p0(kctx, KClass_Array, classes->NodeClass->typeId);

	return true;
}

static kbool_t defineNodeObject(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace, struct domClasses *classes)
{
	int KType_DomNode  = classes->NodeClass->typeId;
	int KType_NodeList = classes->NodeListClass->typeId;
	int KType_Attr     = classes->AttrClass->typeId;
	int FN_node = KFieldName_("node");
	int FN_x = KFieldName_("x");

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Dummy_dummy), KType_Attr,    KType_DomNode, KMethodName_("getattributes"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getbaseURI"), 0,
		_Public, _F(Dummy_dummy), KType_NodeList,KType_DomNode, KMethodName_("getchildNodes"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getfirstChild"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getlastChild"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getlocalName"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getnamespaceURI"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getnodeName"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getnextSibiling"), 0,
		_Public, _F(Dummy_dummy), KType_Int,     KType_DomNode, KMethodName_("getnodeType"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getnodeValue"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getownerDocument"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getparentNode"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("getprefix"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("getpreviousSibiling"), 0,
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("gettextContent"), 0,

		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setattributes"), 1, KType_Attr, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setbaseURI"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setchildNodes"), 1, KType_NodeList, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setfirstChild"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setlastChild"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setlocalName"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setnamespaceURI"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setnodeName"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setnextSibiling"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setnodeType"), 1, KType_Int, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setnodeValue"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setownerDocument"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setparentNode"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setprefix"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("setpreviousSibiling"), 1, KType_DomNode, FN_x,
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("settextContent"), 1, KType_String, FN_x,

		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("appendChild"), 1, KType_DomNode, FN_node,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("cloneNode"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("cloneNode"), 1, KType_Boolean, FN(deep),
		_Public, _F(Dummy_dummy), KType_Int,     KType_DomNode, KMethodName_("compareDocumentPosition"), 1, KType_DomNode, FN_node,
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("hasAttributes"), 0,
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("hasChildNodes"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("insertBefore"), 2, KType_DomNode, FN(newnode), KType_DomNode, FN(existingnode),
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("isDefaultNamespace"), 1, KType_String, FN(namespaceURI),
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("isEqualNode"), 1, KType_DomNode, FN_node,
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("isSameNode"), 1, KType_DomNode, FN_node,
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("isSupported"), 1, KType_String, FN(feature),
		_Public, _F(Dummy_dummy), KType_Boolean, KType_DomNode, KMethodName_("isSupported"), 2, KType_String, FN(feature), KType_String, FN(version),
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("lookupNamespaceURI"), 1, KType_String, FN(prefix),
		_Public, _F(Dummy_dummy), KType_String,  KType_DomNode, KMethodName_("lookupPrefix"), 1, KType_String, FN(namespaceURI),
		_Public, _F(Dummy_dummy), KType_void,    KType_DomNode, KMethodName_("normalize"), 0,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("removeChild"), 1, KType_DomNode, FN_node,
		_Public, _F(Dummy_dummy), KType_DomNode, KType_DomNode, KMethodName_("replaceChild"), 2, KType_DomNode, FN(newnode), KType_DomNode, FN(oldnode),
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	KDEFINE_INT_CONST IntData[] = {
		{"ELEMENT_NODE",                KType_Int, 1},
		{"ATTRIBUTE_NODE",              KType_Int, 2},
		{"TEXT_NODE",                   KType_Int, 3},
		{"CDATA_SECTION_NODE",          KType_Int, 4},
		{"ENTITY_REFERENCE_NODE",       KType_Int, 5},
		{"ENTITY_NODE",                 KType_Int, 6},
		{"PROCESSING_INSTRUCTION_NODE", KType_Int, 7},
		{"COMMENT_NODE",                KType_Int, 8},
		{"DOCUMENT_NODE",               KType_Int, 9},
		{"DOCUMENT_TYPE_NODE",          KType_Int, 10},
		{"DOCUMENT_FRAGMENT_NODE",      KType_Int, 11},
		{"NOTATION_NODE",               KType_Int, 12},
		{}
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);

	return true;
}

static kbool_t defineDocumentObject(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace, struct domClasses *classes)
{
	int KType_DomNode  = classes->NodeClass->typeId;
	int KType_Element  = classes->ElementClass->typeId;
	int KType_Attr     = classes->AttrClass->typeId;
	int KType_Text     = classes->TextClass->typeId;
	int KType_CDATASection = classes->CDataSectionClass->typeId;
	int KType_Comment  = classes->CommentClass->typeId;
	int KType_Document = classes->DocumentClass->typeId;
	int KType_DocumentType = classes->DocumentTypeClass->typeId;
	int KType_Flagment = classes->DocumentFragmentClass->typeId;
	int KType_NodeList = classes->NodeListClass->typeId;

	int FN_x = KFieldName_("x");

	KDEFINE_METHOD MethodData[] = {
		_Public, _F(Dummy_dummy), KType_DocumentType, KType_Document, KMethodName_("getdoctype"), 0,
		_Public, _F(Dummy_dummy), KType_Element,      KType_Document, KMethodName_("getdocumentElement"), 0,
		_Public, _F(Dummy_dummy), KType_String,       KType_Document, KMethodName_("getdocumentURI"), 0,
		_Public, _F(Dummy_dummy), KType_Object,       KType_Document, KMethodName_("getimplementation"), 0,
		_Public, _F(Dummy_dummy), KType_String,       KType_Document, KMethodName_("getinputEncoding"), 0,
		_Public, _F(Dummy_dummy), KType_Boolean,      KType_Document, KMethodName_("getstrictErrorChecking"), 0,
		_Public, _F(Dummy_dummy), KType_String,       KType_Document, KMethodName_("getxmlEncoding"), 0,
		_Public, _F(Dummy_dummy), KType_Boolean,      KType_Document, KMethodName_("getxmlStandalone"), 0,
		_Public, _F(Dummy_dummy), KType_String,       KType_Document, KMethodName_("getxmlVersion"), 0,

		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setdoctype"), 1, KType_DocumentType, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setdocumentElement"), 1, KType_Element, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setdocumentURI"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setimplementation"), 1, KType_Object, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setinputEncoding"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setstrictErrorChecking"), 1, KType_Boolean, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setxmlEncoding"), 1, KType_String, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setxmlStandalone"), 1, KType_Boolean, FN_x,
		_Public, _F(Dummy_dummy), KType_void, KType_Document, KMethodName_("setxmlVersion"), 1, KType_String, FN_x,

		_Public, _F(Dummy_dummy), KType_Attr,         KType_Document, KMethodName_("createAttribute"), 1, KType_String, FN(attributename),
		_Public, _F(Dummy_dummy), KType_CDATASection, KType_Document, KMethodName_("createCDATASection"), 0,
		_Public, _F(Dummy_dummy), KType_CDATASection, KType_Document, KMethodName_("createCDATASection"), 1, KType_String, FN(text),
		_Public, _F(Dummy_dummy), KType_Comment,      KType_Document, KMethodName_("createComment"), 0,
		_Public, _F(Dummy_dummy), KType_Comment,      KType_Document, KMethodName_("createComment"), 1, KType_String, FN(text),
		_Public, _F(Dummy_dummy), KType_Flagment,     KType_Document, KMethodName_("createDocumentFragment"), 0,
		_Public, _F(Dummy_dummy), KType_Element,      KType_Document, KMethodName_("createElement"), 1, KType_String, FN(elementname),
		_Public, _F(Dummy_dummy), KType_Text,         KType_Document, KMethodName_("createTextNode"), 1, KType_String, FN(text),
		_Public, _F(Dummy_dummy), KType_Element,      KType_Document, KMethodName_("getElementById"), 1, KType_String, FN(elementId),
		_Public, _F(Dummy_dummy), KType_NodeList,     KType_Document, KMethodName_("getElementsByTagName"), 1, KType_String, FN(tagName),
		_Public, _F(Dummy_dummy), KType_NodeList,     KType_Document, KMethodName_("getElementsByTagNameNS"), 2, KType_String, FN(namespoceURI), KType_String, FN(tagName),
		_Public, _F(Dummy_dummy), KType_DomNode,      KType_Document, KMethodName_("importNode"), 2, KType_DomNode, FN(node), KType_Boolean, FN(deep),
		_Public, _F(Dummy_dummy), KType_void,         KType_Document, KMethodName_("normalizeDocument"), 0,
		
		_Public, _F(Dummy_dummy), KType_Document,     KType_NameSpace,KMethodName_("getdocument"), 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);

	return true;
}



static kbool_t Document_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	struct domClasses classes = { NULL };
	defineDomClasses(kctx, ns, option, trace, &classes);
	defineNodeObject(kctx, ns, option, trace, &classes);
	defineDocumentObject(kctx, ns, option, trace, &classes);
	//defineElementObject(kctx, ns, option, trace, &classes);
	//defineAttrObject(kctx, ns, option, trace, &classes);
	return true;
}

static kbool_t Document_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Document_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "Lib.Document", "1.0");
	d.PackupNameSpace = Document_PackupNameSpace;
	d.ExportNameSpace = Document_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif
