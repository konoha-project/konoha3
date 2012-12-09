import("cstyle", "while");
import("xml");

void main() {
	String K_PATH = "/path/to/minikonoha"; // ADD YOUR minikonoha DIR PATH
	XmlReader reader = new XmlReader(K_PATH + "/package/xml/test/Debug/rss.xhtml");
	while (reader.read()) {
		String type = reader.nodeType().toString();
		System.p(type);
		if (reader.hasAttributes() && reader.getAttributeCount() > 0) {
			while (reader.moveToNextAttribute()) {
				System.p(reader.constName() + "=" + reader.constValue());
			}
		}
		if (type == TYPE_ELEMENT.toString() && reader.constName() == "title"){
		//if (type == TYPE_ELEMENT && reader.constName() == "title"){
			reader.read();
			if(reader.nodeType() == TYPE_TEXT){
				System.p(reader.constValue());
			}
		}
		else if(type == TYPE_ATTRIBUTE.toString()){
			System.p("att:" + reader.getAttribute());
		}
	}
	reader.close();
}

main();
