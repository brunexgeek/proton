#include "neutron.hh"

using namespace proton::neutron;

static void hexdump( const MemoryStream &value )
{
    const uint8_t *p = value.data();
    size_t i = 0, t = value.offset();
    while (i < t)
    {
        printf("%02X ", (int) p[i]);
        if (i > 0 && (i % 16) == 0) putchar('\n');
        ++i;
    }
}

int main( int argc, char ** argv )
{
    MemoryStream ms(256);
    Serializer serial(ms);

    serial.begin_document(1);
    serial.field_bool(1, true);
    serial.field_float(2, 5.5);
    serial.field_uint64(3, 1251485695U);
    serial.end_document();

    hexdump(ms);
}