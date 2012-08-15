#include <fstream>
#include <string>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <vector>

using namespace std;

void split(string &str, string &type, string &name)
{
    unsigned i;
    const char *s = str.c_str();
    for (i = 0; i < str.size(); ++i) {
        if (s[i] == ':') {
            break;
        }
    }
    type = str.substr(0, i);
    name = str.substr(i+1);
}

string trim(string &str)
{
    const char *s = str.c_str();
    for (unsigned i = 0; i < str.size(); ++i) {
        if (s[i] != ' ') {
            return str.substr(i);
        }
    }
    return str;
}

#define TAB "  "
class idlgen {
protected:
    ofstream ofs;
public:
    ostream &os() { return ofs; }
    virtual void init(const char *file) {};
    virtual ostream &begin() { return os(); }
    virtual ostream &end() { return os(); }
    virtual ostream &type_begin(string &message_name) { return os(); }
    virtual ostream &type_end(string &message_name) { return os(); }
    virtual ostream &field(string &type, string &field) { return os(); }
};

class cppgen : public idlgen {
    string filename;
public:
    void init(const char *file) {
        filename = string(file);
        filename = filename + ".data.h";
#ifdef IO_IDL_OUTPUT
        ofs.open(IO_IDL_OUTPUT, ios::out);
#else
        ofs.open(filename.c_str(), ios::out);
#endif
    }
    struct ToUpper {
        char operator()(char c) {
            if (c == '/') return '_';
            if (c == '-') return '_';
            if (c == ' ') return '_';
            if (c == '.') return '_';
            return toupper(c);
        }
    };
    ostream &begin() {
        string str = filename;
        transform(str.begin(), str.end(), str.begin(), ToUpper());
        os() << "#include <stdint.h>" << endl;
        os() << "#ifndef " << str << endl;
        os() << "#define " << str << endl;
        return os();
    }
    ostream &end() {
        os() << "#endif /* end of include guard */" << endl;
        return os();
    }
    ostream &type_begin(string &message_name) {
        os() << "struct " << message_name;
        return os();
    }
    ostream &type_end(string &message_name) {
        return os();
    }
    ostream &field(string &type, string &field) {
        os() << TAB;
        if (type == "u16") {
            os() << "uint16_t";
        }
        if (type == "u32") {
            os() << "uint32_t";
        }
        else if (type == "string") {
            os() << "char";
        }
        os() << " " << field;
        if (type == "string") {
            os() << "[]";
        }
        os() << ";";
        return os();
    }
};

void message(const char *file, ifstream &ifs, string &name, idlgen &gen)
{
    string str;
    vector<string> v;
    string T = name.substr(8);
    while(getline(ifs, str)) {
        if (str.size() == 0) {
            break;
        }
        v.push_back(trim(str));
    }
    gen.type_begin(T) << " {" << endl;
    for (vector<string>::iterator I = v.begin(),
            E = v.end(); I != E; ++I) {
        string type, name;
        split(*I, type, name);
        gen.field(type, name) << endl;
    }
    gen.type_end(T) << "};" << endl << "/* end of " << T << " */" << endl;
    //gen.os() << "static inline void " << name << "_init(char buffer[]";
    //for (vector<string>::iterator I = v.begin(),
    //        E = v.end(); I != E; ++I) {
    //    string type, name;
    //    split(*I, type, name);
    //    gen.field(type, name) << endl;
    //}
    //gen.os() << ") {" << endl << TAB;
    //gen.type_begin(T) << "tmp;" << endl << TAB;
    //gen.os() << "size_t len" << endl << TAB;
    //for (vector<string>::iterator I = v.begin(),
    //        E = v.end(); I != E; ++I) {
    //    string type, name;
    //    split(*I, type, name);
    //    gen.os() << "tmp." << name << " = ";
    //    if (type == "string") {
    //    }
    //    << name << endl << TAB;
    //    gen.field(type, name) << endl;
    //}
    ////tmp.vlen = strlen(q);
    ////len = LOG_PROTOCOL_SIZE+log.vlen;
    ////memcpy(buf, &log, LOG_PROTOCOL_SIZE);
    ////memcpy(buf+LOG_PROTOCOL_SIZE, q, log.vlen);
    ////fprintf(stderr, "len=%zd, buf=%s\n", len, buf+LOG_PROTOCOL_SIZE);
    ////assert(lio_write(lio, buf, len) == LIO_OK);

}

void service(const char *file, ifstream &ifs, string &name, idlgen &gen)
{
    string str;
    vector<string> v;
    while(getline(ifs, str)) {
        if (str.size() == 0) {
            break;
        }
        v.push_back(trim(str));
    }
    for (vector<string>::iterator I = v.begin(),
            E = v.end(); I != E; ++I) {
        //string type, name, param;
        //split(*I, type, name);
        //gen.field(type, name) << endl;
    }
}

int main(int argc, char const* argv[])
{
    if (argc <= 1) {
        cerr << "usage " << argv[0] << " idlfile" << endl;
        return 1;
    }
    ifstream ifs(argv[1]);
    string str;
    cppgen gen;
    gen.init(argv[1]);

    if(ifs.fail()) {
        cerr << "File do not exist.: " << argv[1] << endl;
        return 1;
    }
    gen.begin();
    while(getline(ifs, str)) {
        const char *t = str.c_str();
        if (str.compare(0, 8, "message ") == 0) {
            message(argv[1], ifs, str, gen);
        }
        else if (strncmp(t, "service ", 8) == 0) {
            service(argv[1], ifs, str, gen);
        }
    }
    gen.end();
    return 0;
}
