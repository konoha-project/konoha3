import("MiniKonoha.Sql");
import("Syntax.CStyleWhile");
import("Type.File");

void main() {
	Connection con = new Connection("sqlite://" + FILE.scriptPath("./konoha.sqlite"));
	ResultSet r = con.query("SELECT * FROM people");
	while(r.next()) {
		System.p(" ----------------------- ");
		System.p("name:\t\t" + r.getString("name"));
		System.p("age:\t\t" + r.getInt("age"));
		System.p("height:\t" + r.getFloat("height"));
		System.p("village:\t" + r.getString("village"));
	}
	System.p(" ----------------------- ");
	con.close();
}

main();
