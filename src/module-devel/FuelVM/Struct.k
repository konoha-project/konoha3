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

import("js4.regexp");
import("konoha.new");
import("java2.string");
import("konoha.foreach");
import("konoha.ns");  // konoha.const is obsolete
import("js4.array");
import("konoha.file");
import("konoha.class");
import("konoha.json");
import("cstyle");

String readFile(String path) {
    FILE file = new FILE(path, "r");
    int len = 0;
    String tmp = "";
    while (true) {
        int ch = file.getc();
        if(ch == -1) {
            break;
        }
        tmp = tmp + String.fromCharCode(ch);
        len = len + 1;
    }
    file.close();
    return tmp;
}

void parseStruct(String[] lines, int i) {
    int begin = i;
    int end   = begin + 1;
    i = end;
    Param[] data = [];
    while (i < lines.length) {
        String s = lines.get(i);
        if(s == "};") {
            break;
        }
        s = s.replace(";", "").trim();
        s = s.split("//").get(0);
        String[] a = s.split(" ");
        String type = a.get(0);
        String name = a.get(1);
        data.add(new Param(type, name));
        i = i + 1;
    }
    end = i;
    i = 0;
    while (i < data.length) {
        Param p = data.get(i);
        System.p(p.type + " " + p.name);
        i = i + 1;
    }
}

void parseFile(String data) {
    Json json = Json.parse(data);
    int i = 0;
    int len = json.getSize();
    System.p(json.toString());
    while (i < len) {
        Json val = json.get(i);
        System.p(val.toString());
        String type = val.getString("type");
        String name = val.getString("name");
        System.p(type + ":" + name);
        i = i + 1;
    }

}

void main (String[] args) {
    String data = readFile("headerinfo.json");
    parseFile(data);
}

main([]);
