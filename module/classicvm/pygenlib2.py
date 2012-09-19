import os, sys

###########

def verbose_print(msg):
	print msg

###########

def linetrim(s):
	return s.replace('\r', '').replace('\n','')
###

def sublast(s, s2):
	return s[s.find(s2)+len(s2):]

def parse_package(fpath):
	#p = fpath[:fpath.rfind('/')]
	p = fpath
	package = 'konoha'
	if p.find('/konoha/') != -1:
		return 'konoha'
	elif p.find('/class/') != -1:
		package = sublast(p, '/class/')
	elif p.find('/package/') != -1:
		package = '+' + sublast(p, '/package/')
	elif p.find('/api/') != -1:
		package = sublast(p, '/api/')
	elif p.find('/driver/') != -1:
		package = '#' + sublast(p, '/driver/')
	if package.find('_.') > 0: return 'konoha'
	if package.find('/') > 0:
		return package.split('/')[0]
	return package
#	p = fpath.split('/')
#	if p[-1].find('.') == -1: return p[-1]
#	return p[-2]
###

def fpath_shortfilename(fpath):
	p = fpath.split('/')
	return p[-1].replace('.c', '')
###


def safedict(d, key, defv):
	if d.has_key(key): return d[key]
	d[key] = defv
	return defv
###

###


def list_topair(list):
	t1 = list[0]
	t2 = list[1]
	return t1, t2, list[2:]

def parse_funcparams(functype):
	if not functype.endswith(')'):
		debug_print('Invalid functype: %s' % functype)
	t = functype.replace('(', ' ').replace(',', ' ').replace(')', '').split()
	params = []
	while len(t) > 1:
		tt, tn, t = list_topair(t)
		params.append(nz_cparam(tt, tn))
	return params








###########

# ---------------------------------------------------------------------------

LINE = '''
/* ------------------------------------------------------------------------ */
'''

DLINE = '''
/* ------------------------------------------------------------------------ */
'''

# ---------------------------------------------------------------------------

def write_println(f, msg = ''):
	f.write(msg+'\n')

def write_line(f):
	f.write(LINE)

def write_dline(f):
	f.write(DLINE)

def write_comment(f, msg):
	f.write('/* %s */\n' % msg)

def write_chapter(f, msg):
	f.write(DLINE)
	write_comment(f, msg)

def write_section(f, msg):
	f.write(LINE)
	write_comment(f, msg)

def write_define(f, name, value='', n=40):
	s = '#define %s ' % name
	while(len(s) < n) : s+=' '
	f.write(s)
	f.write(value)
	f.write('\n')
###

def write_ifndefine(f, name, value='', n=40):
	f.write('#ifndef %s\n' % name)
	write_define(f, name, value, n)
	f.write('#endif\n')
###



def write_ifndef(f, name, value='', n=40):
	f.write('#ifndef %s\n' % name)
	write_define(f, name, value, n)
	f.write('#endif\n')

def write_ifdef(f, n):
	f.write('''
#ifdef  %s''' % n.upper())

def write_else(f, n):
	f.write('''
#else /*%s*/
''' % n.upper())

def write_endif(f, n):
	f.write('''
#endif/*%s*/
''' % n.upper())

# ---------------------------------------------------------------------------

def write_BOM(f):
	f.write("%c%c%c" % (0xef, 0xbb, 0xbf))

def write_license(f):
	f.write('''/****************************************************************************
 * KONOHA2 COPYRIGHT, LICENSE NOTICE, AND DISCRIMER
 * 
 * Copyright (c) 2006-2012, Kimio Kuramitsu <kimio at ynu.ac.jp>
 *           (c) 2008-      Konoha Team konohaken@googlegroups.com
 * All rights reserved.
 * 
 * You may choose one of the following two licenses when you use konoha.
 * If you want to use the latter license, please contact us.
 * 
 * (1) GNU General Public License 3.0 (with K_UNDER_GPL)
 * (2) Konoha Non-Disclosure License 1.0
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 ****************************************************************************/
''')

def write_begin_c(f):
	f.write('''
#ifdef __cplusplus 
extern "C" {
#endif
''')

def write_end_c(f):
	f.write('''
#ifdef __cplusplus
}
#endif
''')

# ---------------------------------------------------------------------------

def getdict(d, n, defv):
	if d.has_key(n): return d[n]
	return defv

def read_settings(fn):
	KNH_DATA = {}
	try:
		f = open(fn)
		exec(f)
		f.close()
		return KNH_DATA
	except OSError, e:
		print e
		return KNH_DATA

