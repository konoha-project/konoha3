import("konoha.string");
import("konoha.float");
import("konoha.object");

//## @Public int String.codePointAt(int index)
//## Returns the char value at the specified index.
//@Native @Public int String.codePointAt(int index)

//## @Public int String.codePointBefore(int index)
//## Returns the character (Unicode code point) at the specified index.
//@Native @Public int String.codePointBefore(int index)

//## @Public int String.codePointCount(int beginIndex, int endIndex)
//## Returns the character (Unicode code point) before the specified index.
//@Native @Public int String.codePointCount(int beginIndex, int endIndex)

//## @Public int String.compareTo(String anotherString)
//## Returns the number of Unicode code points in the specified text range of this String.
//@Native @Public int String.compareTo(String anotherString)

//## @Public int String.compareToIgnoreCase(String str)
//## Compares two strings lexicographically.
//@Native @Public int String.compareToIgnoreCase(String str)

//## @Public String String.concat(String str)
//## Compares two strings lexicographically, ignoring case differences.
//@Native @Public String String.concat(String str)

////## @Public boolean String.contains(CharSequence s)
////## Concatenates the specified string to the end of this string.
//@Public boolean String.contains(CharSequence s) {
//}
//
////## @Public boolean String.contentEquals(CharSequence cs)
////## Returns true if and only if this string contains the specified sequence of char values.
//@Public boolean String.contentEquals(CharSequence cs) {
//}
//
////## @Public boolean String.contentEquals(StringBuffer sb)
////## Compares this string to the specified CharSequence.
//@Public boolean String.contentEquals(StringBuffer sb) {
//}
//
////## @Public @Static String String.copyValueOf(char[] data)
////## Compares this string to the specified StringBuffer.
//@Public @Static String String.copyValueOf(char[] data) {
//}
//
////## @Public @Static String String.copyValueOf(char[] data, int offset, int count)
////## Returns a String that represents the character sequence in the array specified.
//@Public @Static String String.copyValueOf(char[] data, int offset, int count) {
//}

//## @Public boolean String.endsWith(String suffix)
//## Returns a String that represents the character sequence in the array specified.
//@Native @Public boolean String.endsWith(String suffix)

//## @Public boolean String.equals(Object anObject)
//## Tests if this string ends with the specified suffix.
@Public boolean String.equals(String anObject) {
    if(this.hashCode() != anObject.hashCode()) {
        return false;
    }
    if(this.length() != anObject.length()) {
        return false;
    }
    return (this == anObject);
}

//## @Public boolean String.equalsIgnoreCase(String anotherString)
//## Compares this string to the specified object.
@Public boolean String.equalsIgnoreCase(String anotherString) {
    return this.compareToIgnoreCase(anObject) == 0;
}

////## @Public @Static String String.format(Locale l, String format, Object... args)
////## Compares this String to another String, ignoring case considerations.
//@Public @Static String String.format(Locale l, String format, Object... args) {
//}
//
////## @Public @Static String String.format(String format, Object... args)
////## Returns a formatted string using the specified locale, format string, and arguments.
//@Public @Static String String.format(String format, Object... args) {
//}
//
////## @Public byte[] String.getBytes()
////## Returns a formatted string using the specified format string and arguments.
//@Public byte[] String.getBytes() {
//}
//
////## @Public byte[] String.getBytes(Charset charset)
////## Encodes this String into a sequence of bytes using the platform's default charset, storing the result into a new byte array.
//@Public byte[] String.getBytes(Charset charset) {
//}
//
////## @Public void String.getBytes(int srcBegin, int srcEnd, byte[] dst, int dstBegin)
////## Encodes this String into a sequence of bytes using the given charset, storing the result into a new byte array.
//@Public void String.getBytes(int srcBegin, int srcEnd, byte[] dst, int dstBegin) {
//}
//
////## @Public byte[] String.getBytes(String charsetName)
////## This method does not properly convert characters into bytes. As of JDK 1.1, the preferred way to do this is via the getBytes() method, which uses the platform's default charset.
//@Public byte[] String.getBytes(String charsetName) {
//}
//
////## @Public void String.getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin)
////## Encodes this String into a sequence of bytes using the named charset, storing the result into a new byte array.
//@Public void String.getChars(int srcBegin, int srcEnd, char[] dst, int dstBegin) {
//}

//## @Public int String.hashCode()
//## Copies characters from this string into the destination character array.
//@Native @Public int String.hashCode()

//## @Public int String.indexOf(int ch)
//## Returns a hash code for this string.
@Public int String.indexOf(int ch) {
    return this.indexOf(ch, 0);
}

//## @Public int String.indexOf(int ch, int fromIndex)
//## Returns the index within this string of the first occurrence of the specified character.
//@Native @Public int String.indexOf(int ch, int fromIndex)

//## @Public int String.indexOf(String str)
//## Returns the index within this string of the first occurrence of the specified character, starting the search at the specified index.
@Public int String.indexOf(String str) {
    return this.indexOf(str, 0);
}

//## @Public int String.indexOf(String str, int fromIndex)
//## Returns the index within this string of the first occurrence of the specified substring.
//@Native @Public int String.indexOf(String str, int fromIndex)

//## @Public boolean String.isEmpty()
//## Returns a canonical representation for the string object.
@Public boolean String.isEmpty() {
    return this.length() == 0;
}

//## @Public int String.lastIndexOf(int ch)
//## Returns true if, and only if, length() is 0.
@Public int String.lastIndexOf(int ch) {
    return this.lastIndexOf(ch, this.length());
}

//## @Public int String.lastIndexOf(int ch, int fromIndex)
//## Returns the index within this string of the last occurrence of the specified character.
//@Native @Public int String.lastIndexOf(int ch, int fromIndex)

