#ifndef NEUTRON_API
#define NEUTRON_API

#include <cstddef>
#include <cstdint>
#include <string>
#include <list>
#include <cstring>
#include <type_traits>

namespace proton {
namespace neutron {

struct Stream
{
    virtual void write( const uint8_t *buffer, size_t size ) = 0;
    virtual void write( const char *buffer, size_t size ) = 0;
    virtual void write_int8( int8_t value ) = 0;
    virtual void write_uint8( uint8_t value ) = 0;
    virtual void write_int16( int16_t value ) = 0;
    virtual void write_uint16( uint16_t value ) = 0;
    virtual void write_int32( int32_t value ) = 0;
    virtual void write_uint32( uint32_t value ) = 0;
    virtual void write_int64( int64_t value ) = 0;
    virtual void write_uint64( uint64_t value ) = 0;
    virtual void write_float( float value ) = 0;
    virtual void write_double( double value ) = 0;
    virtual void grow( size_t size ) = 0;
    virtual size_t size() const = 0;
    virtual size_t offset() const = 0;
    virtual const uint8_t *data() const = 0;
};

class MemoryStream : public Stream
{
    public:
        MemoryStream( size_t size );
        inline void write( const uint8_t *buffer, size_t size ) override;
        inline void write( const char *buffer, size_t size ) override
        {
            write((const uint8_t*) buffer, size);
        }
        void grow( size_t size ) override;
        inline size_t size() const override
        {
            return size_;
        }
        inline size_t offset() const override
        {
            return off_;
        }
        inline void write_int8( int8_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_uint8( uint8_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_int16( int16_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_uint16( uint16_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_int32( int32_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_uint32( uint32_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_int64( int64_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_uint64( uint64_t value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_float( float value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline void write_double( double value ) override
        {
            write((uint8_t*) &value, sizeof(value));
        }
        inline const uint8_t *data() const override
        {
            return data_;
        }

    protected:
        uint8_t *data_;
        size_t size_, off_;
};

class Serializer
{
    public:
        Serializer( MemoryStream &stream ) : out_(stream) {}
        inline void begin_document( uint16_t id )
        {
            offs_.push_back(out_.offset());
            out_.write_uint32(0U);
            out_.write_uint16((uint16_t)0);
            out_.write_uint16(id);
        }

        inline void end_document()
        {
            out_.write_uint8((uint8_t)'\x00');
            auto off = offs_.back();
            offs_.pop_back();
            // TODO: update 'size' and 'count16'
        }

        inline void field_float( uint16_t id, float value )
        {
            out_.write_uint8('\x05');
            out_.write_uint16(id);
            out_.write_float(value);
        }

        inline void field_double( uint16_t id, double value )
        {
            out_.write_uint8('\x05');
            out_.write_uint16(id);
            out_.write_double(value);
        }

        inline void field_string( uint16_t id, const std::string &value )
        {
            out_.write_uint8('\x09');
            out_.write_uint16(id);
            out_.write_int32((uint32_t) value.size() + 1); // TODO: check size
            out_.write(value.c_str(), value.length());
            out_.write_uint8('\x00');
        }

        inline void field_bool( uint16_t id, bool value )
        {
            out_.write_uint8('\x08');
            out_.write_uint16(id);
            out_.write_uint8(value ? 1 : 0);
        }

        inline void field_int16( uint16_t id, int16_t value )
        {
            out_.write_uint8('\x09');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_uint16( uint16_t id, uint16_t value )
        {
            out_.write_uint8('\x0A');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_int32( uint16_t id, int32_t value )
        {
            out_.write_uint8('\x0B');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_uint32( uint16_t id, uint32_t value )
        {
            out_.write_uint8('\x0C');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_int64( uint16_t id, int64_t value )
        {
            out_.write_uint8('\x0D');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_uint64( uint16_t id, int64_t value )
        {
            out_.write_uint8('\x0E');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_begin_document( uint16_t id, int64_t value )
        {
            out_.write_uint8('\x01');
            out_.write_uint16(id);
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void field_end_document( uint16_t id, int64_t value )
        {
            out_.write_uint8('\x0E');
            out_.write((uint8_t*)&value, sizeof(value));
        }


    protected:
        MemoryStream &out_;
        std::list<uint32_t> offs_;
};


} // neutron
} // proton

#endif // NEUTRON_API