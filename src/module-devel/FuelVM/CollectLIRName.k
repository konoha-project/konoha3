/****************************************************************************
 * Copyright (c) 2012, Masahiro Ide <ide@konohascript.org> All rights reserved.
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

import("konoha.regexp");
import("konoha.new");
import("konoha.string");
import("konoha.foreach");
import("konoha.ns");  // konoha.const is obsolete
import("konoha.array");
import("konoha.file");
import("konoha.class");
import("cstyle");

String readFile(String path) {
    FILE file = new FILE(path, "r");
    int len = 0;
    String tmp = "";
    while (true) {
        int ch = file.getc();
        if (ch == -1) {
            break;
        }
        tmp = tmp + String.fromCharCode(ch);
        len = len + 1;
    }
    file.close();
    return tmp;
}

void emit(String path, String data) {
    FILE file = new FILE(path, "w");
    file.print(data);
}

String[] String[].filter(String pattern) {
    String[] ret = [];
    int i = 0;
    while (i < this.length) {
        if (this.get(i).startsWith(pattern)) {
            ret.add(this.get(i));
        }
        i = i + 1;
    }
    return ret;
}

void main (String[] args) {
    String[] data = readFile("./LIR.def").split("\n").filter("def ");
    int i = 0;
    String ret = "";
    while (i < data.length) {
        String name = data[i].split("(").get(0).substr("def ".length);
        ret = ret + "\tEMIT_LIR(builder, ${name});\n";
        i = i + 1;
    }
    emit("./LIRName.h", ret);
}

main([]);
