#include <protop/protop.hh>
#include <fstream>
#include <vector>
#include <cmake.h>

using namespace protop;

struct Context
{
    std::ostream &header;
    std::ostream &source;
    std::string ifname;
    std::vector<std::string> nspace;
};

static const char *TYPES[] =
{
    "double",
    "float",
    "int32_t",
    "int64_t",
    "uint32_t",
    "uint64_t",
    "sint32_t",
    "sint64_t",
    "uint32_t",
    "uint64_t",
    "sint32_t",
    "sint64_t",
    "bool",
    "std::string",
    "std::string",
    nullptr,
};

//extern const char *content_proton_hh;
#include <proton.hh.cc>

static std::vector<std::string> split_package( const std::string &package )
{
    std::vector<std::string> out;
    std::string current;

    const char *ptr = package.c_str();
    while (true)
    {
        if (*ptr == '.' || *ptr == 0)
        {
            if (!current.empty())
            {
                out.push_back(current);
                current.clear();
            }
            if (*ptr == 0) break;
        }
        else
            current += *ptr;
        ++ptr;
    }
    return out;
}

static std::string get_native_type( int type, const std::string &name, bool is_enum, bool is_repeated, bool is_ptr )
{
    std::string result;
    if (is_repeated)
        result = "std::list<";

    if (!is_enum && type >= TYPE_DOUBLE && type <= TYPE_BYTES)
        result += TYPES[type - TYPE_DOUBLE];
    else
    if (is_enum)
        result += "int32_t";
    else
    {
        if (is_ptr)
            result += "std::shared_ptr<" + name + ">";
        else
            result += name;
    }

    if (is_repeated)
        result += ">";

    return result;
}

static void generate_field( Context &ctx, std::shared_ptr<Field> field )
{
    ctx.header << "    ";
    if (field->type.repeated)
        ctx.header << "std::list<";

    if (field->type.id >= TYPE_DOUBLE && field->type.id <= TYPE_BYTES)
        ctx.header << TYPES[field->type.id - TYPE_DOUBLE];
    else
    if (field->type.eref != nullptr)
        ctx.header << "int32_t";
    else
        ctx.header << field->type.name;

    if (field->type.repeated)
        ctx.header << ">";
    ctx.header << ' ' << field->name;

    if (!field->type.repeated)
    {
        if (field->type.id >= TYPE_DOUBLE && field->type.id <= TYPE_SINT64)
            ctx.header << " = 0;\n";
            else
        if (field->type.id == TYPE_BOOL)
            ctx.header << " = false;\n";
        else
            ctx.header << ";\n";
    }
    else
        ctx.header << ";\n";
}
/*
static void print( Context &ctx, std::shared_ptr<Constant> entity )
{
    out << "    " << entity->name << " = " << entity->value << ";\n";
}

static void print( Context &ctx, std::shared_ptr<Enum> entity )
{
    out << "enum " << entity->name << "\n{" << '\n';

    for (auto it : entity->constants)
        print(out, it);
    out << '}' << '\n';
}*/

static void generate_operators( Context &ctx, std::shared_ptr<Message> message )
{
    // not equal
    ctx.source << "bool " << message->name << "::operator!=( const " << message->name << "&that ) const\n{\n"
        << "\treturn !(*this == that);\n}\n";
    // equal
    ctx.source << "bool " << message->name << "::operator==( const " << message->name << "&that ) const\n{\n";
    if (message->fields.size() == 0) ctx.source << "\t(void) that;\n";
    ctx.source << "\treturn\n";
    for (auto it : message->fields)
        ctx.source << "\t\t" << it->name << " == that." << it->name << " &&\n";
    ctx.source << "\t\ttrue;\n";
    ctx.source << "}\n";
}

