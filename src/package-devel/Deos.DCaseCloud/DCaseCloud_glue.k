Import("Type.Json");
Import("Lib.Openssl");
Import("Syntax.CStyleFor");
Import("Syntax.CStyleWhile");
Import("Syntax.Null");
Import("Java.Class");
Import("MiniKonoha.NameSpace");
Import("MiniKonoha.Map");
Import("MiniKonoha.Sql");
Import("JavaStyle.Object");
Import("JavaScript.Array");
Import("JavaScript.Date");

class DBNode {
	int node_id;
	String description;
	int argument_id;
	int branch_id;
	boolean isEvidence;

	DBNode parent;
	Json lazy_parent;
	DBNode[] children;
	Json lazy_children;

	DBNode(int node_id, String description, boolean isEvidence) {
		this.node_id      = node_id;
		this.description = description;
		this.isEvidence  = isEvidence;
		this.children    = [];
		this.lazy_children = new Json([]);
		this.argument_id = -1;
	}

	DBNode(int node_id, String description) {
		this.node_id      = node_id;
		this.description = description;
		this.isEvidence  = false;
		this.children    = [];
		this.lazy_children = new Json([]);
		this.argument_id = -1;
	}

	DBNode(int node_id) {
		this.node_id      = node_id;
		this.description = "";
		this.isEvidence  = false;
		this.children    = [];
		this.lazy_children = new Json([]);
		this.argument_id = -1;
	}

	DBNode(Json content) {
		this.argument_id = -1;
		this.description = "";
		this.isEvidence  = false;
		this.children    = [];
		this.PrivateUpdate(content);
		this.lazy_children = new Json([]);
	}

	void AddChild(DBNode child) {
		this.children.add(child);
		child.parent = this;
	}

	void RemoveChild(DBNode child) {
		for(int i = 0; i < this.children.getSize(); i = i + 1) {
			if(this.children[i].node_id == child.node_id){
				children.removeAt(i);
				break;
			}
		}
		child.parent = NULL;
	}

	void AddParent(DBNode parent) {
		this.parent = parent;
		parent.children.add(this);
	}

	//Private
	void PrivateUpdate(Json content) {
		if(content.hasKey("description")) {
			this.description = content.getString("description");
		}
		if(content.hasKey("isEvidence")) {
			this.isEvidence = content.getBoolean("isEvidence");
		}
	}

	Json Update(Json content) {
		Json json = new Json();
		json.set("before",this.toJson());
		this.PrivateUpdate(content);
		json.set("after", this.toJson());
		return json;
	}

	Json PrivateToJson() {
		Json json = new Json();
		json.setString("node_id",this.node_id.toString());
		json.setString("description",this.description);
		json.setBoolean("isEvidence",this.isEvidence);
		return json;
	}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		return json;
	}

	@Public Json GetContext() {
		//Abstract
		return NULL;
	}

	@Public int AddContext(Json context) {
		//Abstract
	}
}

class Context extends DBNode {
	Json context; //FIXME

	Context(Json content) {
		//TODO Create Argument
		this.context = new Json();

		this.PrivateUpdate(content);
		if(content.hasKey("properties")) {
			Json array = content.get("properties");
			String[] keys = array.keys();
			int i = 0;
			while(i < keys.getSize()) {
				this.context.setString(keys[i],array.getString(keys[i]));//TODO validation
				i = i + 1;
			}
		}
	}

	String searchValue(String key) {
		if(this.context.hasKey(key)) {
			return this.context.getString(key);
		}
		return NULL;
	}

	void addKeyValue(String key, String value) {
		this.context.setString(key,value);
	}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Context");
		json.set("properties",this.context);
		return json;
	}

	@Public Json GetContext() {
		return this.context;
	}

	@Public int AddContext(Json context) {
		String[] keys = context.keys();
		//for(int i = 0; i < keys.getSize(); i = i + 1) {
		int i = 0;
		while(i < keys.getSize()) {
			this.context.setString(keys[i],context.getString(keys[i])); //TODO Need validation
			i = i + 1;
		}
		return this.node_id;
	}
}

