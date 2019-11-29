#include <string>
#include "tabtreeparser.hpp"
using namespace std;

tabtreeparser::tabtreeparser()
    : indent(0) { }

tabtreeparser::tabtreeparser(size_t indent)
    : indent(indent) { }

#define MORECHARS (this->index < this->end)
#define FETCHCHAR (data[this->index++])

tabtree tabtreeparser::parse(const string& data)
{
    return parse(data, "", "", false, 0, nullptr);
}

tabtree tabtreeparser::parse(const string& data, string key, string val, bool has_val, size_t off, size_t* setindex)
{
    this->index = off; this->end = data.length();
    tabtree node(key, val, has_val);
    // Read lines as long as they have the right indentation and construct children
    while (MORECHARS) {
        // Check indentation
        if (! skip_tabs(data, this->indent))
            break;
        // Read title: key
        size_t start = this->index;
        size_t omit;
        if (read_key(data, omit)) {
            // Child has val
            string child_key(data, start, this->index - start);
            this->index += omit;
            start = this->index;
            read_line(data, omit);
            string child_val(data, start, this->index - start);
            // Skip newline and recurse
            this->index += omit;
            tabtreeparser p(indent + 1);
            node.children.push_back(p.parse(data, child_key, child_val, true, this->index, &this->index));
        } else {
            // Child has no val
            string child_key(data, start, this->index - start);
            string child_val;
            // Skip newline and recurse
            this->index += omit;
            tabtreeparser p(indent + 1);
            node.children.push_back(p.parse(data, child_key, child_val, false, this->index, &this->index));
        }
    }
    if (setindex != nullptr)
        *setindex = this->index;
    return node;
}

bool tabtreeparser::skip_tabs(const string& data, size_t count)
{
    size_t start = index;
    while (count > 0 && MORECHARS && FETCHCHAR == '\t')
        --count;
    if (count > 0) {
        index = start;
        return false;
    }
    return true;
}

bool tabtreeparser::read_key(const string& data, size_t& omit)
{
    omit = 0;
    int lastc = 256;
    while (MORECHARS) {
        char c = FETCHCHAR;
        if (lastc == ':' && c == ' ') {
            index -= 2;
            omit = 2;
            return true;
        }
        if (c == '\n') {
            --index;
            ++omit;
            if (lastc == '\r') {
                --index;
                ++omit;
            }
            return false;
        }
        lastc = c;
    }
    return false;
}

void tabtreeparser::read_line(const string& data, size_t& omit)
{
    omit = 0;
    int lastc = 256;
    while (MORECHARS) {
        char c = FETCHCHAR;
        if (c == '\n') {
            --index;
            ++omit;
            if (lastc == '\r') {
                --index;
                ++omit;
            }
            return;
        }
        lastc = c;
    }
}

