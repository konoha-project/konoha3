# DCase DB
---
##開発環境
* Ubuntu 12.04(amd64)
* minikonoha 0.3

##必要なライブラリ
* OpenSSL
* MySQL

##Class
各クラスにはtoJson()メソッドが用意されており、Jsonにdumpできる。
* DBNode
    * Goal,Strategy,Evidence,Contextの親クラス
* Link
    * DBNode間のRelation Link
* Commit
    * DBの操作履歴

## API
### DCaseDB Class
* DCaseDB DCaseDB.new()
    * DCaseDBを扱うクラスの作成
    * 各操作はArgumentのTopGoalのnameを渡し、どのArgumentへの操作か決定する。

* DBNode CreateDBNode(String ArgumentName, Json content)
    * DBNodeを作成する。Json contentに引数を渡す。
    * Json 例:
<pre>
    content = {
        "name" : "ReturnFunction",        //必須
        "DBNodeType" : "Goal",  //必須
        "description" : "", //任意
    }
</pre>

    * DBNodeTypeに渡した名前のクラスを生成する。
    * nameは検索に使う。
    * Contextへの渡し方
<pre>
    content = {
        "name" : "hogehoge",
        "DBNodeType" : "Context",
        "context" : {"architecture" : "x86_64", "OS" : "CentOS 6.3",  ...}
    }
</pre>
* void   InsertLink(String ArgumentName, String parentName, String childName)
    * 各DBNodeを繋げる。DBNodeのparentNameとchildNameを渡す。

* void   UpdateDBNode(String ArgumentName, String DBNodeName , Json contents)
    * 変更したいDBNodeのnameとそのアップデート内容をJsonに渡す。
    * Jsonの内容はCreateDBNodeと同様。

* void   UpdateParentLink(String Treename, String OldParentName, String NewParentName, String ChildName)
    * 親Linkの差し替え。old->childを削除してnew->childを新しく作成するため、oldNameが必要。

* void   UpdateChildLink(String ArgumentName, String parent, String child_old, String child_new)
    * 子Linkの差し替え。UpdateParentLinkと同様

* void   DeleteDBNode(String ArgumentName, String nodeName)
    * DBNodeの削除。このDBNodeにつながっているLinkも自動削除されます。

* void   DeleteLink(String ArgumentName, String ParentName, String ChildName)
    * Linkの削除。

* DBNode SearchDBNode(String ArgumentName, String searchText)
    * DBNodeの検索。現状はnameが完全一致するDBNodeを返す。

* Argument SearchArgument(String ArgumentName)
    * Argumentの検索。現状はnameが完全一致するArgumentを返す。

* void   Connect(String userName, String password)
    * DBにアクセスする。今はMySQL依存の実装だが、一般化していきたい。

* void   CreateArgument(String name, String content)
    * Argumentの作成。nameにはTopGoalの名前、contentにはTopGoalの引数をいれる。
    * この時点でContextを作成するべき。

* void   AddContext(String ArgumentName, String context_name,Json condition) //TODO
    * あるArgumentにcontextの条件を追加する。(既存のkeyは更新されます。)
    * 現在Argument名だけだとContextへの参照を作るのが面倒なため、リファクタリングが必要。
    * Json 例:
<pre>
    condition = {
        "OS" : "Ubuntu 12.04",
        "C" : "gcc 4.6.3"
    }
</pre>
* Json getContext(String ArgumentName) //TODO
    * ArgumentのContextを取得する。
* Argument ForkArgument(Argument from_argument ,int depth) //TODO
    * Argumentのフォークを行う。depthにどの階層までコピーするか書く。
* DBNode[] SearchArgumentDBNode(Argument argument,String DBNodeType) //TODO
    * Argument内のあるタイプのノードだけ抜き出す。
    * API名は変更予定。

### Argument Class
* Json   toJson()
    * Argumentの内部にある情報をすべてdumpする。<br />
    * 例:
<pre>
    {
        "HEAD"      : "asfhdafjs",
        "root"      : {...},
        "links"     : {"name":"Goal", "children" : [{...}, ...]},
        "nodes"     : [{...}, ...],
        "CommitLog" : [{...}, ...]
    }
</pre>
    * HEAD      ... 最新リビジョンのSHA-1
    * root      ... ArgumentのTop Goal
    * links     ... DBNode間のLinkをTree構造に復元する
    * nodes     ... DBNodeの情報(Array[Json])
    * CommitLog ... Commitの情報(Array[Json])

##Web API
作成予定(JSON-RPCにする予定)
wiki参照

## Usage
test/ ディレクトリ参照<br>
misc/dcasedb.sql ... DCaseDBのテーブル作成用クエリ<br>
misc/rm.sql      ... DBのデータ削除用クエリ
