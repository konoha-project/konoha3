#!/usr/bin/python
import os, sys
import copy
from pygenlib2 import *

#------------------------------------------------------------------------------

INSTRUCTIONS = """
# OP       FLAG              ARGS
NOP        0
THCODE     0                  threadCode:f
ENTER      0
EXIT       0

NSET       0                  a:rn n:int ty:cid
NMOV       0                  a:rn b:rn  ty:cid
NMOVx      0                  a:rn b:ro  bx:u ty:cid
XNMOV      0                  a:ro ax:u  b:rn ty:cid

NEW        0                  a:ro p:u   ty:cid
NULL       0                  a:ro ty:cid
#BOX        0                  a:ro b:rn ty:cid
#UNBOX      0                  a:rn b:ro ty:cid

LOOKUP     0                  thisidx:ro ns:NameSpace mtd:Method
CALL       0                  uline:u thisidx:ro espshift:ro tyo:co
RET        0
NCALL      0

BNOT       0                  c:rn a:rn
JMP        0                  addr:addr
JMPF       0                  addr:addr a:rn
TRYJMP     0                  addr:addr
YIELD      0                  

ERROR      0                  uline:u msg:String esp:ro
SAFEPOINT  0                  uline:u esp:ro
CHKSTACK   0                  uline:u
TRACE      0                  uline:u thisidx:ro trace:f

#SCALL      0                 uline:u thisidx:ro espshift:ro mtd:Method  tyo:co
#VCALL      0                  uline:u thisidx:ro espshift:ro mtd:Method  tyo:co

#P          _CONST            print:f flag:u  msg:String n:sfpidx2
#PROBE      0                 sfpidx:sfpidx2 probe:f n:u n2:u
#TRY        0                 addr:addr hn:ro
#TRYEND     0                 hn:ro
#THROW      0                 start:sfpidx
#ASSERT     0                 start:sfpidx uline:u
#CATCH      _CONST            addr:addr en:ro eid:int
#CHKIN      0                 on:ro checkin:f
#CHKOUT     0                 on:ro checkout:f
#HALT      0
#VEXEC      0
#YIELD      0                 n:sfpidx

#@NNMOV     _DEF              a:rn b:rn c:rn d:rn
#@NSET2     _DEF|_JIT         a:rn n:int n2:int
#@NSET3     _DEF|_JIT         a:rn n:u n2:u n3:u
#@NSET4     _DEF|_JIT         a:rn n:u n2:u n3:u n4:u

#XNSET      0                 a:sfx b:int
#XNMOV      0                 a:sfx b:rn
#XNMOVx     0                 a:sfx b:sfx
#@iINC      _DEF|_JIT         a:rn
#@iDEC      _DEF|_JIT         a:rn

#bNUL       _DEF|_JIT         c:rn a:ro
#bNN        _DEF|_JIT         c:rn a:ro
#iNEG       _DEF|_JIT         c:rn a:rn
#fNEG       _DEF|_JIT         c:rn a:rn       
#iTR        _DEF|_JIT         c:rn a:rn  inttr:f
#fTR        _DEF|_JIT         c:rn a:rn  floattr:f

#iADD       _DEF|_JIT         c:rn a:rn b:rn
#iSUB       _DEF|_JIT         c:rn a:rn b:rn
#iMUL       _DEF|_JIT         c:rn a:rn b:rn
#iDIV       _DEF|_JIT         c:rn a:rn b:rn
#iMOD       _DEF|_JIT         c:rn a:rn b:rn
#iEQ        _DEF|_JIT         c:rn a:rn b:rn
#iNEQ       _DEF|_JIT         c:rn a:rn b:rn
#iLT        _DEF|_JIT         c:rn a:rn b:rn
#iLTE       _DEF|_JIT         c:rn a:rn b:rn
#iGT        _DEF|_JIT         c:rn a:rn b:rn
#iGTE       _DEF|_JIT         c:rn a:rn b:rn
#iAND       _DEF|_JIT         c:rn a:rn b:rn
#iOR        _DEF|_JIT         c:rn a:rn b:rn
#iXOR       _DEF|_JIT         c:rn a:rn b:rn
#iLSFT      _DEF|_JIT         c:rn a:rn b:rn
#iRSFT      _DEF|_JIT         c:rn a:rn n:rn

#iADDC      _DEF|_JIT         c:rn a:rn n:int
#iSUBC      _DEF|_JIT         c:rn a:rn n:int
#iMULC      _DEF|_JIT         c:rn a:rn n:int
#iDIVC      _DEF|_JIT         c:rn a:rn n:int
#iMODC      _DEF|_JIT         c:rn a:rn n:int
#iEQC       _DEF|_JIT         c:rn a:rn n:int
#iNEQC      _DEF|_JIT         c:rn a:rn n:int
#iLTC       _DEF|_JIT         c:rn a:rn n:int
#iLTEC      _DEF|_JIT         c:rn a:rn n:int
#iGTC       _DEF|_JIT         c:rn a:rn n:int
#iGTEC      _DEF|_JIT         c:rn a:rn n:int
#iANDC      _DEF|_JIT         c:rn a:rn n:int
#iORC       _DEF|_JIT         c:rn a:rn n:int
#iXORC      _DEF|_JIT         c:rn a:rn n:int
#iLSFTC     _DEF|_JIT         c:rn a:rn n:int
#iRSFTC     _DEF|_JIT         c:rn a:rn n:int

#fADD       _DEF|_JIT         c:rn a:rn b:rn  
#fSUB       _DEF|_JIT         c:rn a:rn b:rn  
#fMUL       _DEF|_JIT         c:rn a:rn b:rn  
#fDIV       _DEF|_JIT         c:rn a:rn b:rn  
#fEQ        _DEF|_JIT         c:rn a:rn b:rn  
#fNEQ       _DEF|_JIT         c:rn a:rn b:rn  
#fLT        _DEF|_JIT         c:rn a:rn b:rn  
#fLTE       _DEF|_JIT         c:rn a:rn b:rn  
#fGT        _DEF|_JIT         c:rn a:rn b:rn  
#fGTE       _DEF|_JIT         c:rn a:rn b:rn  

#fADDC      _DEF|_JIT         c:rn a:rn n:float 
#fSUBC      _DEF|_JIT         c:rn a:rn n:float 
#fMULC      _DEF|_JIT         c:rn a:rn n:float 
#fDIVC      _DEF|_JIT         c:rn a:rn n:float 
#fEQC       _DEF|_JIT         c:rn a:rn n:float 
#fNEQC      _DEF|_JIT         c:rn a:rn n:float 
#fLTC       _DEF|_JIT         c:rn a:rn n:float 
#fLTEC      _DEF|_JIT         c:rn a:rn n:float 
#fGTC       _DEF|_JIT         c:rn a:rn n:float 
#fGTEC      _DEF|_JIT         c:rn a:rn n:float 

#RCINC      _JIT              a:ro
#RCDEC      _JIT              a:ro

#@OSET      _DEF|_JIT|_CONST  a:ro o:Object
#@OMOV      _DEF|_JIT         a:ro b:ro
#@OOMOV     _DEF|_JIT         a:ro b:ro c:ro d:ro
#@ONMOV     _DEF|_JIT         a:ro b:ro c:rn d:rn
#@OSET2     _JIT|_CONST       a:ro v:Object v2:Object
#@OSET3     _JIT|_CONST       a:ro v:Object v2:Object v3:Object
#@OSET4     _JIT|_CONST       a:ro v:Object v2:Object v3:Object v4:Object

#RCINCx     0                 a:sfx
#RCDECx     0                 a:sfx
#@OMOVx     _DEF|_JIT         a:ro b:sfx
#@XMOV      _JIT              a:sfx b:ro
#@XOSET     _JIT|_CONST       a:sfx b:Object
#@XMOVx     0                 a:sfx b:sfx

#MOVe   a:sfpidx xspidx:u

#CHKSTACK   0                 n:sfpidx
#LDMTD      0                 thisidx:sfpidx loadmtd:f cache:hcache mtdNC:mtd
#SCALL      _DEF|_JIT         a:r thisidx:sfpidx espshift:sfpidx mtdNC:mtd
#VCALL      _DEF|_JIT         a:r thisidx:sfpidx espshift:sfpidx mtdNC:mtd
#VCALL_     _DEF|_JIT         a:r thisidx:sfpidx espshift:sfpidx mtdNC:mtd
#FASTCALL0  _DEF|_JIT         a:r thisidx:sfpidx rix:i espshift:sfpidx fcall:f

#TR         _DEF|_JIT         a:r  b:sfpidx rix:i cid:cid tr:f

#SCAST      _DEF              a:r b:sfpidx rix:i espshift:sfpidx cast:tmr
#TCAST      _DEF              a:r b:sfpidx rix:i espshift:sfpidx cast:tmr
#ACAST      _DEF              a:r b:sfpidx rix:i espshift:sfpidx cast:tmr
#iCAST      _DEF|_JIT         a:rn b:rn
#fCAST      _DEF|_JIT         a:rn b:rn

#JMP_       _JIT              addr:addr
#NEXT       _DEF              addr:addr a:r b:sfpidx rix:i espshift:sfpidx

#BGETIDX    _DEF|_JIT         c:rn a:ro n:rn 
#BSETIDX    _DEF|_JIT         c:rn a:ro n:rn  v:rn
#BGETIDXC   _DEF|_JIT         c:rn a:ro n:u 
#BSETIDXC   _DEF|_JIT         c:rn a:ro n:u  v:rn

#NGETIDX    _DEF|_JIT         c:rn a:ro n:rn 
#NSETIDX    _DEF|_JIT         c:rn a:ro n:rn v:rn
#NGETIDXC   _DEF|_JIT         c:rn a:ro n:u 
#NSETIDXC   _DEF|_JIT         c:rn a:ro n:u  v:rn

#@OGETIDX    _DEF|_JIT         c:ro a:ro n:rn
#@OSETIDX    _DEF|_JIT         c:ro a:ro n:rn v:ro  
#@OGETIDXC   _DEF|_JIT         c:ro a:ro n:u 
#@OSETIDXC   _DEF|_JIT         c:ro a:ro n:u  v:ro 

# level 3 instruction

#bJNUL     _DEF|_JIT   addr:addr a:ro
#bJNN      _DEF|_JIT   addr:addr a:ro

#bJNOT     _DEF|_JIT   addr:addr a:rn
#iJEQ      _DEF|_JIT   addr:addr a:rn b:rn
#iJNEQ     _DEF|_JIT   addr:addr a:rn b:rn
#iJLT      _DEF|_JIT   addr:addr a:rn b:rn
#iJLTE     _DEF|_JIT   addr:addr a:rn b:rn
#iJGT      _DEF|_JIT   addr:addr a:rn b:rn
#iJGTE     _DEF|_JIT   addr:addr a:rn b:rn
#iJEQC     _DEF|_JIT   addr:addr a:rn n:int
#iJNEQC    _DEF|_JIT   addr:addr a:rn n:int
#iJLTC     _DEF|_JIT   addr:addr a:rn n:int
#iJLTEC    _DEF|_JIT   addr:addr a:rn n:int
#iJGTC     _DEF|_JIT   addr:addr a:rn n:int
#iJGTEC    _DEF|_JIT   addr:addr a:rn n:int

#fJEQ      _DEF|_JIT   addr:addr a:rn b:rn
#fJNEQ     _DEF|_JIT   addr:addr a:rn b:rn
#fJLT      _DEF|_JIT   addr:addr a:rn b:rn
#fJLTE     _DEF|_JIT   addr:addr a:rn b:rn
#fJGT      _DEF|_JIT   addr:addr a:rn b:rn
#fJGTE     _DEF|_JIT   addr:addr a:rn b:rn
#fJEQC     _DEF|_JIT   addr:addr a:rn n:float
#fJNEQC    _DEF|_JIT   addr:addr a:rn n:float
#fJLTC     _DEF|_JIT   addr:addr a:rn n:float
#fJLTEC    _DEF|_JIT   addr:addr a:rn n:float
#fJGTC     _DEF|_JIT   addr:addr a:rn n:float
#fJGTEC    _DEF|_JIT   addr:addr a:rn n:float

#CHKIDX     _JIT              a:ro n:rn
#CHKIDXC    _JIT              a:ro n:u


"""

