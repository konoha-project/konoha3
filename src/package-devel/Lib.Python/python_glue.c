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

#include <Python.h>
#include <konoha3/konoha.h>
#include <konoha3/sugar.h>
#include <konoha3/konoha_common.h>
#include <konoha3/import/methoddecl.h>

#ifdef __cplusplus
extern "C"{
#endif

//#define OB_TYPE(obj) (((PyObject *)obj->self)->ob_type)

typedef const struct kPyObjectVar kPyObject;
struct kPyObjectVar {
	kObjectHeader h;
	PyObject *self;  // don't set NULL
};

static void kPyObject_Init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct kPyObjectVar *pyo = (struct kPyObjectVar *)o;
	if(conf == NULL) {
		pyo->self = Py_None;
		Py_INCREF(Py_None);
	}
	else {
		pyo->self = (PyObject *)conf;
	}
}

static void kPyObject_format(KonohaContext *kctx, KonohaValue *v, int pos, KBuffer *wb)
{
	PyObject *pyo =  ((kPyObject *)v[pos].asObject)->self;
	PyObject *str = pyo->ob_type->tp_str(pyo);
	Py_INCREF(str);
	KLIB KBuffer_printf(kctx, wb, "%s", PyString_AsString(str));
	Py_DECREF(str);
}

static void kPyObject_Free(KonohaContext *kctx, kObject *o)
{
	// Py_none is not deleted in python.
	// so, it is not free safe
	// it is not better using NULL
	// make struct null stab (or Py_None?).
	struct kPyObjectVar *pyo = (struct kPyObjectVar *)o;
	//OB_TYPE(pyo)->tp_Free(pyo->self);
	Py_DECREF(pyo->self);
	Py_INCREF(Py_None);
	pyo->self = Py_None;
}

#define KReturnPyObject(O)  KReturnPyObject_(kctx, sfp, O)

static void KReturnPyObject_(KonohaContext *kctx, KonohaStack *sfp, PyObject *pyo)
{
	if(pyo != NULL) {
		KReturn(KLIB new_kObject(kctx, OnStack, KGetReturnType(sfp), (uintptr_t)pyo));
	}
	else {
		// ERROR if python object is NULL
		// add ktrace.
		// looks stupid
		PyErr_Print();
	}
}

// --------------------------------------------------------------------------
/* [type transfer] */

// [TODO] add following konoha to python type transfer function.
// it is difficult to transfer UCS2 to konoha String.
// Do not forget test ...

static KMETHOD Int_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnPyObject(PyInt_FromLong(sfp[0].intValue));
}

static KMETHOD PyObject_toInt(KonohaContext *kctx, KonohaStack *sfp)
{
	kPyObject *po = (kPyObject *)sfp[0].asObject;
	long v = PyInt_AsLong(po->self);
	if(PyErr_Occurred()) {
		v = 0;
	}
	KReturnUnboxValue(v);
}

static KMETHOD Boolean_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnPyObject(PyBool_FromLong(sfp[0].intValue));
}

static KMETHOD PyObject_toBoolean(KonohaContext *kctx, KonohaStack *sfp)
{
	kPyObject *po = (kPyObject *)sfp[0].asObject;
	KReturnUnboxValue(po->self == Py_True ? 1 : 0);
}

static KMETHOD Float_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnPyObject(PyFloat_FromDouble(sfp[0].floatValue));
}

static KMETHOD PyObject_toFloat(KonohaContext *kctx, KonohaStack *sfp)
{
	kPyObject *po = (kPyObject *)sfp[0].asObject;
	double v = PyFloat_AsDouble(po->self);
	if(PyErr_Occurred()) {
		v = 0;
	}
	KReturnFloatValue(v);
}

