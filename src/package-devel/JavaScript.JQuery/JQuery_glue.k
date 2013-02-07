import("Type.Float");
import("Syntax.JavaStyleClass");
import("Syntax.GlobalVariable");
import("Syntax.Null");
import("MiniKonoha.Map");
import("JavaStyle.Object");
import("JavaScript.Array");
import("JavaScript.Document");

// Typing for the jQuery library, version 1.7.x

/*
   Interface for the jqXHR object
 */
class JQueryXHR /*extends XMLHttpRequest*/ {
	@Public void overrideMimeType(){}
}

/*
   Interface for the AJAX setting that will configure the AJAX request 
 */

class JQueryAjaxSettings {
	@Public Object accepts;
	@Public boolean async;
	@Public JQueryAjaxSettings beforeSend(JQueryXHR jqXHR, JQueryAjaxSettings settings){ return null; }
	@Public boolean cache;
	@Public String complete(JQueryXHR jqXHR, String textStatus){ return null; }
	@Public Map[Object] contents;
	@Public String contentType;
	@Public Object context;
	@Public Map[Object] converters;
	@Public boolean crossDomain;
	@Public Object data;
	@Public Object dataFilter(Object data, Object ty){ return null; }
	@Public String dataType;
	@Public Object error(JQueryXHR jqXHR, String textStatus, String errorThrow){ return null; }
	@Public boolean global;
	@Public Map[Object] headers;
	@Public boolean ifModified;
	@Public boolean isLocal;
	@Public String jsonp;
	@Public Object jsonpCallback;
	@Public String mimeType;
	@Public String password;
	@Public boolean processData;
	@Public String scriptCharset;
	@Public Map[Object] statusCode;
	@Public JQueryXHR success(Object data, String textStatus, JQueryXHR jqXHR){ return null; }
	@Public int timeout;
	@Public boolean traditional;
	@Public String type;
	@Public String url;
	@Public String username;
	@Public Object xhr;
	@Public Map[Object] xhrFields;
}

/*
   Interface for the JQuery callback
 */

class JQueryCallback {
	@Public Object add(Func[void] callback1){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15){ return null; }
	@Public Object add(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15, Func[void] callback16){ return null; }

	@Public Object disable(){ return null; }
	@Public Object empty(){ return null; }
	@Public Object fire(Object argument1){ return null; }
	@Public Object fire(Object argument1, Object argument2){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14, Object argument15){ return null; }
	@Public Object fire(Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14, Object argument15, Object argument16){ return null; }

	@Public boolean fired(){ return null; }
	@Public Object fireWith(Object context, Object argument1){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14, Object argument15){ return null; }
	@Public Object fireWith(Object context, Object argument1, Object argument2, Object argument3, Object argument4, Object argument5, Object argument6, Object argument7, Object argument8, Object argument9, Object argument10, Object argument11, Object argument12, Object argument13, Object argument14, Object argument15, Object argument16){ return null; }

	
	@Public boolean has(Func[void] callback1){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15){ return null; }
	@Public boolean has(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15, Func[void] callback16){ return null; }

	@Public boolean lock(){ return null; }
	@Public boolean locked(){ return null; }
	@Public Object removed(Func[void] callback1){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15){ return null; }
	@Public Object removed(Func[void] callback1, Func[void] callback2, Func[void] callback3, Func[void] callback4, Func[void] callback5, Func[void] callback6, Func[void] callback7, Func[void] callback8, Func[void] callback9, Func[void] callback10, Func[void] callback11, Func[void] callback12, Func[void] callback13, Func[void] callback14, Func[void] callback15, Func[void] callback16){ return null; }

}

/*
   Interface for the JQuery deferred, part of callbacks
 */



class JQueryPromise {

}

