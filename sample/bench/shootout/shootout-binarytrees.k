/* The Great Computer Language Shootout
   http://shootout.alioth.debian.org/
   contributed by Kimio Kuramitsu, imasahiro, Wakamatsu
*/

import("cstyle");
import("Syntax.JavaStyleClass");
import("Syntax.GlobalVariable");
import("MiniKonoha.NameSpace");

const MINDEPTH = 4;
const N=14;

class TreeNode {
	TreeNode left, right;
	int item;

	TreeNode(TreeNode left, TreeNode right, int item) {
		this.left = left;
		this.right = right;
		this.item = item;
	}

	int itemCheck(){
		// if necessary deallocate here
		if(this.left == null) return this.item;
		return this.item + this.left.itemCheck() - this.right.itemCheck();
	}
}

TreeNode bottomUpTree(int item, int depth){
	if (depth > 0){
		return new TreeNode(bottomUpTree(2*item-1, depth-1), bottomUpTree(2*item, depth-1), item);
	}
	else {
		return new TreeNode(null, null, item);
	}
}

void run() {
	int n = N;
	int maxDepth = n;
	if(MINDEPTH+2 > n) {maxDepth = MINDEPTH + 2;}
	int stretchDepth = maxDepth + 1;
	int check = bottomUpTree(0,stretchDepth).itemCheck();
	System.p("stretch tree of depth " + stretchDepth + "\t check: " + check);
	TreeNode longLivedTree = bottomUpTree(0, maxDepth);
	int depth = MINDEPTH;
	while (depth <= maxDepth) {
		int iterations = 1 << (maxDepth - depth + MINDEPTH);
		check = 0;
		int i=1;
		while (i <= iterations) {
			check = check + bottomUpTree(i, depth).itemCheck();
			check = check + bottomUpTree(-i, depth).itemCheck();
			i = i + 1;
		}
		System.p(""+(iterations*2) + "\t trees of depth " + depth + "\t check: " + check);
		depth = depth + 2;
	}
	System.p("long lived tree of depth " + maxDepth + "\t check: "+ longLivedTree.itemCheck());
}

run();

