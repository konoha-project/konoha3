// written by imasahiro

import("konoha.regexp");

void regex_literal() {
    /Hello/;
    /w /
}

regex_literal();


void global_option() {
    RegExp patt = /Hello/g;
}

global_option();

void invalid0() {
    8 / 2;
}

invalid0();