CTYPE = {
	'sfpidx' :  'intptr_t', 
	'int':      'kint_t',
	'float':    'kfloat_t',
	'cid':      'KonohaClass*',
	'co':       'kObject*',
	'hcache':   'kcachedata_t',
#	'addr':     'knh_KLRInst_t*',
	'u':        'uintptr_t',
	'i':        'intptr_t',
	'rn':       'kreg_t',
	'ro':       'kreg_t',
	'r':        'kreg_t',
}

def cap(t):
    return t[0].upper() + t[1:] 

def getctype(t, v):
	if CTYPE.has_key(t): return CTYPE[t]
	if t == 'f': return '%sFunc' % (cap(v))
	return 'k%s*' % t

def getVMT(t):
	tt = ', VMT_%s' % (t.upper())
	return tt

def getsize(t):
#	if t == 'sfx': return '+1'
#	if t == 'int': return '+VMTSIZE_int'
#	if t == 'float': return '+VMTSIZE_float'
	return ''

class KCODE:
	def __init__(self, opcode, line):
		self.tokens = line.split()
		self.name = self.tokens[0].replace('@', '')
		self.flag = self.tokens[1]
		self.NAME = self.name.upper()
		self.opcode = opcode
		self.OPCODE = 'OPCODE_%s' % self.name
		self.OPLABEL = 'L_%s' % self.name
