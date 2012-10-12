// written by chenji

import("konoha.null");
import("sugar");

syntax "hoge" $Block;

boolean tyCheckHoge(Stmt stmt, Gamma gma) {
	Block b = stmt.getBlock("$Block", null);
	boolean ret = false;
	if(b != null) {
		ret = b.tyCheckAll(gma);
		if(ret) {
			stmt.setType(TSTMT_BLOCK);
		}
	}
	return ret;
}

addStatement("hoge", tyCheckHoge);

void test() {
	boolean executed = false;
	hoge {
		executed = true;
	}
	assert(executed);
}

test();