class Goal extends DBNode {
	Goal(){}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Goal");
		return json;
	}

	//FIXME validation
	@Public Json GetContext() {
		Json json = this.parent.GetContext();
		Json context_array = new Json([]);
		//for(int i = 0; i < this.children.getSize();i = i + 1) {
		int i = 0;
		while(i < this.children.getSize()) {
			if(this.children[i].toJson().getString("type") == "Context") { //FIXME instanceOf
				Context c = this.children[i] as Context;
				context_array.add(c.GetContext());
			}
			i = i + 1;
		}
		//for(int i = 0; i < context_array.getSize();i = i + 1) {
		i = 0;
		while(i < context_array.getSize()) {
			Json context = context_array.get(i);
			String[] keys = context.keys();
			//for(int j = 0; j < keys.getSize(); j = j + 1) {
			int j = 0;
			while(j < keys.getSize()) {
				json.setString(keys[j],context.getString(keys[j]));
				j = j + 1;
			}
			i = i + 1;
		}
		return json;
	}

	@Public int AddContext(Json context) {
		//for(int i = 0; i < this.children.getSize();i = i + 1) {
		int i = 0;
		while(i < this.children.getSize()) {
			if(this.children[i].toJson().getString("type") == "Context") { //FIXME instanceOf
				Context c = this.children[i] as Context;
				return c.AddContext(context);
			}
			i = i + 1;
		}
		return -1;
		//FIXME if there is no Context, add new Context(not here?)
	}
}

class Strategy extends DBNode {
	Strategy(){}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Strategy");
		return json;
	}

	//FIXME validation
	@Public Json GetContext() {
		Json json = this.parent.GetContext();
		Json context_array = new Json([]);
		//for(int i = 0; i < this.children.getSize();i = i + 1) {
		int i = 0;
		while(i < this.children.getSize()) {
			if(this.children[i].toJson().getString("type") == "Context") { //FIXME instanceOf
				Context c = this.children[i] as Context;
				context_array.add(c.GetContext());
			}
			i = i + 1;
		}
		//for(int i = 0; i < context_array.getSize();i = i + 1) {
		i = 0;
		while(i < context_array.getSize()) {
			Json context = context_array.get(i);
			String[] keys = context.keys();
			//for(int j = 0; j < keys.getSize(); j = j + 1) {
			int j = 0;
			while(j < keys.getSize()) {
				json.setString(keys[j],context.getString(keys[j]));
				j = j + 1;
			}
			i = i + 1;
		}
		return json;
	}

	@Public void AddContext(Json context) {
		//for(int i = 0; i < this.children.getSize();i = i + 1) {
		int i = 0;
		while(i < this.children.getSize()) {
			if(this.children[i].toJson().getString("type") == "Context") { //FIXME instanceOf
				Context c = this.children[i] as Context;
				return c.AddContext(context);
			}
			i = i + 1;
		}
		return -1;
		//FIXME if there is no Context, add new Context(not here?)
	}
}

class Evidence extends DBNode {
	Evidence() {}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Evidence");
		return json;
	}

	@Public Json GetContext(){
		return this.parent.GetContext();
	}
}

//TODO
class Justification extends DBNode {
	Justification(){}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Justification");
		return json;
	}
}

//TODO
class Rebuttal extends DBNode {
	Rebuttal(){}

	@Public Json toJson() {
		Json json = this.PrivateToJson();
		json.setString("type","Rebuttal");
		return json;
	}
}

class Commit {
	String method;
	Json   args;
	String revision;
	int time;

	Commit() {
	}

	Commit(String method, Json args, String revision) {
		this.method   = method;
		this.args     = args;
		this.revision = revision;
		this.time     = new Date().getTime();
	}

	Json toJson() {
		Json json = new Json();
		json.setString("method",this.method);
		json.set("args",this.args);
		json.setString("revision",this.revision);
		json.setInt("time",this.time);
		return json;
	}
}

class CommitLog {
	Commit[] commits;
	String head;
	SHA1 sha1;

	CommitLog() {
		this.commits = [];
		this.sha1 = new SHA1();
		this.sha1.update(new Date().toString());
	}

