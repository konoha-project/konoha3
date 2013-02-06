#include "karray.h"
#include "codegen.h"
#include <stdio.h>

struct BlockNode;
typedef struct BlockNode BlockNode;
typedef BlockNode *BlockNodePtr;
DEF_ARRAY_STRUCT(BlockNodePtr);
DEF_ARRAY_T(BlockNodePtr);
DEF_ARRAY_OP_NOPOINTER(BlockNodePtr);

DEF_ARRAY_OP_NOPOINTER(INodePtr);

struct BlockNode {
	Block *block;
	short preorderId;   /* Depth First Number, Preorder */
	short rpostoderId; /* Depth First Number, Reverse Postorder */
	ARRAY(BlockNodePtr) preds;    /* List of predecessor nodes */
	ARRAY(BlockNodePtr) succs;    /* List of successor nodes */
	ARRAY(BlockNodePtr) children; /* Subnodes in the Dominator Tree */
	ARRAY(BlockNodePtr) dfront;   /* Dominance Frontier */
	struct BlockNode *idom;   /* Immediate Dominator */
	int *domset; /* Dominator Set Bit Vector */
	ARRAY(INodePtr) phis;
};

typedef struct ControlFlowGraph {
	ARRAY(BlockNodePtr) BlockNodes;
	struct BlockNode *EntryBlock;
} CFG;

/* Compute Dominator Tree, Dominance Frontiers */

static void addOnce(ARRAY(BlockNodePtr) *list, BlockNode *node)
{
	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(*list, x, e) {
		if(*x == node)
			return;
	}
	ARRAY_add(BlockNodePtr, list, node);
}

static BlockNode *CreateBlockNode(CFG *g, Block *block)
{
	BlockNode *p = (BlockNode *) malloc(sizeof(BlockNode));
	p->idom  = NULL;
	p->block = block;
	ARRAY_init(BlockNodePtr, &p->preds, 0);
	ARRAY_init(BlockNodePtr, &p->succs, 0);
	ARRAY_init(INodePtr, &p->phis, 0);
	ARRAY_add(BlockNodePtr, &g->BlockNodes, p);
	return p;
}

static void addedge(BlockNode *from, BlockNode *to)
{
	ARRAY_add(BlockNodePtr, &from->succs, to);
	ARRAY_add(BlockNodePtr, &to->preds, from);
}

/* Compute Depth First Spanning Tree */
static int dfst(BlockNode *v, int preorderId, int rpostoderId)
{
	int n = 1;
	v->preorderId = preorderId;

	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(v->succs, x, e) {
		if(!(*x)->preorderId) {
			n += dfst(*x, preorderId + n, rpostoderId - n + 1);
		}
	}
	v->rpostoderId = rpostoderId - n + 1;
	return n;
}

#define BITS (sizeof(int) * 8)
#define AddEntry(SET, E)    ((SET)[(E) / BITS] |=  (1 << ((E) % BITS)))
#define RemoveEntry(SET, E) ((SET)[(E) / BITS] &= ~(1 << ((E) % BITS)))

static int highestbit(int word)
{
	return word ? (BITS - __builtin_clz(word)) - 1 : 0;
}

static void clear_list(int *bitset, int w, int init)
{
	int i;
	for(i = 0; i < w; i++)
		bitset[i] = init;
}