//## @Public int String.lastIndexOf(String str)
//## Returns the index within this string of the last occurrence of the specified character, searching backward starting at the specified index.
@Public int String.lastIndexOf(String str) {
    return this.lastIndexOf(str, this.length());
}

//## @Public int String.lastIndexOf(String str, int fromIndex)
//## Returns the index within this string of the rightmost occurrence of the specified substring.
//@Native @Public int String.lastIndexOf(String str, int fromIndex)

//## @Public int String.length()
//## Returns the index within this string of the last occurrence of the specified substring, searching backward starting at the specified index.
//@Native @Public int String.length()

//## @Public boolean String.matches(String regex)
//## Returns the length of this string.
//@Native @Public boolean String.matches(String regex)

//## @Public int String.offsetByCodePoints(int index, int codePointOffset)
//## Tells whether or not this string matches the given regular expression.
//@Native @Public int String.offsetByCodePoints(int index, int codePointOffset)

//## @Public boolean String.regionMatches(boolean ignoreCase, int toffset, String other, int ooffset, int len)
//## Returns the index within this String that is offset from the given index by codePointOffset code points.
//@Native @Public boolean String.regionMatches(boolean ignoreCase, int toffset, String other, int ooffset, int len)

//## @Public boolean String.regionMatches(int toffset, String other, int ooffset, int len)
//## Tests if two string regions are equal.
@Public boolean String.regionMatches(int toffset, String other, int ooffset, int len) {
    return this.regionMatches(toffset, true, other, ooffset, len);
}

//## @Public String String.replace(char oldChar, char newChar)
//## Tests if two string regions are equal.
//TODO
//@Public String String.replace(char oldChar, char newChar) {
//}

////## @Public String String.replace(CharSequence target, CharSequence replacement)
////## Returns a new string resulting from replacing all occurrences of oldChar in this string with newChar.
//@Public String String.replace(CharSequence target, CharSequence replacement)

//## @Public String String.replaceAll(String regex, String replacement)
//## Replaces each substring of this string that matches the literal target sequence with the specified literal replacement sequence.
//@Native @Public String String.replaceAll(String regex, String replacement)

//## @Public String String.replaceFirst(String regex, String replacement)
//## Replaces each substring of this string that matches the given regular expression with the given replacement.
//@Native @Public String String.replaceFirst(String regex, String replacement)

//## @Public String[] String.split(String regex)
//## Replaces the first substring of this string that matches the given regular expression with the given replacement.
@Public String[] String.split(String regex) {
    return this.split(regex, -1);
}

//## @Public String[] String.split(String regex, int limit)
//## Splits this string around matches of the given regular expression.
//@Public @Native String[] String.split(String regex, int limit);

//## @Public boolean String.startsWith(String prefix)
//## Splits this string around matches of the given regular expression.
@Public boolean String.startsWith(String prefix) {
    return this.startsWith(prefix, 0);
}

//## @Public boolean String.startsWith(String prefix, int toffset)
//## Tests if this string starts with the specified prefix.
//@Native @Public boolean String.startsWith(String prefix, int toffset)

////## @Public CharSequence String.subSequence(int beginIndex, int endIndex)
////## Tests if the substring of this string beginning at the specified index starts with the specified prefix.
//@Public CharSequence String.subSequence(int beginIndex, int endIndex) {
//}

//## @Public String String.substring(int beginIndex)
//## Returns a new character sequence that is a subsequence of this sequence.
@Public String String.substring(int beginIndex) {
    return this.substring(beginIndex, this.length());
}

//## @Public String String.substring(int beginIndex, int endIndex)
//## Returns a new string that is a substring of this string.
//@Native @Public String String.substring(int beginIndex, int endIndex);

////## @Public char[] String.toCharArray()
////## Returns a new string that is a substring of this string.
//@Public char[] String.toCharArray() {
//}

//## @Public String String.toLowerCase()
//## Converts this string to a new character array.
@Public String String.toLowerCase() {
    return this.toLowerCase("UTF-8");
}

//## @Public String String.toLowerCase(Locale locale)
//## Converts all of the characters in this String to lower case using the rules of the default locale.
//@Native @Public String String.toLowerCase(String/*FIXME Locale*/ locale);


//## @Public String String.toString()
//## Converts all of the characters in this String to lower case using the rules of the given Locale.
@Public String String.toString() {
    return this;
}

//## @Public String String.toUpperCase()
//## This object (which is already a string!) is itself returned.
@Public String String.toUpperCase() {
    return this.toUpperCase("UTF-8");
}

//## @Public String String.toUpperCase(Locale locale)
//## Converts all of the characters in this String to upper case using the rules of the default locale.
//@Native @Public String String.toUpperCase(String/*FIXME Locale*/ locale)


//## @Public String String.trim()
//## Converts all of the characters in this String to upper case using the rules of the given Locale.
//@Native @Public String String.trim()

//## @Public @Static String String.valueOf(boolean b)
//## Returns a copy of the string, with leading and trailing whitespace omitted.
@Public @Static String String.valueOf(boolean b) {
    return b.toString();
}

//## @Public @Static String String.valueOf(float f)
//## Returns the string representation of the double argument.
@Public @Static String String.valueOf(float f) {
    return f.toString();
}

//## @Public @Static String String.valueOf(int i)
//## Returns the string representation of the float argument.
@Public @Static String String.valueOf(int i) {
    return i.toString();
}

//## @Public @Static String String.valueOf(Object obj)
//## Returns the string representation of the long argument.
@Public @Static String String.valueOf(Object obj) {
    return obj.toString();
}
