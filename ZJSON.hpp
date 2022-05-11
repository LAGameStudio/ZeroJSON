#include "ZeroTypes.h"

// JSON implements a simplified, slightly fault tolerant, UTF-8 JSON reader/writer.
class JSON {
	public:
		static string EscapeString( const char *in ) {
			Zstring out;
			const char *p = in;
			while (*p != '\0') {
				switch (*p) {
					case '"': out += "\\\""; break;
					case '\\' : out += "\\\\"; break;
					case '\b': out += "\\b"; break;
					case '\r': out += "\\r"; break;
					case '\t': out += "\\t"; break;
					case '\n': out += "\\n"; break;
					case '\f': out += "\\f"; break;
					default: out += *p; break;
				}
				p++;
			}
			return out.value;
		}
		class Node;
		class Array {
		public:
			Zpointer<JSON> container;
			Zpointer<Node> parent;
			ZIndexed<Zdisposable<JSON>> elements;
			int Length() { return (int)this->elements.length; }
			JSON *Get(int index) { return this->elements(index).pointer; }
			JSON *Append() {
			 elements.Increase();
			 elements[Length() - 1].Instantiate();
			 return elements[Length() - 1].pointer;
			}
			string toString();
		};
		class Object : public LinkedList {
		public:
			Zpointer<JSON> container;
			Zpointer<Node> parent;
			Zbool parseError;
			Node *Has(const char *key);
			Node *Get(const char *key);
			void Add(const char *key, const char *inner, const char *outer);
			void Drop(const char *key);
			void fromString(const char *in);
			string toString();
			Object() : LinkedList() {}
		};
		class Node : public ListItem {
		public:
			enum Type { Null, Boolean, Numeric, String, Array, Object } type;
			Zstring key;
			Zstring s;
			Zbool b;
			Zdouble d;
			Zint i;
			Zdisposable<JSON::Array> array;
			Zdisposable<JSON::Object> object;
			Zpointer<JSON> container;
			Zpointer<Node> parent;
			Node() : ListItem() {
				type = Type::Null;
			}
			void Clear() {
				key = "";
				s = "";
				b = false;
				d = 0.0;
				i = 0;
				array.Deinstantiate();
				object.Deinstantiate();
				type = Type::Null;
			}
			bool isNull() {	return type == Type::Null; }
			bool isString() { return type == Type::String; }
			bool isNumeric() { return type == Type::Numeric; }
			bool isObject() { return type == Type::Object; }
			bool isArray() { return type == Type::Array; }
			bool isBool() { return type == Type::Boolean; }
			void SetParent(Node *p) {
				this->parent = p;
				if (this->type == Type::Object) this->object->parent = p;
				else if (this->type == Type::Array) this->array->parent = p;
			}
			void parseArray(const char *in);
			void parseObject(const char *in);
			void parseValue(const char *in);
			void fromString(const char *in, const char *outer);
			string toString();
			string asKeyPair() {
				return string("\"") + JSON::EscapeString(key) + string("\" : ") + toString();
			}
		} node;
		void Clear() {
			parseError = false;
			node.Clear();
			content = "";
			outer = "";
		}
		JSON *Load(const char *fn) {
			JSON *j = new JSON;
			j->fromString(file_as_string(fn).c_str());
			return j;
		}
		bool Save(const char *fn) {
			return string_as_file(toString().c_str(), fn);
		}
		Zstring outer,content;
		void fromString( const char *in, bool memCleanup=true ) {
			this->Clear();
			content = in;
			const char *p = NULL;
			while (*(p = content.Next(outer)) != '\0') {
				if (outer == " ") {
					Zstring inner(p);
					if (inner == "null") {
						return;
					} else {
						parseError = true;
						cout << "JSON::parseError was set true.  This means an unexpected value was encountered, or other parsing issue." << endl;
						return;
					}
				} else if (outer == "[") {
					node.fromString(p,outer);
				} else if (outer == "{") {
					node.fromString(p,outer);
				} else {
					parseError = true;
					cout << "JSON::parseError was set true.  This means an unexpected value was encountered, or other parsing issue." << endl;
					return;
				}
			}
			if (memCleanup) {
				content = "";
			}
		}
		string toString() {
			return node.toString();
		}
		Zbool parseError;
		void UnitTests() {
			// Test1 is designed to succeed.
			const char *test1="[\n\r{ able: bodied },\n\r{ \"workers\" : untie },\n\r{\n\r\t \"test_bool\" : true,\t\r \"test_string\" : \"a string \tof\r\nchars\",\n \"test_array\" : [ \"one\", \"two\", \"three\", 4, 5 ],\n\r\t \"test_object\" : { \"one\" : \"1\", \"two\" : 2, \"three\" : 3.33, \"fourth\" : null, \"fifth\" : \"jack daniels\", \"sixth\" : false }, \"test_null\" : null}]";
			cout << "====> JSON Test1 (should succeed):" << endl;
			fromString(test1);
			cout << "> Input String:" << endl << test1 << endl << "< Output:" << endl << toString() << endl;
			// Test2 is designed to succeed.
			const char *test2="{ \"test_bool\" : true, \"test_string\" : \"a string \tof\r\nchars\", \"test_array\" : [ \"one\", \"two\", \"three\", 4, 5 ], \"test_object\" : { \"one\" : \"1\", \"two\" : 2, \"three\" : 3.33, \"fourth\" : null, \"fifth\" : \"jack daniels\", \"sixth\" : false }, \"test_null\" : null} ";
			cout << "====> JSON Test2 (should succeed):" << endl;
			fromString(test2);
			cout << "> Input String:" << endl << test2 << endl << "< Output:" << endl << toString() << endl;
			// Test3 is designed to succeed.
			const char *test3="{ \"test_bool\" : true}";
			cout << "====> JSON Test3 (should succeed):" << endl;
			fromString(test3);
			cout << "> Input String:" << endl << test3 << endl << "< Output:" << endl << toString() << endl;
			// Test4 is designed to succeed.
			const char *test4="[ 5,6,7,8 ]";
			cout << "====> JSON Test4 (should succeed):" << endl;
			fromString(test4);
			cout << "> Input String:" << endl << test4 << endl << "< Output:" << endl << toString() << endl;
			// Test5 is designed to succeed.
			const char *test5="[ { \"id\":5 },{ \"id\": 6, id:12 } ]";
			cout << "====> JSON Test5 (should succeed):" << endl;
			fromString(test5);
			cout << "> Input String:" << endl << test5 << endl<< "< Output:"  << endl << toString() << endl;
			// Test6 is designed to succeed.
			const char *test6="[ \"apple\", \"mango\", \"peach\" ]";
			cout << "====> JSON Test6 (should succeed):" << endl;
			fromString(test6);
			cout << "> Input String:" << endl << test6 << endl << "< Output:" << endl << toString() << endl;
			// Test7 is designed to fail.
			const char *test7="[ \"apple, \"mango\", \"peach\" ]";
			cout << "====> JSON Test7 (should fail with errors):" << endl;
			fromString(test7);
			cout << "> Input String:" << endl << test7 << endl << "< Output:" << endl << toString() << endl;
			// Last test!
			cout << "====> Done!" << endl;
		}
		bool Test(CommandLine::Option *test) {
			if (test) {
				if (file_exists(test->value.c_str())) {
					fromString(file_as_string(test->value.c_str()).c_str());
					cout << "JSON Test (from file `" << test->value.c_str() << "`):" << endl << toString() << endl;
				}
				else {
					fromString(test->value.c_str());
					cout << "JSON Test (from parameter value: `" << test->value.c_str() << "`):" << endl << toString() << endl;
				}
				return true;
			}
			return false;
		}
		JSON() { node.container = this; }
};


