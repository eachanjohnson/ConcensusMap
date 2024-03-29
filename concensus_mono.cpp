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
#include <cassert>

#include "BKTree.h"
#include "fastq_reader.hpp"
#include "fastq_writer.hpp"
#include "barcode_loader.hpp"

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
        void write_outfile(std::string outfile);
        std::string get_key_val(unsigned int i5_pos, unsigned int i7_pos, unsigned int sbc_pos);
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
        unsigned int total_run = 0;
  
        unsigned long unique_exact_count = 0;
        unsigned long unique_nonexact_count = 0;
        unsigned long ambiguous_count = 0;
        unsigned long no_match_count = 0;
        unsigned long total_count = 0;
        

        po::options_description desc;

        BCLoader i5_loader;
        BCLoader i7_loader;
        BCLoader sbc_loader;
        BCLoader stagger_loader;
        std::unordered_map<std::string, unsigned int> count_map_exact; 
        std::unordered_map<std::string, unsigned int> count_map_nonexact; 
        

};


void concensus_mono::print_help() {
    std::cout << desc << "\n";
	std::cout << "Usage: concensus --i5 <i5 file> --i7 <i7 file> "
        "--sbc <sbc file> --file1 <file1> --file2 <file2> "
        " -p <prefix_str> -o <outdir>\n\n";
}

std::string concensus_mono::get_key_val(unsigned int i5_pos, unsigned int i7_pos, unsigned int sbc_pos) {
    std::string i5_pos_str = std::to_string(i5_pos);
    std::string i7_pos_str = std::to_string(i7_pos);
    std::string sbc_pos_str = std::to_string(sbc_pos);
    std::string combined = i5_pos_str + "_" + i7_pos_str + "_" + sbc_pos_str;
    return combined;
}

