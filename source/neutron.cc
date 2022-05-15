#include "neutron.hh"

#define IS_BIG_ENDIAN                  ((0xBBAA & 0xFF) == 0xBB)
#define IS_LITTLE_ENDIAN               (!IS_BIG_ENDIAN)

namespace proton {
namespace neutron {

void Stream::write( const char *buffer, size_t size )
{
    write((const uint8_t*) buffer, size);
}

MemoryStream::MemoryStream( size_t size ) : off_(0)
{
    size_ = size & 0xFFFFFFFF;
    data_ = (uint8_t*) malloc(size_); // TODO: error handling
}

void MemoryStream::write( const uint8_t *buffer, size_t size )
{
    if (off_ + size > size_) grow(size);
    memcpy(data_ + off_, buffer, size); // TODO: error handling
    off_ += size;
}

void MemoryStream::grow( size_t size )
{
    size = (size + 3) & (~3);
    size_ += size + (size & 0x3FF);
    data_ = (uint8_t*) realloc(data_, size_);
}

} // neutron
} // proton
