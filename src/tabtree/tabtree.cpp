#include <string>
#include <vector>
#include <iostream>
#include "tabtree.hpp"
#include "tabtreeparser.hpp"
using namespace std;

tabtree::tabtree() : key(""), val(""), has_val(false) { }

tabtree::tabtree(string key, string val, bool has_val) : key(key), val(val), has_val(has_val) { }

void tabtree::print(size_t ind) const
{
    string indstr(ind, '\t');
    cout << indstr << "'" << key << "' '" << val << "'" << " (" << children.size() << " children)" << endl;
    if (children.size() > 0) {
        cout << indstr << "{" << endl;
        for (tabtree child : children) {
            child.print(ind + 1);
        }
        cout << indstr << "}" << endl;
    }
}

string tabtree::get_content(const char* linesep, size_t ind) const
{
    string data;
    string tabs(ind, '\t');
    for (const tabtree& c : children) {
        data += tabs;
        data += c.key;
        if (c.has_val) {
            data += ": ";
            data += c.val;
        }
        data += linesep;
        data += c.get_content(linesep, ind + 1);
    }
    return data;
}

static const string common_prefix(string a, string b)
{
    if (a.empty()) return b;
    if (b.empty()) return a;
    size_t i, len = a.length() < b.length() ? a.length() : b.length();
    for (i = 0; i < len; ++i)
        if (((a[i] & ~0x20) != (b[i] & ~0x20))) // ASCII case insensitive
            break;
    return string(a, 0, i);
}

string tabtree::search_suggest(const string& filter) const
{
    if (filter.empty())
        return string();
    size_t fl = filter.length();
    // Check if the filter matches a child node
    string best;
    for (const tabtree& c : children) {
        // Check if the filter (partially) matches the child title
        bool samestart = true;
        size_t i, len = (fl < c.key.length()) ? fl : c.key.length();
        for (i = 0; i < len; ++i) {
            if (! ((c.key[i] & ~0x20) == (filter[i] & ~0x20))) { // ASCII case insensitive
                samestart = false;
                break;
            }
        }
        if (! samestart)
            continue;
        // Exact match
        if (i == fl && i == c.key.length()) {
            return c.key;
        }
        // Filter longer than child title
        if (i < fl) {
            if (filter[i] != ' ')
                continue;
            // Child title is fully in the title; get further suggestions from child
            string sub = c.search_suggest(string(filter, i + 1));
            if (! sub.empty()) {
                best = common_prefix(best, c.key + ' ' + sub);
            }
            continue;
        }
        // Child title matches filter but is longer
        if (best.empty()) {
            best = c.children.empty() ? c.key : c.key + ' ';
        } else {
            // Another child was already found that also matches the filter and is longer
            best = common_prefix(best, c.key);
        }
    }
    return best;
}

tabtree* tabtree::search(const string& filter)
{
    if (filter.empty())
        return this;
    size_t fl = filter.length();
    // Check if the filter matches a child node
    for (tabtree& c : children) {
        // Check if the filter (partially) matches the child title
        bool samestart = true;
        size_t i, len = (fl < c.key.length()) ? fl : c.key.length();
        for (i = 0; i < len; ++i) {
            if (! ((c.key[i] & ~0x20) == (filter[i] & ~0x20))) { // ASCII case insensitive
                samestart = false;
                break;
            }
        }
        if (! samestart)
            continue;
        // Check if filter is longer than child title
        if (i < fl) {
            if (filter[i] != ' ')
                continue;
            // Child title is fully in the title; search child subtree
            tabtree* node = c.search(string(filter, i + 1));
            if (node != nullptr) {
                return node;
            }
        } else if (i == fl && fl == c.key.length()) {
            return &c;
        }
    }
    return nullptr;
}

void tabtree::search_prefixes(const string& filter, vector<tabtree*>& list)
{
    if (filter.empty()) {
        list.push_back(this);
        return;
    }
    size_t fl = filter.length();
    bool contains_this = false;
    // Check if the filter matches a child node
    for (tabtree& c : children) {
        // Check if the filter (partially) matches the child title
        bool samestart = true;
        size_t i, len = (fl < c.key.length()) ? fl : c.key.length();
        for (i = 0; i < len; ++i) {
            if (! ((c.key[i] & ~0x20) == (filter[i] & ~0x20))) { // ASCII case insensitive
                samestart = false;
                break;
            }
        }
        if (! samestart)
            continue;
        if (i < fl) {
            // Filter is longer than child title
            if (filter[i] == ' ') {
                // Continue in child subtree
                c.search_prefixes(string(filter, i + 1), list);
            }
        } else {
            list.push_back(&c);
        }
    }
}

void tabtree::set_key(string s)
{
    key = s;
}

void tabtree::set_val(string s)
{
    val = s;
    has_val = (! s.empty());
}

void tabtree::set_content(string s)
{
    tabtreeparser parser;
    tabtree tree = parser.parse(s);
    this->children = tree.children;
}

