Type.File-ファイル入出力ライブラリ
====================

class File
--------------------
### コンストラクタ
#### File File.new(String filename, String mode)
### 静的メソッド
#### String File.scriptPath(String filename)
### メソッド
#### void close()
#### void putc(int char)
#### String readLine()
#### String print(@Coercion String str)
#### void setWriterCharset(String charset)
#### void setReaderCharset(String charset)
#### int getc()
#### void println(@Coercion String str)
#### void println()
#### void flush()
#### boolean isatty()
#### int getfileno()
#### int read(Bytes buf)
#### int read(Bytes buf, int offset, int len)
#### int write(Bytes buf)
#### int write(Bytes buf, int offset, int len)
