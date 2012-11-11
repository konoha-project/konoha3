// written by chenji

import("cstyle"); // konoha.null is obsolate; use cstyle
import("sugar");

syntax "hoge" $Block;

boolean TypeCheckHoge(Stmt stmt, Gamma gma) {
	Block b = stmt.getBlock("$Block", null);
	boolean ret = false;
	if(b != null) {
		ret = b.TypeCheckAll(gma);
		if(ret) {
			stmt.setType(TSTMT_BLOCK);
		}
	}
	return ret;
}

addStatement("hoge", TypeCheckHoge);

void test() {
	boolean executed = false;
	hoge {
		executed = true;
	}
	assert(executed);
}

test();
