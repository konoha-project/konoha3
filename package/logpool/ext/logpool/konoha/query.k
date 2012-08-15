K.import("logpool");
K.import("sugar");
K.import("konoha.assignment");

PoolPlugin initPlugin(Object ev) {
    PoolPlugin p0 = Printer.create();
    PoolPlugin p1 = Response.create(ev);
    p0.apply(p1);
    return p0;
}