	Commit addCommit(String method, Json argument) {
		Commit commit = new Commit(method, argument, this.sha1.final());
		this.commits.add(commit);
		this.head = commit.revision;
		return commit;
	}

	Json toJson() {
		Json json = new Json([]);
		for(int i = 0;i < this.commits.getSize(); i = i + 1) {
			json.add(this.commits[i].toJson());
		}
		return json;
	}

}

class Branch {
	int branch_id;
	int argument_id;
	DBNode root;
	DBNode[] nodes;

	Branch() {
		this.root = NULL;
		this.nodes = [];
	}

	Branch(DBNode[] nodes, DBNode root) {
		this.nodes = nodes;
		this.root = root;
	}

	DBNode GetDBNode(int node_id) {
		if(this.root.node_id == node_id) {
			return this.root;
		}
		//for(int i = 0; i < this.nodes.getSize(); i = i + 1) {
		int i = 0;
		while(i < this.nodes.getSize()) {
			if(this.nodes[i].node_id == node_id) {
				return this.nodes[i];
			}
			i = i + 1;
		}
		return NULL;
	}

	Json DBNodeListToJson() {
		Json json = new Json([]);
		int max = this.nodes.getSize();
		//for(int i = 0; i < max; i = i + 1) {
		int i = 0;
		while(i < max) {
			json.add(this.nodes[i].toJson());
			i = i + 1;
		}
		return json;
	}

	Json PrivateArgumentTreeToJson(DBNode node) {
		Json json = new Json();
		json.setString("node_id",node.node_id.toString());
		Json children = new Json([]);
		//for(int i = 0; i < node.children.getSize(); i = i + 1) {
		int i = 0;
		int id = node.argument_id;
		while(i < node.children.getSize()) {
			if(node.children[i].argument_id != id) {
				Json json = new Json();
				json.setString("type","Argument");
				//json.setString("description",); //FIXME
				json.setString("argument_id",id.toString());
				children.add(json);
			}else {
				children.add(this.PrivateArgumentTreeToJson(node.children[i]));
			}
			i = i + 1;
		}
		json.set("children",children);
		return json;
	}

	Json ArgumentTreeToJson() {
		Json json = new Json();
		json.setString("node_id",this.root.node_id.toString());
		Json children = new Json([]);
		//for(int i = 0; i < this.root.children.getSize(); i = i + 1) {
		int i = 0;
		int id = this.root.argument_id;
		while(i < this.root.children.getSize()) {
			if(this.root.children[i].argument_id != id) {
				Json json = new Json();
				json.setString("type","Argument");
				//json.setString("description",); //FIXME
				json.setString("argument_id",id.toString());
				children.add(json);
			}else {
				children.add(this.PrivateArgumentTreeToJson(this.root.children[i]));
			}
			i = i + 1;
		}
		json.set("children",children);
		return json;
	}

	boolean AddNode(DBNode node) {
		//for(int i = 0; i < this.nodes.getSize(); i = i + 1) {
		int i = 0;
		while(i < this.nodes.getSize()) {
			if(node.node_id == this.nodes[i].node_id) {
				return false;
			}
			i = i + 1;
		}
		if(node.argument_id < 1) {
			this.nodes.add(node);
			node.argument_id = this.argument_id;
			return true;
		}
		return false;
	}

	void RemoveNode(DBNode node) {
		//for(int i = 0; i < this.nodes.getSize(); i = i + 1) {
		int i = 0;
		while(i < this.nodes.getSize()) {
			if(node.node_id == this.nodes[i].node_id) {
				this.nodes.removeAt(i);
				break;
			}
			i = i + 1;
		}
	}

	@Public Json toJson() {
		Json json = new Json();
		json.set("root",this.root.toJson());
		json.set("nodes",this.DBNodeListToJson());
		json.set("links",this.ArgumentTreeToJson());
		return json;
	}
}

class DCaseArgument {
	int argument_id;
	String name;
	String description;
	int masterBranchId;
	Map[Branch] branches;
	Map[CommitLog] commits;

