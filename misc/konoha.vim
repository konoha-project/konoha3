" Vim syntax file
" Language:     Konoha
" Maintainer:   Masahiro Ide <imasahiro9@gmail.com>
" URL:          http://code.google.com/p/konoha/
" Last Change:  2012 Apl 21
"
"=========
" INSTALL
"=========
" 1. First, copy misc/vim/konoha.vim to your vim/gvim syntax directory or $HOME/.vim/syntax
"    $ cp misc/vim/konoha.vim $HOME/.vim/syntax
" 2. Second, just type in
"    $ echo "au! BufRead,BufNewFile *.k  setfiletype konoha" >> $HOME/.vim/filetype.vim
"
" Quit when a syntax file was already loaded
if version < 600
    syntax clear
elseif exists("b:current_syntax")
    finish
endif

syn region kParen transparent start='(' end=')' contains=ALLBUT, kError,kTodo
syn match  kError ")"
"syn match  kError "}"
syn match  kError "\<\%(catch\|finally\|else\|from\)\>"
syn match  kLabel  display +^\s*\I\i*\s*:\([^:]\)\@=+

syn keyword kType int Int Integer float double Float
syn keyword kType Bytes String Array Map Object Regex
syn keyword kType boolean Boolean void var Func

syn keyword kStructure class namespace function
syn keyword kStorage extends interface this final protected private public

syn keyword kConstant false null true

syn keyword kStatement as default from is isa lock pragma where instanceof

syn keyword kFunc print typeof defined using import typeof assert format new

syn keyword kException try catch finally throw
syn keyword kRepeat break continue do for foreach return while assure
syn keyword kCond else if switch

syn keyword kTodo contained TODO FIXME XXX NOTE

syn region  kComment start="/\*"  end="\*/" contains=kTodo,@Spell
syn match   kComment "\%^#!.*" contains=kTodo,@Spell
syn match   kComment "//.*$" contains=kTodo,@Spell

" Strings and constants
syn match   kSpecialError contained "\\."
" ?
" syn match   kSpecialCharError contained "[^']"

syn match   kVerbatimSpec +@"+he=s+1 contained
syn region  kVerbatimString start=+@"+ end=+"+ end=+$+ skip=+""+ contains=kVerbatimSpec,@Spell

syn region  kString start=+"+  end=+"+ end=+$+ contains=kSpecialChar,kSpecialError,@Spell
syn region  kString start=+'+ end=+'+ contains=kSpecialChar,@Spell
syn region  kString start=+[uU]\="""+ end=+"""+ contains=kSpecialChar,@Spell
syn region  kString start=+[uU]\='''+ end=+'''+ contains=kSpecialChar,@Spell
syn region  kURN  start=+\w\+://+ end=+[a-zA-Z0-9\.]\+/*[a-zA-Z0-9/\\%_.]*?*[a-zA-Z0-9/\\%_.=&amp;]*+

syn match   kCharacter "'[^']*'" contains=kSpecialChar,kSpecialCharError
syn match   kCharacter "'\\''" contains=kSpecialChar
syn match   kCharacter "'[^\\]'"
syn match   kNumber "\<\d\+\>"
syn match   kNumber "\(\<\d\+\.\d*\|\.\d\+\)\([eE][-+]\=\d\+\)\=[fFdD]\="
syn match   kNumber  "\<0x\x\+\>"

hi def link kStorage StorageClass
hi def link kRepeat Repeat
hi def link kVerbatimString String
hi def link kString String
hi def link kURN String
hi def link kNumber Number
hi def link kLabel Label
hi def link kCond  Conditional
hi def link kComment Comment
hi def link kType Type
hi def link kTodo Todo
hi def link kError Error
hi def link kFunc Function
hi def link kConstant Constant
hi def link kException Exception
hi def link kStructure Structure

let b:current_syntax = "konoha"

" vim: ts=8
