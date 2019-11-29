// Tabtree: A tree that contains text, structured by tabs like the example below.
// A
//    AA
//    AB
//       ABA
// B
// It is used to quickly access hierarchical text data.
#include "tabtree.hpp"
#include "tabtreeparser.hpp"
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <streambuf>
using namespace std;

int main()
{
    string s(
        "\r\n"
        "\t\r\n"
        "Server: 123\r\n"
        "\tPasswort: askfaldfjk\r\n"
        "\troot: sldkfj45kj235klj\r\n"
        "\t\r\n"
        "\r\n"
        "HFU\r\n"
        "\tEmail\r\n"
        "\t\tAdresse: stefan.lederer@hs-furtwangen.de\r\n"
        "\t\t\r\n"
        "\t\tPasswort: lkj345klj345klj\r\n"
        "\tBenutzer: lederers\r\n"
        "\tSemester: 7\r\n"
        "Yo: Go home\r\n"
        "H\r\n"
        "O\r\n"
        "A dolf\r\n"
        "A dorno\r\n"
        "\r\n"
        "Bear\r\n"
        "Beatles\r\n"
        "\r\n"
        "A B\r\n"
        "A B C\r\n"
        "A\r\n"
        "\r\n"
    );
    
    tabtreeparser parser;
    tabtree tree = parser.parse(s);
    
    cout << "Constructed tabtree of size " << (sizeof tree) << " with " << tree.children.size() << " children." << endl;
    
    // tree.print(0);
    // cout << "'" << tree.get_content() << "'" << endl;
    
    for (;;) {
        string filter;
        cout << "Filter: ";
        getline(cin, filter);
        if (cin.eof()) {
            cout << endl;
            break;
        }
        vector<tabtree*> list;
        tree.search_prefixes(filter, list);
        cout << "Found: " << list.size() << endl;
        cout << "----------------" << endl;
        for (const tabtree* node : list) {
            cout << "Title key: '" << node->key << "'" << endl;
            cout << "Title val: '" << node->val << "'" << endl;
            cout << "Content:   '" << node->get_content() << "'" << endl;
            cout << "----------------" << endl;
        }
        
        /*
        string sugg = tree.search_suggest(filter);
        tabtree* res = tree.search(filter);
        cout << "Suggestion: '" << sugg << "'" << endl;
        if (res == nullptr) {
            cout << "No results." << endl;
        } else {
            cout << "Title key: '" << res->key << "'" << endl;
            cout << "Title val: '" << res->val << "'" << endl;
            cout << "Content:   '" << res->get_content() << "'" << endl;
            
            
            cout << "Do you want to edit the content, the key or the val (1/2/3)? ";
            string choice;
            getline(cin, choice);
            if (cin.eof()) {
                cout << endl;
                break;
            }
            if (choice == "1") {
                // Update the content (newlines possible, terminate at empty line)
                string content;
                string line;
                do {
                    getline(cin, line);
                    content += line;
                    content += "\r\n";
                } while (! line.empty());
                res->set_content(content);
            } else if (choice == "2") {
                string line;
                getline(cin, line);
                if (cin.eof()) {
                    cout << endl;
                    break;
                }
                res->set_key(line);
            } else if (choice == "3") {
                string line;
                getline(cin, line);
                if (cin.eof()) {
                    cout << endl;
                    break;
                }
                res->set_val(line);
            }
        }
        */
    }
    return 0;
}