#		self.ctype = 'klr_%s_t' % self.name		
		self.ctype = 'OP%s' % self.name		
		self.level=''
		self.ifdef = 'CASE'
		self.size = '%d' % len(self.tokens[2:])
		self.struct = '{'
		for a in self.tokens[2:]:
			if a.startswith('#') :
				self.ifdef = a[1:]
				self.size = '%d' % len(self.tokens[2:]) - 1
				continue
			if len(a.split(':')) == 1: print line
			n, t = a.split(':')
			ctype = getctype(t, n)
			self.struct = self.struct + getVMT(t)
			self.size = self.size + getsize(t)
		self.struct += ', VMT_VOID}'
		self.struct = self.struct.replace('{,', '{')

#####################################################################

CPROTO = []

KCODE_LIST = []
KSTRUCT_LIST = []
KCODE_STRUCT ={}

c = 0
for line in INSTRUCTIONS.split('\n'):
	if line.startswith("#") or len(line) == 0: continue
	if len(line.split()) > 1:
		kc = KCODE(c, line)
		KCODE_LIST.append(kc)
		c += 1

#------------------------------------------------------------------------------

def write_KCODE_h(f, kc):
	f.write('''
#define %s ((kopcode_t)%d)''' % (kc.OPCODE, kc.opcode))
	f.write('''
typedef struct %s {
	KCODE_HEAD;''' % kc.ctype)
	for a in kc.tokens[2:]:
		n, t = a.split(':')
		if t == "addr" : 
			f.write('''
	VirtualMachineInstruction  *jumppc;''')
		else: 
			f.write('''
	%s %s;''' % (getctype(t, n), n))
	f.write('''
} %s;
''' % kc.ctype)