	@Public DCaseArgument(int masterId, Branch branch) {
		this.branches = new Map[Branch];
		this.masterBranchId =masterId;
		this.branches.set(masterId.toString(), branch);
		this.commits.set(masterId.toString(), new CommitLog());
	}

	Commit Commit(String method, Json argument) {
		//TODO
		//return this.commits.addCommit(method,argument);
	}

	@Public Branch GetBranch(int BranchId) {
		return this.branches.get(BranchId.toString());
	}

	@Public Branch Fork(String originId, String newId, Context ctx) {
		this.Fork(originId, newId, ctx, this.GetBranch(originId).commitLog.head);
	}

	@Public Branch Fork(String originalId, String newId, Context ctx, String revision) {
		return new Branch(); //FIXME
	}

	@Public DBNode GetDBNode(int node_id) {
		return this.GetDBNode(this.masterBranchId, node_id);
	}

	@Public DBNode GetDBNode(int branch_id, int node_id) {
		return this.GetBranch(branch_id).GetDBNode(node_id);
	}

	@Public void SetRoot(Goal g) {
		this.SetRoot(this.masterBranchId,g);
	}

	@Public void SetRoot(int branch_id, Goal g) {
		this.GetBranch(branch_id).root = g;
	}

	@Public DBNode GetRoot() {
		return this.GetBranch(this.masterBranchId).root;
	}

	@Public Json toJson(int branch_id) {
		Json json = this.GetBranch(branch_id).toJson();
		CommitLog clog = this.commits.get(""+branch_id);//FIXME wrapper
		json.set("CommitLog",clog.toJson());
		json.setString("HEAD",clog.head);
		json.setString("argument_id",this.argument_id.toString());
		json.setString("branch_id",branch_id.toString());
		return json;
	}

	@Public Json toJson() {
		return this.toJson(this.masterBranchId);
	}
}

class DBDriver {
	DBDriver(){}

	Json GetContext(int id) {}

	Json SearchContext(String text) {}

	DCaseArgument GetArgument(int argument_id) {}

	DCaseArgument CreateArgument() {}

	DCaseArgument CreateArgument(String description) {}

	void SetRoot(int argument_id, int topGoalId) {}

	DBNode CreateDBNode(String nodeType, Json content) {}

	//TODO
	//void InsertCommit(String argument_name, String method, Commit c) {}

	void InsertLink(int argument_id, int branch_id, int parentId, int childId) {}

	void AddContext(int id, Json context) {}

	void UpdateDBNode(DBNode node, int argument_id, int branch_id) {}

	void UpdateDBNode(DBNode node, String description) {}

	void UpdateDBNode(DBNode node, String description, boolean isEvidence) {}

	void UpdateDBNode(DBNode node, boolean isEvidence) {}

	Json GetArgumentIdList() {}

	int GetArgumentIdByName(String name) {}

	Json Search(String text) {}
	Json Search(String text,String nodeType) {}
	boolean RemoveNode(DBNode node) {}
}

class DriverMySQL extends DBDriver {
	Connection con;

	DBDriver(String userName, String password) {
		this.con = new Connection("mysql://" + userName + ":" + password + "@localhost:3306");
		this.con.query("USE dcasedb;");
	}

	Json GetContext(int id) {
		Json json = new Json();
		ResultSet r = this.con.query("SELECT context_key,value FROM Context WHERE node_id="+id);
		while(r.next()) {
			String key = r.getString("context_key");
			String value = r.getString("value");
			json.setString(key,value);
		}
		return json;
	}

	Json SearchContext(String text) {
		Json json = new Json([]);
		ResultSet r = this.con.query("SELECT DBNode.id as node_id,argument_id,branch_id,context_key,value FROM DBNode,Context WHERE DBNode.id=Context.node_id AND value LIKE '%"+text+"%'");
		while(r.next()) {
			Json j = new Json();
			j.setString("node_id",r.getInt("node_id").toString());
			j.setString("argument_id",r.getInt("argument_id").toString());
			j.setString("branch_id",r.getInt("branch_id").toString());
			j.setString("key",r.getString("context_key"));
			j.setString("value",r.getString("value"));
			json.add(j);
		}
		return json;
	}

