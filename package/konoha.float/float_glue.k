// K.import("sugar");
// K.import("konoha.while");
// K.import("konoha.string");
// K.import("konoha.assignment");

// int floatToken(Token tk, String source) {
//     int i = 0;
//     int len = source.getSize();
//     boolean negative = false;
//     int ch0 = "0".charCodeAt(0);
//     int ch1 = "1".charCodeAt(0);
//     int ch9 = "9".charCodeAt(0);
//     int che  = "e".charCodeAt(0);
//     int che2 = "E".charCodeAt(0);
//     int c = source.charCodeAt(i);
//     if (c == ch0) {
//         i = i + 1;
//         c = source.charCodeAt(i);
//     }
//     else if (ch0 <= c && c <= ch9) {
//         i = i + 1;
//         c = source.charCodeAt(i);
//         while (ch0 <= c && c <= ch9 && i < len) {
//             i = i + 1;
//             c = source.charCodeAt(i);
//         }
//     }
//     if (c != ".".charCodeAt(0) && c != che && c != che2) {
//         String intText = source.substring(0, i);
//         tk.setKeyword("$INT");
//         tk.setText(intText);
//         return i;
//     }
//     if (c == ".".charCodeAt(0)) {
//         i = i + 1;
//         c = source.charCodeAt(i);
//         while (ch0 <= c && c <= ch9 && i < len) {
//             i = i + 1;
//             c = source.charCodeAt(i);
//         }
//     }
//     if (c == che || c == che2) {
//         i = i + 1;
//         c = source.charCodeAt(i);
//         if (c == "+".charCodeAt(0) || c == "-".charCodeAt(0)) {
//             i = i + 1;
//             c = source.charCodeAt(i);
//         }
//         while (ch0 <= c && c <= ch9 && i < len) {
//             i = i + 1;
//             c = source.charCodeAt(i);
//         }
//     }
//     String floatText = source.substring(0, i);
//     tk.setKeyword("$FLOAT");
//     tk.setText(floatText);
//     return i;
// }

// NameSpace.compileAllDefinedMethods();

// K.addTokenizeFunc("-", floatToken);
// K.addTokenizeFunc("0", floatToken);
// K.addTokenizeFunc("1", floatToken);
// K.addTokenizeFunc("2", floatToken);
// K.addTokenizeFunc("3", floatToken);
// K.addTokenizeFunc("4", floatToken);
// K.addTokenizeFunc("5", floatToken);
// K.addTokenizeFunc("6", floatToken);
// K.addTokenizeFunc("7", floatToken);
// K.addTokenizeFunc("8", floatToken);
// K.addTokenizeFunc("9", floatToken);
