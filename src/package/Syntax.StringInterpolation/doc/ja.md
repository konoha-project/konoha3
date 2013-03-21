Syntax.StringInterpolation-文字列への式の埋込み
====================
文字列への式埋込み

double quotationで囲まれた文字列中に"${変数 or 式}"を使って任意の式を埋め込むことができます。

```
String w = "World"
String message = "Hello ${w}!"
System.p(message)  // Hello World! と表示される
```
