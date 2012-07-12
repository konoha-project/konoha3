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

#include<minikonoha/minikonoha.h>
#include<minikonoha/sugar.h>
#include<minikonoha/float.h>
//#include<../konoha.bytes/bytes_glue.h>
//#include<../konoha.array/array_glue.h>
#include<Python.h>


#define OB_TYPE(obj) (((PyObject*)obj->self)->ob_type)

typedef const struct _kPyObject kPyObject;
struct _kPyObject {
	KonohaObjectHeader h;
	PyObject *self;  // don't set NULL
};

static void PyObject_init(KonohaContext *kctx, kObject *o, void *conf)
{
	struct _kPyObject *pyo = (struct _kPyObject*)o;
	if(conf == NULL) {
		pyo->self = Py_None;
		Py_INCREF(Py_None);
	}
	else {
		pyo->self = (PyObject*)conf;
	}
}

static void PyObject_p(KonohaContext *kctx, KonohaStack *sfp, int pos, kwb_t *wb, int level)
{
	// Now, level value has no effect.
	PyObject *pyo =  ((kPyObject*)sfp[pos].o)->self;
	PyObject* str = pyo->ob_type->tp_str(pyo);
	Py_INCREF(str);
	kwb_printf(wb, "%s", PyString_AsString(str));
	Py_DECREF(str);
}

static void PyObject_free(KonohaContext *kctx, kObject *o)
{
	// Py_none is not deleted in python.
	// so, it is not free safe
	// it is not better using NULL
	// make struct null stab (or Py_None?).
	struct _kPyObject *pyo = (struct _kPyObject*)o;
	//OB_TYPE(pyo)->tp_free(pyo->self);
	Py_DECREF(pyo->self);
	Py_INCREF(Py_None);
	pyo->self = Py_None;
}

#define RETURN_PyObject(O)  RETURN_PyObject_(kctx, sfp, O K_RIXPARAM)

static void RETURN_PyObject_(KonohaContext *kctx, KonohaStack *sfp, PyObject *pyo _RIX)
{
	if(pyo != NULL) {
    	RETURN_(new_kObject(O_ct(sfp[K_RTNIDX].o), pyo));
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

static KMETHOD Int_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	RETURN_PyObject(PyInt_FromLong(sfp[0].ivalue));
}

static KMETHOD PyObject_toInt(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kPyObject *po = (kPyObject*)sfp[0].o;
	long v = PyInt_AsLong(po->self);
	if(PyErr_Occurred()) {
		v = 0;
	}
	RETURNi_(v);
}

static KMETHOD Boolean_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	RETURN_PyObject(PyBool_FromLong(sfp[0].ivalue));
}

static KMETHOD PyObject_toBoolean(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kPyObject *po = (kPyObject*)sfp[0].o;
	RETURNb_(po->self == Py_True ? 1 : 0);
}

static KMETHOD Float_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	RETURN_PyObject(PyFloat_FromDouble(sfp[0].fvalue));
}

static KMETHOD PyObject_toFloat(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kPyObject *po = (kPyObject*)sfp[0].o;
	double v = PyFloat_AsDouble(po->self);
	if(PyErr_Occurred()) {
		v = 0;
	}
	RETURNf_(v);
}

// [TODO] warning caused ... because some bytes_gule.h function (ex. kdlclose) is not use.
//static KMETHOD Bytes_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	RETURN_PyObject(PyString_FromString(sfp[0].s));
//}
//
//static KMETHOD PyObject_toBytes(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	kPyObject *po = (kPyObject*)sfp[0].o;
//	kwb_t wb;
//	if(po->self == NULL) {
//		// [TODO] throw Exception
//	}
//	kwb_init(&(kctx->stack->cwb), &wb);
//	O_ct(sfp[0].o)->p(kctx, sfp, 0, &wb, 0);
//	struct _kBytes* ba = (struct _kBytes*)new_Bytes(kctx, kwb_bytesize(&wb));
//	ba->buf = kwb_top(&wb, 1);
//	kwb_free(&wb);
//	RETURN_(ba);
//}

//static KMETHOD Complex_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	//RETURN_PyObject(PyBool_FromLong(sfp[0].ivalue));
//}
//
//static KMETHOD PyObject_toComplex(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	//kPyObject *po = (kPyObject*)sfp[0].o;
//	//RETURNb_(po->self == Py_True ? 1 : 0);
//}

static KMETHOD String_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	RETURN_PyObject(PyUnicode_FromString(S_text(sfp[0].s)));
}

