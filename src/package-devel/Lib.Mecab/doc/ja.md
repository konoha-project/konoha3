Lib.Mecab-パッケージの1行説明文
====================
パッケージの説明文
=======
Lib.Mecab-Mecabのkonohaバインド
=====

# Tagger クラス
## メソッド
### Tagger Tagger.new();

---

### String Tagger.parse(String input)

---

### String Tagger.NBestParse(int n, String input)

---

### Boolean Tagger.NBestInit(String input)

---

### String Tagger.NBestNext()

---

### MecabNode Tagger.ParseToNode(String input)

---

### void Tagger.destory()

---


# MecabNode クラス
## メソッド
### MecabNode MecabNode.next()

---

### MecabNode MecabNode.prev()

---

### MecabNode MecabNode.enext()

---

### MecabNode MecabNode.bnext()

---

### String MecabNode.getSurface()

---

### String MecabNode.getFeature()

---

### int MecabNode.getLength()

---

### int MecabNode.getRLength()

---

### int MecabNode.getRCAttr()

---

### int MecabNode.getLCAttr()

---

### int MecabNode.getCharType()

---

### int MecabNode.getStat()

---

### int MecabNode.getID()

---

### Boolean MecabNode.isBest()

---

### int MecabNode.wcost()

---

### int MecabNode.cost()

---

## 定数
* MECAB_NOR_NODE
* MECAB_UNK_NODE
* MECAB_BOS_NODE
* MECAB_EOS_NODE