static void generate_message_decl( Context &ctx, std::shared_ptr<Message> message )
{
    ctx.header << "struct " << message->name << " : public ::" << PROTON_NAMESPACE << "::Document\n{" << '\n';

    // fields
    for (auto it : message->fields) generate_field(ctx, it);
    // functions
    ctx.header << "\n\t" << message->name << "() = default;\n";
    ctx.header << "\t" << message->name << "( " << message->name << "&& ) = default;\n";
    ctx.header << "\t" << message->name << "( const " << message->name << "& ) = default;\n";
    ctx.header << "\tbool operator!=( const " << message->name << "& ) const;\n";
    ctx.header << "\tbool operator==( const " << message->name << "& ) const;\n";
    ctx.header << "\t" << message->name << " &operator=( const " << message->name << "& ) = default;\n";
    ctx.header << "\t::" NEUTRON_NAMESPACE "::MemoryStream serialize();\n";
    ctx.header << "\tvoid deserialize( ::" NEUTRON_NAMESPACE "::MemoryStream & );\n";

    ctx.header << "};" << '\n';
}

static void generate_source( Context &ctx, Proto &proto )
{
    ctx.source << "#include \"" << ctx.ifname << "\"\n";

    // begin prettify namespace
    for (auto item : ctx.nspace)
        ctx.source << "namespace " << item << "{\n";
    // functions
    for (auto it : proto.messages)
    {
        generate_operators(ctx, it);
    }
    // end prettify namespace
    for (auto item : ctx.nspace)
        ctx.source << "} // namespace " << item << "\n";
}

static void generate_header( Context &ctx, Proto &proto )
{
    auto sentinel = proto.package;
    for (auto &c : sentinel) if (c == '.') c = '_';

    ctx.header << "#ifndef " << sentinel << "_header\n";
    ctx.header << "#define " << sentinel << "_header\n";

    ctx.header << "#include <stdint.h>\n";
    ctx.header << "#include <string>\n";
    ctx.header << "#include <list>\n";
    ctx.header << "#include <memory>\n";

    ctx.header << content_proton_hh;

    ctx.nspace = split_package(proto.package);
    // begin prettify namespace
    for (auto item : ctx.nspace)
        ctx.header << "namespace " << item << "{\n";
    // messages
    for (auto it : proto.messages) generate_message_decl(ctx, it);
    // end prettify namespace
    for (auto item : ctx.nspace)
        ctx.header << "} // namespace " << item << "\n";

    ctx.header << "#endif // " << sentinel << "_header\n";
}

static std::string replace_ext( const std::string &name, const std::string &ext )
{
    auto spos = name.rfind("/");
    auto dpos = name.rfind(".");
    if (dpos != std::string::npos && (spos == std::string::npos || spos < dpos))
        return name.substr(0, dpos) + ext;
    else
        return name + ext;
}

static std::string filename( const std::string &name, bool ext = true )
{
    std::string out;
    auto pos = name.rfind("/");
    if (pos == std::string::npos)
        out = name;
    else
        out = name.substr(pos+1);

    if (!ext && (pos = out.rfind(".")) != std::string::npos)
        out = out.substr(0, pos);
    return out;
}

int main( int argc, char **argv )
{
    if (argc != 4)
    {
        std::cerr << "Usage: proton <proto file> <header> <source>\n";
        return 1;
    }

    std::string hfname = argv[2];
    std::string sfname = hfname;
    if (hfname.empty() || hfname.back() == '/') hfname += "out.hh";
    std::string ifname = filename(hfname);
    sfname = replace_ext(hfname, ".cc");

    std::cout << " Proto: " << argv[1] << "\n";
    std::cout << "Header: " << hfname << " (" << ifname << ")\n";
    std::cout << "Source: " << sfname << '\n';

    std::ifstream input(argv[1]);
    if (!input.good()) return 1;

    std::ofstream header(hfname);
    if (!header.good()) return 1;
    std::ofstream source(sfname);
    if (!source.good()) return 1;

    Context context{header, source, ifname, {} };

    Proto tree;
    Proto::parse(tree, input, argv[1]);
    generate_header(context, tree);
    generate_source(context, tree);

    return 0;
}