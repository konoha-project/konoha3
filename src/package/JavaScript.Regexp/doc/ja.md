JavaScript.Regexp-JavaScriptで提供される正規表現リテラルとライブラリ
====================

正規表現リテラル
--------------------
/で囲まれた文字列は正規表現として解釈される。

class RegExp
--------------------
### フィールド
#### boolean global;
#### boolean ignoreCase;
#### boolean multiline;
#### String source;
#### int lastIndex;
### Staticメソッド
#### @Static RegExp RegExp.create(String patter, String option);
### コンストラクタ
#### RegExp RegExp.new(String pattern);
#### RegExp RegExp.new(String pattern, String option);
### メソッド
#### String[] exec(String str);
#### boolean test(String str);

Stringクラスに追加されるメソッド
--------------------
#### int String.search(RegExp searchvalue);
#### String[] String.match(RegExp regexp);
#### String String.replace(RegExp searchvalue, String newvalue);
#### String[] String.split(RegExp regex);
#### String[] String.split(RegExp regex, int limit);
