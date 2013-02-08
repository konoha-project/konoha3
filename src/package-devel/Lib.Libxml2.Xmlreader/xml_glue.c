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

#define USE_STRINGLIB 1

#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlreader.h>

#ifdef __cplusplus
extern "C" {
#endif

///* ------------------------------------------------------------------------ */
///* [XmlDoc] */
//
//static void XmlDoc_init(KonohaContext *kctx, kRawPtr *po)
//{
//	po->rawptr = NULL;
//}
//
//static void XmlDoc_free(KonohaContext *kctx, kRawPtr *po)
//{
//	if(po->rawptr != NULL) {
//		xmlFreeDoc((xmlDocPtr)po->rawptr);
//		po->rawptr = NULL;
//	}
//}
//
//static void XmlDoc_checkout(KonohaContext *kctx, kRawPtr *po, int isFailed)
//{
//	XmlDoc_free(kctx, po);
//}
//
//DEFAPI(void) defXmlDoc(KonohaContext *kctx, kclass_t cid, kclassdef_t *cdef)
//{
//	cdef->name = "XmlDoc";
//	cdef->init = XmlDoc_init;
//	cdef->free = XmlDoc_free;
//	cdef->checkout = XmlDoc_checkout;
//}
//
////static knh_IntData_t XmlDocConstint[] = {
////		{"TYPE_NONE",         XML_READER_TYPE_NONE},
////		{"TYPE_ELEMENT",      XML_READER_TYPE_ELEMENT},
////		{"TYPE_ATTRIBUTE",    XML_READER_TYPE_ATTRIBUTE},
////		{"TYPE_TEXT",         XML_READER_TYPE_TEXT},
////		{"TYPE_CDATA",        XML_READER_TYPE_CDATA},
////		{"TYPE_REFERENCE",    XML_READER_TYPE_ENTIKType_REFERENCE},
////		{"TYPE_ENTITY",       XML_READER_TYPE_ENTITY},
////		{"TYPE_PROCESSING_INSTRUCTION",  XML_READER_TYPE_PROCESSING_INSTRUCTION},
////		{"TYPE_COMMENT",      XML_READER_TYPE_COMMENT},
////		{"TYPE_DOCUMENT",     XML_READER_TYPE_DOCUMENT},
////		{"TYPE_DOCUMENT_TYPE",XML_READER_TYPE_DOCUMENT_TYPE},
////		{"TYPE_DOCUMENT_FRAGMENT",      XML_READER_TYPE_DOCUMENT_FRAGMENT},
////		{"TYPE_NOTATION",     XML_READER_TYPE_NOTATION},
////		{"TYPE_WHITESPACE",   XML_READER_TYPE_WHITESPACE},
////		{"TYPE_SIGNIFICANT_WHITESPACE", XML_READER_TYPE_SIGNIFICANT_WHITESPACE},
////		{"TYPE_END_ELEMENT",  XML_READER_TYPE_END_ELEMENT},
////		{"TYPE_END_ENTITY",   XML_READER_TYPE_END_ENTITY},
////		{"TYPE_XML_DECLATION",XML_READER_TYPE_XML_DECLARATION},
////		{NULL} // end of const
////};
////
////DEFAPI(void) constXmlDoc(KonohaContext *kctx, kclass_t cid, const knh_LoaderAPI_t *kapi)
////{
////	kapi->loadClassIntConst(kctx, cid, XmlDocConstint);
////}
//
////## @Native @Throwable XmlDoc XmlDoc.new(String version, XmlDoc _);
//static KMETHOD XmlDoc_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//	xmlChar* version = String_to(xmlChar*, sfp[1]);
//	xmlDocPtr doc = xmlNewDoc(version);
//	kRawPtr *po = new_RawPtr(kctx, sfp[2].p, doc);
//	KReturn(po);
//}

//static KMETHOD Xml_setAttr(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = Glue_to(xmlDocPtr, sfp[0]);
//    xmlChar *name  = String_to(xmlChar*, sfp[1]);
//    xmlChar *value = String_to(xmlChar*, sfp[2]);
//    xmlNewDocProp(doc, name, value);
//    KNH_RETURN_void(kctx,sfp);
//}
//
//static KMETHOD Xml_createNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = Glue_to(xmlDocPtr, sfp[0]);
//    xmlChar *name = String_to(xmlChar*, sfp[1]);
//    doc->children = xmlNewDocNode(doc, NULL, name , NULL);
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",doc->children,NULL));
//}
//
//static KMETHOD Xml_getRoot(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = Glue_to(xmlDocPtr, sfp[0]);
//    xmlNodePtr node = xmlDocGetRootElement(doc);
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",node,NULL));
//}
//
//static KMETHOD Xml_dump(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = Glue_to(xmlDocPtr, sfp[0]);
//    xmlChar* ret;
//    int   size;
//    xmlDocDumpMemory(doc,&ret,&size);
//    KNH_RETURN(kctx, sfp, new_String(kctx, B((char *)ret), NULL));
//}
//
//static KMETHOD Xml_dumpEnc(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = Glue_to(xmlDocPtr, sfp[0]);
//    char* enc = String_to(char*, sfp[1]);
//    xmlChar* ret;
//    int   size;
//    xmlDocDumpMemoryEnc(doc,&ret,&size,enc);
//    KNH_RETURN(kctx, sfp, new_String(kctx, B((char *)ret), NULL));
//}
//
///* XmlNode */
//static KMETHOD XmlNode_createNode(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr parent = Glue_to(xmlNodePtr, sfp[0]);
//    xmlChar *name = String_to(xmlChar*, sfp[1]);
//    xmlChar *val  = String_to(xmlChar*, sfp[2]);
//
//    xmlNodePtr node = xmlNewChild(parent, NULL, name, val);
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",node,NULL));
//}
//
//static KMETHOD XmlNode_addChild(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr parent = (xmlNodePtr) p_cptr(sfp[0]);
//    xmlNodePtr child  = (xmlNodePtr) p_cptr(sfp[1]);
//    xmlAddChild(parent,child);
//    KNH_RETURN_void(kctx,sfp);
//}
//
//static KMETHOD XmlNode_setAttr(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    xmlChar *name = String_to(xmlChar *, sfp[1]);
//    xmlChar *val  = String_to(xmlChar *, sfp[2]);
//    xmlSetProp(node,name,val);
//    KNH_RETURN_void(kctx,sfp);
//}
//
//static KMETHOD XmlNode_getContent(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    xmlChar* ret = (xmlChar *)"";
//    if(node->content){
//        ret = node->content;
//        fprintf(stdout,"[%s]\n",(char *)node->content);
//    }
//    KNH_RETURN(kctx, sfp, new_String(kctx, B((char *)ret), NULL));
//}
//
//static KMETHOD XmlNode_getName(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    xmlChar* ret = (xmlChar *)"";
//    if(node->name){
//        ret = (xmlChar *)node->name;
//    }
//    KNH_RETURN(kctx, sfp, new_String(kctx, B((char *)ret), NULL));
//}
//
//static KMETHOD XmlNode_getAttr(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    xmlChar *name = String_to(xmlChar *, sfp[1]);
//    xmlChar *ret  = (xmlChar *)"";
//    if(node->properties){
//        ret  = xmlGetProp(node,name);
//    }
//    KNH_RETURN(kctx, sfp, new_String(kctx, B((char *)ret), NULL));
//}
//
//
//static KMETHOD XmlNode_getChild(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node  = (xmlNodePtr) p_cptr(sfp[0]);
//    xmlNodePtr child = (xmlNodePtr) node->children;
//    if(child == NULL){
//        KNH_THROW__T(kctx, "XmlNode: dont have child!");
//    }
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",child,NULL));
//}
//
//static KMETHOD XmlNode_getNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node = (xmlNodePtr) p_cptr(sfp[0]);
//    xmlNodePtr next = (xmlNodePtr) node->next;
//    if(next == NULL){
//        KNH_THROW__T(kctx, "XmlNode: dont have next!");
//    }
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",next,NULL));
//}
//
//static KMETHOD XmlNode_hasChild(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    if(node->children) {
//        KNH_RETURN_boolean(kctx, sfp, 1);
//    }
//    KNH_RETURN_boolean(kctx, sfp, 0);
//}
//
//static KMETHOD XmlNode_hasNext(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    if(node->next) {
//        KNH_RETURN_boolean(kctx, sfp, 1);
//    }
//    KNH_RETURN_boolean(kctx, sfp, 0);
//}
//
///*
//static xmlDocPtr document = NULL;
//static KMETHOD XmlNode_dump(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlNodePtr node= (xmlNodePtr) p_cptr(sfp[0]);
//    xmlElemDump(stdout,document,node);
//    KNH_RETURN_void(kctx,sfp);
//}
//*/
//
///* XPath */
//static void knh_xpath_gfree(KonohaContext *kctx, knh_Glue_t *g)
//{
//    xmlXPathContextPtr xctx = (xmlXPathContextPtr) g->ptr;
//    xmlXPathFreeContext(xctx);
//    xmlCleanupParser();
//}
//
//static KMETHOD XPath_new(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlDocPtr doc = (xmlDocPtr) p_cptr(sfp[1]);
//    xmlXPathContextPtr xctx;
//    if(!(xctx = xmlXPathNewContext(doc))) {
//        KNH_THROW__T(kctx, "XPATH: cant create xmlXPathContext");
//    }
//    knh_Glue_init(kctx,sfp[0].glue,xctx,knh_xpath_gfree);
//    KReturn(sfp[0].o);
//}
//
//static KMETHOD XPath_find(KonohaContext *kctx, KonohaStack *sfp)
//{
//    xmlXPathContextPtr xctx = (xmlXPathContextPtr) p_cptr(sfp[0]);
//    xmlChar *xpath = String_to(xmlChar *, sfp[1]);
//    xmlNodePtr node = NULL;
//    xmlXPathObjectPtr xpobj;
//    if(!(xpobj = xmlXPathEvalExpression( xpath, xctx))) {
//        fprintf(stderr,"xpath:%s",(char *)xpath);
//        KNH_THROW__T(kctx, "XPATH: cant execute xmlXPathEvalExpression");
//    }
//    if(!xmlXPathNodeSetIsEmpty(xpobj->nodesetval)) {
//        node = xmlXPathNodeSetItem(xpobj->nodesetval, 0);
//    }
//    xmlXPathFreeObject(xpobj);
//    KReturn(new_Glue(kctx,(char *)"libxml2.XmlNode",node,NULL));
//}

/* ------------------------------------------------------------------------ */
/* [XmlReader] */

struct kXmlReaderVar {
	kObjectHeader h;
	xmlTextReaderPtr reader;
};

typedef struct kXmlReaderVar kXmlReader;

static void XmlReader_init(KonohaContext *kctx, kObject *o, void *conf)
{
	//kXmlReaderVar* xml = (kXmlReaderVar *)o;
	struct kXmlReaderVar* xml = (struct kXmlReaderVar *)o;
	xml->reader = NULL;
}

static void XmlReader_free(KonohaContext *kctx, kObject *o)
{
	struct kXmlReaderVar* xml = (struct kXmlReaderVar *)o;
	if(xml->reader != NULL) {
		xmlFreeTextReader((xmlTextReaderPtr)xml->reader);
		xml->reader = NULL;
	}
}

static void XmlReader_reftrace(KonohaContext *kctx, kObject *p, KObjectVisitor *visitor)
{
}

#define getRawXmlReader(obj) ((struct kXmlReaderVar *)(obj.asObject))->reader

//## @Native XmlReader XmlReader.new(String path);
static KMETHOD XmlReader_new(KonohaContext *kctx, KonohaStack *sfp)
{
	const char *path = kString_text(sfp[1].asString);
	xmlTextReaderPtr r = xmlNewTextReaderFilename(path);
	struct kXmlReaderVar *xml = (struct kXmlReaderVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	if(r == NULL) {
		//kreportf(ErrTag, sfp[K_RTNIDX].uline, "could not create XmlReader Object from %s", path);
	}
	xml->reader = (xmlTextReaderPtr)r;
	KReturn(xml);
}

//## @Native XmlReader String.convertToXml();
static KMETHOD String_convertToXml(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlChar* input = (xmlChar *)kString_text(sfp[0].asString);
	xmlTextReaderPtr r = xmlReaderForDoc(input, NULL, NULL, 1);
	//xmlTextReaderPtr r = xmlReaderForDoc(input, NULL, "UTF-8", 1);
	struct kXmlReaderVar *xml = (struct kXmlReaderVar *)KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), 0);
	if(r == NULL) {
		//kreportf(ErrTag, sfp[K_RTNIDX].uline, "could not create XmlReader Object from String");
	}
	xml->reader = (xmlTextReaderPtr)r;
	KReturn(xml);
}

