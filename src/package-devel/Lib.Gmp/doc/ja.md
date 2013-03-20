Lib.Gmp-多倍長演算ライブラリGMPのKonohaバインド
====================

Mpz
--------------------
多倍長整数型。Mpz型同士、Mpzとint型同士で四則演算と比較が可能。

### コンストラクタ
#### Mpz.new(int n)
nと同じ値のMpzオブジェクトを返す。
#### Mpz.new(Mpz n)
別のMpzオブジェクトをコピーする。
#### Mpz.new(String n)
10進表記した数値を表す文字列からMpzオブジェクトを作る。
###　メソッド
#### boolean isEven()
#### Mpz power(int x)
#### Mpz abs()
#### int getsize()

Mpf
--------------------
多倍長浮動小数点数型。Mpf、Mpz、int、floatの間で四則演算と比較が可能。
### コンストラクタ
#### Mpf Mpf.new()
#### Mpf Mpf.new(float x)
#### Mpf Mpf.new(Mpf x)
#### Mpf Mpf.new(int x)
#### Mpf Mpf.new(Mpz x)
#### Mpf Mpf.new(String x)
###　メソッド
#### Mpf Mpf.power(int x)
#### Mpf Mpf.abs()
#### Mpf Mpf.sqrt()