static KMETHOD PyObject_toString(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	kPyObject *po = (kPyObject*)sfp[0].o;
	kwb_t wb;
	// assert
	DBG_ASSERT(po->self != NULL);
	kwb_init(&(kctx->stack->cwb), &wb);
	O_ct(sfp[0].o)->p(kctx, sfp, 0, &wb, 0);
	kString* s = new_kString(kwb_top(&wb, 1), kwb_bytesize(&wb), 0);
	kwb_free(&wb);
	RETURN_(s);
	//if (PyString_Check(po->self)) {
	//	//dec
	//	t = PyString_AsString(po->self);
	//	RETURN_(new_kString(t, strlen(t), 0));
	//}
	//else if (PyUnicode_Check(po->self)) {
	//	//dec
	//	PyObject* s = PyUnicode_AsUTF8String(po->self);
	//	// [TODO] there is no t's NULL check. Is it OK?
	//	t = PyString_AsString(s);
	//	RETURN_(new_kString(t, strlen(t), 0));
	//}
	//else if (PyByteArray_Check(po->self)) {
	//	//dec
	//	t = PyByteArray_AsString(po->self);
	//	RETURN_(new_kString(t, strlen(t), 0));
	//}
	//else {
	//	kwb_t wb;
	//	kwb_init(&(kctx->stack->cwb), &wb);
	//	O_ct(sfp[0].o)->p(kctx, sfp, 0, &wb, 0);
	//	kString* s = new_kString(kwb_top(&wb, 1), kwb_bytesize(&wb), 0);
	//	kwb_free(&wb);
	//	RETURN_(s);
	//}
}

//static KMETHOD Buffer_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toBuffer(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Tuple_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toTuple(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}

#define _BITS 8
#define PY_SSIZE_MAX (size_t)(1 << 31)

//static KMETHOD Array_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	kArray *a = sfp[0].a;
//	size_t i, n = kArray_size(a);
//	Py_ssize_t pa_size = (n < PY_SSIZE_MAX)? n : PY_SSIZE_MAX - 1;
//	PyObject* pa = PyList_New((Py_ssize_t)n);
//	if (kArray_isUnboxData(a)) {
//		for (i = 0; i < pa_size; i++) {
//			// [TODO] transfer array element to PyObject
//			PyList_SetItem(pa, i, PyInt_FromLong(a->ndata[n]));
//		}
//	}
//	else {
//		for (i = 0; i < pa_size; i++) {
//			// [TODO] transfer array element to PyObject
//			//PyList_Append(pa, i, a->list[n]);
//		}
//	}
//	RETURN_PyObject(pa);
//}

//static KMETHOD PyObject_toList(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//	//kPyObject *po = (kPyObject*)sfp[0].o;
//	//RETURNb_(po->self == Py_True ? 1 : 0);
//}

//static KMETHOD Dict_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toDict(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Class_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toClass(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Function_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toFunction(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Method_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toMethod(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD File_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toFile(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Module_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toModule(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD SeqIter_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toSeqIter(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Slice_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toSlice(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Weakref_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toWeakref(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Capsule_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toCapsule(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Cell_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toCell(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Gen_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toGen(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Date_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toDate(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Set_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toSet(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD Code_toPyObject(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}
//
//static KMETHOD PyObject_toCode(KonohaContext *kctx, KonohaStack *sfp _RIX)
//{
//}

// --------------------------------------------------------------------------

//## Boolean Python.eval(String script);
static KMETHOD Python_eval(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	RETURNb_(PyRun_SimpleString(S_text(sfp[1].s)) == 0);
}

#define DEFAULT_SIZE 16

