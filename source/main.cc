#include "neutron.hh"
#include <algorithm>

using namespace neutron_X_X_X;

static void hexdump( const MemoryStream &value )
{
    static const int LINE = 16;
    static const int CHARS = 16 * 3 + 4;
    const uint8_t *p = value.data();
    size_t i = 0, t = value.offset();
    for (i = 0; i < t; i += LINE)
    {
        int w = 0;
        auto c = std::min((size_t)LINE, t - i);
        for (size_t j = 0; j < c; ++j)
        {
            if (j > 0 && (j % 4) == 0)
            {
                putchar(' ');
                ++w;
            }
            printf("%02X ", (int) p[i + j]);
            w += 3;
        }
        while (w++ < CHARS) putchar(' ');

        for (size_t j = 0; j < c; ++j)
        {
            auto d = (int) p[i + j];
            if (d > 32 && d < 127)
                putchar(p[i + j]);
            else
                putchar('.');
        }
        putchar('\n');
    }
    putchar('\n');
}

int main( int argc, char ** argv )
{
    (void) argc;
    (void) argv;
    {
        MemoryStream ms(256);
        Serializer serial(ms);
        serial.begin_root_document();
        serial.bool_single(1, true);
        serial.float_single(2, 5.5);
        serial.uint64_single(3, 1251485695U);
        serial.end_root_document();
        hexdump(ms);
    }
    putchar('\n');
    {
        MemoryStream ms(256);
        Serializer serial(ms);
        serial.begin_root_document();
        serial.string_single(1, "Example of string");
        serial.begin_document_single(2);
        serial.uint32_single(3, 100);
        serial.double_single(4, 100.01);
        serial.end_document_single();
        serial.end_root_document();
        hexdump(ms);
    }
    putchar('\n');
    {
        MemoryStream ms(256);
        Serializer serial(ms);
        serial.begin_root_document();
        const bool values[] = {true, false, true};
        serial.bool_array(1, values, 3);
        serial.begin_document_array(2);
        serial.begin_document_value();
        serial.begin_string_array(1);
        serial.string_value("Test value");
        serial.string_value("Another one");
        serial.end_string_array();
        serial.end_document_value();
        serial.begin_document_value();
        serial.uint32_single(3, 100);
        serial.end_document_value();
        serial.end_document_array();
        serial.end_root_document();
        hexdump(ms);
    }
}