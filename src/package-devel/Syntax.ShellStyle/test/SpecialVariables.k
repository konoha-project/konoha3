// written by chenji

import("Syntax.ShellStyle");

test -d not/exist

assert($? == 1);
