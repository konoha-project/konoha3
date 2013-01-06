
import("Syntax.JavaStyleClass");
import("Type.Float");

@Public int Json.getInt(int index){
	return this.get(index).toint();
}

@Public String Json.getString(int index){
	return this.get(index).toString();
}

@Public boolean Json.getBoolean(int index){
	return this.get(index).toBoolean();
}

@Public float Json.getFloat(int index){
	return this.get(index).tofloat();
}

@Public void Json.set(int index, String val){
	this.set(index, val.toJson());
}

@Public void Json.set(int index, int val){
	this.set(index, val.toJson());
}

@Public void Json.set(int index, float val){
	this.set(index, Json.parse(val.toString());
}

@Public void Json.set(int index, boolean val){
	this.set(index, Json.parse(val.toString());
}

@Public void Json.add(String val){
	this.add(val.toJson());
}

@Public void Json.add(int val){
	this.add(val.toJson());
}

@Public void Json.add(float val){
	this.add(Json.parse(val.toString()));
}

@Public void Json.add(boolean val){
	this.add(Json.parse(val.toString()));
}

