#ifndef NEUTRON_API
#define NEUTRON_API

#include <cstddef>
#include <cstdint>
#include <string>
#include <list>
#include <cstring>
#include <stdexcept>

namespace proton {
namespace neutron {

class MemoryStream
{
    public:
        uint8_t *data_;
        size_t size_, off_;

        MemoryStream() : data_(nullptr), size_(0), off_(0)
        {
        }

        MemoryStream( size_t size ) : off_(0)
        {
            size_ = size & 0xFFFFFFFF;
            data_ = (uint8_t*) malloc(size_);
            if (data_ == nullptr) throw std::bad_alloc();
        }

        MemoryStream( const MemoryStream& ) = delete;

        MemoryStream( MemoryStream &&that ) : data_(that.data_), size_(that.size_), off_(that.off_)
        {
            that.data_ = nullptr;
            that.size_ = that.off_ = 0;
        }

        MemoryStream &operator=( const MemoryStream & ) = delete;

        inline void write( const uint8_t *buffer, size_t size )
        {
            if (off_ + size > UINT32_MAX) throw std::out_of_range("Cannot write more then UINT_MAX bytes");
            if (off_ + size > size_) grow(size);
            memcpy(data_ + off_, buffer, size);
            off_ += size;
        }

        inline void write( const char *buffer, size_t size )
        {
            write((const uint8_t*) buffer, size);
        }

        void grow( size_t size )
        {
            size = (size + 3) & (~3);
            size_ += size + (size & 0x3FF);
            data_ = (uint8_t*) realloc(data_, size_);
        }

        inline size_t size() const
        {
            return size_;
        }

        inline size_t offset() const
        {
            return off_;
        }

