import("Syntax.CStyleWhile");
import("Syntax.Null");
import("Syntax.JavaStyleClass");
import("Syntax.GlobalVariable");
import("Syntax.CStyleBitwiseOperator");
import("JavaScript.Array");
import("MiniKonoha.NameSpace");

AllowImplicitGlobalVariable(true);

/* The Great Computer Language Shootout
   http://shootout.alioth.debian.org/
   contributed by Kimio Kuramitsu
*/

const c_MINDEPTH = 4;

class TreeNode {
	TreeNode left, right;
	int item;
	boolean isNull;

	TreeNode(TreeNode left0, TreeNode right0, int item0, boolean isnull0) {
		left = left0;
		right = right0;
		item = item0;
		isNull = isnull0;
	}

	int itemCheck(){
		// if necessary deallocate here
		if(isNull) { return item; }
		return item + left.itemCheck() - right.itemCheck();
	}
}

TreeNode bottomUpTree(int item, int depth) {
	if(depth > 0){
		return new TreeNode(bottomUpTree(2*item-1, depth-1), bottomUpTree(2*item, depth-1), item, false);
	}
	else {
		TreeNode t = null;
		return new TreeNode(t, t, item, true);
	}
}

void main() {
	int n = 10;
	int maxDepth;
	if(c_MINDEPTH + 2 > n) { maxDepth = c_MINDEPTH + 2; } else { maxDepth = n; }
	int stretchDepth = maxDepth + 1;
	int check = bottomUpTree(0,stretchDepth).itemCheck();
	assert(check == -1);
	System.p("stretch tree of depth " + stretchDepth + " check: " + check);
	TreeNode longLivedTree = bottomUpTree(0,maxDepth);
	int depth = c_MINDEPTH;
	while (depth <= maxDepth) {
		int iterations = 1 << (maxDepth - depth + c_MINDEPTH);
		check = 0;
		int i = 1;
		while (i<= iterations){
			check = check + bottomUpTree(i, depth).itemCheck();
			check = check + bottomUpTree(-i, depth).itemCheck();
			i=i+1;
		}
		assert(check == iterations * -2);
		System.p("" + (iterations*2) + " trees of depth " + depth + " check: " + check);
		depth = depth + 2;
	}
	check = longLivedTree.itemCheck();
	assert(check == -1);
	System.p("long lived tree of depth " + maxDepth + " check: "+ check);
}

main();
