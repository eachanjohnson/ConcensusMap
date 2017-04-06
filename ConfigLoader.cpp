#include <iostream>
#include <string>
#include <map>
#include <fstream>

#include <boost/regex.hpp>

#include "my_exception.h"

class config_loader {

    public:
        config_loader(std::string i5_to_stagger_file);
        bool bc_loader(bool hasHeader = true);

    private:
        std::map<std::string, std::string> i5_to_stagger_map;
        std::string i5_to_stagger_file;


};

config_loader::config_loader (std::string i5_to_stagger_file):
    i5_to_stagger_file(i5_to_stagger_file) {
}


bool config_loader::bc_loader(bool hasHeader) {
    std::ifstream words(i5_to_stagger_file);

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
                std::string i5 = what[1];
                std::string stagger = what[2];
                i5_to_stagger_map.insert(std::pair<std::string, std::string>(i5, stagger));
            } else {
                throw my_exception("Problem in the parsing the barcode lines.\n");
            }
        }
    }

    for (const auto& val_pair : i5_to_stagger_map) {
        auto & i5 = val_pair.first;
        auto & stagger = val_pair.second;
        int stagger_len = stagger.length();
        std::cout << "i5: " << i5 << " stagger: "  << stagger << " s_len: " << stagger_len << "\n";
    
    }
    return true; 
    
}

int main(int argc, char* argv[]) {
    std::string i5_to_stagger_file = argv[1];

    config_loader conf_loader = config_loader(i5_to_stagger_file);
    conf_loader.i5_to_stagger_loader();
}