// [TODO] warning caused ... because some bytes_gule.h function (ex. kdlclose) is not use.
//static KMETHOD Bytes_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	KReturnPyObject(PyString_FromString(sfp[0].asString));
//}
//
//static KMETHOD PyObject_toBytes(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kPyObject *po = (kPyObject *)sfp[0].asObject;
//	KBuffer wb;
//	if(po->self == NULL) {
//		// [TODO] throw Exception
//	}
//	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
//	kObject_class(sfp[0].asObject)->format(kctx, sfp, 0, &wb, 0);
//	struct kBytesVar *ba = (struct kBytesVar *)new_Bytes(kctx, KBuffer_bytesize(&wb));
//	ba->buf = KLIB KBuffer_text(kctx, &wb, 1);
//	KLIB KBuffer_Free(&wb);
//	KReturn(ba);
//}

//static KMETHOD Complex_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	//KReturnPyObject(PyBool_FromLong(sfp[0].intValue));
//}
//
//static KMETHOD PyObject_toComplex(KonohaContext *kctx, KonohaStack *sfp)
//{
//	//kPyObject *po = (kPyObject *)sfp[0].asObject;
//	//KReturnUnboxValue(po->self == Py_True ? 1 : 0);
//}

static KMETHOD String_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnPyObject(PyUnicode_FromString(kString_text(sfp[0].asString)));
}

static KMETHOD PyObject_toString(KonohaContext *kctx, KonohaStack *sfp)
{
	kPyObject *po = (kPyObject *)sfp[0].asObject;
	KBuffer wb;
	// assert
	DBG_ASSERT(po->self != NULL);
	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	kObject_class(sfp[0].asObject)->format(kctx, sfp, 0, &wb);
	kString *s = KLIB new_kString(kctx, OnStack, KLIB KBuffer_text(kctx, &wb, 1), KBuffer_bytesize(&wb), 0);
	KLIB KBuffer_Free(&wb);
	KReturn(s);
	//if(PyString_Check(po->self)) {
	//	//dec
	//	t = PyString_AsString(po->self);
	//	KReturn(KLIB new_kString(kctx, t, strlen(t), 0));
	//}
	//else if(PyUnicode_Check(po->self)) {
	//	//dec
	//	PyObject *s = PyUnicode_AsUTF8String(po->self);
	//	// [TODO] there is no t's NULL check. Is it OK?
	//	t = PyString_AsString(s);
	//	KReturn(KLIB new_kString(kctx, t, strlen(t), 0));
	//}
	//else if(PyByteArray_Check(po->self)) {
	//	//dec
	//	t = PyByteArray_AsString(po->self);
	//	KReturn(KLIB new_kString(kctx, t, strlen(t), 0));
	//}
	//else {
	//	KBuffer wb;
	//	KLIB KBuffer_Init(&(kctx->stack->cwb), &wb);
	//	kObject_class(sfp[0].asObject)->format(kctx, sfp, 0, &wb, 0);
	//	kString *s = KLIB new_kString(kctx, KLIB KBuffer_text(kctx, &wb, 1), KBuffer_bytesize(&wb), 0);
	//	KLIB KBuffer_Free(&wb);
	//	KReturn(s);
	//}
}

//static KMETHOD Buffer_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toBuffer(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Tuple_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toTuple(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

//#define _BITS 8
//#define PY_SSIZE_MAX (size_t)(1 << 31)

//static KMETHOD Array_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//	kArray *a = sfp[0].asArray;
//	size_t i, n = kArray_size(a);
//	Py_ssize_t pa_size = (n < PY_SSIZE_MAX)? n : PY_SSIZE_MAX - 1;
//	PyObject *pa = PyList_New((Py_ssize_t)n);
//	if(kArray_Is(UnboxData, a)) {
//		for (i = 0; i < pa_size; i++) {
//			// [TODO] transfer array element to PyObject
//			PyList_SetItem(pa, i, PyInt_FromLong(a->unboxItems[n]));
//		}
//	}
//	else {
//		for (i = 0; i < pa_size; i++) {
//			// [TODO] transfer array element to PyObject
//			//PyList_Append(pa, i, a->ObjectItems[n]);
//		}
//	}
//	KReturnPyObject(pa);
//}

