import("Type.Float");
import("Syntax.JavaStyleClass");
import("Syntax.GlobalVariable");
import("Syntax.Null");
import("MiniKonoha.Map");
import("JavaStyle.Object");
import("JavaScript.Array");

class Window{
	@Public void setInterval(Func[void] code, int millisec){ };
	@Public void setTimeout(Func[void] code, int millisec){ };
}

/*
class DomElement;
class DomAttr;
class DomText;
class DomCDATASection;
class DomEntityReference;
class DomEntity;
class DomProcessingInstraction;
class DomComment;
class DomDocument;
class DomDocumentType;
class DomDocumentFragment;
class DomNotation;
class DomNodeList;
*/
class DomNode{
	@Public DomNode/*DomAttr*/     attributes;
	@Public String      baseURI;
	@Public DomNode[]/*DomNodeList*/ childNodes;
	@Public DomNode     firstChild;
	@Public DomNode     lastChild;
	@Public String      localName;
	@Public String      namespaceURI;
	@Public String      nodeName;
	@Public DomNode     nextSibiling;
	@Public int         nodeType;
	@Public String      nodeValue;
	@Public DomNode     ownerDocument;
	@Public DomNode     parentNode;
	@Public String      prefix;
	@Public DomNode     previousSibiling;
	@Public String      textContent;

	@Public DomNode  DomNode.appendChild(DomNode node){ return null; }
	@Public DomNode  DomNode.cloneNode(){ return null; }
	@Public DomNode  DomNode.cloneNode(boolean deep){ return null; }
	@Public int      DomNode.compareDocumentPosition(DomNode node){ return 0; }
	@Public boolean  DomNode.hasAttributes(){ return false; }
	@Public boolean  DomNode.hasChildNodes(){ return false; }
	@Public DomNode  DomNode.insertBefore(DomNode newnode, DomNode existingnode){ return null; }
	@Public boolean  DomNode.isDefaultNamespace(String namespaceURI){ return false; }
	@Public boolean  DomNode.isEqualNode(DomNode node){ return false; }
	@Public boolean  DomNode.isSameNode(DomNode node){ return false; }
	@Public boolean  DomNode.isSupported(String feature){ return false; }
	@Public boolean  DomNode.isSupported(String feature, String version){ return false; }
	@Public String   DomNode.lookupNamespaceURI(String prefix){ return null; }
	@Public String   DomNode.lookupPrefix(String namespaceURI){ return null; }
	@Public void     DomNode.normalize(){}
	@Public DomNode  DomNode.removeChild(DomNode node){ return null; }
	@Public DomNode  DomNode.replaceChild(DomNode newnode, DomNode oldnode){ return null; }
}

class DomElement extends DomNode{
	@Public Object schemaTypeInfo;
	@Public String tagName;
}

class DomAttr extends DomNode{
	@Public boolean    isId;
	@Public String     name;
	@Public DomElement ownerElement;
	@Public Object     schemaTypeInfo;
	@Public boolean    specified;
	@Public String     value;
}

class DomText extends DomNode{
}

class DomCDATASection extends DomNode{
}

class DomEntityReference extends DomNode{
}

class DomEntity extends DomNode{
}

class DomProcessingInstraction extends DomNode{
}

class DomComment extends DomNode{
}

class DomDocumentFragment extends DomNode{
}

class DomDocument extends DomNode{
	@Public DomAttr     getattributes;
	@Public String      getbaseURI;
	@Public DomNode[]/*DomNodeList*/ getchildNodes;
	@Public DomNode     getfirstChild;
	@Public DomNode     getlastChild;
	@Public String      getlocalName;
	@Public String      getnamespaceURI;
	@Public String      getnodeName;
	@Public DomNode     getnextSibiling;
	@Public int         getnodeType;
	@Public String      getnodeValue;
	@Public DomNode     getownerDocument;
	@Public DomNode     getparentNode;
	@Public String      getprefix;
	@Public DomNode     getpreviousSibiling;
	@Public String      gettextContent;

	@Public DomAttr             createAttribute(String attributename){ return null; }
	@Public DomCDATASection     createCDATASection(){ return null; }
	@Public DomCDATASection     createCDATASection(String text){ return null; }
	@Public DomComment          createComment(){ return null; }
	@Public DomComment          createComment(String text){ return null; }
	@Public DomDocumentFragment createDocumentFragment(){ return null; }
	@Public DomElement          createElement(String elementname){ return null; }
	@Public DomText             createTextNode(String text){ return null; }
	@Public DomElement          getElementById(String elementId){ return null; }
	@Public DomNode[]/*DomNodeList*/         getElementsByTagName(String tagName){ return null; }
	@Public DomNode[]/*DomNodeList*/         getElementsByTagNameNS(String namespoceURI, String tagName){ return null; }
	@Public DomNode             importNode(DomNode node, boolean deep){ return null; }
	@Public void                normalizeDocument(){}
}

