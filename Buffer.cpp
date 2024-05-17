#include "Buffer.h"

Buffer::Buffer(uint16_t sep) : sep_(sep)
{
}

Buffer::~Buffer()
{
}

void Buffer::append(const char *data, size_t size)
{
    buf_.append(data, size);
}// 把数据追加到buf_中

void Buffer::appendwithsep(const char *data, size_t size) // 把数据追加到buf_中, 加上报文头部。
{
    if (sep_ == 0)
    {
        buf_.append(data, size);
    }
    else if (sep_ == 1)
    {
        buf_.append((char *)&size, 4);
        buf_.append(data, size);
    }
    else if (sep_ == 2)
    {
        //待完善
    }
    
}


size_t Buffer::size()
{
    return buf_.size();
}                             // 返回buf_的大小
const char *Buffer::data()
{
    return buf_.data();
}                         // 返回buf_的首地址
void Buffer::clear()
{
    buf_.clear();
}

void Buffer::erase(size_t pos, size_t nn)
{
    buf_.erase(pos, nn);
}

bool Buffer::pickmessage(std::string &ss)
{
    if (buf_.size() == 0)
        return false;

    if (sep_ == 0) // 没有分隔符。
    {
        ss = buf_;
        buf_.clear();
    }
    else if (sep_ == 1) // 四字节的报头。
    {
        int len;
        memcpy(&len, buf_.data(), 4); // 从buf_中获取报文头部。

        if (buf_.size() < len + 4)
            return false; // 如果buf_中的数据量小于报文头部，说明buf_中的报文内容不完整。

        ss = buf_.substr(4, len); // 从buf_中获取一个报文。
        buf_.erase(0, len + 4);   // 从buf_中删除刚才已获取的报文。
    }

    return true;
}