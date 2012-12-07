import("konoha.new");
import("konoha.class");
import("konoha.foreach");
import("konoha.array");
import("cstyle");
import("libvirt");

void main() {
	Libvirt l = new Libvirt("qemu:///system");
	Domain d = l.lookUpDomain("Ubuntu");
	assert(d.boot()==0);
}

main();