/* Compute Dominators by Aho-Ullman's bit vector algorithm */
static void dominator_ahoullman_bit(CFG *g)
{
	int BlockNodeSize = ARRAY_size(g->BlockNodes);
	int i, j, w = (BlockNodeSize + BITS - 1) / BITS;
	int *work = alloca(sizeof(int) * w);

	BlockNodePtr *x, *e;
	BlockNode **nodes = alloca(sizeof(BlockNode *) * (BlockNodeSize + 1));
	FOR_EACH_ARRAY(g->BlockNodes, x, e) {
		(*x)->preorderId = 0;
		(*x)->domset = alloca(sizeof(int) * w);
		clear_list((*x)->domset, w, ~0);
	}

	clear_list(g->EntryBlock->domset, w, 0);
	dfst(g->EntryBlock, 1, BlockNodeSize);
	AddEntry(g->EntryBlock->domset, 1);

	nodes[0] = NULL;
	FOR_EACH_ARRAY(g->BlockNodes, x, e) {
		nodes[(*x)->rpostoderId] = *x;
	}

	bool change;
	do {
		change = false;
		for(i = 2; i <= BlockNodeSize; i++) {
			clear_list(work, w, ~0);

			BlockNodePtr *x, *e;
			FOR_EACH_ARRAY(nodes[i]->preds, x, e) {
				for(j = 0; j < w; ++j) {
					work[j] &= (*x)->domset[j];
				}
			}
			AddEntry(work, i);
			for(j = 0; j < w; ++j) {
				if(work[j] != nodes[i]->domset[j]) {
					nodes[i]->domset[j] = work[j];
					change = true;
				}
			}
		}
	} while(change);

	/* Convert dominator sets to dominator tree */
	g->EntryBlock->idom = NULL;
	FOR_EACH_ARRAY(g->BlockNodes, x, e) {
		BlockNode *p = *x;
		RemoveEntry(p->domset, p->rpostoderId);
		for(i = w-1; i >= 0; --i) {
			if(p->domset[i]) {
				p->idom = nodes[(i * BITS) + highestbit(p->domset[i])];
				break;
			}
		}
		p->domset = NULL;
	}
}

/* Set up Subnode Pointers in the Dominator Tree */
static void setup_domtree(CFG *g)
{
	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(g->BlockNodes, x, e) {
		ARRAY_init(BlockNodePtr, &(*x)->children, 0);
	}
	FOR_EACH_ARRAY(g->BlockNodes, x, e) {
		BlockNode *p = *x;
		if(p->idom) {
			ARRAY_add(BlockNodePtr, &(p->idom->children), p);
		}
	}
}

/* Compute Dominance Frontier */
static void domfront(BlockNode *node)
{
	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(node->children, x, e) {
		domfront(*x);
	}
	ARRAY_init(BlockNodePtr, &node->dfront, 0);

	FOR_EACH_ARRAY(node->succs, x, e) {
		if((*x)->idom != node)
			addOnce(&node->dfront, *x);
	}
	FOR_EACH_ARRAY(node->children, x, e) {
		BlockNodePtr *itr, *end;
		FOR_EACH_ARRAY((*x)->dfront, itr, end) {
			if((*itr)->idom != node) {
				addOnce(&node->dfront, *itr);
			}
		}
	}
}

static BlockNode *GetBlockNode(CFG *cfg, Block *block)
{
	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(cfg->BlockNodes, x, e) {
		if((*x)->block == block) {
			return *x;
		}
	}
	assert(0 && "unreachable");
	return 0;
}

static CFG *CreateControlFlowGraph(FuelIRBuilder *builder)
{
	CFG *cfg = (CFG *) malloc(sizeof(CFG));
	ARRAY_init(BlockNodePtr, &cfg->BlockNodes, ARRAY_size(builder->Blocks));

	BlockPtr *x, *e;
	FOR_EACH_ARRAY(builder->Blocks, x, e) {
		CreateBlockNode(cfg, (*x));
	}

	BlockNodePtr *itr, *end;
	FOR_EACH_ARRAY(cfg->BlockNodes, itr, end) {
		Block *block = (*itr)->block;
		FOR_EACH_ARRAY(block->succs, x, e) {
			addedge(*itr, GetBlockNode(cfg, *x));
		}
	}

	cfg->EntryBlock = ARRAY_get(BlockNodePtr, &cfg->BlockNodes, 0);
	return cfg;
}

