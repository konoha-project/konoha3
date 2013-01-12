import("MiniKonoha.Syntax")
import("MiniKonoha.NameSpace")
import("Syntax.Null")
import("Syntax.CStyleWhile")
import("JavaScript.Array")

/*
int parse(Node node, Symbol symbol, Token[] tokenList, int beginIndex, int operatorIndex, int endIndex){
	if(node == null){ return -1; }
	return -1;
}

Node type(Node node, NameSpace ns, Object obj){
	if(node == null){ return null; }
	node.done();
	return node;
}
*/

Node typeIf(Node node, NameSpace ns, Object obj){
	node.setType(KNode_If);
	node.TypeCheckNode("$Expr", boolean, 0);
	return node;
}

int parseEnd(Node node, Symbol symbol, Token[] tokenList, int beginIdx, int opIdx, int endIdx){
	if(tokenList[beginIdx].getParsedText() == "end"){
		return endIdx;
	}
	return -1;
}

Node createBlockNode(NameSpace ns){
	Node blockNode = new Node(ns);
	blockNode.setType(KNode_Block);
	return blockNode;
}

Node Node.createBlockNodeAt(Symbol symbol){
	Node blockNode = createBlockNode(this.getNameSpace());
	this.AddParsedObject(symbol, blockNode);
	return blockNode;
}

void MoveNodeToBlock(Node[] nodeList, Node targetBlockNode, int i){
	targetBlockNode.addNode(nodeList[i]);
	nodeList[i] = null;
}

void MoveNodesToBlock(Node[] nodeList, Node targetBlockNode, int beginIndex, int endIndex){
	int i = beginIndex;
	while(i < endIndex){
		MoveNodeToBlock(nodeList, targetBlockNode, i);
		i = i + 1;
	}
}

Node Node.lookupIfNodeWithoutElse(){
	// lookup if
	Node block = this.getParentNode();
	Node[] nodeList = block.getNodeList();
	Node ifNode = null;
	int i = 0;
	while(i < nodeList.getSize()){
		if(nodeList[i] == this){
			break;
		}
		if(nodeList[i] != null){
			String term = nodeList[i].getTermToken().getParsedText();
			if(term == "if"){
				ifNode = nodeList[i];
			}
		}
		i = i + 1;
	}
	// find if without else
	if(ifNode != null){
		Node[] elseBlockList = ifNode.getNode("else").getNodeList();
		while(elseBlockList != null && elseBlockList.getSize() > 0){
			Node firstNode = elseBlockList[0];
			if(firstNode.getType() == KNode_If){
				ifNode = firstNode;
				elseBlockList = ifNode.getNode("else").getNodeList();
			}else{
				break;
			}
		}
	}
	return ifNode;
}

void closeBlock(Node[] nodeList, int unclosedBlockNodeIndex, int endNodeIndex){
	// move nodes into the block
	if(unclosedBlockNodeIndex != -1){
		Node unclosedNode = nodeList[unclosedBlockNodeIndex];
		int type =  unclosedNode.getType();
		String term = unclosedNode.getTermToken().getParsedText();
		Node targetBlockNode = null;

		if(type == KNode_If){
			targetBlockNode = unclosedNode.createBlockNodeAt("$Block");
		}else if(term == "else"){
			Node ifWithoutElse = unclosedNode.lookupIfNodeWithoutElse();
			targetBlockNode = ifWithoutElse.createBlockNodeAt("else");
		}
		MoveNodesToBlock(nodeList, targetBlockNode, unclosedBlockNodeIndex + 1, endNodeIndex);
		if(term == "elsif"){
			Node ifWithoutElse = unclosedNode.lookupIfNodeWithoutElse();
			Node elseBlock = ifWithoutElse.createBlockNodeAt("else");
			MoveNodeToBlock(nodeList, elseBlock, unclosedBlockNodeIndex);
		}
	}
}