	Json Search(String text, String nodeType) {
		Json json = new Json([]);
		//FIXME
		ResultSet r = this.con.query("SELECT DBNode.id,argument_id,branch_id,type_name,description FROM DBNode,NodeType WHERE nodeType_id=NodeType.id AND type_name='"+nodeType+"' AND description LIKE '%"+text+"%'");
		while(r.next()) {
			Json j = new Json();
			j.setString("node_id",r.getInt("id").toString());
			j.setString("argument_id",r.getInt("argument_id").toString());
			j.setString("branch_id",r.getInt("branch_id").toString());
			j.setString("type",r.getString("type_name"));
			j.setString("description",r.getString("description"));
			json.add(j);
		}
		return json;
	}

	Json Search(String text) {
		Json json = new Json([]);
		ResultSet r = this.con.query("SELECT DBNode.id,argument_id,branch_id,type_name,description FROM DBNode,NodeType WHERE nodeType_id=NodeType.id AND description LIKE '%"+text+"%'");
		while(r.next()) {
			Json j = new Json();
			j.setString("node_id",r.getInt("id").toString());
			j.setString("argument_id",r.getInt("argument_id").toString());
			j.setString("branch_id",r.getInt("branch_id").toString());
			j.setString("type",r.getString("node_type"));
			j.setString("description",r.getString("description"));
			json.add(j);
		}
		return json;
	}

	DBNode MakeNode(String NodeType, Json content) {
		if(NodeType == "Goal") {
			return new Goal(content);
		} else if (NodeType == "Strategy") {
			return new Strategy(content);
		} else if (NodeType == "Context") {
			return new Context(content);
		} else if (NodeType == "Evidence") {
			return new Evidence(content);
		}
		return null; //FIXME
	}

	DBNode PrivateGetNode(DBNode[] nodes,int id) {
		int i = 0;
		while(i<nodes.getSize()) {
			if(nodes[i].node_id == id) {
				return nodes[i];
			}
			i = i + 1;
		}
		return null;
	}

	Json GetDBNodeById(int nodeId) {
		ResultSet r = this.con.query("SELECT argument_id,branch_id FROM DBNode WHERE id="+ nodeId +"");
		r.next();
		Json json = new Json();
		json.setInt("argument_id",r.getInt("argument_id"));
		json.setInt("branch_id",r.getInt("branch_id"));
		return json;
	}

	DBNode[] GetLinks(DBNode[] nodes) {
		int i = 0;
		while(i<nodes.getSize()) {
			ResultSet r = this.con.query("SELECT parent_Node_id FROM NodeLink WHERE child_Node_id="+nodes[i].node_id+"");
			while(r.next()){
				int id = r.getInt("parent_Node_id");
				nodes[i].parent = this.PrivateGetNode(nodes,id);
				if(nodes[i].parent == null) {
					Json json = this.GetDBNodeById(id);
					nodes[i].lazy_parent = json;
				}
			}

			ResultSet r1 = this.con.query("SELECT child_Node_id FROM NodeLink WHERE parent_Node_id="+nodes[i].node_id+"");
			while(r1.next()){
				int id = r1.getInt("child_Node_id");
				DBNode node = this.PrivateGetNode(nodes,id);
				if(node!=null) {
					nodes[i].children.add(this.PrivateGetNode(nodes,id));
				}else{
					Json json = this.GetDBNodeById(id);
					nodes[i].lazy_children.add(json);
				}
			}
			i = i + 1;
		}
		return nodes;
	}

	DBNode[] GetDBNodeList(int argument_id, int branch_id) {
		DBNode[] nodes = [];
		ResultSet r = this.con.query("SELECT DBNode.id,description,evidence_flag,type_name FROM DBNode,NodeType WHERE argument_id="+argument_id+" AND branch_id="+ branch_id +" AND nodeType_id=NodeType.id;");
		while(r.next()) {
			Json json = new Json();
			json.setString("description",r.getString("description"));
			String nodeType = r.getString("type_name");
			json.setString("type",nodeType);
			json.setBoolean("isEvidence", r.getBoolean("evidence_flag"));
			DBNode node = this.MakeNode(nodeType,json);
			node.node_id     = r.getInt("id");
			node.argument_id = argument_id;
			node.branch_id   = branch_id;
			if(nodeType == "Context") {
				Context context = node as Context;
				context.context = this.getContext(context.node_id);
				nodes.add(context);
			} else {
				nodes.add(node);
			}
		}
		return this.GetLinks(nodes);
	}

