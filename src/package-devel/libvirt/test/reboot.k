//import("konoha.new"); Don't use this
import("Syntax.JavaStyleClass");
import("konoha.foreach");
import("JavaScript.Array");
import("cstyle");
import("Lib.Libvirt");

void main() {
	Libvirt l = new Libvirt("qemu:///system");
	Domain d = l.lookUpDomain("Ubuntu");
	assert(d.reboot(VIR_DOMAIN_REBOOT_DEFAULT)==0);
}

main();
