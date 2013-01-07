import("Syntax.CStyleWhile");
import("Lib.Libxml2.Xmlreader");

void main() {
	String K_PATH = "/path/to/minikonoha"; // set absolute path of libxml pkg dir
	XmlReader reader = new XmlReader(K_PATH + "/rss.xml");
	while (reader.read()) {
		String type = reader.nodeType().toString();
		System.p(type);
		if(reader.hasAttributes() && reader.getAttributeCount() > 0) {
			while (reader.moveToNextAttribute()) {
				System.p(reader.constName() + "=" + reader.constValue());
			}
		}
		if(type == XML_READER_TYPE_ELEMENT.toString() && reader.constName() == "title"){
			//if(type == TYPE_ELEMENT && reader.constName() == "title"){
			reader.read();
			if(reader.nodeType() == XML_READER_TYPE_TEXT){
				System.p(reader.constValue());
			}
		}
			else if(type == XML_READER_TYPE_ATTRIBUTE.toString()){
				System.p("att:" + reader.getAttribute());
			}
		}
		reader.close();
	}

	main();
