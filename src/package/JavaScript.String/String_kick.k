import("Common.StringUtil");
import("JavaScript.Regexp");

/* 15.5.4.2 String.prototype.toString() */
@Public @Override String String.toString() { return this; }

/* 15.5.4.3 String.prototype.valueOf() */
@Public @Override String String.valueOf() { return this; }

/* 15.5.4.4 String.prototype.charAt (pos) */
@Public @Override String String.charAt(int pos) {
    return StringUtil.charAt(this, pos);
}

/* 15.5.4.5 int.prototype.charCodeAt (pos) */
@Public @Override int String.charCodeAt(int pos) {
    return StringUtil.charCodeAt(this, pos);
}

/* 15.5.4.6 String.prototype.concat ( [ string1 [ , string2 [ , ... ] ] ] ) */
@Public @Override String String.concat() {
    return this;
}
@Public @Override String String.concat(String string1) {
    return StringUtil.concat(this, string1);
}
//@Public @Override String String.concat(String string1, String string2) {
//    return StringUtil.concat(StringUtil.concat(this, string1), string2);
//}
@Public @Override String String.concat(String string1, String string2, String string3) {
    return StringUtil.concat(StringUtil.concat(StringUtil.concat(this, string1), string2), string3);
}

/* 15.5.4.7 String.prototype.indexOf (searchString, position) */
@Public @Override int String.indexOf(String searchString) {
    return this.indexOf(searchString, 0);
}

@Public @Override int String.indexOf(String searchString, int position) {
    return StringUtil.indexOf(this, searchString, position);
}

/* 15.5.4.8 String.prototype.lastIndexOf (searchString, position) */
@Public @Override int String.lastIndexOf(String searchString) {
    return this.lastIndexOf(searchString, this.getlength());
}

@Public @Override int String.lastIndexOf(String searchString, int position) {
    return StringUtil.lastIndexOf(this, searchString, position);
}

/* 15.5.4.9 String.prototype.localeCompare (that) */
@Public @Override int String.localeCompare(String that) {
    return StringUtil.compareTo(this, that);
}

/* 15.5.4.10 String.prototype.match (regexp) */
/* Regexp version is defined at JavaScript.Regexp package */
@Public @Override String[] String.match(String that) {
    return StringUtil.match(this, that);
}

/* 15.5.4.11 String.prototype.replace (searchValue, replaceValue) */
/* Regexp version is defined at JavaScript.Regexp package */
@Public @Override String String.replace(String searchValue, String replaceValue) {
    return StringUtil.replaceAll(this, searchValue, replaceValue);
}

/* 15.5.4.12 String.prototype.search (regexp) */
/* Regexp version is defined at JavaScript.Regexp package */
@Public @Override int String.search(String regexp) {
    return this.search(new RegExp(regexp));
}

/* 15.5.4.13 String.prototype.slice (start, end) */
@Public @Override String String.slice(int beginIndex, int endIndex) {
    int len = this.getlength();
    if (beginIndex < 0) {
        beginIndex = beginIndex + len;
    }
    if (endIndex < 0) {
        endIndex = endIndex + len;
    }
    if (beginIndex > endIndex) {
        return "";
    } else {
        return this.substring(beginIndex, endIndex);
    }
}

@Public @Override String String.slice(int index) {
    return this.slice(index, this.getlength());
}

/* 15.5.4.14 String.prototype.split (separator, limit) */
/* Regexp version is defined at JavaScript.Regexp package */
@Public @Override String[] String.split(String regex) {
    return this.split(regex, -1);
}

@Public @Override String[] String.split(String regex, int limit) {
    return StringUtil.split(this, regex, limit);
}

@Public @Override String[] String.split() {
    /* XXX(ide)
     * 'new' and '[]' expression is not defined in string package.
     * To remove package dependency, we use stupid way to allocate
     * Array[String] object. */
    String[] ret = this.split(this);
    ret.set(0, this);
    return ret;
}

/* 15.5.4.15 String.prototype.substring (start, end) */
@Public @Override String String.substring(int beginIndex) {
    return this.substring(beginIndex, this.getlength());
}

@Override @Public @Override String String.substring(int beginIndex, int endIndex) {
    return StringUtil.substring(this, beginIndex, endIndex);
}

/* 15.5.4.16 String.prototype.toLowerCase ( ) */
@Public @Override String String.toLowerCase() {
    return StringUtil.toLowerCase(this, "UTF-8");
}

/* 15.5.4.17 String.prototype.toLocaleLowerCase ( ) */
@Public @Override String String.toLocaleLowerCase() {
    return StringUtil.toLowerCase(this, "UTF-8");
}

/* 15.5.4.18 String.prototype.toUpperCase ( ) */
@Public @Override String String.toUpperCase() {
    return StringUtil.toUpperCase(this, "UTF-8");
}

/* 15.5.4.19 String.prototype.toLocaleUpperCase ( ) */
@Public @Override String String.toLocaleUpperCase() {
    return StringUtil.toLowerCase(this, "UTF-8");
}

/* 15.5.4.20 String.prototype.trim ( ) */
@Override @Public String String.trim() {
    return StringUtil.trim(this);
}

/* 15.5.5.1 length */
@Public @Override int String.getlength() {
    return StringUtil.getlength(this);
}

@Public @Override String String.get(int pos) {
    return StringUtil.getString(this, pos);
}

@Public @Override @Static String String.fromCharCode(int pos) {
    return StringUtil.fromCharCode(pos);
}

/*FIXME String.substr() is not ECMA-262 */
@Public @Override String String.substr(int index) {
    if (index >= 0) {
        return this.substring(index, this.getlength());
    } else {
        return this.substring(this.getlength() + index, this.getlength());
    }
}

@Public @Override String String.substr(int index, int length) {
    assert(length >= 0);
    if (index >= 0) {
        return this.substring(index, index + length);
    } else {
        int beginIndex = this.getlength() + index;
        return this.substring(beginIndex, beginIndex + length);
    }
}