/// JSON::Array
string JSON::Array::toString() {
	Zstring out;
	for (int i = 0; i < (int)elements.length; i++) {
		out += elements[i]->toString();
		if (i + 1 == elements.length) continue;
		out += ", ";
	}
	return string("[") + out.value + string("]");
}

/// JSON::Object
JSON::Node *JSON::Object::Has(const char *key) {
	FOREACH(JSON::Node, n) if (n->key == key) return n;
	return nullptr;
}
JSON::Node *JSON::Object::Get(const char *key) {
	return Has(key);
}
void JSON::Object::Add(const char *key, const char *inner, const char *outer) {
	JSON::Node *n = Has(key);
	if (n) {
		Remove(n);
		delete n;
	}
	n=new JSON::Node();
	n->key = key;
	n->fromString(inner, outer);
	n->parent = this->parent;
	n->container = this->container;
	Append(n);
}
void JSON::Object::Drop(const char *key) {
	JSON::Node *n = Has(key);
	if (n) {
		Remove(n);
		delete n;
	}
}
void JSON::Object::fromString(const char *in) {
	Clear();
	parseError = false;
	Zstring content(in);
	Zstring outer;
	const char *p = NULL;
	while (*(p = content.Next(outer)) != '\0') {
		if (outer != " " && outer != "\"" ) {
			parseError = true;
			cout << "JSON::Object::fromString set parseError to true.  Error has occurred while parsing an object key.  Hint: there is an issue with an object property name." << endl;
			return;
		}
		Zstring key = p;
		Zstring value;
		bool colon = false;
		if (key.contains(':')) {
			key = "";
			colon = true;
			const char *q = p;
			while (*q != ':' && *q != '\0') { key += *q; q++;	}
			if  ( *q != '\0' ) q++;
			if (*q != '\0') value = q;
		}
		if (value.length == 0) {
			p = content.Next(outer);
			value = p;
			if (value.contains(':')) {
				colon = true;
				if ( *p == ':' ) value = (p + 1);
				if (value.length == 0) value = (p = content.Next(outer));
			}
		}
		if ( !colon ) {
			parseError = true;
			cout << "JSON::Object::fromString set parseError to true.  Error has occurred while parsing an object.  Hint: a colon is out of place." << endl;
			return;
		}
		Add(key.c_str(), value.c_str(), outer.c_str());	
	}
}
string JSON::Object::toString() {
	Zstring out;
	FOREACH(JSON::Node, n) {
		out += n->asKeyPair();
		if (n->next) out += ", ";
	}
	return string("{ ") + out.value + string(" }");
}