class DomDocumentType extends DomNode{
}

class DomNotation extends DomNode{
}

class DomNodeList extends DomNode{
	@Public int length;
	@Public DomNode item(int index){ return null; }
}

class HTMLElement extends DomElement{
}

class HTMLDocument extends DomDocument{
}

class CanvasGradient {
    @Public void addColorStop(float offset, String color){}
}

class ImageData {
}

class CanvasRenderingContext2D {
    @Public float shadowOffsetX;
    @Public float lineWidth;
    @Public float miterLimit;
    @Public HTMLElement canvas;
    @Public Object strokeStyle;
    @Public String font;
    @Public float globalAlpha;
    @Public String globalCompositeOperation;
    @Public float shadowOffsetY;
    @Public Object fillStyle;
    @Public String lineCap;
    @Public float shadowBlur;
    @Public String textAlign;
    @Public String textBaseline;
    @Public String shadowColor;
    @Public String lineJoin;
	
    @Public void restore(){}
    @Public void setTransform(float m11, float m12, float m21, float m22, float dx, float dy){}
    @Public void save(){}
    @Public void arc(float x, float y, float radius, float startAngle, float endAngle){}
    @Public void arc(float x, float y, float radius, float startAngle, float endAngle, boolean anticlockwise){}
    @Public Object measureText(String text){ return null; }
    @Public boolean isPointInPath(float x, float y){ return false; }
    @Public void quadraticCurveTo(float cpx, float cpy, float x, float y){}
    @Public void putImageData(ImageData imagedata, float dx, float dy){}
    @Public void putImageData(ImageData imagedata, float dx, float dy, float dirtyX){}
    @Public void putImageData(ImageData imagedata, float dx, float dy, float dirtyX, float dirtyY){}
    @Public void putImageData(ImageData imagedata, float dx, float dy, float dirtyX, float dirtyY, float dirtyWidth){}
    @Public void putImageData(ImageData imagedata, float dx, float dy, float dirtyX, float dirtyY, float dirtyWidth, float dirtyHeight){}
    @Public void rotate(float angle){}
    @Public void fillText(String text, float x, float y){}
    @Public void fillText(String text, float x, float y, float maxWidth){}
    @Public void translate(float x, float y){}
    @Public void scale(float x, float y){}
    @Public Object createRadialGradient(float x0, float y0, float r0, float x1, float y1, float r1){ return null; }
    @Public void lineTo(float x, float y){}
    @Public void fill(){}
    @Public Object createPattern(HTMLElement image, String repetition){ return null; }
    @Public void closePath(){}
    @Public void rect(float x, float y, float w, float h){}
    @Public void clip(){}
    @Public ImageData createImageData(Object imageDataOrSw){ return null; }
    @Public ImageData createImageData(Object imageDataOrSw, float sh){ return null; }
    @Public void clearRect(float x, float y, float w, float h){}
    @Public void moveTo(float x, float y){}
    @Public ImageData getImageData(float sx, float sy, float sw, float sh){ return null; }
    @Public void fillRect(float x, float y, float w, float h){}
    @Public void bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y){}
    @Public void drawImage(HTMLElement image, float offsetX, float offsetY){}
    @Public void drawImage(HTMLElement image, float offsetX, float offsetY, float width, float height){}
    @Public void drawImage(HTMLElement image, float offsetX, float offsetY, float width, float height, float canvasOffsetX, float canvasOffsetY){}
    @Public void drawImage(HTMLElement image, float offsetX, float offsetY, float width, float height, float canvasOffsetX, float canvasOffsetY, float canvasImageWidth, float canvasImageHeight){}
    @Public void transform(float m11, float m12, float m21, float m22, float dx, float dy){}
    @Public void stroke(){}
    @Public void strokeRect(float x, float y, float w, float h){}
    @Public void strokeText(String text, float x, float y){}
    @Public void strokeText(String text, float x, float y, float maxWidth){}
    @Public void beginPath(){}
    @Public void arcTo(float x1, float y1, float x2, float y2, float radius){}
    @Public Object createLinearGradient(float x0, float y0, float x1, float y1){ return null; }
}
class HTMLCanvasElement extends HTMLElement {
    @Public float width;
    @Public float height;
    @Public String toDataURL(){}
    //@Public String toDataURL(String type, ...Object args[]){}
    @Public CanvasRenderingContext2D getContext(String contextId){}
}

