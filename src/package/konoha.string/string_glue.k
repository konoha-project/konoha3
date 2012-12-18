/*
 * Konoha String Package
 * written by imasahiro
 * rewritten by kimio
 */

@Public boolean String.equals(String anObject) {
    if(this.hashCode() != anObject.hashCode()) {
        return false;
    }
    if(this.length() != anObject.length()) {
        return false;
    }
}

@Public boolean String.equalsIgnoreCase(String anObject) {
    return this.compareToIgnoreCase(anObject) == 0;
}

@Public boolean String.isEmpty() {
    return this.length() == 0;
}

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

@Public boolean String.startsWith(String prefix) {
    return this.startsWith(prefix, 0);
}

@Public String String.substring(int beginIndex) {
    return this.substring(beginIndex, this.length() - beginIndex);
}

@Public String String.replace(String oldText, String newText) {
    return this.replaceAll(oldText, newText);
}

@Public int String.indexOf(int ch) {
    return this.indexOf(ch, 0);
}

@Public int String.indexOf(String str) {
    return this.indexOf(str, 0);
}

@Public int String.search(String str) {
    return this.indexOf(str);
}

@Public int String.lastIndexOf(int ch) {
    return this.lastIndexOf(ch, this.length());
}

@Public int String.lastIndexOf(String str) {
    return this.lastIndexOf(str, this.length());
}

@Public boolean String.regionMatches(int toffset, String other, int ooffset, int len) {
    return this.regionMatches(toffset, true, other, ooffset, len);
}

@Public String String.toLowerCase() {
    return this.toLowerCase("UTF-8");
}

@Public String String.toUpperCase() {
    return this.toUpperCase("UTF-8");
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

@Public String String.slice(int beginIndex, int endIndex) {
    if(beginIndex < 0) {
        beginIndex = beginIndex + this.length();
    }
    if(endIndex < 0) {
        endIndex = endIndex + this.length();
    }
    if(beginIndex > endIndex) {
        return "";
    } else {
        return this.substring(beginIndex, endIndex);
    }
}

@Public String String.slice(int index) {
    return this.slice(index, this.length());
}

@Public boolean String.opHAS(String str) {
    return this.indexOf(str) >= 0;
}

@Public String String.substr(int index, int length) {
    assert(length >= 0);
    if(index >= 0) {
        return this.substring(index, index + length);
    } else {
        int beginIndex = this.length() + index;
        return this.substring(beginIndex, beginIndex + length);
    }
}

@Public String String.substr(int index) {
    if(index >= 0) {
        return this.substring(index, this.length());
    } else {
        return this.substring(this.length() + index, this.length());
    }
}
@Public int String.localeCompare(String that) {
    /*FIXME(ide) to care about locale */
    return this.compareTo(that);
}
