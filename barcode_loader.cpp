#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <tuple>
#include <vector>

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
        void print_map();
        void print_index();
        bool load_tree();
        std::string val_from_bc_map(std::string lbc);
        std::vector<std::string> vals_from_tree(std::string barcode, int mm);
        std::tuple<std::string, std::string, int, std::string> match_barcode(std::string barcode_str, int cutoff);
        int distance(std::string source, std::string target, bool remove_last = false);
        int get_index(std::string lbc);
        std::vector<std::string> get_bc_vector();
        std::string id_from_rev_map(std::string lbc_val);
        void print_rev_map();
        unsigned int get_bc_lim();
        unsigned int get_bc_vec_size();
    private:
        bool hasHeader;
        std::unordered_map<std::string, std::string> bc_map;
        std::unordered_map<std::string, std::string> rev_bc_map;
        std::unordered_map<std::string, int> seq_to_ind;
        std::string bc_file;
        BKTree<std::string> tree;
        std::vector<std::string> bc_vec;

};

unsigned int BCLoader::get_bc_vec_size() {
    return bc_vec.size();
}

unsigned int BCLoader::get_bc_lim() {
    int bc_vec_size = bc_vec.size();
    double val_log2 = log2(bc_vec_size);
    double val_log2_ceil = ceil(val_log2);
    double pow_val = pow(2.0, val_log2_ceil);
    unsigned int retval = (unsigned int) pow_val;
    return retval;
}


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

int BCLoader::get_index(std::string lbc) {
    int lindex = seq_to_ind[lbc];
    return lindex;
}

std::string BCLoader::val_from_bc_map(std::string lbc) {
    std::string val = bc_map[lbc];
    return val;
}

std::string BCLoader::id_from_rev_map(std::string lbc_val) {
    std::string lid = rev_bc_map[lbc_val];
    return lid;
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

std::vector<std::string> BCLoader::get_bc_vector() {
    return bc_vec;
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
        int l_index = 0;
        while (std::getline(words, lstr)) {
            bool res = boost::regex_search(lstr, what, expr);
            if (res) {
                std::string bc_name = what[1];
                std::string  bc_val = what[2];
                bc_map[bc_name] = bc_val;
                rev_bc_map[bc_val] = bc_name;
                seq_to_ind[bc_val] = l_index; 
                bc_vec.push_back(bc_val);
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

void BCLoader::print_rev_map() {
     
    std::cout << "Printing forward bc map for : " << bc_file << "\n";
    std::cout << "------------------------------\n";
    for (const auto& val_pair : rev_bc_map) {
        auto & bc_val = val_pair.first;
        auto & bc_name = val_pair.second;
        int bc_len = bc_val.length();
        std::cout << "val: " << bc_val << " barcode : "  << bc_name << " bc_len: " << bc_len << "\n";
    
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


