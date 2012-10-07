// import("sugar");
// import("konoha.while");
// import("konoha.string");
// import("konoha.assign");

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
//         tk.setKeyword("$Number");
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
//     tk.setKeyword("$Float");
//     tk.setText(floatText);
//     return i;
// }

// NameSpace.compileAllDefinedMethods();

// addTokenFunc("-", floatToken);
// addTokenFunc("0", floatToken);
// addTokenFunc("1", floatToken);
// addTokenFunc("2", floatToken);
// addTokenFunc("3", floatToken);
// addTokenFunc("4", floatToken);
// addTokenFunc("5", floatToken);
// addTokenFunc("6", floatToken);
// addTokenFunc("7", floatToken);
// addTokenFunc("8", floatToken);
// addTokenFunc("9", floatToken);
