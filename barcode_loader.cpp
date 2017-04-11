#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>

#include <boost/regex.hpp>

#include "my_exception.h"
#include "BKTree.h"

class BCLoader {

    public:
        BCLoader() = default ;
        BCLoader(std::string bc_file, bool hasHeader = true);

        // Copy constructor
        BCLoader(const BCLoader& bcl) {
            bc_file = bcl.bc_file;
            hasHeader = bcl.hasHeader;
        }

        bool load_map();
        void save_tree(std::string save_file);
        int get_index(std::string seq);
        void print_map();
        void print_index();
        bool load_tree();
        std::string val_from_bc_map(std::string lbc);
        std::vector<std::string> vals_from_tree(std::string barcode, int mm);

    private:
        bool hasHeader;
        std::unordered_map<std::string, std::string> bc_map;
        std::unordered_map<std::string, std::string> rev_bc_map;
        std::unordered_map<std::string, int> seq_to_ind;
        std::string bc_file;
        BKTree<std::string> tree;

};

std::string BCLoader::val_from_bc_map(std::string lbc) {
    std::string val = bc_map[lbc];
    return val;
}

std::vector<std::string> BCLoader::vals_from_tree(std::string barcode, int mm) {
    std::vector<std::string> vals = tree.find(barcode, mm);
    return vals;
}

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

int BCLoader::get_index(std::string seq) {
    int l_index = seq_to_ind[seq];
    return l_index;
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
        int l_index = 0;
        while (std::getline(words, lstr)) {
            bool res = boost::regex_search(lstr, what, expr);
            if (res) {
                std::string bc_name = what[1];
                std::string  bc_val = what[2];
                bc_map[bc_name] = bc_val;
                seq_to_ind[bc_val] = l_index; 
                l_index++;
            } else {
                throw my_exception("Problem in the parsing the barcode lines.\n");
            }
        }
    }
    return true; 
}

void BCLoader::print_map() {
     
    std::cout << "Printing forward bc map for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    for (const auto& val_pair : bc_map) {
        auto & bc_name = val_pair.first;
        auto & bc_val = val_pair.second;
        int bc_len = bc_val.length();
        std::cout << "barcode: " << bc_name << " val : "  << bc_val << " bc_len: " << bc_len << "\n";
    
    }
    std::cout << "...............................\n";
    
}


void BCLoader::print_index() {
    std::cout << "Printing bc index for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    for (const auto& val_pair : seq_to_ind) {
        auto & bc_name = val_pair.first;
        auto & bc_ind = val_pair.second;
        std::cout << "barcode: " << bc_name << " index : "  << bc_ind << "\n";
    
    }
    std::cout << "...............................\n";

}

/*
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

*/