Node typeElsif(Node node, NameSpace ns, Object obj){
	node.setType(KNode_If);
	node.TypeCheckNode("$Expr", boolean, 0);
	
	// lookup unclosed if block
	Node block = node.getParentNode();
	Node[] nodeList = block.getNodeList();
	int unclosedBlockNodeIndex = -1;
	int endNodeIndex = -1;
	int i = 0;
	while(i < nodeList.getSize()){
		int type =  nodeList[i].getType();
		String term = nodeList[i].getTermToken().getParsedText();
		boolean hasNoBlock = nodeList[i].getNode("$Block") == null;
	
		if(nodeList[i] == node){
			endNodeIndex = i;
			break;
		}
		if(term == "if" && hasNoBlock){
			unclosedBlockNodeIndex = i;
			//System.p("unclosed 'if' found");
		}else if(term == "elsif" && hasNoBlock){
			unclosedBlockNodeIndex = i;
			//System.p("unclosed 'elsif' found");
		}
		i = i + 1;
	}
	
	closeBlock(nodeList, unclosedBlockNodeIndex, endNodeIndex);
	return node;
}

Node typeElse(Node node, NameSpace ns, Object obj){
	// lookup unclosed if block
	Node block = node.getParentNode();
	Node[] nodeList = block.getNodeList();
	int unclosedBlockNodeIndex = -1;
	int endNodeIndex = -1;
	int i = 0;
	while(i < nodeList.getSize()){
		int type =  nodeList[i].getType();
		String term = nodeList[i].getTermToken().getParsedText();
		boolean hasNoBlock = nodeList[i].getNode("$Block") == null;
		
		if(nodeList[i] == node){
			endNodeIndex = i;
			break;
		}
		if(term == "if" && hasNoBlock){
			unclosedBlockNodeIndex = i;
		}else if(term == "elsif" && hasNoBlock){
			unclosedBlockNodeIndex = i;
		}
		i = i + 1;
	}
	
	closeBlock(nodeList, unclosedBlockNodeIndex, endNodeIndex);
	node.done();
	return node;
}

Node typeEnd(Node node, NameSpace ns, Object obj){
	// lookup unclosed block
	Node block = node.getParentNode();
	Node[] nodeList = block.getNodeList();
	int unclosedBlockNodeIndex = -1;
	int endNodeIndex = -1;
	int i = 0;
	while(i < nodeList.getSize()){
		int type =  nodeList[i].getType();
		String term = nodeList[i].getTermToken().getParsedText();
		boolean hasNoBlock = nodeList[i].getNode("$Block") == null;
		
		if(nodeList[i] == node){
			endNodeIndex = i;
			break;
		}
		if(term == "if" && hasNoBlock){
			unclosedBlockNodeIndex = i;
		}else if(term == "elsif" && hasNoBlock){
			unclosedBlockNodeIndex = i;
		}else if(term == "else"){
			unclosedBlockNodeIndex = i;
		}
		i = i + 1;
	}

	closeBlock(nodeList, unclosedBlockNodeIndex, endNodeIndex);
	node.done();
	return node;
}

void defineRubySyntax(){
	// avoid razy compilation
	CompileAllDefinedMethod();

	Syntax ifSyntax = new Syntax("if");
	ifSyntax.SetParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	ifSyntax.SetTypeFunc(typeIf);
	DefineSyntax(ifSyntax);
	ifSyntax.SetPattern("\"if\" $Expr [\"then\"]");

	Syntax elsifSyntax = new Syntax("elsif");
	elsifSyntax.SetParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	elsifSyntax.SetTypeFunc(typeElsif);
	DefineSyntax(elsifSyntax);
	elsifSyntax.SetPattern("\"elsif\" $Expr [\"then\"]");

	Syntax elseSyntax = new Syntax("else");
	elseSyntax.SetParseFunc(Precedence_Statement, 0, GetPatternParseFunc());
	elseSyntax.SetTypeFunc(typeElse);
	DefineSyntax(elseSyntax);
	elseSyntax.SetPattern("\"else\"");

	Syntax endSyntax = new Syntax("end");
	endSyntax.SetParseFunc(Precedence_Statement, 0, parseEnd);
	endSyntax.SetTypeFunc(typeEnd);
	DefineSyntax(endSyntax);
}

defineRubySyntax();