char** pyenv_split(char* line, char target)
{
	char* c = line;
	size_t slen, size = 0, maxsize = DEFAULT_SIZE;
	char **tmp, **ret = (char**)malloc(sizeof(char**) * DEFAULT_SIZE);
	memset(ret, '\0', sizeof(char**) * DEFAULT_SIZE);
	while (line[0] != '\0'){
		if (line[0] == target){
			if (size >= maxsize) {
				maxsize *= 2;
				tmp = (char**)realloc(ret, maxsize);
				assert(tmp != NULL);
				memset(tmp, '\0', maxsize);
				memcpy(ret, tmp, maxsize);
			}
			slen = line - c + 1;
			char* p = (char*)malloc(slen);
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
	char* p = (char*)malloc(slen);
	memset(p, '\0', slen);
	strncpy(p, c, slen);
	ret[size] = p;
	return ret;
}

//## PyObject PyObject.importPtModule(String name);
//[TODO] devide each function PYTHONPATH and default search path.
static KMETHOD PyObject_import(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	PySys_SetPath("."); // add home dir to python search path.
	PyListObject* ppath;
	ppath = (PyListObject*)PyList_New(0);
	PyList_Append((PyObject*)ppath, PyString_FromString("."));
	char *path = getenv("PYTHONPATH"); // add home dir to python search path.
	if (path != NULL) {
		size_t i;
		char** pathes = pyenv_split(path, ':');
		for (i = 0; pathes[i] != NULL; i++) {
			PyList_Append((PyObject*)ppath, PyString_FromString(pathes[i]));
			free(pathes[i]);
		}
		free(pathes);
	}
	PySys_SetObject("path", (PyObject*)ppath);
	RETURN_PyObject(PyImport_ImportModule(S_text(sfp[1].s)));
}

//## PyObject PyObject.(PyObject o);
static KMETHOD PyObject_(KonohaContext *kctx, KonohaStack *sfp _RIX)
{
	// consider about module function and class method.
	// Now, PyObject_() support only module function.
	// [TODO] Support class method.
	//
	int argc = kctx->esp - sfp - 2;   // believe me
	kPyObject *pmod = (kPyObject*)sfp[0].o;
	PyObject  *pFunc = PyObject_GetAttrString(pmod->self, S_text(kctx->esp[-1].s));
	PyObject  *pArgs = NULL, *pValue = NULL;
	if(pFunc != NULL) {
		if(PyCallable_Check(pFunc)) {
			int i;
			pArgs = PyTuple_New(argc);
			for (i = 0; i < argc; ++i) {
				pValue = ((kPyObject*)sfp[i+1].o)->self;
				Py_INCREF(pValue);
				PyTuple_SetItem(pArgs, i, pValue);
			}
			pValue = PyObject_CallObject(pFunc, pArgs);
		}
	}
	Py_XDECREF(pFunc);
	Py_XDECREF(pArgs);
	RETURN_PyObject(pValue);
}

// --------------------------------------------------------------------------

static int python_init_count = 0;

#define _Public   kMethod_Public
#define _Const    kMethod_Const
#define _Coercion kMethod_Coercion
#define _Im       kMethod_Immutable
#define _F(F)   (intptr_t)(F)

static	kbool_t python_initPackage(KonohaContext *kctx, kNameSpace *ks, int argc, const char**args, kfileline_t pline)
{
	python_init_count++;
	if(python_init_count == 1) {
		Py_Initialize();
	}

	static KDEFINE_CLASS PythonDef = {
			STRUCTNAME(PyObject),
			.cflag = 0,
			.init = PyObject_init,
			.free = PyObject_free,
			.p    = PyObject_p,
	};

	KonohaClass *cPython = Konoha_addClassDef(ks->packageId, ks->packdom, NULL, &PythonDef, pline);
	int TY_PyObject = cPython->cid;
	KDEFINE_METHOD MethodData[] = {
		_Public|_Const|_Im|_Coercion, _F(PyObject_toBoolean), TY_Boolean, TY_PyObject, MN_to(TY_Boolean), 0,
		_Public|_Const|_Im|_Coercion, _F(Boolean_toPyObject), TY_PyObject, TY_Boolean, MN_to(TY_PyObject), 0,
		_Public|_Const|_Im|_Coercion, _F(PyObject_toInt), TY_Int, TY_PyObject, MN_to(TY_Int), 0,
		_Public|_Const|_Im|_Coercion, _F(Int_toPyObject), TY_PyObject, TY_Int, MN_to(TY_PyObject), 0,
		_Public|_Const|_Im|_Coercion, _F(PyObject_toString), TY_String, TY_PyObject, MN_to(TY_String), 0,
		_Public|_Const|_Im|_Coercion, _F(String_toPyObject), TY_PyObject, TY_String, MN_to(TY_PyObject), 0,
		_Public|_Const|_Im|_Coercion, _F(PyObject_toString), TY_String, TY_PyObject, MN_("toString"),  0,
		//_Public,                      _F(Array_add), TY_void, TY_Array, MN_("add"), 1, TY_T0, FN_("value"),
		// [TODO] add following konoha class.
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toList), TY_Array, TY_PyObject, MN_to(TY_Array), 0,
		//_Public|_Const|_Im|_Coercion, _F(Array_toPyObject), TY_PyObject, TY_Array, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toComplex), TY_Complex, TY_PyObject, MN_to(TY_Complex), 0,
		//_Public|_Const|_Im|_Coercion, _F(Complex_toPyObject), TY_PyObject, TY_Complex, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toBuffer), TY_Buffer, TY_PyObject, MN_to(TY_Buffer), 0,
		//_Public|_Const|_Im|_Coercion, _F(Buffer_toPyObject), TY_PyObject, TY_Buffer, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toTuple), TY_Tuple, TY_PyObject, MN_to(TY_Tuple), 0,
		//_Public|_Const|_Im|_Coercion, _F(Tuple_toPyObject), TY_PyObject, TY_Tuple, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toDict), TY_Dict, TY_PyObject, MN_to(TY_Dict), 0,
		//_Public|_Const|_Im|_Coercion, _F(Dict_toPyObject), TY_PyObject, TY_Dict, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toClass), TY_Class, TY_PyObject, MN_to(TY_Class), 0,
		//_Public|_Const|_Im|_Coercion, _F(Class_toPyObject), TY_PyObject, TY_Class, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toFunction), TY_Function, TY_PyObject, MN_to(TY_Function), 0,
		//_Public|_Const|_Im|_Coercion, _F(Function_toPyObject), TY_PyObject, TY_Function, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toMethod), TY_Method, TY_PyObject, MN_to(TY_Method), 0,
		//_Public|_Const|_Im|_Coercion, _F(Method_toPyObject), TY_PyObject, TY_Method, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toFile), TY_File, TY_PyObject, MN_to(TY_File), 0,
		//_Public|_Const|_Im|_Coercion, _F(File_toPyObject), TY_PyObject, TY_File, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toModule), TY_Module, TY_PyObject, MN_to(TY_Module), 0,
		//_Public|_Const|_Im|_Coercion, _F(Module_toPyObject), TY_PyObject, TY_Module, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSeqIter), TY_SeqIter, TY_PyObject, MN_to(TY_SeqIter), 0,
		//_Public|_Const|_Im|_Coercion, _F(SeqIter_toPyObject), TY_PyObject, TY_SeqIter, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSlice), TY_Slice, TY_PyObject, MN_to(TY_Slice), 0,
		//_Public|_Const|_Im|_Coercion, _F(Slice_toPyObject), TY_PyObject, TY_Slice, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toWeakref), TY_Weakref, TY_PyObject, MN_to(TY_Weakref), 0,
		//_Public|_Const|_Im|_Coercion, _F(Weakref_toPyObject), TY_PyObject, TY_Weakref, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCapsule), TY_Capsule, TY_PyObject, MN_to(TY_Capsule), 0,
		//_Public|_Const|_Im|_Coercion, _F(Capsule_toPyObject), TY_PyObject, TY_Capsule, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCell), TY_Cell, TY_PyObject, MN_to(TY_Cell), 0,
		//_Public|_Const|_Im|_Coercion, _F(Cell_toPyObject), TY_PyObject, TY_Cell, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toGen), TY_Gen, TY_PyObject, MN_to(TY_Gen), 0,
		//_Public|_Const|_Im|_Coercion, _F(Gen_toPyObject), TY_PyObject, TY_Gen, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(Date_toPyObject), TY_PyObject, TY_Date, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toDate), TY_Date, TY_PyObject, MN_to(TY_Date), 0,
		//_Public|_Const|_Im|_Coercion, _F(Set_toPyObject), TY_PyObject, TY_Set, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toSet), TY_Set, TY_PyObject, MN_to(TY_Set), 0,
		//_Public|_Const|_Im|_Coercion, _F(Code_toPyObject), TY_PyObject, TY_Code, MN_to(TY_PyObject), 0,
		//_Public|_Const|_Im|_Coercion, _F(PyObject_toCode), TY_Code, TY_PyObject, MN_to(TY_Code), 0,
		_Public|_Im, _F(Python_eval), TY_Boolean, TY_System, FN_("pyEval"), 1, TY_String, FN_("script"),
		_Public|_Im, _F(PyObject_import), TY_PyObject, TY_PyObject, FN_("import"), 1, TY_String, FN_("name"),
		_Public|_Im, _F(PyObject_), TY_PyObject, TY_PyObject, 0, 1, TY_PyObject, 0,
		DEND,
	};
	kNameSpace_loadMethodData(ks, MethodData);
	if(IS_defineFloat()) {
		KDEFINE_METHOD MethodData[] = {
			_Public|_Const|_Im|_Coercion, _F(PyObject_toFloat), TY_Float, TY_PyObject, MN_to(TY_Float), 0,
			_Public|_Const|_Im|_Coercion, _F(Float_toPyObject), TY_PyObject, TY_Float, MN_to(TY_PyObject), 0,
			DEND,
		};
		kNameSpace_loadMethodData(ks, MethodData);
	}
	return true;
}

static kbool_t python_setupPackage(KonohaContext *kctx, kNameSpace *ks, kfileline_t pline)
{
	return true;
}

static kbool_t python_initNameSpace(KonohaContext *kctx,  kNameSpace *ks, kfileline_t pline)
{
	return true;
}

static kbool_t python_setupNameSpace(KonohaContext *kctx, kNameSpace *ks, kfileline_t pline)
{
	return true;
}
