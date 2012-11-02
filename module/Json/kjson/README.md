JSON(JavaScript Object Notation) parser for KonohaScript

[![Build Status](https://secure.travis-ci.org/imasahiro/kjson.png)](http://travis-ci.org/imasahiro/kjson)

[Parse API]
`  JSON *parseJSON(char *start, char *end);`

[Stringify API]
`
  char *JSON_toString(JSON json);
  char *JSON_toStringWithLength(JSON json, size_t *length);
`

[Getter API]
`
  unsigned JSON_length (JSON json);
  JSON*  JSON_get      (JSON json, JSONString *Key);
  JSON** JSON_getArray (JSON json, JSONString *Key, size_t *len);
  char*  JSON_getString(JSON json, JSONString *Key, size_t *len);
  double JSON_getDouble(JSON json, JSONString *Key);
  int    JSON_getBool  (JSON json, JSONString *Key);
  int    JSON_getInt   (JSON json, JSONString *Key);
`

[Iterator API]
`
  /* Object Iteration */
  void f(JSON o) {
    JSON Key; JSON Val;
    JSONObject_iterator Itr;
    JSON_OBJECT_EACH(o, Itr, Key, Val) {
        JSON_dump(stdout, Key);
        JSON_dump(stdout, Val);
    }
  }
  /* Array Iteration */
  void f(JSON json) {
    JSONArray *a;
    JSON *s, *e;
    JSON_ARRAY_EACH(json, a, s, e) {
        JSON_dump(*s);
    }
  }
`

[Other API]
`
  void JSON_free(JSON json);
  void JSON_dump(FILE *fp, JSON json);
`

[test/benchmark\*.json]
  Original code was copied from:
  kraken-1.1/kraken/tests/kraken-1.1/ai-astar-data.js
  kraken-1.1/audio-beat-detection-data.js
  kraken-1.1/stanford-crypto-ccm-data.js
  kraken-1.1/imaging-desaturate-data.js
  kraken-1.1/json-parse-financial-data.js
