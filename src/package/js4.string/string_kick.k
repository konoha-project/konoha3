import("java2.string");
import("konoha.regexp");

@Public String[] String.split() {
    /* XXX(ide)
     * 'new' and '[]' expression is not defined in string package.
     * To remove package dependency, we use stupid way to allocate
     * Array[String] object. */
    String[] ret = this.split(this);
    ret.set(0, this);
    return ret;
}

@Public int String.search(String str) {
    return this.indexOf(str);
}

@Public String String.slice(int beginIndex, int endIndex) {
    if (beginIndex < 0) {
        beginIndex = beginIndex + this.length();
    }
    if (endIndex < 0) {
        endIndex = endIndex + this.length();
    }
    if (beginIndex > endIndex) {
        return "";
    } else {
        return this.substring(beginIndex, endIndex);
    }
}

@Public String String.slice(int index) {
    return this.slice(index, this.length());
}

@Public String String.substr(int index, int length) {
    assert(length >= 0);
    if (index >= 0) {
        return this.substring(index, index + length);
    } else {
        int beginIndex = this.length() + index;
        return this.substring(beginIndex, beginIndex + length);
    }
}

@Public String String.substr(int index) {
    if (index >= 0) {
        return this.substring(index, this.length());
    } else {
        return this.substring(this.length() + index, this.length());
    }
}

@Public int String.localeCompare(String that) {
    /*FIXME(ide) to care about locale */
    return this.compareTo(that);
}

@Public String String.valueOf() {
    return this;
}