class JQueryDeferred extends JQueryPromise {
	@Public JQueryDeferred notify(Object arg1){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred notify(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred reject(Object arg1){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred reject(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred resolve(Object arg1){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred resolve(Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred notifyWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred rejectWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15){ return null; }
	@Public JQueryDeferred resolveWith(Object context, Object arg1, Object arg2, Object arg3, Object arg4, Object arg5, Object arg6, Object arg7, Object arg8, Object arg9, Object arg10, Object arg11, Object arg12, Object arg13, Object arg14, Object arg15, Object arg16){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11, Object progressCallback12){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11, Object progressCallback12, Object progressCallback13){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11, Object progressCallback12, Object progressCallback13, Object progressCallback14){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11, Object progressCallback12, Object progressCallback13, Object progressCallback14, Object progressCallback15){ return null; }
	@Public JQueryDeferred progress(Object progressCallback1, Object progressCallback2, Object progressCallback3, Object progressCallback4, Object progressCallback5, Object progressCallback6, Object progressCallback7, Object progressCallback8, Object progressCallback9, Object progressCallback10, Object progressCallback11, Object progressCallback12, Object progressCallback13, Object progressCallback14, Object progressCallback15, Object progressCallback16){ return null; }

	@Public JQueryPromise pipe(Object doneFilter, Object failFilter, Object progressFilter){ return null; }
	@Public String state(){ return null; }
	@Public JQueryDeferred then(Object doneCallbacks, Object failCallbacks, Object progressCallbacks){ return null; }

}


/*
   Interface for the JQuery promise, part of callbacks
 */

JQueryDeferred JQueryPromise.always(Object alwaysCallback1){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11, Object alwaysCallback12){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11, Object alwaysCallback12, Object alwaysCallback13){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11, Object alwaysCallback12, Object alwaysCallback13, Object alwaysCallback14){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11, Object alwaysCallback12, Object alwaysCallback13, Object alwaysCallback14, Object alwaysCallback15){ return null; }
JQueryDeferred JQueryPromise.always(Object alwaysCallback1, Object alwaysCallback2, Object alwaysCallback3, Object alwaysCallback4, Object alwaysCallback5, Object alwaysCallback6, Object alwaysCallback7, Object alwaysCallback8, Object alwaysCallback9, Object alwaysCallback10, Object alwaysCallback11, Object alwaysCallback12, Object alwaysCallback13, Object alwaysCallback14, Object alwaysCallback15, Object alwaysCallback16){ return null; }

JQueryDeferred JQueryPromise.fail(Object failCallback1){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11, Object failCallback12){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11, Object failCallback12, Object failCallback13){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11, Object failCallback12, Object failCallback13, Object failCallback14){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11, Object failCallback12, Object failCallback13, Object failCallback14, Object failCallback15){ return null; }
JQueryDeferred JQueryPromise.fail(Object failCallback1, Object failCallback2, Object failCallback3, Object failCallback4, Object failCallback5, Object failCallback6, Object failCallback7, Object failCallback8, Object failCallback9, Object failCallback10, Object failCallback11, Object failCallback12, Object failCallback13, Object failCallback14, Object failCallback15, Object failCallback16){ return null; }

JQueryDeferred JQueryPromise.done(Object doneCallback1){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11, Object doneCallback12){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11, Object doneCallback12, Object doneCallback13){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11, Object doneCallback12, Object doneCallback13, Object doneCallback14){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11, Object doneCallback12, Object doneCallback13, Object doneCallback14, Object doneCallback15){ return null; }
JQueryDeferred JQueryPromise.done(Object doneCallback1, Object doneCallback2, Object doneCallback3, Object doneCallback4, Object doneCallback5, Object doneCallback6, Object doneCallback7, Object doneCallback8, Object doneCallback9, Object doneCallback10, Object doneCallback11, Object doneCallback12, Object doneCallback13, Object doneCallback14, Object doneCallback15, Object doneCallback16){ return null; }

JQueryPromise JQueryPromise.pipe(Func[Object, Object] doneFilter, Func[Object, Object] failFilter, Func[Object, Object] progressFilter){ return null; }
JQueryDeferred JQueryPromise.then(Object doneCallbacks, Object failCallbacks, Object progressCallbacks){ return null; }

/*
   Interface of the JQuery extension of the W3C event object
 */

class JQueryEventObject /*extends Event*/ {
	@Public Object data;
	@Public DomElement delegateTarget;
	@Public boolean isDefaultPrevented(){ return null; }
	@Public boolean isImmediatePropogationStopped(){ return null; }
	@Public boolean isPropogationStopped(){ return null; }
	@Public String namespace;
	@Public Object preventDefault(){ return null; }
	@Public DomElement relatedTarget;
	@Public Object result;
	@Public void stopImmediatePropagation(){ return; }
	@Public void stopPropagation(){ return; }
	@Public int pageX;
	@Public int pageY;
	@Public int which;
	@Public Object metaKey;
}

/*
   Collection of properties of the current browser
 */

class JQueryBrowserInfo {
	@Public boolean safari;
	@Public boolean opera;
	@Public boolean msie;
	@Public boolean mozilla;
	@Public String version;
}

class JQuerySupport {
	@Public boolean ajax;
	@Public boolean boxModel;
	@Public boolean changeBubbles;
	@Public boolean checkClone;
	@Public boolean checkOn;
	@Public boolean cors;
	@Public boolean cssFloat;
	@Public boolean hrefNormalized;
	@Public boolean htmlSerialize;
	@Public boolean leadingWhitespace;
	@Public boolean noCloneChecked;
	@Public boolean noCloneEvent;
	@Public boolean opacity;
	@Public boolean optDisabled;
	@Public boolean optSelected;
	@Public boolean scriptEval;
	@Public boolean style;
	@Public boolean submitBubbles;
	@Public boolean tbody;
}

/*
   The jQuery instance members
 */

class JQueryPosition{
	@Public int top;
	@Public int left;
}

class JQuery {

	// AJAX
	@Public JQuery ajaxComplete(Object handler){ return null; }
	@Public JQuery ajaxError(Func[Object, Object, Object, Object] handler){ return null; }
	@Public JQuery ajaxSend(Func[Object, Object, Object, Object] handler){ return null; }
	@Public JQuery ajaxStart(Func[Object] handler){ return null; }
	@Public JQuery ajaxStop(Func[Object] handler){ return null; }
	@Public JQuery ajaxSuccess(Func[Object, Object, Object, Object] handler){ return null; }

	@Public JQuery load(String url, Object data, Object complete){ return null; }

	@Public String serialize(){ return null; }
	@Public Object[] serializeArray(){ return null; }

	// ATTRIBUTES
	@Public JQuery addClass(String classNames){ return null; }
	@Public Object addClass(Func[JQuery, Object, Object] func){ return null; }

	@Public String attr(String attributeName){ return null; }
	@Public JQuery attr(String attributeName, Object value){ return null; }
	@Public JQuery attr(Map[Object] map){ return null; }
	@Public JQuery attr(String attributeName, Func[Object, Object, Object] func){ return null; }

	@Public boolean hasClass(String className){ return null; }

	@Public JQuery html(String htmlString){ return null; }
	@Public String html(){ return null; }

	@Public String prop(String propertyName){ return null; }
	@Public JQuery prop(String propertyName, Object value){ return null; }
	@Public JQuery prop(Object map){ return null; }
	@Public JQuery prop(String propertyName, Func[Object, Object, Object] func){ return null; }

	@Public JQuery removeAttr(Object attributeName){ return null; }

	@Public JQuery removeClass(Object className){ return null; }
	@Public JQuery removeClass(Func[Object, Object, Object] func){ return null; }

	@Public JQuery removeProp(Object propertyName){ return null; }

	@Public JQuery toggleClass(Object className, boolean swtch){ return null; }
	@Public JQuery toggleClass(boolean swtch){ return null; }
	@Public JQuery toggleClass(Func[Object, Object, Object, Object] func){ return null; }

	@Public Object val(){ return null; }
	@Public JQuery val(String[] value){ return null; }
	@Public JQuery val(String value){ return null; }
	@Public JQuery val(Func[Object, Object, Object] func){ return null; }

	// CSS
	@Public Object css(String propertyName, Object value){ return null; }
	@Public Object css(Object propertyName, Object value){ return null; }

	@Public int height(){ return null; }
	@Public JQuery height(int value){ return null; }
	@Public JQuery height(Func[Object, Object, Object] func){ return null; }

	@Public int innerHeight(){ return null; }
	@Public int innerWidth(){ return null; }

	@Public Object offset(){ return null; }
	@Public JQuery offset(Object coordinates){ return null; }
	@Public JQuery offset(Func[Object, Object, Object] func){ return null; }

	@Public int outerHeight(boolean includeMargin){ return null; }
	@Public int outerWidth(boolean includeMargin){ return null; }

	@Public JQueryPosition position(){ return null; }

	@Public int scrollLeft(){ return null; }
	@Public JQuery scrollLeft(int value){ return null; }

	@Public int scrollTop(){ return null; }
	@Public JQuery scrollTop(int value){ return null; }

	@Public int width(){ return null; }
	@Public JQuery width(int value){ return null; }
	@Public JQuery width(Func[Object, Object, Object] func){ return null; }

	// DATA
	@Public JQuery clearQueue(String queueName){ return null; }

	@Public JQuery data(String key, Object value){ return null; }
	@Public JQuery data(Map[Object] obj){ return null; }
	@Public Object data(String key){ return null; }

	@Public JQuery dequeue(String queueName){ return null; }

	@Public JQuery removeData(Object nameOrList){ return null; }

	// DEFERRED
	@Public JQueryPromise promise(Object type, Object target){ return null; }

	// EFFECTS
	@Public JQuery animate(Object properties, Object duration, String easing, Func complete){ return null; }
	//Object animate(Object properties, options: { Object duration; String easing; Func complete; Func step; boolean queue; specialEasing; }){ return null; }

	@Public JQuery delay(int duration, String queueName){ return null; }

	@Public JQuery fadeIn(){ return null; }
	@Public JQuery fadeIn(Func[void] callback){ return null; }
	@Public JQuery fadeIn(String easing, Func[void] callback){ return null; }
	@Public JQuery fadeIn(String duration){ return null; }
	@Public JQuery fadeIn(int duration){ return null; }
	@Public JQuery fadeIn(int duration, Func[void] callback){ return null; }
	@Public JQuery fadeIn(String duration, String easing){ return null; }
	@Public JQuery fadeIn(int duration, String easing){ return null; }
	@Public JQuery fadeIn(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery fadeIn(int duration, String easing, Func[void] callback){ return null; }
	
	@Public JQuery fadeOut(){ return null; }
	@Public JQuery fadeOut(Func[void] callback){ return null; }
	@Public JQuery fadeOut(String easing, Func[void] callback){ return null; }
	@Public JQuery fadeOut(String duration){ return null; }
	@Public JQuery fadeOut(int duration){ return null; }
	@Public JQuery fadeOut(int duration, Func[void] callback){ return null; }
	@Public JQuery fadeOut(String duration, String easing){ return null; }
	@Public JQuery fadeOut(int duration, String easing){ return null; }
	@Public JQuery fadeOut(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery fadeOut(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery fadeToggle(){ return null; }
	@Public JQuery fadeToggle(Func[void] callback){ return null; }
	@Public JQuery fadeToggle(String easing, Func[void] callback){ return null; }
	@Public JQuery fadeToggle(String duration){ return null; }
	@Public JQuery fadeToggle(int duration){ return null; }
	@Public JQuery fadeToggle(int duration, Func[void] callback){ return null; }
	@Public JQuery fadeToggle(String duration, String easing){ return null; }
	@Public JQuery fadeToggle(int duration, String easing){ return null; }
	@Public JQuery fadeToggle(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery fadeToggle(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery fadeTo(String duration, float opacity){ return null; }
	@Public JQuery fadeTo(int duration, float opacity){ return null; }
	@Public JQuery fadeTo(String duration, float opacity, Func[void] callback){ return null; }
	@Public JQuery fadeTo(int duration, float opacity, Func[void] callback){ return null; }
	@Public JQuery fadeTo(String duration, float opacity, String easing, Func[void] callback){ return null; }
	@Public JQuery fadeTo(int duration, float opacity, String easing, Func[void] callback){ return null; }

	@Public JQuery hide(){ return null; }
	@Public JQuery hide(Func[void] callback){ return null; }
	@Public JQuery hide(String easing, Func[void] callback){ return null; }
	@Public JQuery hide(String duration){ return null; }
	@Public JQuery hide(int duration){ return null; }
	@Public JQuery hide(int duration, Func[void] callback){ return null; }
	@Public JQuery hide(String duration, String easing){ return null; }
	@Public JQuery hide(int duration, String easing){ return null; }
	@Public JQuery hide(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery hide(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery show(){ return null; }
	@Public JQuery show(Func[void] callback){ return null; }
	@Public JQuery show(String easing, Func[void] callback){ return null; }
	@Public JQuery show(String duration){ return null; }
	@Public JQuery show(int duration){ return null; }
	@Public JQuery show(int duration, Func[void] callback){ return null; }
	@Public JQuery show(String duration, String easing){ return null; }
	@Public JQuery show(int duration, String easing){ return null; }
	@Public JQuery show(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery show(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery slideUp(){ return null; }
	@Public JQuery slideUp(Func[void] callback){ return null; }
	@Public JQuery slideUp(String easing, Func[void] callback){ return null; }
	@Public JQuery slideUp(String duration){ return null; }
	@Public JQuery slideUp(int duration){ return null; }
	@Public JQuery slideUp(int duration, Func[void] callback){ return null; }
	@Public JQuery slideUp(String duration, String easing){ return null; }
	@Public JQuery slideUp(int duration, String easing){ return null; }
	@Public JQuery slideUp(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery slideUp(int duration, String easing, Func[void] callback){ return null; }
	
	@Public JQuery slideDown(){ return null; }
	@Public JQuery slideDown(Func[void] callback){ return null; }
	@Public JQuery slideDown(String easing, Func[void] callback){ return null; }
	@Public JQuery slideDown(String duration){ return null; }
	@Public JQuery slideDown(int duration){ return null; }
	@Public JQuery slideDown(int duration, Func[void] callback){ return null; }
	@Public JQuery slideDown(String duration, String easing){ return null; }
	@Public JQuery slideDown(int duration, String easing){ return null; }
	@Public JQuery slideDown(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery slideDown(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery slideToggle(){ return null; }
	@Public JQuery slideToggle(Func[void] callback){ return null; }
	@Public JQuery slideToggle(String easing, Func[void] callback){ return null; }
	@Public JQuery slideToggle(String duration){ return null; }
	@Public JQuery slideToggle(int duration){ return null; }
	@Public JQuery slideToggle(int duration, Func[void] callback){ return null; }
	@Public JQuery slideToggle(String duration, String easing){ return null; }
	@Public JQuery slideToggle(int duration, String easing){ return null; }
	@Public JQuery slideToggle(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery slideToggle(int duration, String easing, Func[void] callback){ return null; }

	@Public JQuery stop(boolean clearQueue){ return null; }
	@Public JQuery stop(boolean clearQueue, boolean jumpToEnd){ return null; }
	@Public JQuery stop(String queue, boolean clearQueue){ return null; }
	@Public JQuery stop(String queue, boolean clearQueue, boolean jumpToEnd){ return null; }

	@Public JQuery toggle(){ return null; }
	@Public JQuery toggle(Func[void] callback){ return null; }
	@Public JQuery toggle(String easing, Func[void] callback){ return null; }
	@Public JQuery toggle(String duration){ return null; }
	@Public JQuery toggle(int duration){ return null; }
	@Public JQuery toggle(int duration, Func[void] callback){ return null; }
	@Public JQuery toggle(String duration, String easing){ return null; }
	@Public JQuery toggle(int duration, String easing){ return null; }
	@Public JQuery toggle(String duration, String easing, Func[void] callback){ return null; }
	@Public JQuery toggle(int duration, String easing, Func[void] callback){ return null; }
	@Public JQuery toggle(boolean showOrHide){ return null; }
	
	// EVENTS
	@Public JQuery bind(String eventType, Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery bind(String eventType, Object eventData, boolean preventBubble){ return null; }
	@Public JQuery bind(String eventType, boolean preventBubble){ return null; }
	@Public void bind(Object event1){ return; }
	@Public void bind(Object event1, Object event2){ return; }
	@Public void bind(Object event1, Object event2, Object event3){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11, Object event12){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11, Object event12, Object event13){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11, Object event12, Object event13, Object event14){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11, Object event12, Object event13, Object event14, Object event15){ return; }
	@Public void bind(Object event1, Object event2, Object event3, Object event4, Object event5, Object event6, Object event7, Object event8, Object event9, Object event10, Object event11, Object event12, Object event13, Object event14, Object event15, Object event16){ return; }
	
	@Public JQuery blur(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery blur(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery change(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery change(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery click(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery click(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery dblclick(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery dblclick(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery delegate(Object selector, String eventType, Func[Object, JQueryEventObject] handler){ return null; }


	@Public JQuery focus(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery focus(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery focusin(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery focusin(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery focusout(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery focusout(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery hover(Func[Object, JQueryEventObject] handlerIn, Func[Object, JQueryEventObject] handlerOut){ return null; }
	@Public JQuery hover(Func[Object, JQueryEventObject] handlerInOut){ return null; }

	@Public JQuery keydown(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery keydown(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery keypress(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery keypress(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery keyup(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery keyup(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mousedown(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mousedown(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mouseevent(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mouseevent(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mouseleave(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mouseleave(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mousemove(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mousemove(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mouseout(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mouseout(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mouseover(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mouseover(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery mouseup(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery mouseup(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery off(String events, Object selector, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery off(Map[Object] eventsMap, Object selector){ return null; }

	@Public JQuery on(String events, Object selector, Object data, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery on(Map[Object] eventsMap, Object selector, Object data){ return null; }

	@Public JQuery one(String events, Object selector, Object data, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery one(Map[Object] eventsMap, Object selector, Object data){ return null; }

	@Public JQuery ready(Object handler){ return null; }

	@Public JQuery resize(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery resize(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery scroll(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery scroll(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery select(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery select(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery submit(Object eventData, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery submit(Func[Object, JQueryEventObject] handler){ return null; }

	@Public JQuery trigger(String eventType, Object param1){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14, Object param15){ return null; }
	@Public JQuery trigger(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14, Object param15, Object param16){ return null; }
	@Public JQuery trigger(JQueryEventObject event){ return null; }

	@Public Object triggerHandler(String eventType, Object param1){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14, Object param15){ return null; }
	@Public Object triggerHandler(String eventType, Object param1, Object param2, Object param3, Object param4, Object param5, Object param6, Object param7, Object param8, Object param9, Object param10, Object param11, Object param12, Object param13, Object param14, Object param15, Object param16){ return null; }

	@Public JQuery unbind(String eventType, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery unbind(String eventType, boolean fls){ return null; }
	@Public JQuery unbind(Object evt){ return null; }

	@Public JQuery undelegate(){ return null; }
	@Public JQuery undelegate(Object selector, String eventType, Func[Object, JQueryEventObject] handler){ return null; }
	@Public JQuery undelegate(Object selector, Object events){ return null; }
	@Public JQuery undelegate(String namespace){ return null; }

	// INTERNALS
	@Public DomElement context;
	@Public String jquery;
	@Public JQuery pushStack(Object[] elements){ return null; }
	@Public JQuery pushStack(Object[] elements, Object name, Object arguments){ return null; }

	// MANIPULATION
	@Public JQuery after(Object content1){ return null; }
	@Public JQuery after(Object content1, Object content2){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15){ return null; }
	@Public JQuery after(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15, Object content16){ return null; }
	@Public JQuery after(Func[Object, Object] func){ return null; }

	@Public JQuery append(Object content1){ return null; }
	@Public JQuery append(Object content1, Object content2){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15){ return null; }
	@Public JQuery append(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15, Object content16){ return null; }
	@Public JQuery append(Func[Object, Object] func){ return null; }

	@Public JQuery appendTo(Object target){ return null; }

	@Public JQuery before(Object content1){ return null; }
	@Public JQuery before(Object content1, Object content2){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15){ return null; }
	@Public JQuery before(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15, Object content16){ return null; }
	@Public JQuery before(Func[Object, Object] func){ return null; }

	@Public JQuery clone(boolean withDataAndEvents, boolean deepWithDataAndEvents){ return null; }

	@Public JQuery detach(Object selector){ return null; }

	@Public JQuery empty(){ return null; }

	@Public JQuery insertAfter(Object target){ return null; }
	@Public JQuery insertBefore(Object target){ return null; }

	@Public JQuery prepend(Object content1){ return null; }
	@Public JQuery prepend(Object content1, Object content2){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15){ return null; }
	@Public JQuery prepend(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15, Object content16){ return null; }
	@Public JQuery prepend(Func[Object, Object, Object] func){ return null; }

	@Public JQuery prependTo(Object target){ return null; }

	@Public JQuery remove(Object selector){ return null; }

	@Public JQuery replaceAll(Object target){ return null; }

	@Public JQuery replaceWith(Object func){ return null; }

	@Public JQuery text(String textString){ return null; }
	@Public String text(){ return null; }

	@Public Object[] toArray(){ return null; }

	@Public JQuery unwrap(){ return null; }

	@Public JQuery wrap(Object wrappingElement){ return null; }
	@Public JQuery wrap(Func[Object, Object] func){ return null; }

	@Public JQuery wrapAll(Object wrappingElement){ return null; }

	@Public JQuery wrapInner(Object wrappingElement){ return null; }
	@Public JQuery wrapInner(Func[Object, Object] func){ return null; }

	// MISCELLANEOUS
	@Public DomElement each(Func[JQuery, Object, DomElement] func){ return null; }

	//Object get(int index){ return null; }

	@Public int index(Object selectorOrElement){ return null; }

	// PROPERTIES
	@Public int length;
	@Public HTMLElement get(String index){ return null; }
	@Public HTMLElement get(int index){ return null; }

	// TRAVERSING
	@Public JQuery add(String selector, Object context){ return null; }
	@Public JQuery add(Object content1){ return null; }
	@Public JQuery add(Object content1, Object content2){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15){ return null; }
	@Public JQuery add(Object content1, Object content2, Object content3, Object content4, Object content5, Object content6, Object content7, Object content8, Object content9, Object content10, Object content11, Object content12, Object content13, Object content14, Object content15, Object content16){ return null; }

	@Public JQuery add(String html){ return null; }
	@Public JQuery add(JQuery obj){ return null; }

	@Public JQuery andSelf(){ return null; }

	@Public JQuery children(Object selector){ return null; }

	@Public JQuery closest(String selector){ return null; }
	@Public JQuery closest(String selector, DomElement context){ return null; }
	@Public JQuery closest(JQuery obj){ return null; }
	@Public JQuery closest(Object element){ return null; }
	@Public Object closest(Object selectors, DomElement context)[];

	@Public JQuery contents(){ return null; }

	@Public JQuery end(){ return null; }

	@Public JQuery eq(int index){ return null; }

	@Public JQuery filter(String selector){ return null; }
	@Public JQuery filter(Func[Object, Object] func){ return null; }
	@Public JQuery filter(Object element){ return null; }
	@Public JQuery filter(JQuery obj){ return null; }

	@Public JQuery find(String selector){ return null; }
	@Public JQuery find(Object element){ return null; }
	@Public JQuery find(JQuery obj){ return null; }

	@Public JQuery first(){ return null; }

	@Public JQuery has(String selector){ return null; }
	@Public JQuery has(DomElement contained){ return null; }

	@Public JQuery is(String selector){ return null; }
	@Public JQuery is(Func[Object, Object] func){ return null; }
	@Public JQuery is(Object element){ return null; }
	@Public JQuery is(JQuery obj){ return null; }

	@Public JQuery last(){ return null; }

	@Public JQuery map(Func[Object, Object, DomElement] callback){ return null; }

	@Public JQuery next(String selector){ return null; }

	@Public JQuery nextAll(String selector){ return null; }

	@Public JQuery nextUntil(String selector, String filter){ return null; }
	@Public JQuery nextUntil(DomElement element, String filter){ return null; }

	@Public JQuery not(String selector){ return null; }
	@Public JQuery not(Func[Object, Object] func){ return null; }
	@Public JQuery not(Object element){ return null; }
	@Public JQuery not(JQuery obj){ return null; }

	@Public JQuery offsetParent(){ return null; }

	@Public JQuery parent(String selector){ return null; }

	@Public JQuery parents(String selector){ return null; }

	@Public JQuery parentsUntil(String selector, String filter){ return null; }
	@Public JQuery parentsUntil(DomElement element, String filter){ return null; }

	@Public JQuery prev(String selector){ return null; }

	@Public JQuery prevAll(String selector){ return null; }

	@Public JQuery prevUntil(String selector, String filter){ return null; }
	@Public JQuery prevUntil(DomElement element, String filter){ return null; }

	@Public JQuery siblings(String selector){ return null; }

	@Public JQuery slice(int start, int end){ return null; }

	// UTILITIES
	@Public Object queue(String queueName)[];
	@Public JQuery queue(String queueName, Object newQueueOrCallback){ return null; }
	@Public JQuery queue(Object newQueueOrCallback){ return null; }
}


/*
   Static members of jQuery (those on $ and jQuery themselves)
 */

class JQueryStatic {

	// AJAX
	@Public JQueryAjaxSettings ajax(String url, JQueryAjaxSettings settings){ return null; }

	@Public Object ajaxPrefilter(String dataTypes, Func[Object, Object, Object, JQueryXHR] handler){ return null; }
	@Public Object ajaxPrefilter(Func[Object, Object, Object, JQueryXHR] handler){ return null; }

	@Public void ajaxSetup(Object options){ return; }

	@Public JQueryXHR get(String url, Object data, Object success, Object dataType){ return null; }
	@Public JQueryXHR getJSON(String url, Object data, Object success){ return null; }
	@Public JQueryXHR getScript(String url, Object success){ return null; }

	@Public String param(Object obj){ return null; }
	@Public String param(Object obj, boolean traditional){ return null; }

	@Public JQueryXHR post(String url, Object data, Object success, Object dataType){ return null; }

	// CALLBACKS
	@Public JQueryCallback Callbacks(Object flags){ return null; }

	// CORE
	@Public Object holdReady(boolean hold){ return null; }
/*
	@Public JQuery (String selector, Object context){ return null; }
	@Public JQuery (Element element){ return null; }
	@Public JQuery (object: { }){ return null; }
	@Public JQuery (Element[] elementArray){ return null; }
	@Public JQuery (JQuery object){ return null; }
	@Public JQuery (Func func){ return null; }
	@Public JQuery (){ return null; }
*/
	@Public Object noConflict(boolean removeAll){ return null; }

	@Public JQueryPromise when(Object deferred1){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11, Object deferred12){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11, Object deferred12, Object deferred13){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11, Object deferred12, Object deferred13, Object deferred14){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11, Object deferred12, Object deferred13, Object deferred14, Object deferred15){ return null; }
	@Public JQueryPromise when(Object deferred1, Object deferred2, Object deferred3, Object deferred4, Object deferred5, Object deferred6, Object deferred7, Object deferred8, Object deferred9, Object deferred10, Object deferred11, Object deferred12, Object deferred13, Object deferred14, Object deferred15, Object deferred16){ return null; }
	// CSS
	@Public Object css(Object e, String propertyName, Object value){ return null; }
	@Public Object css(Object e, Object propertyName, Object value){ return null; }
	@Public Map[Object] cssHooks;

	// DATA
	@Public Object data(DomElement element, String key, Object value){ return null; }

	@Public Object dequeue(DomElement element, String queueName){ return null; }

	@Public boolean hasData(DomElement element){ return null; }

	@Public Object[] queue(DomElement element, String queueName);
	@Public JQuery queue(DomElement element, String queueName, Object newQueueOrCallback){ return null; }

	@Public JQuery removeData(DomElement element, String name){ return null; }

	// EFFECTS
	//Object fxFunc[void] tick, double interval; Func[void] stop; JQuerySpeeds speeds, boolean off, Object step };

	// EVENTS
	@Public Object proxy(Object context, Object name){ return null; }

	// INTERNALS
	@Public void error(Object message){ return; }

	// MISCELLANEOUS
	@Public Object expr;
	@Public Object fn;  //TODO how we want to type this
	@Public boolean isReady;

	// PROPERTIES
	@Public JQueryBrowserInfo browser;
	@Public JQuerySupport support;

	// UTILITIES
	@Public boolean contains(DomElement container, DomElement contained){ return null; }

	@Public Object each(Object collection, Func[Object, Object, Object] callback){ return null; }

	@Public Object extend(Object target, Object objs1){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14, Object objs15){ return null; }
	@Public Object extend(Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14, Object objs15, Object objs16){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14, Object objs15){ return null; }
	@Public Object extend(boolean deep, Object target, Object objs1, Object objs2, Object objs3, Object objs4, Object objs5, Object objs6, Object objs7, Object objs8, Object objs9, Object objs10, Object objs11, Object objs12, Object objs13, Object objs14, Object objs15, Object objs16){ return null; }

	@Public Object globalEval(String code){ return null; }

	@Public Object[] grep(Object[] array, Object func, boolean invert){ return null; }

	@Public int inArray(Object value, Object[] array, int fromIndex){ return null; }

	@Public boolean isArray(Object obj){ return null; }
	@Public boolean isEmptyObject(Object obj){ return null; }
	@Public boolean isFunc(Object obj){ return null; }
	@Public boolean isNumeric(Object value){ return null; }
	@Public boolean isPlainObject(Object obj){ return null; }
	@Public boolean isWindow(Object obj){ return null; }
	@Public boolean isXMLDoc(DomNode node){ return null; }

	@Public Object makeArray(Object obj)[];

	@Public JQuery map(Object[] array, Func[Object, Object, Object] callback){ return null; }

	@Public Object merge(Object[] first, Object[] second)[];

	@Public Object noop(){ return null; }

	@Public int now(){ return null; }

	@Public Object parseJSON(String json){ return null; }

	//This FIXME should return an XMLDocument
	@Public Object parseXML(String data){ return null; }

	@Public JQuery queue(DomElement element, String queueName, Object[] newQueue){ return null; }

	@Public String trim(String str){ return null; }

	@Public String type(Object obj){ return null; }

	@Public Object[] unique(Object[] arr){ return null; }
}

//JQueryStatic jQuery = null;

//JQueryStatic NameSpace.getjQuery(){ return null; }
//JQuery jQuery(String selector){ return null; }
//JQuery jQuery(String selector, Object context){ return null; }
//JQuery jQuery(Element element){ return null; }
//JQuery jQuery(object: { }){ return null; }
//JQuery jQuery(Element[] elementArray){ return null; }
//JQuery jQuery(JQuery object){ return null; }
//JQuery jQuery(Func func){ return null; }
//JQuery jQuery(){ return null; }