def write_define_h(f):
	for p in CPROTO:
		f.write(p + ';\n')
	for kc in KCODE_LIST:
		write_KCODE_h(f,kc)
	n = len(KCODE_LIST)
	f.write('''
	
#define KOPCODE_MAX ((kopcode_t)%d)

#define VMT_VOID       0
#define VMT_ADDR       1
#define VMT_R          2
#define VMT_RN         2
#define VMT_RO         2
#define VMT_U          3
#define VMT_I          4
#define VMT_CID        5
#define VMT_CO         6
#define VMT_INT        7
#define VMT_FLOAT      8
#define VMT_HCACHE     9
#define VMT_F         10/*function*/
#define VMT_STRING    11
#define VMT_METHOD    12
#define VMT_NAMESPACE 13

''' % (n))

#------------------------------------------------------------------------------

def write_common_c(f):
	write_chapter(f, '[common]')

def write_data_c(f):
	write_chapter(f, '[data]')
	f.write('''
#define _CONST 1
#define _JIT   (1<<1)
#define _DEF   (1<<2)
typedef struct {
	const char *name;
	kshortflag_t   flag;
	kushort_t size;
	kushort_t types[6];
} kOPDATA_t;

static const kOPDATA_t OPDATA[] = {''')
	for kc in KCODE_LIST:
		n = kc.name
		if n.endswith("_"): n = n[:-1]
		f.write('''
	{"%s", %s, %s, %s}, ''' % (n, kc.flag, kc.size, kc.struct))
	f.write('''
};

static void opcode_check(void)
{''')
	for kc in KCODE_LIST:
		f.write('''
	assert(sizeof(%s) <= sizeof(VirtualMachineInstruction));''' % (kc.ctype))
	f.write('''
}

static const char *T_opcode(kopcode_t opcode)
{
	return OPDATA[opcode].name;
}

#ifdef OLD
static size_t kopcode_size(kopcode_t opcode)
{
	return OPDATA[opcode].size;
}

static kbool_t kopcode_hasjump(kopcode_t opcode)
{
	return (OPDATA[opcode].types[0] == VMT_ADDR);
}
#endif

/* ------------------------------------------------------------------------ */


''')

