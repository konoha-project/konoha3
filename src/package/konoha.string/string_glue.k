/*
 * Konoha String Package
 * written by imasahiro
 */

@Public String[] String.split() {
    /* XXX(ide)
     * 'new' and '[]' expression is not defined in string package.
     * To remove package dependency, we use stupid way to allocate
     * Array[String] object. */
    String[] ret = this.split(this);
    ret.set(0, this);
    return ret;
}

@Public String[] String.split(String regex) {
    return this.split(regex, -1);
}

@Public String String.replace(String oldText, String newText) {
    return this.replaceAll(oldText, newText);
}

@Public int String.search(String str) {
    return this.indexOf(str);
}

@Public String String.toLower() {
    return this.toLowerCase("UTF-8");
}

@Public String String.toUpper() {
    return this.toUpperCase("UTF-8");
}

@Public String String.valueOf() {
    return this;
}

@Public int String.getSize() {
    return this.length();
}