	DCaseArgument GetArgument(int argument_id,int branch_id) {
		//FIXME branch
		ResultSet r = this.con.query("SELECT goal_id,master_branch_id FROM Argument WHERE id="+ argument_id +"");
		r.next();
		int goalId = r.getInt("goal_id");
		int master_branch_id = r.getInt("master_branch_id");

		DBNode[] nodes = this.GetDBNodeList(argument_id,branch_id);
		DBNode   root  = this.PrivateGetNode(nodes,goalId);
		Branch branch = new Branch(nodes,root);
		DCaseArgument argument = new DCaseArgument(branch_id,branch);
		return argument;
	}

	//FIXME
	int GetArgumentIdByName(String name) {
		ResultSet r = this.con.query("SELECT id FROM Argument WHERE description LIKE '%"+ name +"%'");
		r.next();
		return r.getInt("id");
	}

	DCaseArgument GetArgument(int argument_id) {
		ResultSet r = this.con.query("SELECT goal_id,master_branch_id FROM Argument WHERE id="+ argument_id +"");
		r.next();
		int goalId = r.getInt("goal_id");
		int branch_id = r.getInt("master_branch_id");

		DBNode[] nodes = this.GetDBNodeList(argument_id,branch_id);
		DBNode   root  = this.PrivateGetNode(nodes,goalId);
		Branch branch = new Branch(nodes,root);
		DCaseArgument argument = new DCaseArgument(branch_id,branch);
		return argument;
	}

	Branch CreateBranch() {
		this.con.query("INSERT INTO Branch() VALUES()");
		Branch branch = new Branch();
		branch.branch_id = this.con.getInsertId();
		return branch;
	}

	void UpdateBranch(Branch branch, int argument_id) {
		this.con.query("UPDATE Branch SET Argument_id=" + argument_id + " WHERE id=" + branch.branch_id);
		branch.argument_id = argument_id;
	}

	void SetRoot(int argument_id, int topGoalId) {
		this.con.query("UPDATE Argument SET goal_id="+topGoalId+" WHERE id="+argument_id);
	}

	DCaseArgument CreateArgument() {
		Branch branch = this.CreateBranch();
		this.con.query("INSERT INTO Argument(master_branch_id) VALUES("+branch.branch_id+")");
		DCaseArgument argument = new DCaseArgument(branch.branch_id, branch);
		argument.argument_id = this.con.getInsertId();
		this.UpdateBranch(branch,argument.argument_id);
		return argument;
	}

	DCaseArgument CreateArgument(String description) {
		Branch branch = this.CreateBranch();
		this.con.query("INSERT INTO Argument(description,master_branch_id) VALUES('"+description+"',"+branch.branch_id+")");
		DCaseArgument argument = new DCaseArgument(branch.branch_id, branch);
		argument.argument_id = this.con.getInsertId();
		argument.description = description;
		this.UpdateBranch(branch,argument.argument_id);
		return argument;
	}

	int GetNodeType(String nodeType) {
		ResultSet nodeTypeId = this.con.query("SELECT id FROM NodeType WHERE type_name='" + nodeType + "'");
		int typeId;
		while(nodeTypeId.next()) {
			typeId = nodeTypeId.getInt("id");
		}
		return typeId;
	}

	DBNode CreateDBNode(String nodeType, Json content) {
		DBNode node = this.MakeNode(nodeType,content);
		int typeId = this.GetNodeType(nodeType);

		String description = content.getString("description");
		boolean isEvidence = content.getBoolean("isEvidence");
		String evidence = "FALSE";
		if(isEvidence) {
			evidence = "TRUE";
		}
		this.con.query("INSERT INTO DBNode(description,evidence_flag,nodeType_id) VALUES('"+ description + "'," + evidence + ","+typeId+") ");
		node.node_id = this.con.getInsertId();
		return node;
	}

