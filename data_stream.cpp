#include <sstream>
#include "data_stream.h"

data_stream::data_stream(const char* data) : _data_ptr(data), _line(1), _pos(1)
{

}

char data_stream::operator++()
{
    if ((_data_ptr != nullptr) && *_data_ptr != '\0')
    {
        ++_pos;
        if (*_data_ptr == '\n')
        {
            _pos = 1;
            ++_line;
        }
        return *_data_ptr++;
    }
    return '\0';
}

char data_stream::operator*() const
{
    if (_data_ptr) return *_data_ptr;
    return '\0';
}

bool data_stream::operator>(const data_stream& o) const
{
    return _data_ptr > o._data_ptr;
}

data_stream::operator bool() const
{
    return _data_ptr != nullptr;
}

data_stream::data_stream(const data_stream& other)
{
    _data_ptr = other._data_ptr;
    _line = other._line;
    _pos = other._pos;
}

data_stream data_stream::combine(const data_stream& data1, const data_stream& data2, int priority)
{
    if (!data1) return data2;
    if (!data2) return data1;
    switch (priority)
    {
        case 1:
            return data1;
        case 2:
            return data2;
        case 0:
            return (data1 > data2) ? data1 : data2;
        default:
            return (data1 > data2) ? data2 : data1;
    }
}

std::ostream& operator<<(std::ostream& o, const data_stream& data)
{
    if (data._data_ptr)
    {
        if (*data._data_ptr) o << "(" << data._line << ":" << data._pos << ") : " << *data._data_ptr;
        else o << "(" << data._line << ":" << data._pos << ") : " << "EOF";
    }
    else o << "stream invalid";
    return o;
}

int data_stream::validest(const data_stream& data1, const data_stream& data2)
{
    if (!data2) return data1 ? 1 : 0;
    if (!data1) return 2;
    return (data1 > data2) ? 2 : 1;
}

parsing_state::parsing_state(const data_stream& ds, const parsing_fault_data &pfd) : _ds(ds), _pfd(pfd) { }

bool parsing_state::is_valid() const
{
    return _pfd._valid;
}

parsing_state parsing_state::join(const parsing_state& initial, const parsing_state& result)
{
    if (result.is_valid()) return {data_stream::combine(initial._ds, result._ds, 0), parsing_fault_data(true)};
    if (initial.is_valid()) return {data_stream::combine(initial._ds, result._ds, -1), result._pfd};
    int validest = data_stream::validest(initial._ds, result._ds);
    switch (validest)
    {
        case 1:
            return initial;
        case 2:
            return result;
        default:
            return {data_stream(), parsing_fault_data()};
    }
}

parsing_state parsing_state::both(const parsing_state& ps1, const parsing_state& ps2)
{
    if (ps1.is_valid() || ps2.is_valid())
    {
        parsing_fault_data pfd(true);
        int priority = 0;
        if (!ps1.is_valid())
        {
            pfd = ps1._pfd;
            priority = -1;
        }
        if (!ps2.is_valid())
        {
            pfd = ps2._pfd;
            priority = -1;
        }
        return {data_stream::combine(ps1._ds, ps2._ds, priority), pfd};
    }
    int validest = data_stream::validest(ps1._ds, ps2._ds);
    switch (validest)
    {
        case 1:
            return ps1;
        case 2:
            return ps2;
        default:
            return {data_stream(), parsing_fault_data()};
    }
}

std::ostream& operator<<(std::ostream& o, const parsing_state& data)
{
//    o << data._ds << (data._valid ? ", valid " : ", invalid, ") << "msg = " << data._what << std::endl;
    o << data._pfd;
    return o;
}

void parsing_state::fault(const std::string& why, bool add_ds)
{
    std::stringstream ss;
    if (add_ds) ss << _ds << ": ";
    ss << why;
    _pfd = {ss.str(), _ds};
}

bool parsing_state::is_EOF() const
{
    return *_ds == '\0';
}

std::ostream& operator<<(std::ostream& o, const parsing_fault_data& data)
{
    o /*<< data._fault_point << ": " */<< data._what;
    return o;
}
