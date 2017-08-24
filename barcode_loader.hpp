#ifndef _BARCODE_LOADER_HPP
#define _BARCODE_LOADER_HPP
#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <tuple>
#include <vector>

#include <boost/regex.hpp>

#include "my_exception.h"
#include "BKTree.h"
#include "utils.hpp"

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
        void print_map();
        void print_name_to_index();
        void print_seq_to_index();
        bool load_tree();
        std::string val_from_bc_map(std::string lbc);
        std::vector<std::string> vals_from_tree(std::string barcode, int mm);
        std::tuple<std::string, std::string, int, std::string> match_barcode(std::string barcode_str, int cutoff);
        int distance(std::string source, std::string target, bool remove_last = false);
        int get_name_to_index(std::string lname);
        int get_seq_to_index(std::string lseq);
        std::vector<std::string> get_name_vector();
    private:
        bool hasHeader;
        std::unordered_map<std::string, std::string> bc_map;
        std::unordered_map<std::string, int> seq_to_ind;
        std::unordered_map<std::string, int> name_to_ind;
        std::string bc_file;
        BKTree<std::string> tree;
        std::vector<std::string> name_vec;

};


int BCLoader::distance(std::string source, std::string target, bool remove_last) {

    const int n = source.length();
    const int m = target.length();
    if (n == 0) {
        throw std::invalid_argument("Length of source is zero.");
    }
    if (m == 0) {
        throw std::invalid_argument("Length of target is zero.");
    }

    if (m !=n ) {
        throw std::invalid_argument("Source and target have different length");
	}

    int ldist = 0;

    int n1 = n;
    if (remove_last)
    {
        n1--;
    }

    for (int j = 0; j < n1; j++) {
        if (source[j] != target[j]) {
            ldist++;
        }
    }

    return ldist;
}	

std::tuple<std::string, std::string, int, std::string> BCLoader::match_barcode(std::string barcode_str, int cutoff) {

    std::vector<std::string> results;
    std::string match_type = "no_match";
    std::string final_barcode = "";
    int smallest_dist = cutoff + 1;
    
    for (int j = 0; j <= cutoff; j++) {
        results = tree.find(barcode_str, j);
        if (results.size() == 1) {
            match_type = "unique";
            final_barcode = results[0];
            smallest_dist = j;
            break;
        } else if (results.size() > 1) {
            match_type = "ambiguous";
            smallest_dist = j;
            break;
        }
    }
    auto match_obj = std::make_tuple(match_type, final_barcode, smallest_dist, barcode_str);
    return match_obj;
}

int BCLoader::get_seq_to_index(std::string lbc) {
            
    int lindex = seq_to_ind[lbc];
    if (lindex == 0) {
        // The valid value for a barcode will always be positive
        std::string mystr = "Probably illegal barcode: " + lbc;
        throw my_exception(mystr);
    }
    
    return lindex;
    
}

int BCLoader::get_name_to_index(std::string lname) {
    int lindex = name_to_ind[lname];
    if (lindex == 0) {
        // The valid value for a barcode will always be positive
        std::string mystr = "Probably illegal barcode name: " + lname;
        throw my_exception(mystr);
    }
   
    return lindex;
}

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

std::vector<std::string> BCLoader::get_name_vector() {
    return name_vec;
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
    boost::regex expr ("(\\S+)\\s+(\\S*)");

    boost::smatch what;
    if (!words.is_open()) {
        std::cerr << "The infile cannot be open!\n";
        return false;
    } else {
        
        if (hasHeader) {
            std::cout << "Ignored the header. " << "\n";
            std::getline(words, lstr);
        }
        // the initial value of l_index is changed from zero to one as
        // zero leads to map an emply barcode to first barcode.
        int l_index = 1;
        while (std::getline(words, lstr)) {
            bool res = boost::regex_search(lstr, what, expr);
            if (res) {
                std::string bc_name = what[1];
                std::string  bc_seq = what[2];
                bc_map[bc_name] = bc_seq;
                seq_to_ind[bc_seq] = l_index; 
                name_to_ind[bc_name] = l_index;
                name_vec.push_back(bc_name);
                l_index++;
            } else {
                throw my_exception("Problem in the parsing the barcode lines.\n");
            }
        }
    }
    return true; 
}

void BCLoader::print_map() {
     
    std::cout << "Printing name to seq for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    UtilC::print_map(bc_map);
    std::cout << "...............................\n";
}

void BCLoader::print_name_to_index() {
    std::cout << "Printing name to index for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    UtilC::print_map(name_to_ind);
    std::cout << "...............................\n";
}


void BCLoader::print_seq_to_index() {
    std::cout << "Printing seq to index for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    UtilC::print_map(seq_to_ind);
    std::cout << "...............................\n";

}

#endif

