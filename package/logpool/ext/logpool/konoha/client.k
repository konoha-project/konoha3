import("konoha");
import("konoha.assignment");
import("cstyle"); // use cstyle
import("cstyle"); // konoha.null is obsolate; use cstyle
import("logpool");

void main() {
    LogPool lp = new LogPool("0.0.0.0", 14801);
    lp.loadFile("dump", "query.k");
    while (true) {
        Log log = lp.get();
        if(log.isNull()) {
            break;
        }
        System.p(log);
    }
}

main();
