#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <cmake.h>
#include <filesystem>

static std::filesystem::path input_path;

enum State
{
    PROCESS,
    COPY,
    IGNORE
};

struct OutputStream
{
    virtual void start() = 0;
    virtual void finish() = 0;
    virtual OutputStream &put( int c ) = 0;
    virtual OutputStream &puts( const std::string &s ) = 0;
};

struct PlainOutputStream : public OutputStream
{
    std::ostream &out_;
    PlainOutputStream( std::ostream &out ) : out_(out) {}
    void start() {};
    void finish() {};
    inline OutputStream &put( int c ) { out_.put((char)c); return *this; }
    inline OutputStream &puts( const std::string &s ) { out_ << s; return *this; }
};

struct CodeOutputStream : public OutputStream
{
    std::ostream &out_;
    std::string name_;
    int last = 0;

    CodeOutputStream( std::ostream &out, const std::string &name ) : out_(out), name_(name) {}

    inline OutputStream &put( int c )
    {
        if (c == '\n')
        {
            if (last != c) out_ << "\\n\"\n\"";
        }
        else
        if (c == '\\')
        {
            out_ << "\\\\";
        }
        else
        if (c == '"')
            out_ << "\\042";
        else
            out_.put((char)c);
        last = c;
        return *this;
    }

    inline OutputStream &puts( const char *s )
    {
        auto size = strlen(s);
        for (size_t i = 0; i < size; ++i) put(s[i]);
        return *this;
    }

    inline OutputStream &puts( const std::string &s )
    {
        auto size = s.length();
        for (size_t i = 0; i < size; ++i) put(s[i]);
        return *this;
    }

    inline void start()
    {
        out_ << "const char *" << name_ << " = \n\"";
    }

    inline void finish()
    {
        out_ << "\";";
    }
};

inline bool isws( int c )
{
    return (c == ' ' || c == '\t');
}

inline bool isbreakable( int c )
{
    return strchr(" .{}?;", c) != nullptr;
}

static std::string get_string( const std::string &line )
{
    auto pos = line.find('"');
    if (pos != std::string::npos)
    {
        auto end = line.find('"', pos + 1);
        if (pos != std::string::npos)
            return line.substr(pos + 1, end - pos - 1);
    }
    return "";
}

static void process( std::istream &input, OutputStream &output, bool minify );

static void inject( const std::string &file, OutputStream &output, bool minify )
{
    auto path = std::filesystem::absolute(input_path.string() + "/" + file);
    std::ifstream input(path.c_str());
    if (!input.good())
    {
        output.puts("#error Fail to inject <");
        output.puts(path.c_str());
        output.puts(">\n");
        return;
    }
    std::cerr << "   Injecting '"<< path.c_str() << "'" << std::endl;
    process(input, output, minify);
}

static void process( std::istream &input, OutputStream &output, bool minify )
{
    std::string line;
    bool is_string = false;
    bool is_char = false;
    bool is_special = false;

    int last = 0;
    int cols = 0;
    while (true)
    {
        std::string line;
        std::getline(input, line);
        if (line.empty() && !input.good()) break;

        // check for includes
        if (!line.empty() && line.front() == '#' && line.find("// INJECT") != std::string::npos)
        {
            inject(get_string(line), output, minify);
            continue;
        }
        else
        {
            // discard line comments
            auto pos = line.find("//");
            if (pos != std::string::npos)
                line = line.substr(0, pos);
            line += '\n';
            // replace placeholder namespace
            pos = line.find("_X_X_X");
            if (pos != std::string::npos)
                line = line.replace(pos, 6, PROTON_VERSION_SUFFIX);
            if (!minify)
            {
                output.puts(line);
                continue;
            }
        }

        // minification algorithm
        for (int c : line)
        {
            if (is_special && c == '\n')
            {
                output.put(c);
                cols = 0;
                is_special = false;
                continue;
            }

            if (c == '\n' || c == '\r') continue;
            if (isbreakable(c) && !is_char && cols >= 80)
            {
                if (is_string)
                {
                    output.put('"');
                    output.put('\n');
                    output.put('"');
                    cols = 1;
                }
                else
                {
                    output.put('\n');
                    cols = 0;
                }
            }

            if (!is_string && c == '\'')
                is_char = !is_char;
            else
            if (!is_char && c == '"')
                is_string = !is_string;

            if (c == '#')
            {
                output.put('\n');
                output.put('#');
                is_special = true;
            }
            else
            if (isspace(c))
            {
                if (!is_string && !is_char && isspace(last)) continue;
                output.put(' ');
                ++cols;
            }
            else
            {
                output.put(c);
                ++cols;
            }

            last = c;
        } // for
    } // while
}


int main( int argc, char **argv )
{
    if (argc != 3) return 1;

    std::cerr << "Processing '"<< argv[1] << "' to generate '" << argv[2] << "'" << std::endl;

    input_path = std::filesystem::absolute(argv[1]).parent_path();

    std::ifstream input(argv[1]);
    if (!input.good()) return 1;

    std::ofstream output(argv[2]);
    if (!output.good()) return 1;

    std::string guard = "content_";
    guard += std::filesystem::absolute(argv[1]).filename().c_str();
    for (auto &c : guard) if (!isalpha(c)) c = '_';

    CodeOutputStream stream(output, guard);
    //PlainOutputStream stream(output);
    stream.start();
    process(input, stream, true);
    stream.finish();

    return 0;
}