static void DeleteControlFlowGraph(CFG *cfg)
{
	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(cfg->BlockNodes, x, e) {
		BlockNode *node = *x;
		ARRAY_dispose(BlockNodePtr, &node->preds);
		ARRAY_dispose(BlockNodePtr, &node->succs);
		ARRAY_dispose(BlockNodePtr, &node->children);
		ARRAY_dispose(BlockNodePtr, &node->dfront);
		ARRAY_dispose(INodePtr, &node->phis);
		free(node);
	}
	ARRAY_dispose(BlockNodePtr, &cfg->BlockNodes);
	free(cfg);
}

/* Print Dominator Tree */
static void printdomtree(BlockNode *node, int level)
{
#if 0
	int i;
	BlockNodePtr *x, *e;
	for(i = 0; i < level; i++)
		fputs("   ", stderr);
	fprintf(stderr, "%d (", node->block->base.Id);
	FOR_EACH_ARRAY(node->dfront, x, e) {
		fprintf(stderr, "%d ", (*x)->block->base.Id);
	}
	fprintf(stderr, ")\n");

	FOR_EACH_ARRAY(node->children, x, e) {
		printdomtree(*x, level + 1);
	}
#else
	(void)node;(void)level;
#endif
}

static void ComputeDominanceFrontier(CFG *cfg)
{
	dominator_ahoullman_bit(cfg);
	/* Compute Dominance Frontier */
	setup_domtree(cfg);
	domfront(cfg->EntryBlock);
	/* Print Dominace Frontiers */
	printdomtree(cfg->EntryBlock, 0);
}

static void AddIncoming(Block *BB, IPHI *phi, IField *Val)
{
	BlockPtr *x, *e;
	FOR_EACH_ARRAY(BB->preds, x, e) {
		PHIInst_addParam(phi, (IUpdate *) Val);
		PHIInst_addParam(phi, (IUpdate *) *x);
	}
}

static void addPHI(FuelIRBuilder *builder, BlockNode *block, INode *Node)
{
	IPHI *phi;
	INodePtr *x, *e;
	FOR_EACH_ARRAY(block->phis, x, e) {
		phi = (IPHI *) *x;
		INodePtr *Inst, *End;
		FOR_EACH_ARRAY(phi->Args, Inst, End) {
			if(*Inst == Node) {
				return;
			}
		}
	}
	phi = builder->API->newPHI(builder, Node);
	phi->base.Parent = (INode *) block->block;

	AddIncoming(block->block, phi, (IField *) Node);
	ARRAY_add(INodePtr, &block->phis, (INode *) phi);
}

static void InsertPHI(FuelIRBuilder *builder, BlockNode *node)
{
	BlockNodePtr *x, *e;
	INodePtr *Inst, *End;
	FOR_EACH_ARRAY(builder->LocalVar, Inst, End) {
		FOR_EACH_ARRAY(node->dfront, x, e) {
			addPHI(builder, *x, *Inst);
		}
	}
	FOR_EACH_ARRAY(node->children, x, e) {
		InsertPHI(builder, *x);
	}
}

static void NewName(INode *NewVal, IField *Node, ARRAY(INodePtr) *stack)
{
	unsigned n = Node->Id;
	ARRAY_add(INodePtr, &stack[n], NewVal);
}

static INode *TopStack(IField *Node, ARRAY(INodePtr) *stack)
{
	unsigned n = Node->Id;
	return *ARRAY_last(stack[n]);
}

static void PopStack(IField *Node, ARRAY(INodePtr) *stack)
{
	unsigned n = Node->Id;
	ARRAY_size(stack[n]) = ARRAY_size(stack[n]) - 1;
}

static void FillIncoming(Block *BB, IPHI *phi, ARRAY(INodePtr) *stack)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY(phi->Args, x, e) {
		IField *field = (IField *) *(x);
		Block  *block = (Block  *) *(x + 1);
		if(block == BB) {
			*x = TopStack(field, stack);
		}
		++x;
	}
}

