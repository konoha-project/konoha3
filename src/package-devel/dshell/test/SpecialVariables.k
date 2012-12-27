// written by chenji

import("dshell");

test -d not/exist

assert($? == 1);