//## @Native void XmlReader.close();
static KMETHOD XmlReader_close(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	if(reader != NULL) {
		xmlTextReaderClose(reader);
	}
	KReturnVoid();
}

//## @Native String XmlReader.getQuoteChar();
static KMETHOD XmlReader_getQuoteChar(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char buf[4] = {0};
	const char* ret = NULL;
	if(reader != NULL) {
		int ch = xmlTextReaderQuoteChar(reader);
		buf[0] = ch;
		ret = (const char *)buf;
	}
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native boolean XmlReader.read();
static KMETHOD XmlReader_read(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = 0;
	if(reader != NULL) {
		ret = xmlTextReaderRead(reader);
	}
	KReturnUnboxValue(ret);
}

//## @Native int XmlReader.readState();
static KMETHOD XmlReader_readState(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderReadState(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native int XmlReader.nodeType();
static KMETHOD XmlReader_nodeType(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderNodeType(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.isNamespaceDecl();
static KMETHOD XmlReader_isNamespaceDecl(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderIsNamespaceDecl(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.isEmptyElement();
static KMETHOD XmlReader_isEmptyElement(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderIsEmptyElement(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.hasAttributes();
static KMETHOD XmlReader_hasAttributes(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderHasAttributes(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.hasValue();
static KMETHOD XmlReader_hasValue(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderHasValue(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native int XmlReader.getDepth();
static KMETHOD XmlReader_getDepth(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderDepth(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native int XmlReader.getAttributeCount();
static KMETHOD XmlReader_getAttributeCount(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderAttributeCount(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.moveToFirstAttribute();
static KMETHOD XmlReader_moveToFirstAttribute(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderMoveToFirstAttribute(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.moveToNextAttribute();
static KMETHOD XmlReader_moveToNextAttribute(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderMoveToNextAttribute(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native boolean XmlReader.moveToElement();
static KMETHOD XmlReader_moveToElement(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderMoveToElement(reader) : 0;
	KReturnUnboxValue(ret);
}

//## @Native String XmlReader.constBaseUri();
static KMETHOD XmlReader_constBaseUri(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstBaseUri(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constEncoding();
static KMETHOD XmlReader_constEncoding(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *)xmlTextReaderConstEncoding(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constValue();
static KMETHOD XmlReader_constValue(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstValue(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constNamespaceUri();
static KMETHOD XmlReader_constNamespaceUri(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstNamespaceUri(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constLocalName();
static KMETHOD XmlReader_constLocalName(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstLocalName(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constName();
static KMETHOD XmlReader_constName(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstName(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constXmlLang();
static KMETHOD XmlReader_constXmlLang(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstXmlLang(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.constPrefix();
static KMETHOD XmlReader_constPrefix(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderConstPrefix(reader) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}


//## @Native String XmlReader.getAttribute();
static KMETHOD XmlReader_getAttribute(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	xmlChar* ns = (xmlChar *)kString_text(sfp[1].asString);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderGetAttribute(reader,ns) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.getAttributeNo(int number);
static KMETHOD XmlReader_getAttributeNo(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int num = (int)(sfp[1].intValue);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderGetAttributeNo(reader, num) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.getAttributeNs(String ns, String name);
static KMETHOD XmlReader_getAttributeNs(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	xmlChar* ns = (xmlChar *)kString_text(sfp[1].asString);
	xmlChar* name = (xmlChar *)kString_text(sfp[2].asString);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderGetAttributeNs(reader,ns,name) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native String XmlReader.lookupNameSpace(String ns);
static KMETHOD XmlReader_lookupNameSpace(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	xmlChar* ns = (xmlChar *)kString_text(sfp[1].asString);
	char* ret = (reader != NULL) ? (char *) xmlTextReaderLookupNamespace(reader,ns) : NULL;
	KReturn(KLIB new_kString(kctx, GcUnsafe, ret, strlen(ret), 0));
}

//## @Native int XmlReader.normalization();
static KMETHOD XmlReader_normalization(KonohaContext *kctx, KonohaStack *sfp)
{
	xmlTextReaderPtr reader = getRawXmlReader(sfp[0]);
	int ret = (reader != NULL) ? xmlTextReaderNormalization(reader) : 0;
	KReturnUnboxValue(ret);
}

static kbool_t xml_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	KRequireKonohaCommonModule(trace);
	KDEFINE_CLASS defXml = {
		STRUCTNAME(XmlReader),
		.cflag = KClassFlag_Final,
		.init = XmlReader_init,
		.free = XmlReader_free,
		.reftrace = XmlReader_reftrace,
	};
	KClass *cXmlReader = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &defXml, trace);
#define KType_XmlReader     cXmlReader->typeId


	KDEFINE_METHOD MethodData[] = {
		_Public, _F(XmlReader_new),                  KType_XmlReader,     KType_XmlReader,    KMethodName_("new"),                  1, KType_String, KFieldName_("path"),
		_Public, _F(XmlReader_close),                KType_void,    KType_XmlReader,    KMethodName_("close"),                0,
		_Public, _F(XmlReader_getQuoteChar),         KType_String,  KType_XmlReader,    KMethodName_("getQuoteChar"),         0,
		_Public, _F(XmlReader_read),                 KType_Boolean, KType_XmlReader,    KMethodName_("read"),                 0,
		_Public, _F(XmlReader_readState),            KType_Int,     KType_XmlReader,    KMethodName_("readState"),            0,
		_Public, _F(XmlReader_nodeType),             KType_Int,     KType_XmlReader,    KMethodName_("nodeType"),             0,
		_Public, _F(XmlReader_isNamespaceDecl),      KType_Boolean, KType_XmlReader,    KMethodName_("isNamespaceDecl"),      0,
		_Public, _F(XmlReader_isEmptyElement),       KType_Boolean, KType_XmlReader,    KMethodName_("isEmptyElement"),       0,
		_Public, _F(XmlReader_hasAttributes),        KType_Boolean, KType_XmlReader,    KMethodName_("hasAttributes"),        0,
		_Public, _F(XmlReader_hasValue),             KType_Boolean, KType_XmlReader,    KMethodName_("hasValue"),             0,
		_Public, _F(XmlReader_getDepth),             KType_Int,     KType_XmlReader,    KMethodName_("getDepth"),             0,
		_Public, _F(XmlReader_getAttributeCount),    KType_Int,     KType_XmlReader,    KMethodName_("getAttributeCount"),    0,
		_Public, _F(XmlReader_moveToFirstAttribute), KType_Boolean, KType_XmlReader,    KMethodName_("moveToFirstAttribute"), 0,
		_Public, _F(XmlReader_moveToNextAttribute),  KType_Boolean, KType_XmlReader,    KMethodName_("moveToNextAttribute"),  0,
		_Public, _F(XmlReader_moveToElement),        KType_Boolean, KType_XmlReader,    KMethodName_("moveToElement"),        0,
		_Public, _F(XmlReader_constBaseUri),         KType_String,  KType_XmlReader,    KMethodName_("constBaseUri"),         0,
		_Public, _F(XmlReader_constEncoding),        KType_String,  KType_XmlReader,    KMethodName_("constEncoding"),        0,
		_Public, _F(XmlReader_constValue),           KType_String,  KType_XmlReader,    KMethodName_("constValue"),           0,
		_Public, _F(XmlReader_constNamespaceUri),    KType_String,  KType_XmlReader,    KMethodName_("constNamespaceUri"),    0,
		_Public, _F(XmlReader_constLocalName),       KType_String,  KType_XmlReader,    KMethodName_("constLocalName"),       0,
		_Public, _F(XmlReader_constName),            KType_String,  KType_XmlReader,    KMethodName_("constName"),            0,
		_Public, _F(XmlReader_constXmlLang),         KType_String,  KType_XmlReader,    KMethodName_("constXmlLang"),         0,
		_Public, _F(XmlReader_constPrefix),          KType_String,  KType_XmlReader,    KMethodName_("constPrefix"),          0,
		_Public, _F(XmlReader_getAttribute),         KType_String,  KType_XmlReader,    KMethodName_("getAttribute"),         0,
		_Public, _F(XmlReader_getAttributeNo),       KType_String,  KType_XmlReader,    KMethodName_("getAttributeNo"),       1, KType_Int, KFieldName_("no"),
		_Public, _F(XmlReader_getAttributeNs),       KType_String,  KType_XmlReader,    KMethodName_("getAttributeNs"),       2, KType_String, KFieldName_("ns"), KType_String, KFieldName_("name"),
		_Public, _F(XmlReader_lookupNameSpace),      KType_String,  KType_XmlReader,    KMethodName_("lookupNameSpace"),      1, KType_String, KFieldName_("ns"),
		_Public, _F(XmlReader_normalization),        KType_Int,     KType_XmlReader,    KMethodName_("normalization"),        0,

		_Public, _F(String_convertToXml),            KType_XmlReader,     KType_String, KMethodName_("convertToXml"),         0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	KDEFINE_INT_CONST IntData[] = {
		{KDefineConstInt(XML_READER_TYPE_NONE)},
		{KDefineConstInt(XML_READER_TYPE_ELEMENT)},
		{KDefineConstInt(XML_READER_TYPE_ATTRIBUTE)},
		{KDefineConstInt(XML_READER_TYPE_TEXT)},
		{KDefineConstInt(XML_READER_TYPE_CDATA)},
		{KDefineConstInt(XML_READER_TYPE_ENTITY_REFERENCE)},
		{KDefineConstInt(XML_READER_TYPE_ENTITY)},
		{KDefineConstInt(XML_READER_TYPE_PROCESSING_INSTRUCTION)},
		{KDefineConstInt(XML_READER_TYPE_COMMENT)},
		{KDefineConstInt(XML_READER_TYPE_DOCUMENT)},
		{KDefineConstInt(XML_READER_TYPE_DOCUMENT_TYPE)},
		{KDefineConstInt(XML_READER_TYPE_DOCUMENT_FRAGMENT)},
		{KDefineConstInt(XML_READER_TYPE_NOTATION)},
		{KDefineConstInt(XML_READER_TYPE_WHITESPACE)},
		{KDefineConstInt(XML_READER_TYPE_SIGNIFICANT_WHITESPACE)},
		{KDefineConstInt(XML_READER_TYPE_END_ELEMENT)},
		{KDefineConstInt(XML_READER_TYPE_END_ENTITY)},
		{KDefineConstInt(XML_READER_TYPE_XML_DECLARATION)},
		{} // end of const data
	};
	KLIB kNameSpace_LoadConstData(kctx, ns, KConst_(IntData), trace);
	return true;
}

static kbool_t xml_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Xmlreader_Init(void)
{
	static KDEFINE_PACKAGE d = {0};
	KSetPackageName(d, "konoha", K_VERSION);
	d.PackupNameSpace    = xml_PackupNameSpace;
	d.ExportNameSpace   = xml_ExportNameSpace;
	return &d;
}

#ifdef __cplusplus
}
#endif

