Import("Deos.DCaseCloud");
Import("JavaStyle.Object");
Import("Type.Json");

Strategy makeStrategy(API repo, int argumentId, String description) {
	Json json = new Json();
	json.setString("description",description);
	json.setString("type","Strategy");
	return repo.CreateDBNode(json) as Strategy;
}

Goal makeGoal(API repo, int argumentId, String description) {
	Json json = new Json();
	json.setString("description",description);
	json.setString("type","Goal");
	return repo.CreateDBNode(json) as Goal;
}

Evidence makeEvidence(API repo, int argumentId, String description) {
	Json json = new Json();
	json.setString("description",description);
	json.setString("type","Evidence");
	return repo.CreateDBNode(json) as Evidence;
}

Context makeContext(API repo, int argumentId, Json context) {
	Json json = new Json();
	json.set("properties",context);
	json.setString("type","Context");
	return repo.CreateDBNode(json) as Context;
}

void GenerateTree1(API repo) {
	DCaseArgument argument = repo.CreateArgument("reboot must work");
	int id = argument.argument_id;

	DBNode root = repo.GetArgument(id).GetRoot();

	Json json = new Json();
	json.setString("IP","192.168.0.10");
	json.setString("PORT","1234");
	json.setString("DSE1","0.0.0.0:8080");
	json.setString("DSE2","0.0.0.0:8081");
	repo.AddContext(id, root.node_id, json);

	Evidence e2_1 = makeEvidence(repo, id, "D-Script:sudo shutdown -r now");
	repo.InsertLink(id, root, e2_1);

	System.p(repo.GetArgument(id).toJson());
	System.p(repo.GetArgumentIds());
}

void GenerateTree2(API repo) {
	DCaseArgument argument = repo.CreateArgument("D-Script must work");
	int id = argument.argument_id;

	DBNode root = repo.GetArgument(id).GetRoot();

	Json json = new Json();
	json.setString("D-Script","sudo shutdown -r now");
	repo.AddContext(id, root.node_id, json);

	Strategy s1 = makeStrategy(repo, id, "argument by precondition/postcondition");
	repo.InsertLink(id, root, s1);

	Goal g2_1 = makeGoal(repo, id, "web server must be alive before the fact");
	repo.InsertLink(id, s1, g2_1);

	Json ctx = new Json();
	ctx.setString("DSE2","0.0.0.0:8081");
	Context c2_1 = makeContext(repo, id, ctx);
	repo.InsertLink(id, g2_1, c2_1);

	Goal g2_2 = makeGoal(repo, id, "web server must be alive after the fact");
	repo.InsertLink(id, s1, g2_2);

	Json ctx2 = new Json();
	ctx2.setString("DSE2","0.0.0.0:8081");
	Context c2_2 = makeContext(repo, id, ctx2);
	repo.InsertLink(id, g2_2, c2_2);

	Evidence e3_1 = makeEvidence(repo, id, "D-Script:telnet @{IP} @{PORT}");
	repo.InsertLink(id, g2_1, e3_1);
	repo.InsertLink(id, g2_2, e3_1);

	System.p(repo.GetArgument(id).toJson());
	System.p(repo.GetArgumentIds());
}

