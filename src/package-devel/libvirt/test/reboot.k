import("konoha.new");
import("konoha.class");
import("konoha.foreach");
import("js4.array");
import("cstyle");
import("libvirt");

void main() {
	Libvirt l = new Libvirt("qemu:///system");
	Domain d = l.lookUpDomain("Ubuntu");
	assert(d.reboot(VIR_DOMAIN_REBOOT_DEFAULT)==0);
}

main();
