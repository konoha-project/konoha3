//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");
import("konoha.foreach");
import("JavaScript.Array");
import("cstyle");
import("Lib.Libvirt");

void main() {
	Libvirt l = new Libvirt("qemu:///system");
	Domain d = l.lookUpDomain("Ubuntu");
	assert(d.shutdown()==0);
}

main();
