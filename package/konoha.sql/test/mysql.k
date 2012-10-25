import("konoha");
import("konoha.new");
import("konoha.float");
import("konoha.sql");
import("cstyle", "while");


void main() {
	String USER_NAME = "root"; // Enter Your MySQL UserName
	String PASSWD = "ybkkbhc"     // Enter Your MySQL Password
	Connection con = new Connection("mysql://" + USER_NAME + ":" + PASSWD + "@localhost:3306");
	ResultSet r = con.query("USE konoha");
	ResultSet r = con.query("SELECT * FROM people");
	while(r.next()) {
		System.p(" ----------------------- ");
		System.p("name:\t\t" + r.getString("name"));
		System.p("age:\t\t" + r.getInt("age"));
		System.p("height:\t\t" + r.getFloat("height"));
		System.p("village:\t" + r.getString("village"));
	}
	System.p(" ----------------------- ");
	con.close();
}

main();
