Common.StringUtil-文字列基本ライブラリ
====================
StringUtilパッケージは文字列操作の基本関数を提供します。

### @Static String StringUtil.concat(String t, String s)
### @Static int StringUtil.length(String s)
Returns the length of this String_
### @Static String StringUtil.charAt(String t, int n)
Returns the char value at the specified index.
### @Static String StringUtil.fromCharCode(int n)
### @Static int StringUtil.charCodeAt(String t, int n)
### @Static int StringUtil.codePointAt(String s, int index)
Returns the character
### @Static int StringUtil.codePointBefore(String s, int index)
Returns the character
### @Static int StringUtil.codePointCount(String s, int beginIndex, int endIndex)
Returns the number of Unicode code points in the specified text range of this String_
### @Static int StringUtil.compareTo(String s, String anotherString)
Compares two strings lexicographically.
### @Static int StringUtil.compareToIgnoreCase(String s, String str)
Compares two strings lexicographically
### @Static boolean StringUtil.endsWith(String s, String suffix)
Tests if this string ends with the specified suffix.
### @Static int StringUtil.hashCode(String s)
Returns a hash code for this String
### @Static int StringUtil.indexOf(String s, int ch, int fromIndex)
Returns the index within this string of the first occurrence of the specified character
### @Static int StringUtil.indexOf(String s, String str, int fromIndex)
Returns the index within this string of the first occurrence of the specified substring
### @Static int StringUtil.lastIndexOf(String s, String str, int fromIndex)
Returns the index within this string of the last occurrence of the specified character
### @Static int StringUtil.lastIndexOf(String s, int ch, int fromIndex)
Returns the index within this string of the last occurrence of the specified substring
### @Static boolean StringUtil.matches(String s, String regex)
Tells whether or not this string matches the given regular expression.
### @Static int StringUtil.offsetByCodePoints(String s, int index, int codePointOffset)
Returns the index within this String that is offset from the given index by codePointOffset code points.
### @Static boolean StringUtil.regionMaches(String s, boolean ignoreCase, int toffset, String other, int ooffset, int len)
Tests if two string regions are equal.
### @Static String StringUtil.replace(String s, int oldChar, int newChar)
Returns a new string resulting from replacing all occurrences of oldChar in this string with newChar.
### @Static String StringUtil.replaceAll(String s, String regex, String replacement)
Replaces each substring of this string that matches the given regular expression with the given replacement.
### @Static String StringUtil.replaceFirst(String s, String regex, String replacement)
Replaces the first substring of this string that matches the given regular expression with the given replacement.
### @Static Array[String] StringUtil.split(String s, String regex, int limit)
Splits this string around matches of the given regular expression.
### @Static boolean StringUtil.startsWith(String s, String prefix, int toffset)
Tests if the substring of this string beginning at the specified index starts with the specified prefix.
### @Static String StringUtil.substring(String s, int beginIndex, int endIndex)
Returns a new string that is a substring of this String_
### @Static String StringUtil.getString(String t, int n)
### @Static int StringUtil.getlength(String t)
Returns the length of this String_
### @Static String StringUtil.toLowerCase(String s, String locale)
Converts all of the characters in this String to lower case using the rules of the given Locale.
### @Static String StringUtil.toUpperCase(String s, String locale)
Converts all of the characters in this String to upper case using the rules of the given Locale.
### @Static String StringUtil.trim(String s)
Returns a copy of the string
