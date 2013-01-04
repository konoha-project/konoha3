import("konoha.new");
import("konoha.class");
import("konoha.foreach");
import("JavaScript.Array");
import("cstyle");
import("libvirt");

void main() {
	Libvirt l = new Libvirt("qemu:///system");
	Domain d = l.lookUpDomain("Ubuntu");
	assert(d.shutdown()==0);
}

main();