//static KMETHOD PyObject_toList(KonohaContext *kctx, KonohaStack *sfp)
//{
//	//kPyObject *po = (kPyObject *)sfp[0].asObject;
//	//KReturnUnboxValue(po->self == Py_True ? 1 : 0);
//}

//static KMETHOD Dict_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toDict(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Class_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toClass(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Function_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_asFunction(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Method_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_asMethod(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD File_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toFile(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Module_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toModule(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD SeqIter_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toSeqIter(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Slice_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toSlice(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Weakref_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toWeakref(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Capsule_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toCapsule(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Cell_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toCell(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Gen_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toGen(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Date_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toDate(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Set_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toSet(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD Code_toPyObject(KonohaContext *kctx, KonohaStack *sfp)
//{
//}
//
//static KMETHOD PyObject_toCode(KonohaContext *kctx, KonohaStack *sfp)
//{
//}

// --------------------------------------------------------------------------

//## Boolean Python.eval(String script);
static KMETHOD Python_Eval(KonohaContext *kctx, KonohaStack *sfp)
{
	KReturnUnboxValue(PyRun_SimpleString(kString_text(sfp[1].asString)) == 0);
}

#define DEFAULT_SIZE 16

char **pyenv_split(char *line, char target)
{
	char *c = line;
	size_t slen, size = 0, maxsize = DEFAULT_SIZE;
	char **tmp, **ret = (char**)malloc(sizeof(char**) * DEFAULT_SIZE);
	memset(ret, '\0', sizeof(char**) * DEFAULT_SIZE);
	while(line[0] != '\0'){
		if(line[0] == target){
			if(size >= maxsize) {
				maxsize *= 2;
				tmp = (char**)realloc(ret, maxsize);
				assert(tmp != NULL);
				memset(tmp, '\0', maxsize);
				memcpy(ret, tmp, maxsize);
			}
			slen = line - c + 1;
			char *p = (char *)malloc(slen);
			memset(p, '\0', slen);
			strncpy(p, c, slen - 1);
			ret[size] = p;
			size++;
			c = ++line;
			continue;
		}
		line++;
	}
	slen = line - c;
	char *p = (char *)malloc(slen);
	memset(p, '\0', slen);
	strncpy(p, c, slen);
	ret[size] = p;
	return ret;
}

//## PyObject PyObject.importPtModule(String name);
//[TODO] devide each function PYTHONPATH and default search path.
static KMETHOD PyObject_import(KonohaContext *kctx, KonohaStack *sfp)
{
	PySys_SetPath("."); // add home dir to python search path.
	PyListObject *ppath;
	ppath = (PyListObject *)PyList_New(0);
	PyList_Append((PyObject *)ppath, PyString_FromString("."));
	// add home dir to python search path.
	const char *path = PLATAPI getenv_i("PYTHONPATH");
	if(path != NULL) {
		size_t i;
		char **pathes = pyenv_split((char *)path, ':');
		for (i = 0; pathes[i] != NULL; i++) {
			PyList_Append((PyObject *)ppath, PyString_FromString(pathes[i]));
			free(pathes[i]);
		}
		free(pathes);
	}
	PySys_SetObject("path", (PyObject *)ppath);
	KReturnPyObject(PyImport_ImportModule(kString_text(sfp[1].asString)));
}

//## PyObject PyObject.(PyObject o);
static KMETHOD PyObject_(KonohaContext *kctx, KonohaStack *sfp)
{
	// consider about module function and class method.
	// Now, PyObject_() support only module function.
	// [TODO] Support class method.
	//
	int argc = kctx->esp - sfp - 2;   // believe me
	kPyObject *pmod = (kPyObject *)sfp[0].asObject;
	PyObject  *pFunc = PyObject_GetAttrString(pmod->self, kString_text(kctx->esp[-1].asString));
	PyObject  *pArgs = NULL, *pValue = NULL;
	if(pFunc != NULL) {
		if(PyCallable_Check(pFunc)) {
			int i;
			pArgs = PyTuple_New(argc);
			for (i = 0; i < argc; ++i) {
				pValue = ((kPyObject *)sfp[i+1].asObject)->self;
				Py_INCREF(pValue);
				PyTuple_SetItem(pArgs, i, pValue);
			}
			pValue = PyObject_CallObject(pFunc, pArgs);
		}
	}
	Py_XDECREF(pFunc);
	Py_XDECREF(pArgs);
	KReturnPyObject(pValue);
}