# ---------------------------------------------------------------------------

def nz_fname(fname):
	if fname.rfind('/') > 0: return fname[fname.rfind('/')+1:]
	return fname

def open_h(fname, lists):
	f = open(fname, 'w')
	write_license(f)
	d = nz_fname(fname).replace('.', '_'). upper()
	f.write('''
#ifndef %s
#define %s
''' % (d, d))

	for i in lists:
		f.write('''
#include%s''' % i)
	if len(lists) > 0: f.write('\n\n')
	write_begin_c(f)
	write_dline(f)
	return f

def open_h2(fname, lists):
	f = open(fname, 'w')
	write_license(f)
	d = nz_fname(fname).replace('.', '_'). upper()
	f.write('''
#ifndef %s
#define %s
''' % (d, d))

	for i in lists:
		f.write('''
#include%s''' % i)
	if len(lists) > 0: f.write('\n\n')
	return f

# ---------------------------------------------------------------------------

def close_h(f, fname):
	d = nz_fname(fname).replace('.', '_'). upper()
	write_end_c(f)
	write_dline(f)
	f.write('''
#endif/*%s*/
''' % d)
	f.close()

# ---------------------------------------------------------------------------

def open_c(fname, lists, bom = None):
	f = open(fname, 'w')
	if bom == 'BOM': write_BOM(f)
	write_license(f)
	for i in lists:
		f.write('''
#include%s''' % i)
	if len(lists) > 0: f.write('\n\n')
	write_begin_c(f)
	write_dline(f)
	return f

def close_c(f, fname):
	write_end_c(f)
	f.close()

def get_serial_number():
	f = open('SERIAL_NUMBER')
	n = int(f.readline())
	f.close()
	n += 1
	f = open('SERIAL_NUMBER', 'w')
	f.write('%d\n' % n)
	f.close()
	return n

# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------

def parse_options(option):
	d = {}
	if option is None: return d
	for t in option.split():
		if t.find('(') > 0:
			t = t.replace('(', ' ').replace(')', '')
			t = t.split()
			d[t[0]] = t[1]
		else:
			d[t] = 1
	return d

# ---------------------------------------------------------------------------

def check_ifdef(d):
	ifdef = ''
	endif = ''
	if d.has_key('@ifdef'):
		ifdef = '#ifdef  KNH_IMPORT_%s_\n' % d['@ifdef']
		endif = '#endif/*KNH_IMPORT_%s_*/\n' %d['@ifdef']
	return ifdef, endif

# ---------------------------------------------------------------------------

def alias_lname(cname):
	if cname.find('_') > 0:
		return cname.split('_')[1]
	return cname

def STRUCT_cname(cname):
	return 'STRUCT_%s' % cname

def STRUCT_sname(cname):
	return 'STRUCT_%s' % cname

def SAFE_cname(t) :
	t = t.replace('..', '')
	t = t.replace('!', '')
	t = t.replace('[]', '')
	t = t.replace('::', '__')
	t = t.replace(':', '__')
	return t	

def TY_cname(cname) :
	prefix = ''
	if cname.endswith('[]'): prefix = 'A'
	if cname.endswith('..'): prefix = 'I'
	return '%sTY_%s' % (prefix, SAFE_cname(cname))

def T_cname(t) :
	prefix = ''
	if t.endswith("[]!"): prefix = 'NNA'
	elif t.endswith("!") : prefix = 'NN'
	if t.endswith('[]'): prefix = 'A'
	if t.endswith('..'): prefix = 'NNI'
	return '%sT_%s' % (prefix, SAFE_cname(t))

def DebugTagcname(cname):
	return 'DebugTag%s' % cname

def FN_name(fn):
    return 'FN_%s' % fn

def SAFE_mname(mname):
	return mname.replace('::', '__').replace(':', '__').replace('%', '_')

def MN_mname(mname):
    return 'MN_%s' % SAFE_mname(mname)

# ---------------------------------------------------------------------------

DEBUG = None

def debug_print(msg):
	if not DEBUG: print msg
	
def nz_dir(dir):
	if dir.endswith('/'): return dir[:len(dir)-1]
	return dir

#------------------------------------------------------------------------------------

FUNCMAP = {}

def FUNCMAP_found(funcname):
	FUNCMAP[funcname] = funcname

def FUNCMAP_exists(funcname):
	return FUNCMAP.has_key(funcname)