static void ReplaceValue(INode **NodePtr, ARRAY(INodePtr) *stack)
{
	INode *Node = *NodePtr;
	IField *Field;
	if((Field = CHECK_KIND(Node, IField))) {
		if(Field->Op == LocalScope) {
			*NodePtr = TopStack(Field, stack);
		}
	}
}

static void ReplaceArrayElementWith(ARRAY(INodePtr) *List, ARRAY(INodePtr) *stack)
{
	INodePtr *x, *e;
	FOR_EACH_ARRAY((*List), x, e) {
		ReplaceValue(x, stack);
	}
}

static void RewriteNode(INode *Node, ARRAY(INodePtr) *stack)
{
#define CASE(KIND) case IR_TYPE_##KIND:
	switch(Node->Kind) {
		case IR_TYPE_IConstant:
		case IR_TYPE_IArgument:
			break;
		CASE(IField) {
			IField *Inst = (IField *) Node;
			switch(Inst->Op) {
				case GlobalScope:
				case EnvScope:
				case FieldScope:
					ReplaceValue((INode **)&(Inst->Node), stack);
					break;
				case LocalScope:
					break;
			}
			break;
		}
		case IR_TYPE_INew:
			break;
		case IR_TYPE_ICall:
			ReplaceArrayElementWith(&((ICall *)Node)->Params, stack);
			break;
		case IR_TYPE_IFunction: {
			IFunction *Inst = (IFunction *) Node;
			ReplaceValue((INode **)&(Inst->Func), stack);
			ReplaceValue((INode **)&(Inst->Env), stack);
			ReplaceValue((INode **)&(Inst->Method), stack);
			break;
		}
		CASE(IUpdate) {
			IUpdate *Inst = (IUpdate *) Node;
			ReplaceValue((INode **)&(Inst->RHS), stack);
			if(Inst->LHS->Op == LocalScope) {
				NewName(Node, Inst->LHS, stack);
			}
			break;
		}
		case IR_TYPE_IBranch:
			ReplaceValue(&((IBranch *) Node)->Cond, stack);
			break;
		case IR_TYPE_ITest:
			ReplaceValue((INode **)&((ITest *) Node)->Value, stack);
			break;
		CASE(IReturn) {
			IReturn *Inst = (IReturn *) Node;
			if(Inst->Inst) {
				ReplaceValue(&Inst->Inst, stack);
			}
			break;
		}
		case IR_TYPE_IJump:
		case IR_TYPE_ITry:
			break;
		case IR_TYPE_IThrow:
			ReplaceValue(&((IThrow *)Node)->Val, stack);
			break;
		case IR_TYPE_IYield:
			ReplaceValue(&((IYield *) Node)->Value, stack);
			break;
		case IR_TYPE_IUnary:
			ReplaceValue(&((IUnary *) Node)->Node, stack);
			break;
		case IR_TYPE_IBinary:
			ReplaceValue(&((IBinary *) Node)->LHS, stack);
			ReplaceValue(&((IBinary *) Node)->RHS, stack);
			break;
		case IR_TYPE_IPHI:
			break;
		default:
			assert(0 && "unreachable");
#undef CASE
	}
}

static void Rename(CFG *cfg, BlockNode *b, ARRAY(INodePtr) *stack)
{
	INodePtr *Inst, *End;
	IPHI *PHI;
	IUpdate *Update;
	BlockNodePtr *x, *e;

	FOR_EACH_ARRAY(b->block->insts, Inst, End) {
		if((PHI = CHECK_KIND(*Inst, IPHI))) {
			NewName(*Inst, (IField *) PHI->Val, stack);
		}
	}
	FOR_EACH_ARRAY(b->block->insts, Inst, End) {
		RewriteNode(*Inst, stack);
	}

	FOR_EACH_ARRAY(b->succs, x, e) {
		FOR_EACH_ARRAY((*x)->block->insts, Inst, End) {
			if((PHI = CHECK_KIND(*Inst, IPHI))) {
				FillIncoming(b->block, PHI, stack);
			}
		}
	}

	/* Rename each successor s of b in the dominator tree */
	FOR_EACH_ARRAY(b->children, x, e) {
		Rename(cfg, *x, stack);
	}

	/* clear stack */
	FOR_EACH_ARRAY(b->block->insts, Inst, End) {
		if((PHI = CHECK_KIND(*Inst, IPHI))) {
			PopStack((IField *) PHI->Val, stack);
		}
		if((Update = CHECK_KIND(*Inst, IUpdate))) {
			if(Update->LHS->Op == LocalScope)
				PopStack(Update->LHS, stack);
		}
	}
}

