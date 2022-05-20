#include <fstream>
#include "../../source/neutron.hh"

using proton::neutron::MemoryStream;

void explain_document( MemoryStream &input )
{

}

int main( int argc, char **argv )
{
    std::ifstream input(argv[1]);
    if (!input.good()) return 1;

    input.seekg(0,std::ios_base::end);
    auto size = input.tellg();
    input.seekg(0,std::ios_base::beg);
    if (size > 0xFFFF) return 1;
    MemoryStream ms(size);
    input.read((char*)ms.data(), size);
    explain_document(ms);
    return 0;
}