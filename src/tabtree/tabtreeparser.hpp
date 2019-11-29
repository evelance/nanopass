#ifndef __TABTREEPARSER_HPP
#define __TABTREEPARSER_HPP
#include <string>
#include "tabtree.hpp"

class tabtreeparser
{
    size_t indent;
    size_t index;
    size_t end;
    
    public:
    
    // Initialize parser
    tabtreeparser();
    
    tabtree parse(const std::string& data);
    
    private:
    
    // Initialize parser for subtree
    tabtreeparser(size_t indent);
    
    // Called to construct subtree
    tabtree parse(const std::string& data, std::string key, std::string val, bool has_val, size_t off, size_t* setindex);
    
    // Tries to skip given number of tabs. Advances index and returns true if there are enough tabs
    bool skip_tabs(const std::string& data, size_t count);
    
    // Reads the title key: Read until ': ' or '[\r]\n' is encountered. Sets
    // index until before those characters. Returns true if ': ' is at the end.
    // Sets omit as the number of characters of the following delimiter
    bool read_key(const std::string& data, size_t& omit);
    
    // Reads until '[\r]\n'. Sets index until before those characters.
    // Sets omit as the number of characters of the newline
    void read_line(const std::string& data, size_t& omit);
};

#endif