static void RenamingVariables(CFG *cfg, FuelIRBuilder *builder)
{
	unsigned i = 0, size = ARRAY_size(builder->LocalVar);
	ARRAY(INodePtr) *stack = (ARRAY(INodePtr) *) alloca(sizeof(*stack) * size);
	for(i = 0; i < size; i++) {
		ARRAY_init(INodePtr, &stack[i], 0);
	}

	Rename(cfg, cfg->EntryBlock, stack);

	for(i = 0; i < size; i++) {
		ARRAY_dispose(INodePtr, &stack[i]);
	}
}

static void InsertUndefinedVariables(BlockNode *Node, FuelIRBuilder *builder)
{
	unsigned i, size = ARRAY_size(builder->LocalVar);
	/* create undefined local variables at entryblock */
	int *local = (int *) alloca(sizeof(int) * size);
	INodePtr *x, *e;

	for(i = 0; i < size; i++) {
		local[i] = -1;
	}

	FOR_EACH_ARRAY(Node->block->insts, x, e) {
		IUpdate *Update;
		if((Update = CHECK_KIND(*x, IUpdate))) {
			if(Update->LHS->Op == LocalScope)
				local[Update->LHS->Id] = Update->LHS->Id;
		}
	}

	Block *EntryBB = Node->block;
	IRBuilder_setBlock(builder, EntryBB);
	unsigned len = ARRAY_size(EntryBB->insts);

	SValue C; C.bits = 0;
	int change = 0;
	for(i = 0; i < size; i++) {
		if(local[i] == -1) {
			INode *Val = ARRAY_get(INodePtr, &builder->LocalVar, i);
			INode *RHS = CreateConstant(builder, Val->Type, C);
			CreateUpdate(builder, Val, RHS);
			change += 1;
		}
	}
	if(change) {
		unsigned cur = ARRAY_size(EntryBB->insts);
		INodePtr *list = EntryBB->insts.list;
		INode *jmp = list[len-1];
		for(i = 0; i < cur - len; i++) {
			list[len-1+i] = list[len+i];
		}
		list[cur-1] = jmp;
	}
}

void InsertPHINode(FuelIRBuilder *builder)
{
	CFG *cfg = CreateControlFlowGraph(builder);
	ComputeDominanceFrontier(cfg);
	InsertPHI(builder, cfg->EntryBlock);

	BlockNodePtr *x, *e;
	FOR_EACH_ARRAY(cfg->BlockNodes, x, e) {
		BlockNode *node = *x;
		if(ARRAY_size(node->phis) != 0) {
			ARRAY(INodePtr) *list = &node->block->insts;
			ARRAY(INodePtr) *phis = &node->phis;
			unsigned InstSize = ARRAYp_size(list);
			unsigned PHISize  = ARRAYp_size(phis);
			ARRAY_ensureSize(INodePtr, list, InstSize + PHISize);
			ARRAY_ensureSize(INodePtr, phis, InstSize + PHISize);
			memcpy(phis->list + PHISize, list->list, sizeof(INodePtr)*InstSize);
			memcpy(list->list, phis->list, sizeof(INodePtr)*(InstSize + PHISize));
			list->size += PHISize;
		}
	}

	InsertUndefinedVariables(cfg->EntryBlock, builder);
	RenamingVariables(cfg, builder);
	DeleteControlFlowGraph(cfg);

}
