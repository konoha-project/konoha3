//written by wakamori
Import("JavaScript.Array");

void main() {
	String[] s = ["hoge"];
	s[0] = "fuga"; //Arrayへの代入がメソッドの最後に書かれているとNG
}

main();