def write_inst_c(f):
	write_common_c(f)
	write_data_c(f)

#############################################################################

def getmacro(kc, label):
	m = 'OPEXEC%s_%s(' % (kc.level, kc.name)
	comma=''
	for a in kc.tokens[2:]:
		v, t = a.split(':')
		if t == 'addr':
			m += '%spc = op->jumppc, %s' % (comma, label)
		else:
			m += '%sop->%s' % (comma, v)
		comma=', '
	m += ')'
	return m

def write_exec(f):
	write_chapter(f, '[exec]')
	f.write('''

//#if (defined(K_USING_LINUX_) && (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) && __GNUC__ >= 3))
//#define K_USING_VMASMDISPATCH 1
//#endif

#ifdef K_USING_THCODE_
#define CASE(x)  L_##x : 
#define NEXT_OP   (pc->codeaddr)
#define JUMP      *(NEXT_OP)
#ifdef K_USING_VMASMDISPATCH
#define GOTO_NEXT()     \\
	asm volatile("jmp *%0;": : "g"(NEXT_OP));\\
	goto *(NEXT_OP)

#else
#define GOTO_NEXT()     goto *(NEXT_OP)
#endif
#define TC(c) 
#define DISPATCH_START(pc) goto *OPJUMP[pc->opcode]
#define DISPATCH_END(pc)
#define GOTO_PC(pc)        GOTO_NEXT()
#else/*K_USING_THCODE_*/
#define OPJUMP      NULL
#define CASE(x)     case OPCODE_##x : 
#define NEXT_OP     L_HEAD
#define GOTO_NEXT() goto NEXT_OP
#define JUMP        L_HEAD
#define TC(c)
#define DISPATCH_START(pc) L_HEAD:;switch(pc->opcode) {
#define DISPATCH_END(pc)   } /*KNH_DIE("unknown opcode=%d", (int)pc->opcode)*/; 
#define GOTO_PC(pc)         GOTO_NEXT()
#endif/*K_USING_THCODE_*/

static VirtualMachineInstruction* KonohaVirtualMachine_run(KonohaContext *kctx, KonohaStack *sfp0, VirtualMachineInstruction *pc)
{
#ifdef K_USING_THCODE_
	static void *OPJUMP[] = {''')
	c = 0
	for kc in KCODE_LIST:
		if c % 4 == 0: f.write('\n\t\t')
		f.write('''&&%s, ''' % kc.OPLABEL)
		c += 1
	f.write('''
	};
#endif
	krbp_t *rbp = (krbp_t*)sfp0;
	DISPATCH_START(pc);
''')
	for kc in KCODE_LIST:
# DBG_P("%%p %%s", pc-1, T_opcode((pc-1)->opcode));
		f.write('''
	%s(%s) {
		%s *op = (%s*)pc;
		%s; pc++;
		GOTO_NEXT();
	} ''' % (kc.ifdef, kc.name, kc.ctype, kc.ctype, getmacro(kc, 'JUMP')))
	f.write('''
	DISPATCH_END(pc);
	L_RETURN:;
	return pc;
}

''')

#------------------------------------------------------------------------------

###########

def verbose_print2(msg):
	print msg

###########



def safedict2(d, key, defv):
	if d.has_key(key): return d[key]
	d[key] = defv
	return defv
###


#------------------------------------------------------------------------------

def gen_vm_c(bdir):
#	fname = 'minivm.c'
#	f = open_c(fname, ['"vm.h"', '"minivm.h"'])
#	close_c(f, fname)
	
	f = open('minivm.h', 'w')
	f.write('''/****************************************************************************
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
''');
	f.write('#ifndef %s\n' % 'minivm_h'.upper());
	f.write('#define %s\n' % 'minivm_h'.upper());
	f.write('''// THIS FILE WAS AUTOMATICALLY GENERATED

''')
	write_define_h(f)
	write_inst_c(f)
	write_exec(f)

	f.write('#endif /* %s */\n' % 'minivm_h'.upper());
	f.close()


#------------------------------------------------------------------------------

if __name__ == '__main__':
	bdir = '.'
	gen_vm_c(".")

