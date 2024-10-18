/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/utils-qt
 * Contact:  ihor-drachuk-libs@pm.me  */

#include <utils-cpp/data_to_string.h>

#include <vector>
#include <cctype>
#include <utility>
#include <string>
#include <cstring>
#include <cassert>

namespace {

inline size_t hex_str_size(size_t in_len)
{
    return in_len * 3 - 1;
}

inline size_t hex_str_size_wrapped(size_t in_len)
{
    return hex_str_size(in_len) + 2;
}

size_t hex_str(const void* data, size_t len, char* out, size_t outSz)
{
    size_t need_sz = hex_str_size(len);
    //++len;

    if (!outSz)
        return need_sz;

    assert(data);
    assert(out);
    assert(outSz >= need_sz);

    if (!len) {
        //*out = 0;
        return 0;
    }

    static const char characters[] = "0123456789ABCDEF";

    const char* it = static_cast<const char*>(data);
    const char* it_end = it + len;
    char* outIt = out;

    assert(it != it_end);
    *outIt   = characters[(*it >> 4) & 0x0F];
    *++outIt = characters[*it & 0x0F];

    while (++it != it_end) {
        *++outIt = ' ';
        *++outIt = characters[(*it >> 4) & 0x0F];
        *++outIt = characters[*it & 0x0F];
    }

    //*--outIt = 0;

    return need_sz;
}

void add_non_print_data(std::string& result, const void* data, size_t start, size_t end)
{
    // There are '+2' for size and '+1' for out_offset for '<' and '>' symbols
    const size_t need_sz = hex_str(static_cast<const char*>(data) + start, (end - start), nullptr, 0) + 2;
    const size_t out_offset = result.size();
    result.resize(result.size() + need_sz);
    hex_str(static_cast<const char*>(data) + start, (end - start), result.data() + out_offset + 1, need_sz);
    result[out_offset] = '<';
    result[result.size()-1] = '>';
}

inline bool is_printable(char c)
{
    const auto uc = static_cast<unsigned char>(c);
    return (std::isprint(uc) || std::isspace(uc)) && c != '\xFF';
}

} // namespace

namespace utils_cpp {

std::string data_to_string(const void* data, size_t sz)
{
    if (!sz)
        return {};

    const auto begin_ptr = static_cast<const char*>(data);
    std::vector<std::pair<size_t, size_t>> ranges;
    bool odd_is_printable;

    // First pass: determine ranges & size
    size_t start = 0;
    size_t output_size = 0;
    bool current_is_printable = is_printable(begin_ptr[0]);
    odd_is_printable = current_is_printable;

    for (size_t i = 1; i <= sz; ++i) {
        if (i == sz || is_printable(begin_ptr[i]) != current_is_printable) {
            ranges.emplace_back(start, i);
            output_size += current_is_printable ? (i - start) : hex_str_size_wrapped(i - start);
            if (i < sz) {
                start = i;
                current_is_printable = !current_is_printable;
            }
        }
    }

    // Second pass: build the result string
    std::string result;
    result.reserve(output_size);

    for (size_t i = 0; i < ranges.size(); ++i) {
        const auto& range = ranges[i];
        if ((i % 2 == 0) == odd_is_printable) {
            // Printable range: copy directly
            result.append(begin_ptr + range.first, range.second - range.first);
        } else {
            // Non-printable range: convert to hex
            add_non_print_data(result, data, range.first, range.second);
        }
    }

    assert(output_size == result.size() && "std::string reserve is not effective!");
    return result;
}

} // namespace utils_cpp