void concensus_mono::load_barcodes() {
    i5_loader = BCLoader(i5_file_str);
    i5_loader.load_map();
    i5_loader.load_tree();
    i5_loader.print_map();
    i5_loader.print_name_to_index();
    i5_loader.print_seq_to_index();

    i7_loader = BCLoader(i7_file_str);
    i7_loader.load_map();
    i7_loader.load_tree();
    i7_loader.print_map();
    i7_loader.print_name_to_index();
    i7_loader.print_seq_to_index();

    sbc_loader = BCLoader(sbc_file_str);
    sbc_loader.load_map();
    sbc_loader.load_tree();    
    sbc_loader.print_map();
    sbc_loader.print_name_to_index();
    sbc_loader.print_seq_to_index();

    stagger_loader = BCLoader(stagger_file_str);
    stagger_loader.load_map();
    stagger_loader.print_map();

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
        ("total_run", po::value(&total_run)->default_value(-1), "Optional/total reads to process.")
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

void concensus_mono::write_entry(const match_t& i5_tuple, const match_t& i7_tuple, const match_t& sbc_tuple) {

    std::string no_match_str = "no_match";
    std::string ambiguous_str = "ambiguous";
    std::string unique_str = "unique";

    total_count++;
    std::string i5_mt = std::get<0>(i5_tuple);
    std::string i7_mt = std::get<0>(i7_tuple);
    std::string sbc_mt = std::get<0>(sbc_tuple);

    if (0 == i5_mt.compare(no_match_str) || 
            0 == i7_mt.compare(no_match_str) || 
            0 == sbc_mt.compare(no_match_str)) {

        no_match_count++;

    } else if (0 == i5_mt.compare(ambiguous_str) || 
            0 == i7_mt.compare(ambiguous_str) || 
            0 == sbc_mt.compare(ambiguous_str)) {

        ambiguous_count++;
        
    } else if (0 == i5_mt.compare(unique_str) && 
               0 == i7_mt.compare(unique_str) && 
               0 == sbc_mt.compare(unique_str)) {

        std::string i5_val = std::get<1>(i5_tuple);
        std::string i7_val = std::get<1>(i7_tuple);
        std::string sbc_val = std::get<1>(sbc_tuple);

        unsigned int i5_index = i5_loader.get_seq_to_index(i5_val);
        unsigned int i7_index = i7_loader.get_seq_to_index(i7_val);
        unsigned int sbc_index = sbc_loader.get_seq_to_index(sbc_val);

        // None of the index should be zero
        assert(i5_index > 0);
        assert(i7_index > 0);
        assert(sbc_index > 0);

        std::string key_val = get_key_val(i5_index, i7_index, sbc_index);

        //std::cout << "All unique" << "\n";
        int i5_dist = std::get<2>(i5_tuple);         
        int i7_dist = std::get<2>(i7_tuple);         
        int sbc_dist = std::get<2>(sbc_tuple);   
 
        if (0 == i5_dist && 
            0 == i7_dist && 
            0 == sbc_dist) {

            unique_exact_count++;
            count_map_exact[key_val] += 1;

        } else {   

            unique_nonexact_count++;  
            count_map_nonexact[key_val] += 1;

        }

    } else {
        std::string mystr = "Illegal match type(s): i5_mt: " + i5_mt + 
            " i7_mt: " + i7_mt + " sbc_mt:" + sbc_mt;
        throw my_exception(mystr);
    }
        
}

void concensus_mono::write_outfile(std::string outfile) {
    std::ofstream outf(outfile);
    if (outf.is_open()) {
        // Write header
        outf << "i5_index" << "\t" << "i7_index" << "\t" << "sbc_index" 
             << "\t" << "read_count_exact" << "\t" << "read_count_nonexact" 
             << "\t" << "read_count_all" << "\n";
        std::vector<std::string> i5_vec = i5_loader.get_name_vector();
        std::vector<std::string> i7_vec = i7_loader.get_name_vector();
        std::vector<std::string> sbc_vec = sbc_loader.get_name_vector();

        for (const auto& i5_ : i5_vec) {
            for (const auto& i7_ : i7_vec) {
                for (const auto& sbc_ : sbc_vec) {
                    unsigned int i5_index = i5_loader.get_name_to_index(i5_);
                    unsigned int i7_index = i7_loader.get_name_to_index(i7_);
                    unsigned int sbc_index = sbc_loader.get_name_to_index(sbc_);

                    std::string key_val = get_key_val(i5_index, i7_index, sbc_index);

                    unsigned int val_exact = count_map_exact[key_val];
                    unsigned int val_nonexact = count_map_nonexact[key_val];
                    unsigned int val_all = val_exact + val_nonexact;
                    std::string val_exact_str = std::to_string(val_exact);
                    std::string val_nonexact_str = std::to_string(val_nonexact);
                    std::string val_all_str = std::to_string(val_all);

                    std::string out_str = i5_ + "\t" + i7_ + "\t" + sbc_ + "\t" + val_exact_str + "\t" + val_nonexact_str + "\t" + val_all_str;
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
    // This gap corresponds to PCR primer (whose length is
    // 18, ATCTTGTGGAAAGGACGA)
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
       
  
        
        if (read_count % 1000000 == 0) {
            std::cout << "read_count: " << read_count << "\n";
            std::cout << "count_map_exact_size: " << count_map_exact.size() << 
                 "\n";
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
	
    write_outfile(outfile_str);
    std::cout << "Unique exact read count:\t" << unique_exact_count << "\n";
    std::cout << "Unique nonexact read count:\t" << unique_nonexact_count 
        << "\n";
    std::cout << "Ambiguous read count:\t" << ambiguous_count << "\n";
    std::cout << "No match read count:\t" << no_match_count << "\n";
    std::cout << "Total read count:\t" << total_count << "\n";
    unsigned long total_count_t = unique_exact_count + unique_nonexact_count + 
        ambiguous_count + no_match_count;
    assert(total_count_t == total_count);
    
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


