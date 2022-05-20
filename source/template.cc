#include <fstream>
#include <iostream>
#include <string>

enum State
{
    PROCESS,
    COPY,
    IGNORE
};

inline bool isws( int c )
{
    return (c == ' ' || c == '\t');
}

inline bool isbreakable( int c )
{
    return (c == ' ' || c == '.' || c == '{' || c == '}' || c == ';' || c == '(' || c == ')');
}

static int ignore( std::istream &input )
{
    while (true)
    {
        auto c = input.get();
        if (input.eof()) return 0;
        if (c == '\n') break;
    }
    return '\n';
}

static int copy( std::istream &input, std::ostream &output )
{
    while (true)
    {
        auto c = input.get();
        if (input.eof()) return 0;
        if (c == '\n') break;
        output.put(c);
    }
    output.put('\n');
    return '\n';
}

void process( std::istream &input, std::ostream &output, const std::string &guard )
{
    std::string line;
    bool is_string = false;
    bool is_char = false;
    bool is_special = false;

    int last = 0;
    int cols = 0;
    while (true)
    {
        int c = input.get();
        if (input.eof()) break;

        if (is_special && c == '\n')
        {
            output.put(c);
            cols = 0;
            is_special = false;
            continue;
        }

        if (c == '\n' || c == '\r') c = ' ';
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

        /*if (c == '\n')
        {
            if (cols == 0) c = ' ';
            take_break = false;
        }*/

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
        if (c == '/')
        {
            if (input.get() == '/')
                c = ignore(input);
            else
            {
                input.unget();
                output.put(c);
                ++cols;
            }
        }
        else
        if (isspace(c))
        {
            if (!is_string && !is_char && isspace(last))
                continue;
            //printf("Last is %d\n", last);
            output.put(' ');
            ++cols;
        }
        else
        {
            output.put(c);
            ++cols;
        }

        last = c;
    }
}


int main( int argc, char **argv )
{
    if (argc != 3) return 1;

    std::cout << "Processing '"<< argv[1] << "' to generate '" << argv[2] << "'" << std::endl;

    std::ifstream input(argv[1]);
    if (!input.good()) return 1;

    std::ofstream output(argv[2]);
    if (!output.good()) return 1;

    std::string guard = argv[1];
    #ifdef _WIN32
    const char DIR_SEPARATOR = '\\';
    #else
    const char DIR_SEPARATOR = '/';
    #endif
    auto pos = guard.rfind(DIR_SEPARATOR);
    if (pos != std::string::npos) guard = guard.substr(pos);
    guard = "GENERATED_" + guard;
    for (auto it = guard.begin(); it != guard.end(); ++it)
        if (!isalpha(*it)) *it = '_';

    process(input, output, guard);

    return 0;
}