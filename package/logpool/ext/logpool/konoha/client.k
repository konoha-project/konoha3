K.import("konoha");
K.import("konoha.assignment");
K.import("konoha.while");
K.import("konoha.null");
K.import("logpool");

void main() {
    LogPool lp = new LogPool("0.0.0.0", 14801);
    lp.loadFile("dump", "query.k");
    while (true) {
        Log log = lp.get();
        if (log.isNull()) {
            break;
        }
        System.p(log);
    }
}

main();
