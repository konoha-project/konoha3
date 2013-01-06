import("cstyle");
import("konoha.null");
import("Type.File");
import("Java.String");
//import("konoha.new"); Don't use this
import("JavaScript.Array");
import("konoha.foreach");
import("Syntax.JavaStyleClass");

String varid_type(String orig) {
    String type = orig;
    if(type.indexOf("const") >= 0) {
        type = type.replace("const", "");
    }
    if(type.indexOf("*") >= 0) {
        type = type.replace("*", "Ptr");
    }
    if(type.indexOf("unsigned") >= 0) {
        type = type.replace("unsigned ", "unsigned_");
    }
    if(type.indexOf("long ") >= 0) {
        type = type.replace("long ", "long_");
    }
    String[] tmp = type.split(" ");
    String s = "";
    int i;
    while (i < tmp.getSize()) {
        s = s + tmp.get(i);
        i = i + 1;
    }
    return s;
}

class FunctionInfo {
    String name;
    String returnType;
    String[] argumentsType;
}
void generate_function(String[] data, FunctionInfo info) {
    int i = 0;
    String[] args = info.argumentsType;
    stdout.println("// " + data);
    stdout.println("static KMETHOD K${info.name}(KonohaContext *kctx,  KonohaStack *sfp)");
    stdout.println("{");
    while (i < args.getSize()) {
        stdout.println("\t${args.get(i)} arg${i} = To_${args.get(i)}(sfp[${i}]);");
        i = i + 1;
    }
    i = 0;
    stdout.print("\t");
    if(info.returnType != "void") {
        stdout.print("${info.returnType} ret = ");
    }
    stdout.print("${info.name} (");
    while (i < args.getSize()) {
        if(i != 0) {
            stdout.print(", ");
        }
        stdout.print("arg${i}");
        i = i + 1;
    }
    stdout.println(");");
    if(info.returnType == "void") {
        stdout.println("\treturnvoid(${info.name});");
    } else {
        stdout.println("\tRETURN${info.returnType}(ret);");
    }
    stdout.println("}\n");
}

String generate_signature(FunctionInfo info) {
    String[] args = info.argumentsType;
    String tmp = "_Public, _F(K${info.name}), TY_${info.returnType}, ";
    tmp = tmp + args.getSize() + ", ";
    if(args.getSize() > 0) {
        tmp = tmp + "TY_${args.get(0)}, FN_${0}, ";
    }
    int i = 1;
    while (i < args.getSize()) {
        tmp = tmp + ("TY_${args.get(i)}, FN_${i},");
        i = i + 1;
    }
    return tmp;
}

void main() {
    String path = FILE.scriptPath("./libvirt.info");
    FILE file = new FILE(path, "r");
    String line;
    Array[String] func_info = [];
    stdout.println("#include \"libvirt_glue.h\"\n");
    while ((line = file.readLine()) != null) {
        if(line == "") {
            break;
        }
        String[] data = line.split(", ");
        String func    = data.get(0);
        String retType = varid_type(data.get(1));
        String[] args  = new String[0];
        FunctionInfo info = new FunctionInfo();
        info.name = data.get(0);
        info.returnType = retType;
        if(data.get(data.getSize() - 1) == "") {
            data.pop();
        }
        int i = 2;
        while (i < data.getSize()) {
            args.add(varid_type(data.get(i)));
            i = i + 1;
        }
        info.argumentsType = args;
        generate_function(data, info);
        func_info.add(generate_signature(info));
    }
	stdout.println("#define _Public   kMethod_Public");
	stdout.println("static kbool_t PACKAGE_INIT_PKG(LIBVIRT_PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)");
    stdout.println("{");
    int i = 0;
	stdout.println("\tvirt_loadStructData(kctx, ns, trace);");
    while (i < 8) {
        stdout.println("\tint FN_${i} = FN_(\"arg${i}\");");
        i = i + 1;
    }
    stdout.println("\tKDEFINE_METHOD MethodData[] = {");

    for(String line in func_info) {
        stdout.println("\t\t" + line);
    }
    stdout.println("\t\tDEND,");
    stdout.println("\t};");
    stdout.println("\treturn true;");
    stdout.println("}");

    stdout.println("static kbool_t PACKAGE_SETUP_PKG(LIBVIRT_PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)");
    stdout.println("{");
    stdout.println("\treturn true;");
    stdout.println("}");
    stdout.println("");
    stdout.println("KDEFINE_PACKAGE* PACKAGE_INIT(LIBVIRT_PACKAGE_NAME)(void)");
    stdout.println("{");
    stdout.println("\tstatic KDEFINE_PACKAGE d = {0};");
    stdout.println("\tKSetPackageName(d, LIBVIRT_PACKAGE_NAME, \"1.0\");");
    stdout.println("\td.PackupNameSpace  = PACKAGE_INIT_PKG(LIBVIRT_PACKAGE_NAME);");
    stdout.println("\td.ExportNameSpace = PACKAGE_SETUP_PKG(LIBVIRT_PACKAGE_NAME);");
    stdout.println("\treturn &d;");
    stdout.println("}");


    file.close();
}

main();