//void GenerateTree3(API repo) {
//	DCaseArgument argument = repo.CreateArgument("D-Script must work");
//	int id = argument.argument_id;
//
//	DBNode root = repo.GetArgument(id).GetRoot();
//
//	Json json = new Json()
//	json.setString("D-Script","echo monitor start");
//	repo.AddContext(id, root.node_id, json);
//
//	Strategy s1 = makeStrategy(repo, id, "argument by pre condition");
//	repo.InsertLink(id, root, s1);
//
//	Goal g2_1 = makeGoal(repo, id, "Load balancer must be started");
//	repo.InsertLink(id, s1, g2_1);
//
//	Goal g2_2 = makeGoal(repo, id, "Web server must be started");
//	repo.InsertLink(id, s1, g2_2);
//
//	Goal g2_3 = makeGoal(repo, id, "Data storage must be started");
//	repo.InsertLink(id, s1, g2_3);
//
//	Evidence e3_1 = makeEvidence(repo, id, "D-Script:apachectl start");
//	repo.InsertLink(id, g2_1, e3_1);
//	repo.InsertLink(id, g2_2, e3_1);
//	Evidence e3_2 = makeEvidence(repo, id, "D-Script:service nfs start<br>/etc/init.d/mysqld start");
//	repo.InsertLink(id, g2_3, e3_2);
//
//	Strategy s2 = makeStrategy(repo, id, "argument by runtime condition");
//	repo.InsertLink(id, root, s2);
//
//	Goal g2_4 = makeGoal(repo, id, "Load balancer must be running");
//	repo.InsertLink(id, s2, g2_4);
//
//	Goal g2_5 = makeGoal(repo, id, "Web server must be running");
//	repo.InsertLink(id, s2, g2_5);
//
//	Goal g2_6 = makeGoal(repo, id, "Data storage must be running");
//	repo.InsertLink(id, s2, g2_6);
//
//	Evidence e3_3 = makeEvidence(repo, id, "D-Script:while(true) {<br>String res = $(wget --spider -nv --tries=@TryCount --timeout=@WgetTimeout @ServiceURL);<br>assert(res.endsWith(&quot;200 OK&quot;));<br>sleep @Interval;");
//	repo.InsertLink(id, g2_4, e3_3);
//	Evidence e3_4 = makeEvidence(repo, id, "D-Script:while(true) {<br>ps -ef | grep http<br>sleep @Interval;");
//	repo.InsertLink(id, g2_5, e3_4);
//	Evidence e3_5 = makeEvidence(repo, id, "D-Script:while(true) {<br>String res = $(echo &quot;show status&quot; | mysql -u @MysqlUser -h @DataStorageHost | grep Threads_connected | awk &quot;{print $2}&quot;)<br>assert(stat != &quot;0&quot;);<br>sleep @Interval;");
//	repo.InsertLink(id, g2_6, e3_5);
//
//	Strategy s3 = makeStrategy(repo, id, "argument by post condition");
//	repo.InsertLink(id, root, s3);
//
//	Goal g2_7 = makeGoal(repo, id, "Load balancer must be stopped");
//	repo.InsertLink(id, s3, g2_7);
//
//	Goal g2_8 = makeGoal(repo, id, "Web server must be stopped");
//	repo.InsertLink(id, s3, g2_8);
//
//	Goal g2_9 = makeGoal(repo, id, "Data storage must be stopped");
//	repo.InsertLink(id, s3, g2_9);
//
//	Evidence e3_6 = makeEvidence(repo, id, "D-Script:apachectl stop");
//	repo.InsertLink(id, g2_7, e3_6);
//	repo.InsertLink(id, g2_8, e3_6);
//	Evidence e3_7 = makeEvidence(repo, id, "D-Script:service nfs stop<br>/etc/init.d/mysqld stop");
//	repo.InsertLink(id, g2_9, e3_7);
//
//	System.p(repo.GetArgument(id).toJson());
//	System.p(repo.GetArgumentIds());
//}
//
//void GenerateTree4(API repo) {
//	DCaseArgument argument = repo.CreateArgument("D-Script must work");
//	int id = argument.argument_id;
//
//	DBNode root = repo.GetArgument(id).GetRoot();
//
//	Json json = new Json();
//	json.setString("D-Script","service nfs start<br>/etc/init.d/mysqld start");
//	repo.AddContext(id, root.node_id, json);
//
//	Strategy s1 = makeStrategy(repo, id, "argument by pre condition");
//	repo.InsertLink(id, root, s1);
//
//	Goal g2_1 = makeGoal(repo, id, "portmap service must be running");
//	repo.InsertLink(id, s1, g2_1);
//
//	Evidence e3_1 = makeEvidence(repo, id, "D-Script:service httpd status");
//	repo.InsertLink(id, g2_1, e3_1);
//
//	System.p(repo.GetArgument(id).toJson());
//	System.p(repo.GetArgumentIds());
//}
//
//void GenerateRootTree(API repo) {
//	DCaseArgument argument = repo.CreateArgument("Service must be dependable");
//	int id = argument.argument_id;
//
//	DBNode root = repo.GetArgument(id).GetRoot();
//
//	Json json = new Json();
//	json.setString("LoadBalancerDSE","0.0.0.0:8080");
//	json.setString("WebServerDSE","0.0.0.0:8081");
//	json.setString("DataStorageDSE","0.0.0.0:8082");
//	json.setString("MonitorDSE","0.0.0.0:8083");
//	repo.AddContext(id, root.node_id, json);
//
//	Strategy s1 = makeStrategy(repo, id, "argument by monitor / diagnosis / response");
//	repo.InsertLink(id, root, s1);
//
//	Goal g2_1 = makeGoal(repo, id, "Service must run");
//	repo.InsertLink(id, s1, g2_1);
//	Goal g2_2 = makeGoal(repo, id, "NFS must not have any faults");
//	repo.InsertLink(id, s1, g2_2);
//	Goal g2_3 = makeGoal(repo, id, "NFS must be recovered");
//	repo.InsertLink(id, s1, g2_3);
//
//	Evidence e3_1 = makeEvidence(repo, id, "D-Script:echo monitor start");
//	repo.InsertLink(id, g2_1, e3_1);
//	Evidence e3_2 = makeEvidence(repo, id, "D-Script:int count;<br>int startTime = System.getTime();<br>for(count = 0; count < (@CheckCount).toint(); count++) {<br>dd if=/dev/zero of=@NFSDirectory bs=@DiagnosisBlockSize count=@DiagnosisBlockCount;<br>}<br>int execTime = System.getTime() = startTime;<br>assert(execTime > (@NFSResponseLimit).toint());<br>");
//	repo.InsertLink(id, g2_2, e3_2);
//	Evidence e3_3 = makeEvidence(repo, id, "D-Script:/usr/sbin/exportfs -o rw,sync *:@NFSDirectory;");
//	repo.InsertLink(id, g2_3, e3_3);
//
//	System.p(repo.GetArgument(id).toJson());
//	System.p(repo.GetArgumentIds());
//}
//
//void GenerateTree5(API repo) {
//	DCaseArgument argument = repo.CreateArgument("D-Script must work");
//	int id = argument.argument_id;
//
//	DBNode root = repo.GetArgument(id).GetRoot();
//
//	Json json = new Json();
//	json.setString("D-Script","int count;<br>int startTime = System.getTime();<br>for(count = 0; count < (@CheckCount).toint(); count++) {<br>dd if=/dev/zero of=@NFSDirectory bs=@DiagnosisBlockSize count=@DiagnosisBlockCount;<br>}<br>int execTime = System.getTime() = startTime;<br>assert(execTime > (@NFSResponseLimit).toint());<br>");
//	repo.AddContext(id, root.node_id, json);
//
//	System.p(repo.GetArgument(id).toJson());
//	System.p(repo.GetArgumentIds());
//}
//
//void GenerateTree6(API repo) {
//	DCaseArgument argument = repo.CreateArgument("D-Script must work");
//	int id = argument.argument_id;
//
//	DBNode root = repo.GetArgument(id).GetRoot();
//
//	Json json = new Json();
//	json.setString("D-Script","/usr/sbin/exportfs -o rw,sync *:@NFSDirectory;");
//	json.setString("CheckCount","50");
//	json.setString("NFSDirectory","/etc/export");
//	json.setString("DiagnosisBlockSize","32768");
//	json.setString("DiagnosisBlockCount","10");
//	json.setString("NFSResponseLimit","5000");
//	repo.AddContext(id, root.node_id, json);
//
//	Strategy s1 = makeStrategy(repo, id, "argument by post condition");
//	repo.InsertLink(id, root, s1);
//
//	Goal g2_1 = makeGoal(repo, id, "NFS must be rebooted");
//	repo.InsertLink(id, s1, g2_1);
//
//	Evidence e3_1 = makeEvidence(repo, id, "D-Script:/etc/init.d/nfs restart");
//	repo.InsertLink(id, g2_1, e3_1);
//
//	System.p(repo.GetArgument(id).toJson());
//	System.p(repo.GetArgumentIds());
//}

void main() {
	API repo = new API("dcasedb","dcasedb");
	//GenerateRootTree(repo);
	GenerateTree1(repo);
	GenerateTree2(repo);
	//GenerateTree3(repo);
	//GenerateTree4(repo);
	//GenerateTree5(repo);
	//GenerateTree6(repo);
}

main();
