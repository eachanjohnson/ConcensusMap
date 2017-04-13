#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib> 
#include <cstring>
#include <cctype>
#include <sstream>
#include <algorithm>
#include <memory>
#include <tuple>
#include <map>
#include <vector>
#include <ctime>
#include <iomanip>

#include "BKTree.h"
#include "fastq_reader.hpp"
#include "fastq_writer.hpp"
#include "barcode_loader.cpp"

#include <boost/regex.hpp>
#include <boost/program_options.hpp>
#include <boost/algorithm/string.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

namespace po = boost::program_options;
typedef std::tuple<std::string, std::string, int, std::string> match_t;

class concensus_mono {
    public:
        bool parse_args(int argc, char* argv[]);
        void initialize();
        void core_engine();
        void print_help();
        void load_barcodes();
        std::string vec_to_str(std::vector<std::string> my_vector);
        void write_entry(const match_t& i5_tuple, const match_t& i7_tuple, const match_t& sbc_tuple); 
        std::string get_combined(std::string i5_val, std::string i7_val, std::string sbc_val);
        void write_outfile(std::string outfile);
        unsigned int get_key_val(unsigned int i5_pos, unsigned int i7_pos, unsigned int sbc_pos);
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
        unsigned int i5_lim;
        unsigned int i7_lim;
        unsigned int sbc_lim;
        unsigned int total_run;

        po::options_description desc;

        BCLoader i5_loader;
        BCLoader i7_loader;
        BCLoader sbc_loader;
        BCLoader stagger_loader;
        //std::unordered_map<std::string, long> count_map; 
        //std::map<std::string, long> count_map; 
        std::vector<int> count_map;

};


void concensus_mono::print_help() {
    std::cout << desc << "\n";
	std::cout << "Usage: concensus --i5 <i5 file> --i7 <i7 file> "
        "--sbc <sbc file> --file1 <file1> --file2 <file2> "
        " -p <prefix_str> -o <outdir>\n\n";
}


unsigned int concensus_mono::get_key_val(unsigned int i5_pos, unsigned int i7_pos, unsigned int sbc_pos) {
    unsigned int key_val = (( i5_pos * i7_lim ) + i7_pos )* sbc_lim + sbc_pos;
    return key_val;
}

void concensus_mono::load_barcodes() {
    i5_loader = BCLoader(i5_file_str);
    i5_loader.load_map();
    i5_loader.load_tree();
    //i5_loader.print_map();
    //i5_loader.print_index();
    i5_loader.print_rev_map();
    i5_lim = i5_loader.get_bc_lim();
    std::cout << "i5_lim: " << i5_lim << "\n";

    i7_loader = BCLoader(i7_file_str);
    i7_loader.load_map();
    i7_loader.load_tree();
    //i7_loader.print_map();
    //i7_loader.print_index();
    i7_loader.print_rev_map();
    i7_lim = i7_loader.get_bc_lim();
    std::cout << "i7_lim: " << i7_lim << "\n";

    sbc_loader = BCLoader(sbc_file_str);
    sbc_loader.load_map();
    sbc_loader.load_tree();    
    //sbc_loader.print_map();
    //sbc_loader.print_index();
    sbc_loader.print_rev_map();
    sbc_lim = sbc_loader.get_bc_lim();
    std::cout << "sbc_lim: " << sbc_lim << "\n";

    stagger_loader = BCLoader(stagger_file_str);
    stagger_loader.load_map();
    //stagger_loader.print_map();
    unsigned int i5_bc_vec_size = i5_loader.get_bc_vec_size();
    unsigned int count_map_size = i5_bc_vec_size * i7_lim * sbc_lim;
    count_map = std::vector<int>(count_map_size);

}



