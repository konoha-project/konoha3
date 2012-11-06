// written by chenji

import("konoha.global");
import("konoha.eval");

int g_i = 1;
assert(System.eval("g_i = g_i + 1"));
assert(g_i == 2);

assert(System.eval("boolean f() { return true; }"));
assert(f());