// --------------------------------------------------------------------------

static int python_Init_count = 0;

static kbool_t python_PackupNameSpace(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)
{
	python_Init_count++;
	if(python_Init_count == 1) {
		Py_Initialize();
	}

	static KDEFINE_CLASS PythonDef = {
			STRUCTNAME(PyObject),
			.cflag = 0,
			.init = kPyObject_Init,
			.free = kPyObject_Free,
			.format    = kPyObject_format,
	};

	KClass *cPython = KLIB kNameSpace_DefineClass(kctx, ns, NULL, &PythonDef, trace);
	int KType_PyObject = cPython->typeId;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im|_Coercion, _F(PyObject_toBoolean), KType_Boolean, KType_PyObject, KMethodName_To(KType_Boolean), 0,
		_Public|_Const|_Im|_Coercion, _F(Boolean_toPyObject), KType_PyObject, KType_Boolean, KMethodName_To(KType_PyObject), 0,
		_Public|_Const|_Im|_Coercion, _F(PyObject_toInt), KType_Int, KType_PyObject, KMethodName_To(KType_Int), 0,
		_Public|_Const|_Im|_Coercion, _F(Int_toPyObject), KType_PyObject, KType_Int, KMethodName_To(KType_PyObject), 0,
		_Public|_Const|_Im|_Coercion, _F(PyObject_toString), KType_String, KType_PyObject, KMethodName_To(KType_String), 0,
		_Public|_Const|_Im|_Coercion, _F(String_toPyObject), KType_PyObject, KType_String, KMethodName_To(KType_PyObject), 0,
		//_Public,                      _F(Array_Add), KType_void, KType_Array, KMethodName_("add"), 1, KType_0, KFieldName_("value"),
		// [TODO] add following konoha class.
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toList), KType_Array, KType_PyObject, KMethodName_To(KType_Array), 0,
		//_Public|_Const|_Im|_Coercion, _F(Array_toPyObject), KType_PyObject, KType_Array, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toComplex), KType_Complex, KType_PyObject, KMethodName_To(KType_Complex), 0,
		//_Public|_Const|_Im|_Coercion, _F(Complex_toPyObject), KType_PyObject, KType_Complex, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toBuffer), KType_Buffer, KType_PyObject, KMethodName_To(KType_Buffer), 0,
		//_Public|_Const|_Im|_Coercion, _F(Buffer_toPyObject), KType_PyObject, KType_Buffer, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toTuple), KType_Tuple, KType_PyObject, KMethodName_To(KType_Tuple), 0,
		//_Public|_Const|_Im|_Coercion, _F(Tuple_toPyObject), KType_PyObject, KType_Tuple, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toDict), KType_Dict, KType_PyObject, KMethodName_To(KType_Dict), 0,
		//_Public|_Const|_Im|_Coercion, _F(Dict_toPyObject), KType_PyObject, KType_Dict, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toClass), KType_Class, KType_PyObject, KMethodName_To(KType_Class), 0,
		//_Public|_Const|_Im|_Coercion, _F(Class_toPyObject), KType_PyObject, KType_Class, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_asFunction), KType_Function, KType_PyObject, KMethodName_To(KType_Function), 0,
		//_Public|_Const|_Im|_Coercion, _F(Function_toPyObject), KType_PyObject, KType_Function, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_asMethod), KType_Method, KType_PyObject, KMethodName_To(KType_Method), 0,
		//_Public|_Const|_Im|_Coercion, _F(Method_toPyObject), KType_PyObject, KType_Method, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toFile), KType_File, KType_PyObject, KMethodName_To(KType_File), 0,
		//_Public|_Const|_Im|_Coercion, _F(File_toPyObject), KType_PyObject, KType_File, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toModule), KType_Module, KType_PyObject, KMethodName_To(KType_Module), 0,
		//_Public|_Const|_Im|_Coercion, _F(Module_toPyObject), KType_PyObject, KType_Module, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSeqIter), KType_SeqIter, KType_PyObject, KMethodName_To(KType_SeqIter), 0,
		//_Public|_Const|_Im|_Coercion, _F(SeqIter_toPyObject), KType_PyObject, KType_SeqIter, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSlice), KType_Slice, KType_PyObject, KMethodName_To(KType_Slice), 0,
		//_Public|_Const|_Im|_Coercion, _F(Slice_toPyObject), KType_PyObject, KType_Slice, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toWeakref), KType_Weakref, KType_PyObject, KMethodName_To(KType_Weakref), 0,
		//_Public|_Const|_Im|_Coercion, _F(Weakref_toPyObject), KType_PyObject, KType_Weakref, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCapsule), KType_Capsule, KType_PyObject, KMethodName_To(KType_Capsule), 0,
		//_Public|_Const|_Im|_Coercion, _F(Capsule_toPyObject), KType_PyObject, KType_Capsule, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCell), KType_Cell, KType_PyObject, KMethodName_To(KType_Cell), 0,
		//_Public|_Const|_Im|_Coercion, _F(Cell_toPyObject), KType_PyObject, KType_Cell, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toGen), KType_Gen, KType_PyObject, KMethodName_To(KType_Gen), 0,
		//_Public|_Const|_Im|_Coercion, _F(Gen_toPyObject), KType_PyObject, KType_Gen, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(Date_toPyObject), KType_PyObject, KType_Date, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toDate), KType_Date, KType_PyObject, KMethodName_To(KType_Date), 0,
		//_Public|_Const|_Im|_Coercion, _F(Set_toPyObject), KType_PyObject, KType_Set, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSet), KType_Set, KType_PyObject, KMethodName_To(KType_Set), 0,
		//_Public|_Const|_Im|_Coercion, _F(Code_toPyObject), KType_PyObject, KType_Code, KMethodName_To(KType_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCode), KType_Code, KType_PyObject, KMethodName_To(KType_Code), 0,
		_Public|_Im, _F(Python_Eval), KType_Boolean, KType_System, KFieldName_("pyEval"), 1, KType_String, KFieldName_("script"),
		_Public|_Im, _F(PyObject_import), KType_PyObject, KType_PyObject, KFieldName_("import"), 1, KType_String, KFieldName_("name"),
		_Public|_Im, _F(PyObject_), KType_PyObject, KType_PyObject, 0, 1, KType_PyObject, 0,
		DEND,
	};
	KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	if(KDefinedKonohaCommonModule() == true && KClass_Float != NULL) {
		KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Im|_Coercion, _F(PyObject_toFloat), KType_float, KType_PyObject, KMethodName_To(KType_float), 0,
			_Public|_Const|_Im|_Coercion, _F(Float_toPyObject), KType_PyObject, KType_float, KMethodName_To(KType_PyObject), 0,
			DEND,
		};
		KLIB kNameSpace_LoadMethodData(kctx, ns, MethodData, trace);
	}
	return true;
}

static kbool_t python_ExportNameSpace(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)
{
	return true;
}

KDEFINE_PACKAGE *Python_Init(void)
{
	static KDEFINE_PACKAGE d = {
		KPACKNAME("python", "1.0"),
		.PackupNameSpace    = python_PackupNameSpace,
		.ExportNameSpace   = python_ExportNameSpace,
	};
	return &d;
}

#ifdef __cplusplus
}
#endif