bool concensus_mono::parse_args(int argc, char* argv[]) {
    
    bool all_set = true;
    

    desc.add_options()
		("help,h", "produce help message")
		("i5", po::value<std::string>(&i5_file_str)->required(), "index5 barcode file")
		("i7", po::value<std::string>(&i7_file_str)->required(), "index7 barcode file")
		("sbc", po::value<std::string>(&sbc_file_str)->required(), "strain barcode file")
		("stagger", po::value<std::string>(&stagger_file_str)->required(), "i5 to stagger map file")
		("short_read", po::value<std::string>(&short_read_str)->required(), "First file")
		("long_read", po::value<std::string>(&long_read_str)->required(), "Second file")
		("prefix,p", po::value<std::string>(&prefix_str)->required(), "Prefix string")
		("outdir,o", po::value<std::string>(&outdirpath)->required(), "Output directory")	
		("mm_i5", po::value(&mm_i5)->default_value(1), "Optional/Maximum allowed mismatches for i5 barcode.")
		("mm_i7", po::value(&mm_i7)->default_value(1), "Optional/Maximum allowed mismatches for i7 barcode.")
		("mm_sbc", po::value(&mm_sbc)->default_value(2), "Optional/Maximum allowed mismatches for sbc barcode.")
        ("total_run", po::value(&total_run)->default_value(10000), "Optional/total reads to process.")
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
    return all_set;

}

void concensus_mono::initialize() {
        
	struct stat st = {0};

	if (stat(outdirpath.c_str(), &st) == -1) {
		mkdir(outdirpath.c_str(), 0755);
	}
}


std::string concensus_mono::vec_to_str(std::vector<std::string> lvec) {
    std::string res_str = "";
    for (int j = 0; j < lvec.size(); j++) {
        if (j == 0) {
            res_str += lvec[j];
        } else {
            res_str += ", " + lvec[j];
        }
    }
    return res_str;
    
}

std::string concensus_mono::get_combined(std::string i5_val, std::string i7_val, 
        std::string sbc_val) {
    int i5_index = i5_loader.get_index(i5_val);
    int i7_index = i7_loader.get_index(i7_val);
    int sbc_index = sbc_loader.get_index(sbc_val);

    std::string i5_str = std::to_string(i5_index);
    std::string i7_str = std::to_string(i7_index);
    std::string sbc_str = std::to_string(sbc_index);
    
    std::string combined_str = i5_str + "_" + i7_str + "_" + sbc_str;
    return combined_str;    
}

void concensus_mono::write_entry(const match_t& i5_tuple, const match_t& i7_tuple, const match_t& sbc_tuple) {
    
    std::string i5_mt = std::get<0>(i5_tuple);
    std::string i7_mt = std::get<0>(i7_tuple);
    std::string sbc_mt = std::get<0>(sbc_tuple);
 
    if (i5_mt.compare("unique") == 0 && i7_mt.compare("unique") == 0 && sbc_mt.compare("unique") == 0) {
        //std::cout << "All unique" << "\n";
        std::string i5_val = std::get<1>(i5_tuple);
        std::string i7_val = std::get<1>(i7_tuple);
        std::string sbc_val = std::get<1>(sbc_tuple);
        
        std::string i5_read = std::get<3>(i5_tuple);
        std::string i7_read = std::get<3>(i7_tuple);
        std::string sbc_read = std::get<3>(sbc_tuple);

        std::string combined_str = i5_val + "_" + i7_val + "_" + sbc_val;
        //std::cout << "combined_str: " << combined_str << "\n";
        //std::string combined_str = get_combined(i5_val, i7_val, sbc_val);
        unsigned int i5_index = i5_loader.get_index(i5_val);
        unsigned int i7_index = i7_loader.get_index(i7_val);
        unsigned int sbc_index = sbc_loader.get_index(sbc_val);

       unsigned long key_val = get_key_val(i5_index, i7_index, sbc_index);
        
        count_map[key_val] += 1;
       
        
    }
}

void concensus_mono::write_outfile(std::string outfile) {
    std::ofstream outf(outfile);
    if (outf.is_open()) {
        // Write header
        outf << "i5_index" << "\t" << "i7_index" << "\t" << "sbc_index" << "\t" << "read_count" << "\n";
        std::vector<std::string> i5_vec = i5_loader.get_bc_vector();
        std::vector<std::string> i7_vec = i7_loader.get_bc_vector();
        std::vector<std::string> sbc_vec = sbc_loader.get_bc_vector();

        for (const auto& i5_ : i5_vec) {
            for (const auto& i7_ : i7_vec) {
                for (const auto& sbc_ : sbc_vec) {
                    std::string combined_str = i5_ + "_" + i7_ + "_" + sbc_;
                    unsigned int i5_index = i5_loader.get_index(i5_);
                    unsigned int i7_index = i7_loader.get_index(i7_);
                    unsigned int sbc_index = sbc_loader.get_index(sbc_);

                    unsigned long key_val = get_key_val(i5_index, i7_index, sbc_index);

                    long val = count_map[key_val];
                    std::string val_str = std::to_string(val);
                    std::string i5_id = i5_loader.id_from_rev_map(i5_);
                    std::string i7_id = i7_loader.id_from_rev_map(i7_);
                    std::string sbc_id = sbc_loader.id_from_rev_map(sbc_);
                    std::string out_str = i5_id + "\t" + i7_id + "\t" + sbc_id + "\t" + val_str;
                    outf << out_str << "\n";   
                }
            }
        } 

        outf.close();
    } else {
        throw my_exception("Problem in opening the output file.\n");
    }

    
}

