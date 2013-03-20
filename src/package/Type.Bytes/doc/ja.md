Type.Bytes-バイト列を表す型
====================

Bytes
--------------------
#### Bytes Bytes.new(int size)
#### int getSize()
#### int get(int index)
#### void set(int index, int c)
#### void setAll(int c)

Stringクラスに追加されるメソッド
--------------------
#### String String.new(Bytes ba)
#### String String.new(Bytes ba, String charset)
#### String String.new(Bytes ba, int offset, int length)
#### String String.new(Bytes ba, int offset, int length, String charset)