	void UpdateDBNode(DBNode node, int argument_id, int branch_id) {
		this.con.query("UPDATE DBNode SET argument_id="+argument_id+", branch_id="+branch_id+" WHERE id="+node.node_id);
		node.argument_id = argument_id;
		node.branch_id   = branch_id;
	}

	void UpdateDBNode(DBNode node, String description) {
		this.con.query("UPDATE DBNode SET description='"+description+"' WHERE id="+node.node_id);
	}

	void UpdateDBNode(DBNode node, String description, boolean isEvidence) {
		String evidence = "FALSE";
		if(isEvidence){
			evidence = "TRUE";
		}
		this.con.query("UPDATE DBNode SET description='"+description+"', evidence_flag="+evidence +" WHERE id="+node.node_id);
	}

	void UpdateDBNode(DBNode node, boolean isEvidence) {
		String evidence = "FALSE";
		if(isEvidence) {
			evidence = "TRUE";
		}
		this.con.query("UPDATE DBNode SET evidence_flag="+evidence +" WHERE id="+node.node_id);
	}

	int GetDBNodeByName(String name) {
		ResultSet r = this.con.query("SELECT id FROM DBNode WHERE name='"+ name +"'");
		r.next();
		return r.getInt("id");
	}

	void InsertLink(int argument_id, int branch_id, int parentId, int childId) {
		this.con.query("INSERT INTO NodeLink(argument_id,parent_Node_id,child_Node_id,branch_id) VALUES("+argument_id+","+parentId+","+childId+","+branch_id+")");
	}

	Json GetArgumentIdList() {
		ResultSet r = this.con.query("SELECT id,description FROM Argument");
		Json retVal = new Json([]);
		while(r.next()){
			Json json = new Json();
			json.setString("argument_id",r.getInt("id").toString());
			json.setString("description",r.getString("description"));
			retVal.add(json);
		}
		return retVal;
	}

	void AddContext(int id, Json context) {
		int i = 0;
		String[] keys = context.keys();
		while(i<keys.getSize()) {
			String key = keys[i];
			this.con.query("INSERT INTO Context(context_key,value,node_id) Values('"+ key +"','" +context.getString(key)+ "'," +id+ ")");
			i = i + 1;
		}
	}

	boolean RemoveNode(DBNode node) {
		this.con.query("DELETE FROM NodeLink WHERE parent_Node_id="+node.node_id+" OR child_Node_id="+node.node_id);
		this.con.query("DELETE FROM DBNode WHERE id="+node.node_id);
		return true; //TODO
	}
}

class API {
	Map[DCaseArgument] arguments;
	DBDriver db;

	@Public API() {
		arguments = new Map[DCaseArgument];
	}

	@Public API(String userName, String password) {
		arguments = new Map[DCaseArgument];
		this.Connect(userName, password);
	}

	@Public void Connect(String userName, String password) {
		this.db = new DriverMySQL(userName, password);
	}

	@Public DCaseArgument CreateArgument(String description) {
		Json content = new Json();
		content.setString("description",description);
		content.setString("type","Goal");

		DCaseArgument argument = this.db.CreateArgument(description);
		this.arguments.set(argument.argument_id.toString(), argument);

		Goal g = this.CreateDBNode(content) as Goal;
		argument.SetRoot(g);
		this.db.SetRoot(argument.argument_id,g.node_id);

		Json context_json = new Json();
		context_json.setString("type","Context");
		Context c = this.CreateDBNode(context_json) as Context;
		this.InsertLink(argument.argument_id, g, c);
		this.db.UpdateDBNode(c,argument.argument_id,argument.masterBranchId);
		argument.GetBranch(argument.masterBranchId).nodes.add(c);
		return argument;
	}

