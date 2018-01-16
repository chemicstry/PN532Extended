#ifndef __BYTEBUFFER_H__
#define __BYTEBUFFER_H__

#include <stdlib.h>
#include <vector>
#include <cstdint>

typedef std::vector<uint8_t> BinaryData;

class ByteBuffer
{
public:
    ByteBuffer(): pointer(0) {}
    
    template<typename T>
    ByteBuffer(T data): pointer(0)
    {
        Append<T>(data);
    }
    
    ByteBuffer(BinaryData data): pointer(0), vec(data) {}
    
    ByteBuffer& operator<<(const ByteBuffer& b)
    {
        Append(b.vec);
        return *this;
    }
    ByteBuffer& operator<<(const BinaryData& b)
    {
        Append(b);
        return *this;
    }
    
    template<typename T>
    ByteBuffer& operator<<(const T& b)
    {
        Append<T>(b);
        return *this;
    }
    
    template<typename T>
    ByteBuffer& operator>>(T& b)
    {
        b = Read<T>();
        return *this;
    }
    
    template<typename T>
    void Append(const T data)
    {
        for (uint8_t i = 0; i < sizeof(T); ++i)
            vec.push_back(data >> (i * 8));
    }
    
    void Append(const BinaryData data)
    {
        vec.insert(vec.end(), data.begin(), data.end());
    }

    void Append(const uint8_t* data, size_t size)
    {
        vec.insert(vec.end(), data, data+size);
    }
    
    BinaryData ReadBinary(size_t size)
    {
        if (vec.size() < pointer+size)
            return BinaryData();
        
        pointer += size;
        return BinaryData(vec.begin()+pointer-size, vec.begin()+pointer);
    }

    BinaryData ReadBinary()
    {
        return ReadBinary(vec.size() - pointer); // Read till end
    }
    
    template<typename T>
    T Read()
    {
        size_t size = sizeof(T);
        
        if (vec.size() < pointer+size)
            return 0;
        
        T data = 0;
        for (uint64_t i = 0; i < size; ++i)
            data += (T)vec[pointer+i]<<(i*8);
        
        pointer += size;
        
        return data;
    }
    
    BinaryData& Data()
    {
        return vec;
    }
    
    size_t Size()
    {
        return vec.size();
    }

    size_t RemainingSize()
    {
        return vec.size()-pointer;
    }

    void Clear()
    {
        vec.clear();
    }
    
    size_t pointer;
    BinaryData vec;
};

#endif