void concensus_mono::core_engine() {

    // These are the information we may extract from config
    // in the future
    int i7_len = 8;
    int i5_len = 8;
    int gap_after_stagger = 18;
    int sbc_len = 20;

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

    std::string i5_local;
    std::string i7_local;
    std::string sbc_local;
    std::string stagger_local;


    fastq_reader short_file(short_read_str);
    fastq_reader long_file(long_read_str);


    long read_count = 0;
    while (short_file.getline(short_word1)) {
        if (!short_file.getline(short_word2)) { break; }
        if (!short_file.getline(short_word3)) { break; }
        if (!short_file.getline(short_word3)) { break; }


        if (!long_file.getline(long_word1)) { break; }
        if (!long_file.getline(long_word2)) { break; }
        if (!long_file.getline(long_word3)) { break; }
        if (!long_file.getline(long_word4)) { break; }
        ++read_count;

        // The entire short read consists i7_index
        i7_local = short_word2.substr(0, i7_len);
        i5_local = long_word2.substr(0, i5_len);
        stagger_local = stagger_loader.val_from_bc_map(i5_local);
        int stagger_len = stagger_local.length();
        int sbc_start = i5_len + stagger_len + gap_after_stagger;
        sbc_local = long_word2.substr(sbc_start, sbc_len);

        const auto& i5_tuple = i5_loader.match_barcode(i5_local, mm_i5);
        const auto& i7_tuple = i7_loader.match_barcode(i7_local, mm_i7);
        const auto& sbc_tuple = sbc_loader.match_barcode(sbc_local, mm_sbc);
        //const std::vector<std::string>& i5_vals = i5_loader.vals_from_tree(i5_local, mm_i5);
        //const std::vector<std::string>& i7_vals = i7_loader.vals_from_tree(i7_local, mm_i7);
        //const std::vector<std::string>& sbc_vals = sbc_loader.vals_from_tree(sbc_local, mm_sbc);
        std::string i5_mt = std::get<0>(i5_tuple);
        std::string i7_mt = std::get<0>(i7_tuple);
        std::string sbc_mt = std::get<0>(sbc_tuple);

       /*
        std::cout << "i7_local: " << i7_local << " i5_local: " << i5_local << 
            " stagger_local: " << stagger_local << " sbc_local: " << sbc_local << 
            " i5_match_type: " << i5_mt << " i7_match_type: " << i7_mt  << 
            " sbc_match_type: " << sbc_mt << "\n";
        */
        write_entry(i5_tuple, i7_tuple, sbc_tuple);
        //std::cout << "\n";

       // Get the stagger sequence using i5
       
  
        
        if (read_count % 1000 == 0) {
            std::cout << "read_count: " << read_count << "\n";
            std::cout << "count_map_size: " << count_map.size() << "\n";
            auto t = std::time(nullptr);
			auto tm = *std::localtime(&t);

		    std::ostringstream oss;
		    oss << std::put_time(&tm, "%d-%m-%Y %H-%M-%S");
		    auto str = oss.str();

		    std::cout << str << std::endl;
        }
        if (read_count == total_run) {break;}
    }
    std::string outfile_str = outdirpath + "/" + prefix_str + "_outfile.txt";
	
    //write_outfile(outfile_str);
    
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
        csm.load_barcodes();
		csm.core_engine();
	} catch(std::invalid_argument& e) {
        std::cerr << "error: " << e.what() << "\n";
		//lbs.print_help();
		return 1;
    }

	//csm.write_log();
        
    return 0;
}


