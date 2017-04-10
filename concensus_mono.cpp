#include <string>
#include <map>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib> 
#include <cstring>
#include <set>
#include <cctype>
#include <memory>

#include "BKTree.h"
#include "fastq_reader.hpp"
#include "fastq_writer.hpp"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace po = boost::program_options;

class concensus_mono {
    public:
        bool parse_args(int argc, char* argv[]);
        void initialize();
        void core_engine();
        void print_help();
    private:
		// Input variables pushed through command line
		std::string i5_file_str;
		std::string i7_file_str;
		std::string sbc_file_str;
		std::string stagger_file_str;
		std::string short_read_str;
		std::string long_read_str;
		std::string prefix_str;
		std::string outdirpath;
		int mm_i5;
		int mm_i7;
		int mm_sbc;
        po::options_description desc;

};


void concensus_mono::print_help() {
    std::cout << desc << "\n";
	std::cout << "Usage: concensus --i5 <i5 file> --i7 <i7 file> "
        "--sbc <sbc file> --file1 <file1> --file2 <file2> "
        " -p <prefix_str> -o <outdir>\n\n";
}

bool concensus_mono::parse_args(int argc, char* argv[]) {
    
    bool all_set = true;
    

    desc.add_options()
		("help,h", "produce help message")
		("i5", po::value<std::string>(&i5_file_str), "index5 barcode file")
		("i7", po::value<std::string>(&i7_file_str), "index7 barcode file")
		("sbc", po::value<std::string>(&sbc_file_str), "strain barcode file")
		("stagger", po::value<std::string>(&stagger_file_str), "i5 to stagger map file")
		("short_read", po::value<std::string>(&short_read_str), "First file")
		("long_read", po::value<std::string>(&long_read_str), "Second file")
		("prefix,p", po::value<std::string>(&prefix_str), "Prefix string")
		("outdir,o", po::value<std::string>(&outdirpath), "Output directory")	
		("mm_i5", po::value(&mm_i5)->default_value(1), "Optional/Maximum allowed mismatches for i5 barcode.")
		("mm_i7", po::value(&mm_i7)->default_value(1), "Optional/Maximum allowed mismatches for i7 barcode.")
		("mm_sbc", po::value(&mm_sbc)->default_value(2), "Optional/Maximum allowed mismatches for sbc barcode.")
	;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        //print_help();
        return 0;
    } else {
        //all_set = false;
    }

    if (vm.count("i5")) {
		std::cout << "i5 barcode file is set to: " << i5_file_str << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: i5 barcode file is not set.\n";
	}

    if (vm.count("i7")) {
		std::cout << "i7 barcode file is set to: " << i7_file_str << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: i7 barcode file is not set.\n";
	}

    if (vm.count("sbc")) {
		std::cout << "Strain barcode file is set to: " << sbc_file_str << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: strain barcode file is not set.\n";
	}


    if (vm.count("short_read")) {
		std::cout << "Short read fastq file is set to: " << short_read_str << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: short read fastq file is not set.\n";
	}

	if (vm.count("long_read")) {
		std::cout << "Lond read fastq file is set to: " << long_read_str << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: Long read fastq file is not set.\n";
	}    


    if (vm.count("prefix")) {
		std::cout << "Prefix string is set to: " << prefix_str << ".\n";
	} else {
		std::cout << "Error: Prefix string is not set.\n";
	}
 
    if (vm.count("outdir")) {
		std::cout << "Outdir is set to: " << outdirpath << ".\n";
	} else {
		all_set = false;
		std::cout << "Error: Outdir is not set.\n";
	}

    std::cout << "Maximum allowed mismatches for i5 barcode is set to " << mm_i5 << ".\n";
    std::cout << "Maximum allowed mismatches for i7 barcode is set to " << mm_i7 << ".\n";
    std::cout << "Maximum allowed mismatches for sbc barcode is set to " << mm_sbc << ".\n";

}

void concensus_mono::initialize() {
        
	struct stat st = {0};

	if (stat(outdirpath.c_str(), &st) == -1) {
		mkdir(outdirpath.c_str(), 0755);
	}
}

void concensus_mono::core_engine() {

    // The words starting with lword is for short read
	std::string short_word1;
	std::string short_word2;
	std::string short_word3;
	std::string short_word4;

	// The words starting with rword is for long read
    std::string long_word1;
	std::string long_word2;
	std::string long_word3;
	std::string long_word4;


    fastq_reader short_file(short_read_str);
    fastq_reader long_file(long_read_str);

    long read_count = 0;
    std::cout << "reached here!" << "\n";
    while (short_file.getline(short_word1)) {
        if (!short_file.getline(short_word2)) { break; }
        if (!short_file.getline(short_word3)) { break; }
        if (!short_file.getline(short_word3)) { break; }


        if (!long_file.getline(long_word1)) { break; }
        if (!long_file.getline(long_word2)) { break; }
        if (!long_file.getline(long_word3)) { break; }
        if (!long_file.getline(long_word4)) { break; }
        ++read_count;
        if (read_count % 1000000 == 0) {
            std::cout << "read_count: " << read_count << "\n";
        }
    }
    
}

int main(int argc, char* argv[]) { 

	concensus_mono csm;

	
	bool all_set = true;
	try {
		all_set = csm.parse_args(argc, argv);	
	} catch(std::exception& e) {
		std::cerr << "error: " << e.what() << "\n";
        //lbs.print_help();
        return 1;

	} catch (...) {
		//lbs.print_help();
		return 0;
	}

 	if (!all_set) {
		csm.print_help();
		return 0;
	}

	csm.initialize();
	try {
		csm.core_engine();
	} catch(std::invalid_argument& e) {
        std::cerr << "error: " << e.what() << "\n";
		//lbs.print_help();
		return 1;
    }

	//csm.write_log();
        
    return 0;
}


