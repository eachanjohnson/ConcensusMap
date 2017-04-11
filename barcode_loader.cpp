#include <iostream>
#include <string>
#include <unordered_map>
#include <fstream>
#include <tuple>

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
        std::tuple<std::string, std::string, int> match_barcode(std::string barcode_str, int cutoff);
        int distance(std::string source, std::string target, bool remove_last = false);

    private:
        bool hasHeader;
        std::unordered_map<std::string, std::string> bc_map;
        std::unordered_map<std::string, std::string> rev_bc_map;
        std::unordered_map<std::string, int> seq_to_ind;
        std::string bc_file;
        BKTree<std::string> tree;

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


std::tuple<std::string, std::string, int> BCLoader::match_barcode(std::string barcode_str, int cutoff) {
    
    std::vector<std::string> results;

    results = tree.find(barcode_str, cutoff);

    // calculate the minimum dIstance between the target and references

    int smallest_dist = cutoff + 1;

    // The smallest_barcode initialization could technically be anything
    // since we overwrite this variable.

    // Create something like JJJJJJJJ
    std::string smallest_barcode = "";
    for (int j = 0; j < barcode_str.length(); j++) {
        smallest_barcode += "J";
    }

    std::vector<int> dist_vec;
    for (auto const& val : results) {
        int ldist = distance(val, barcode_str);
        if (ldist < smallest_dist) {
            smallest_dist = ldist;
            smallest_barcode = val;
        }

        dist_vec.push_back(ldist);
    }

    int smallest_count = 0;
    for (auto const& temp_dist : dist_vec) {
        if (temp_dist == smallest_dist) {
            smallest_count++;
        }
    }

    //std::cout << "actual_barcode: " << barcode_str << 
    //  ", smallest barcode: " <<  smallest_barcode <<  
    //  ", sallest dist: " << smallest_dist << 
    //  ", smallest_count: " << smallest_count << "\n";

    // So the smallest dist has to be unique, otherwise we shall put 
    //  them in a fil called unknow.

    std::string final_barcode;
    std::string match_type;

    if (smallest_count == 1) {
        match_type = "unique";
        final_barcode = smallest_barcode;
    } else if (smallest_count > 0) {
        match_type = "ambiguous";
    } else {
        match_type = "no_match";
    }
    auto match_obj = std::make_tuple(match_type, final_barcode, smallest_dist);
    return match_obj;
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


