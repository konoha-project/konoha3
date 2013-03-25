Java.String-Javaで提供される文字列型
=======================

String
-----------------------

### int charAt(int index)
Returns the char value at the specified index.

### int codePointAt(int index)
Returns the char value at the specified index.

### int codePointBefore(int index)
Returns the character (Unicode code point) at the specified index.

### int codePointCount(int beginIndex, int endIndex)
Returns the character (Unicode code point) before the specified index.

### int compareTo(String anotherString)
Returns the number of Unicode code points in the specified text range of this 

### int compareToIgnoreCase(String str)
Compares two strings lexicographically.

### String concat(String str)
Compares two strings lexicographically, ignoring case differences.

### boolean endsWith(String suffix)
Returns a String that represents the character sequence in the array specified.

### boolean equals(Object anObject)
Tests if this string ends with the specified suffix.

### boolean equalsIgnoreCase(String anotherString)
Compares this string to the specified object.

### int hashCode()
Copies characters from this string into the destination character array.

### int indexOf(int ch)
Returns a hash code for this 

### int indexOf(int ch, int fromIndex)
Returns the index within this string of the first occurrence of the specified character.

### int indexOf(String str)
Returns the index within this string of the first occurrence of the specified character, starting the search at the specified index.

### int indexOf(String str, int fromIndex)
Returns the index within this string of the first occurrence of the specified sub

### boolean isEmpty()
Returns a canonical representation for the string object.

### int lastIndexOf(int ch)
Returns true if, and only if, length() is 0.

### int lastIndexOf(int ch, int fromIndex)
Returns the index within this string of the last occurrence of the specified character.

### int lastIndexOf(String str)
Returns the index within this string of the last occurrence of the specified character, searching backward starting at the specified index.

### int lastIndexOf(String str, int fromIndex)
Returns the index within this string of the rightmost occurrence of the specified sub

### int length()
Returns the index within this string of the last occurrence of the specified substring, searching backward starting at the specified index.

### boolean matches(String regex)
Returns the length of this 

### int offsetByCodePoints(int index, int codePointOffset)
Tells whether or not this string matches the given regular expression.

### boolean regionMatches(boolean ignoreCase, int toffset, String other, int ooffset, int len)
Returns the index within this String that is offset from the given index by codePointOffset code points.

### boolean regionMatches(int toffset, String other, int ooffset, int len)
Tests if two string regions are equal.

### String replace(char oldChar, char newChar)
Tests if two string regions are equal.

### String replace(CharSequence target, CharSequence replacement)
Returns a new string resulting from replacing all occurrences of oldChar in this string with newChar.

### String replaceAll(String regex, String replacement)
Replaces each substring of this string that matches the literal target sequence with the specified literal replacement sequence.

### String replaceFirst(String regex, String replacement)
Replaces each substring of this string that matches the given regular expression with the given replacement.

### String[] split(String regex)
Replaces the first substring of this string that matches the given regular expression with the given replacement.

### String[] split(String regex, int limit)
Splits this string around matches of the given regular expression.

### boolean startsWith(String prefix)
Splits this string around matches of the given regular expression.

### boolean startsWith(String prefix, int toffset)
Tests if this string starts with the specified prefix.

### String substring(int beginIndex)
Returns a new character sequence that is a subsequence of this sequence.

### String substring(int beginIndex, int endIndex)
Returns a new string that is a substring of this 

### String toLowerCase()
Converts this string to a new character array.

### String toLowerCase(Locale locale)
Converts all of the characters in this String to lower case using the rules of the default locale.

### String toString()
Converts all of the characters in this String to lower case using the rules of the given Locale.

### String toUpperCase()
This object (which is already a string!) is itself returned.

### String toUpperCase(Locale locale)
Converts all of the characters in this String to upper case using the rules of the default locale.

### String trim()
Converts all of the characters in this String to upper case using the rules of the given Locale.

### @Static String String.valueOf(boolean b)
Returns a copy of the string, with leading and trailing whitespace omitted.

### @Static String String.valueOf(float f)
Returns the string representation of the double argument.

### @Static String String.valueOf(int i)
Returns the string representation of the float argument.

### @Static String String.valueOf(Object obj)
Returns the string representation of the long argument.