	@Public DCaseArgument GetArgument(int argument_id) {
		String argId = ""+argument_id;
		if(this.arguments.has(argId)) {
			return this.arguments.get(argId);
		}else{
			DCaseArgument argument = this.db.GetArgument(argument_id);
			this.arguments.set(argId,argument);
			return argument;
		}
	}

	@Public Json GetArgumentIds() {
		return this.db.GetArgumentIdList();
	}

	@Public DBNode[] SearchDBNode(String searchText) {
		return []; //FIXME
	}

	@Public DBNode CreateDBNode(Json content) {
		String NodeType = content.getString("type");
		DBNode node = this.db.CreateDBNode(NodeType,content);
		if(NodeType == "Context" && content.hasKey("properties")) {
			this.db.AddContext(node.node_id,content.get("properties"));
		}
		return node;
	}

	@Public void InsertLink(int id, DBNode parent, DBNode child) {
		int masterId = this.GetArgument(id).masterBranchId;
		this.InsertLink(id, masterId, parent, child);
	}

	@Public void InsertLink(int id, int branch_id, DBNode parent, DBNode child) {
		Branch branch = this.GetArgument(id).GetBranch(branch_id);
		if(branch.AddNode(parent)){
			this.db.UpdateDBNode(parent,id,branch_id);
		}
		if(branch.AddNode(child)){
			this.db.UpdateDBNode(child,id,branch_id);
		}
		parent.addChild(child);
		this.db.InsertLink(id,branch_id,parent.node_id,child.node_id);
		//TODO Commit
	}

	//FIXME return boolean
	//FIXME Json contents
	@Public boolean UpdateDBNode(int argument_id, DBNode node, Json contents) {
		int masterId = this.GetArgument(argument_id).masterBranchId;
		return this.UpdateDBNode(argument_id, masterId, node, contents);
	}

	@Public boolean UpdateDBNode(int argument_id, int branch_id, DBNode node, Json contents) {
		node.Update(contents);
		if(contents.hasKey("description")) {
			this.db.UpdateDBNode(node, contents.getString("description"));
		}
		if(contents.hasKey("isEvidence")) {
			this.db.UpdateDBNode(node, contents.getBoolean("isEvidence"));
		}
		return true;
	}

	@Public void UpdateParentLink(DBNode oldParent, DBNode newParent, DBNode node) {
		oldParent.RemoveChild(node);
		newParent.AddChild(node);
	}

	@Public void UpdateChildLink(DBNode node, DBNode oldChild, DBNode newChild) {
		node.RemoveChild(oldChild);
		node.AddChild(newChild);
	}

	@Public boolean DeleteDBNode(int argument_id, int node_id) {
		int masterId = this.GetArgument(argument_id).masterBranchId;
		return this.DeleteDBNode(argument_id, masterId, node_id);
	}

	@Public boolean DeleteDBNode(int argument_id, int branch_id, int node_id) {
		Branch branch = this.GetArgument(argument_id).GetBranch(branch_id);
		DBNode node = branch.GetDBNode(node_id);
		branch.RemoveNode(node);
		return this.db.RemoveNode(node);
	}

	@Public void DeleteLink(String argument_id, String parentId, String childId) {
		this.DeleteLink(argument_id, "master", parentId, childId);
	}

	@Public void DeleteLink(String argument_id, String branch_id, String parentId, String childId) {
		//TODO
	}

	@Public void AddContext(int argument_id, int node_id, Json context) {
		int branchid = this.GetArgument(argument_id).masterBranchId;
		this.AddContext(argument_id, branchid, node_id, context);
	}

	@Public void AddContext(int argument_id, int branch_id, int node_id, Json context) {
		DBNode node = this.GetArgument(argument_id).GetBranch(branch_id).GetDBNode(node_id);
		int id = node.addContext(context);
		//TODO Add new Context
		this.db.AddContext(id,context);
	}

	@Public Json GetContext(DBNode node) {
		return node.GetContext();
	}

	@Public Json Search(String text) {
		return this.db.Search(text);
	}

	@Public Json Search(String text,String nodeType) {
		return this.db.Search(text,nodeType);
	}

	@Public Json SearchContext(String text) {
		return this.db.SearchContext(text);
	}
}
