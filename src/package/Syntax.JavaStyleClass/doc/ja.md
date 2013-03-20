Syntax.JavaStyleClass-Javaスタイルのクラス定義構文
====================
Javaスタイルのクラス定義構文が提供されます。

    class C {
        C(){
        }
        int f() {
            return 0;
        }
    }
    class D extends C {
        D(int x){
		    this.x = x;
        }
        @Override int f() {
            return x + 1;
        }
    }