        inline void write_int8( int8_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_uint8( uint8_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_int16( int16_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_uint16( uint16_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_int32( int32_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_uint32( uint32_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_int64( int64_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_uint64( uint64_t value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_float( float value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void write_double( double value )
        {
            write((uint8_t*) &value, sizeof(value));
        }

        inline void padding( size_t count )
        {
            while (count--) write_uint8(0);
        }

        inline const uint8_t *data() const
        {
            return data_;
        }
};

class Serializer
{
    public:
        Serializer( MemoryStream &stream ) : out_(stream) {}

        // single = document field with a single value
        // array  = document field with array of values
        // value  = standalone value for arrays

        inline void begin_root_document( uint16_t id )
        {
            begin_document_value(id);
        }

        inline void end_root_document()
        {
            end_document_value();
        }

        inline void begin_document_value( uint16_t id )
        {
            dstack_.push_back({(uint32_t)out_.offset(),0});
            out_.write_uint32(0U); // size
            out_.write_uint16((uint16_t)0); // count16
            out_.write_uint16(id); // uid
        }

        inline void end_document_value()
        {
            auto item = dstack_.back();
            dstack_.pop_back();
            auto *psize = (uint32_t*)(out_.data_ + item.offset);
            auto *pcount = (uint16_t*)(out_.data_ + item.offset + sizeof(uint32_t));
            *psize = (uint32_t) (out_.offset() - item.offset - sizeof(uint32_t));
            *pcount = (uint16_t) item.counter;
        }

        inline void begin_document_single( uint16_t fid, uint16_t did )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x01'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(fid); // uid
            begin_document_value(did);
        }

        inline void end_document_single()
        {
            end_document_value();
        }

        inline void binary_single( uint16_t id, const uint8_t *value, size_t size )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x02'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            binary_value(value, size);
        }

        inline void datetime_single( uint16_t id, uint64_t value )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x03'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write((uint8_t*)&value, sizeof(value));
        }

        inline void float_single( uint16_t id, float value )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x04'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_float(value);
        }

        inline void double_single( uint16_t id, double value )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x05'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_double(value);
        }

        inline void string_single( uint16_t id, const std::string &value )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x06'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            string_value(value);
        }

        inline void bool_single( uint16_t id, bool value )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x07'); // type
            out_.write_uint8(value ? 1 : 0); // value
            out_.write_uint16(id); // uid
        }

        inline void int32_single( uint16_t id, int32_t value )
        {
            dstack_.back().counter++;
            if (value >= INT8_MIN && value <= INT8_MAX)
            {
                out_.write_uint8('\x48'); // type
                out_.write_uint8((int8_t) value); // value
                out_.write_uint16(id); // uid
            }
            else
            {
                out_.write_uint8('\x08'); // type
                out_.write_uint8(0); // pad
                out_.write_uint16(id); // uid
                out_.write((uint8_t*)&value, sizeof(value));
            }
        }

        inline void uint32_single( uint16_t id, uint32_t value )
        {
            dstack_.back().counter++;
            if (value <= UINT8_MAX)
            {
                out_.write_uint8('\x49'); // type
                out_.write_uint8((uint8_t) value); // value
                out_.write_uint16(id); // uid
            }
            else
            {
                out_.write_uint8('\x09'); // type
                out_.write_uint8(0); // pad
                out_.write_uint16(id); // uid
                out_.write((uint8_t*)&value, sizeof(value));
            }
        }

        inline void int64_single( uint16_t id, int64_t value )
        {
            dstack_.back().counter++;
            if (value >= INT8_MIN && value <= INT8_MAX)
            {
                out_.write_uint8('\x4A'); // type
                out_.write_uint8((int8_t) value); // value
                out_.write_uint16(id); // uid
            }
            else
            {
                out_.write_uint8('\x0A'); // type
                out_.write_uint8(0); // pad
                out_.write_uint16(id); // uid
                out_.write((uint8_t*)&value, sizeof(value));
            }
        }

        inline void uint64_single( uint16_t id, int64_t value )
        {
            dstack_.back().counter++;
            if (value <= UINT8_MAX)
            {
                out_.write_uint8('\x4B'); // type
                out_.write_uint8((uint8_t) value); // value
                out_.write_uint16(id); // uid
            }
            else
            {
                out_.write_uint8('\x0B'); // type
                out_.write_uint8(0); // pad
                out_.write_uint16(id); // uid
                out_.write((uint8_t*)&value, sizeof(value));
            }
        }

        inline void begin_document_array( uint16_t id )
        {
            begin_array('\x81', id);
        }

        inline void end_document_array()
        {
            end_array();
        }

        inline void begin_binary_array( uint16_t id )
        {
            begin_array('\x82', id);
        }

        inline void end_binary_array()
        {
            end_array();
        }

        inline void datetime_array( uint16_t id, const uint64_t *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x83'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(uint64_t))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)value, sizeof(uint64_t) * count);
        }

        inline void float_array( uint16_t id, const float *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x84'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(float))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)value, sizeof(float) * count);
        }

        inline void double_array( uint16_t id, const double *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x85'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(double))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)value, sizeof(double) * count);
        }

        inline void begin_string_array( uint16_t id )
        {
            begin_array('\x86', id);
        }

        inline void end_string_array()
        {
            end_array();
        }

        inline void bool_array( uint16_t id, const bool *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x87'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            auto size = (uint32_t)(count * sizeof(uint8_t) + 3) & (~3);
            out_.write_uint32(size); // size
            out_.write_uint32((uint32_t) count); // count
            if (sizeof(bool) == sizeof(uint8_t))
                out_.write((const uint8_t*)value, sizeof(uint8_t) * count);
            else
                for (size_t i = 0; i < count; ++i)
                    out_.write_uint8((uint8_t) value[i] ? 1 : 0);
            out_.padding(size - count);
        }

        inline void int32_array( uint16_t id, const int32_t *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x88'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(int32_t))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)&value, sizeof(int32_t));
        }

        inline void uint32_array( uint16_t id, const uint32_t *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x89'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(uint32_t))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)&value, sizeof(uint32_t));
        }

        inline void int64_array( uint16_t id, const int64_t *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x8A'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write((uint8_t*)&value, sizeof(value));
            out_.write_uint32((uint32_t) (count * sizeof(int64_t))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)&value, sizeof(int64_t));
        }

        inline void uint64_array( uint16_t id, const int64_t *value, size_t count )
        {
            dstack_.back().counter++;
            out_.write_uint8('\x8B'); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            out_.write_uint32((uint32_t) (count * sizeof(int64_t))); // size
            out_.write_uint32((uint32_t) count); // count
            out_.write((uint8_t*)&value, sizeof(int64_t));
        }

        inline void binary_value( const uint8_t *value, size_t size )
        {
            out_.write(value, size);
            out_.padding(((size + 3) & (~3)) - size);
        }

        inline void string_value( const std::string &value )
        {
            auto len = (uint32_t) value.length();
            if (len == UINT32_MAX) --len;
            auto size = (uint32_t) ((len + 1 + 3) & (~3));
            out_.write_int32(size);
            out_.write(value.c_str(), len);
            out_.padding(size - len);
        }

    protected:
        struct item_t
        {
            uint32_t offset = 0;
            uint32_t counter = 0;
        };
        MemoryStream &out_;
        std::list<item_t> dstack_;
        std::list<item_t> astack_;

        inline void begin_array( uint8_t type, uint16_t id )
        {
            dstack_.back().counter++;
            out_.write_uint8(type); // type
            out_.write_uint8(0); // pad
            out_.write_uint16(id); // uid
            astack_.push_back({(uint32_t) out_.offset(), 0});
            out_.write_uint32(0); // size
            out_.write_uint32(0); // count
        }

        inline void end_array()
        {
            auto item = astack_.back();
            astack_.pop_back();
            auto *ptr = (uint32_t*)(out_.data_ + item.offset);
            *ptr = (uint32_t) (out_.offset() - item.offset - sizeof(uint32_t));
            *(ptr + sizeof(uint32_t)) = item.counter;
        }
};


} // neutron
} // proton

#endif // NEUTRON_API