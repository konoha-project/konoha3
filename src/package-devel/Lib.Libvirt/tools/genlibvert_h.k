import("cstyle");
import("konoha.null");
import("Type.File");
import("Java.String");
//import("konoha.new"); Don't use this
import("JavaScript.Array");
import("konoha.foreach");
import("Syntax.JavaStyleClass");

class String2 {
	String left;
	String right;

}

String2 generate(String type) {
	String type_defs = "";
	String konohaclass_defs = "";

	if(type.endsWith("PtrPtr")) {
		type_defs = type_defs + "typedef struct " + type.substr(0, type.length-3) + " *" + type + ";\n";
	}
	type_defs = type_defs + "typedef struct k" + type + " {\n";
	type_defs = type_defs + "\tKonohaObjectHeader h;\n";
	type_defs = type_defs + "\t" + type + " p;\n";
	type_defs = type_defs + "}k" + type + ";\n";
	type_defs = type_defs + "static int TY_" + type + ";\n\n";

	konohaclass_defs = konohaclass_defs + "\tKDEFINE_CLASS def" + type + " = {\n";
	konohaclass_defs = konohaclass_defs + "\t\t.structname = \"" + type + "\",\n";
	konohaclass_defs = konohaclass_defs + "\t\t.typeId = TypeAttr_NewId,\n";
	konohaclass_defs = konohaclass_defs + "\t\t.cstruct_size = sizeof(k" + type + "),\n";
	konohaclass_defs = konohaclass_defs + "\t\t.cflag = KClassFlag_Final,\n";
	konohaclass_defs = konohaclass_defs + "\t\t.init = virt_common_ptr_Init,\n";
	konohaclass_defs = konohaclass_defs + "\t\t.free = virt_common_ptr_Free,\n";
	konohaclass_defs = konohaclass_defs + "\t};\n";
	konohaclass_defs = konohaclass_defs + "\tKonohaClass *ct" + type + " = KLIB kNameSpace_DefineClass(kctx, nx, NULL, &def" + type + ", trace);\n";
	konohaclass_defs = konohaclass_defs + "\tTY_" + type + " = ct" + type + "->typeId;\n\n";

	String2 res = new String2();
	res.left = type_defs;
	res.right = konohaclass_defs;
	return res;

	//konohaclass_defs = konohaclass_defs + 
}

void main() {
    String path = FILE.scriptPath("./typeinfo.info");
    FILE file = new FILE(path, "r");
	String2 res;
    String line;
	String type_defs = "";
	String konohaclass_defs = "static void virt_loadStructData(KonohaContext *kctx, kNameSpace *ns, KTraceInfo *trace)\n{\n";
    //stdout.println("#include \"libvirt_glue.h\"\n");
    while ((line = file.readLine()) != null) {
        if(line == "###") {
            break;
        }
		res = generate(line);
		type_defs = type_defs + res.left;
		konohaclass_defs = konohaclass_defs + res.right;
    }
	konohaclass_defs = konohaclass_defs + "}\n";
	stdout.println(type_defs);
	stdout.println(konohaclass_defs);
    //stdout.println("static kbool_t PACKAGE_INIT_PKG(PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, int option, KTraceInfo *trace)");
    //stdout.println("{");
    //int i = 0;
	//stdout.println("\tvirt_loadStructData(kctx, ns, trace);");
    //while (i < 8) {
    //    stdout.println("\tint FN_${i} = FN_(\"arg${i}\");");
    //    i = i + 1;
    //}
    //stdout.println("\tKDEFINE_METHOD MethodData[] = {");

    //for(String line in func_info) {
    //    stdout.println("\t\t" + line);
    //}
    //stdout.println("\t\tDEND,");
    //stdout.println("\t};");
    //stdout.println("\treturn true;");
    //stdout.println("}");

    //stdout.println("static kbool_t PACKAGE_SETUP_PKG(PACKAGE_NAME)(KonohaContext *kctx, kNameSpace *ns, kNameSpace *exportNS, int option, KTraceInfo *trace)");
    //stdout.println("{");
    //stdout.println("\treturn true;");
    //stdout.println("}");
    //stdout.println("");
    //stdout.println("KDEFINE_PACKAGE* PACKAGE_INIT(PACKAGE_NAME)(void)");
    //stdout.println("{");
    //stdout.println("\tstatic KDEFINE_PACKAGE d = {0};");
    //stdout.println("\tKSetPackageName(d, PACKAGE_STRING(PACKAGE_NAME), \"1.0\");");
    //stdout.println("\td.PackupNameSpace  = PACKAGE_INIT_PKG(PACKAGE_NAME);");
    //stdout.println("\td.ExportNameSpace = PACKAGE_SETUP_PKG(PACKAGE_NAME);");
    //stdout.println("\treturn &d;");
    //stdout.println("}");


    file.close();
}

main();
