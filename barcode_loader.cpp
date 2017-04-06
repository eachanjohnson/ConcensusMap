#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include <boost/regex.hpp>

#include "my_exception.h"
#include "BKTree.h"

class BCLoader {

    public:
        BCLoader(std::string bc_file, bool hasHeader = true);
        bool load_map();
        bool load_tree();
        void save_tree(std::string save_file);

    private:
        bool hasHeader;
        std::map<std::string, std::string> bc_map;
        std::map<std::string, std::string> rev_bc_map;
        std::string bc_file;
        BKTree<std::string> tree;

};

BCLoader::BCLoader (std::string bc_file, bool hasHeader):
    bc_file(bc_file),
    hasHeader(hasHeader) {
}

bool BCLoader::load_tree() {
    tree = BKTree<std::string>();
    for (const auto& val_pair : bc_map) {
        auto & bc_name = val_pair.first;
        auto & bc_val = val_pair.second;
        tree.insert(bc_val);
    }

    return true;
}


void BCLoader::save_tree(std::string outfile) {
    
    std::ofstream ofs(outfile);
    boost::archive::text_oarchive oa(ofs);
    oa << tree;
    ofs.close();
    return;
}



bool BCLoader::load_map() {
    std::ifstream words(bc_file);

    std::string lstr;
    boost::regex expr ("(\\w+)\\s+(\\w*)");

    boost::smatch what;
    if (!words.is_open()) {
        std::cerr << "The infile cannot be open!\n";
        return false;
    } else {
        
        if (hasHeader) {
            std::cout << "Ignored the header. " << "\n";
            std::getline(words, lstr);
        }
        while (std::getline(words, lstr)) {
            bool res = boost::regex_search(lstr, what, expr);
            if (res) {
                std::string bc_name = what[1];
                std::string  bc_val = what[2];
                bc_map.insert(std::pair<std::string, std::string>(bc_name, bc_val));
            } else {
                throw my_exception("Problem in the parsing the barcode lines.\n");
            }
        }
    }
    
    std::cout << "Printing forward bc map" << "\n";
    for (const auto& val_pair : bc_map) {
        auto & bc_name = val_pair.first;
        auto & bc_val = val_pair.second;
        int bc_len = bc_val.length();
        //std::cout << "barcode: " << bc_name << " val : "  << bc_val << " bc_len: " << bc_len << "\n";
    
    }
    return true; 
    
}

int main(int argc, char* argv[]) {
    std::string i5_to_stagger_file = argv[1];

    std::cout << "args count: " << argc << "\n";

    bool hasHeader = true;
    if (argc == 3) {
        std::string sec_argv(argv[2]); 
        if (!sec_argv.compare("no")) {
            hasHeader = false;
        } else if (sec_argv.compare("yes")) {
            hasHeader = true;
        }
    }

    std::string outfile(argv[3]);

    std::cout << "hasHeader: " << hasHeader << "\n";
    BCLoader bc_loader = BCLoader(i5_to_stagger_file, hasHeader);
    bc_loader.load_map();
    bc_loader.load_tree();
    bc_loader.save_tree(outfile);
}