/// JSON::Node
void JSON::Node::parseArray(const char *in) {
	type = Type::Array;
	this->array.Instantiate();
	this->array->container = container.pointer;
	this->array->parent = parent.pointer;
	Zstring content(in);
	Zstring outer;
	const char *p = NULL;
	while (*(p = content.Next(outer)) != '\0') {
		int index = this->array->Length();
		if (outer == " ") {
			JSON *j=this->array->Append();
			j->node.parseValue(p);
		}
		else if (outer == "\"") {
			JSON *j = this->array->Append();
			j->node.type = Type::String;
			j->node.s = p;
			j->node.SetParent(this);
			j->node.parent = this;
		}
		else if (outer == "[") {
			JSON *j = this->array->Append();
			j->node.SetParent(this);
			j->node.parseArray(p);
		}
		else if (outer == "{") {							
			JSON *j = this->array->Append();
			j->node.SetParent(this);
			j->node.parseObject(p);
		}
	}
}
void JSON::Node::parseObject(const char *in) {
	type = Type::Object;
	this->object.Instantiate();
	this->object->container = container.pointer;
	this->object->parent = parent.pointer;
	this->object->fromString(in);
}
void JSON::Node::parseValue(const char *in) {
	Zstring value(in);
	if (value == "null") {
		type = Type::Null;
	} else if (value == "false") {
		type = Type::Boolean;
		b = false;
	}
	else if (value == "true") {
		type = Type::Boolean;
		b = true;
	} else if (is_integer(value.c_str()) || is_decimal(value)) {
		type = Type::Numeric;
		d = value.c_str();
	}
	else {
		type = Type::String;
		s = value;
	}
}
void JSON::Node::fromString(const char *in, const char *outer) {
	switch (*outer) {
	case '[': parseArray(in); return;
	case '{': parseObject(in); return;
	 default: parseValue(in); return;
	}
}
string JSON::Node::toString() {
	switch (type) {
		case Type::Boolean: return (b ? "true" : "false");
		case Type::Array: return (this->array->toString());
		case Type::Numeric: return d.toString();
		case Type::Object: return this->object->toString();
		case Type::Null: return "null";
		case Type::String: return string("\"")+JSON::EscapeString(s)+string("\"");
		default: cout << "JSON::Node::toString() encountered an invalid type." << endl; return "null";
	}
}
