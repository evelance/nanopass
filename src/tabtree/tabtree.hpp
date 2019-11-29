#ifndef __TABTREE_HPP
#define __TABTREE_HPP
#include <string>
#include <vector>

class tabtree
{
    public:
    std::string key;
    std::string val;
    bool has_val;
    std::vector<tabtree> children;
    
    // Empty node
    tabtree();
    
    // Construct tabtree node
    tabtree(std::string key, std::string val, bool has_val);
    
    // Print tabtree structure
    void print(size_t ind) const;
    
    // Get indented content of this node and its children
    std::string get_content(const char* linesep = "\r\n", size_t ind = 0) const;
    
    // Complete a search string to exactly match a node
    std::string search_suggest(const std::string& filter) const;
    
    // Search for a node by it's key. To find subnodes separate keys with spaces
    tabtree* search(const std::string& filter);
    
    // Like search, but populates the given vector with all matching nodes
    void search_prefixes(const std::string& filter, std::vector<tabtree*>& list);
    
    void set_key(std::string s);
    
    void set_val(std::string s);
    
    void set_content(std::string s);
};

#endif