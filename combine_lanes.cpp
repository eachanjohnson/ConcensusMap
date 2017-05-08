#define CSV_IO_NO_THREAD
#include <iostream>
#include <string>
#include <fstream>
#include <experimental/filesystem>

#include <boost/regex.hpp>
#include <boost/program_options.hpp>

#include "map_loader.hpp"

namespace po = boost::program_options;
namespace fs = std::experimental::filesystem;


class combine_lanes {

    public:
        bool parse_args(int argc, char* argv[]);
        void print_help();
        void get_count_file_map();
        void join_lanes();
        std::vector<std::string> split(const std::string &s, char delim); 
        std::string get_mapped_str(std::string key_str);
        void load_maps();
        template <typename T1, typename T2>
        std::vector<std::string> combine_vectors(std::vector<T1> vec1, 
            std::vector<T2> vec2, std::string delim);
        std::map<std::string, std::string> make_map(std::vector<std::string> vec1, 
            std::vector<std::string> vec2);
        template <typename T>
            std::vector<std::string> change_to_string(std::vector<T> vec);
        template <typename T1, typename T2>
            void print_map(std::map<T1, T2> lmap);
        void print_some_maps();

    private:
        std::string i5_plates_str;
        std::string i7_plates_str;
        std::string strains_map_str;
        std::string count_dir_str;
        std::string summary_file_str;
        std::string plate_bcs_str;
        std::string comp_map_str;
        po::options_description desc;
        std::map<int, std::string> count_file_map;
        template<typename Out>
          void split(const std::string &s, char delim, Out result);

        std::map<std::string, int> i5_key_to_plate;
        std::map<std::string, int> i5_key_to_quadrant;
        std::map<std::string, std::string> i7_map;
        std::map<std::string, std::string> plate_bcs_map;
        std::map<std::string, std::string> compound_map;

};

template <typename T1, typename T2>
void combine_lanes::print_map(std::map<T1, T2> lmap) {
    for (const auto & e : lmap) {
        auto key = e.first;
        auto val = e.second;
        std::cout << "key: " << key << " val: " << val << "\n";
    }
}

template <>
void combine_lanes::print_map<std::string, int>(std::map<std::string, int> lmap) {
    for (const auto & e : lmap) {
        auto key = e.first;
        auto val = e.second;
        std::cout << "key: " << key << " val: " << std::to_string(val) << "\n";
    }
}

void combine_lanes::print_some_maps() {
    std::cout << "Printing i5_key_to_plate." << "\n";
    print_map(i5_key_to_plate);
    std::cout << "Printing i5_key_to_quadrant.\n";
    print_map(i5_key_to_quadrant);
    std::cout << "Printing i7_map.\n";
    print_map(i7_map);
    std::cout << "Printing plate_bcs_map.\n";
    print_map(plate_bcs_map);
    //std::cout << "Printing comp_map.\n";
    //print_map(compound_map);

}

template<typename T> 
std::vector<std::string> combine_lanes::change_to_string(std::vector<T> vec) {
    std::vector<std::string> lvec;
    for (int j = 0; j < vec.size(); j++) {
        lvec.push_back(std::to_string(vec[j]));
    }
    return lvec;
}

template<>
std::vector<std::string> combine_lanes::change_to_string<std::string>(std::vector<std::string> vec) {
     return vec;
}

template <typename T1, typename T2>
std::vector<std::string> combine_lanes::combine_vectors
            (std::vector<T1> vec1, 
            std::vector<T2> vec2, std::string delim) {
    std::vector<std::string> vec1_sv = change_to_string<T1>(vec1);
    std::vector<std::string> vec2_sv = change_to_string<T2>(vec2);

    std::vector<std::string> comb_vec;
    for (size_t j = 0; j < vec1.size(); j++) {
        std::string var1 = vec1_sv[j];
        std::string var2 = vec2_sv[j];
        std::string final_var = var1 + delim + var2;
        comb_vec.push_back(final_var);
    }
    return comb_vec;
}

std::map<std::string, std::string> combine_lanes::make_map(std::vector<std::string> vec1, 
        std::vector<std::string> vec2) {
    std::map<std::string, std::string> lmap;
    //assert(vec1.size() == vec2.size());
    for (size_t i = 0; i < vec1.size(); ++i)
        lmap[vec1[i]] = vec2[i];
    return lmap;
}

void combine_lanes::load_maps() {

    std::string ldelim = "tab";
    MapLoader i5_plates_map(i5_plates_str, ldelim);
    i5_key_to_plate = i5_plates_map.getMap<std::string, int>("i5_key", "i5_plate");
    i5_key_to_quadrant = i5_plates_map.getMap<std::string, int>("i5_key", "quadrant");

    MapLoader i7_plates_map(i7_plates_str, ldelim);
    std::vector<std::string> i7_key = i7_plates_map.getColumn<std::string>("i7_key");
    std::vector<int> i7_quadrant = i7_plates_map.getColumn<int>("quadrant");
    std::vector<std::string> i7_row = i7_plates_map.getColumn<std::string>("row");
    std::vector<int> i7_column = i7_plates_map.getColumn<int>("column");
    std::vector<std::string> i7_key_quadrant = combine_vectors<std::string, int>(i7_key, i7_quadrant, "_");
    std::vector<std::string> i7_row_column = combine_vectors<std::string, int>(i7_row, i7_column, "");
    i7_map = make_map(i7_key_quadrant, i7_row_column);    

    std::string cdelim = "comma";
    MapLoader plate_bcs(plate_bcs_str, cdelim);
    std::vector<std::string> plate_name = plate_bcs.getColumn<std::string>("plate_name");
    std::vector<int> primer_plate = plate_bcs.getColumn<int>("primer_plate");
    std::vector<int> pool = plate_bcs.getColumn<int>("pool");
    std::vector<std::string> primer_plate_pool = combine_vectors<int, int>(primer_plate, pool, "_");
    plate_bcs_map = make_map(primer_plate_pool, plate_name);



    MapLoader comp_map(comp_map_str, cdelim);
    std::vector<std::string> Plate_Barcode = comp_map.getColumn<std::string>("Plate_Barcode");
    std::vector<std::string> Well_Position = comp_map.getColumn<std::string>("Well_Position");
    std::vector<std::string> Broad_Sample = comp_map.getColumn<std::string>("Broad_Sample");
    std::vector<std::string> mg_per_ml = comp_map.getColumn<std::string>("mg_per_ml");
    std::vector<std::string> mmoles_per_liter = comp_map.getColumn<std::string>("mmoles_per_liter");

    std::vector<std::string> Plate_Barcode_Well_Position = combine_vectors<std::string, std::string>(Plate_Barcode, Well_Position, "_");
    std::string tab_str = "\t";
    std::vector<std::string> temp1 = combine_vectors<std::string, std::string>(Broad_Sample, mg_per_ml, tab_str);
    std::vector<std::string> temp2 = combine_vectors<std::string, std::string>(temp1, mmoles_per_liter, tab_str);
    compound_map = make_map(Plate_Barcode_Well_Position, temp2);

   
}

void combine_lanes::get_count_file_map() {
   
    boost::regex reg_str("_\\S+");
    for (auto & p : fs::directory_iterator(count_dir_str)) {
        std::string lfile = fs::path(p).filename();
        std::string lane_str = boost::regex_replace(lfile, reg_str, "");
        int lane = std::stoi(lane_str);
        count_file_map[lane] = lfile;
    }
}

// Obtained from http://stackoverflow.com/questions/236129/split-a-string-in-c
template<typename Out>
void combine_lanes::split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}


// Obtained from http://stackoverflow.com/questions/236129/split-a-string-in-c
std::vector<std::string> combine_lanes::split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string combine_lanes::get_mapped_str(std::string key_str) {
    std::string mapped_str;
    char tab_delim = '\t';
    std::vector<std::string> parts = split(key_str, tab_delim);
    std::string lane_str = parts[0];
    int lane = std::stoi(lane_str); 
    std::string i5_id = parts[1];
    std::string i7_id = parts[2];
    std::string sbc_id = parts[3];
    std::string count_str = parts[4];
    int i5_plate = i5_key_to_plate[i5_id];
    int i5_quadrant = i5_key_to_quadrant[i5_id];
    std::string plate_pool = std::to_string(i5_plate) + "_" + lane_str;
    std::string plate_barcode = plate_bcs_map[plate_pool];
    //std::cout << "plate_pool: " << plate_pool << "\n";
    std::cout << "plate_barcode: " << plate_barcode << "\n";
    std::string i7_key_quadrant = i7_id + "_" + std::to_string(i5_quadrant);
    //std::cout << "i7_key_quadrant: " << i7_key_quadrant << "\n";
    std::string row_column = i7_map[i7_key_quadrant];
    //std::cout << "row_column: " << row_column << "\n";
    std::string barcode_wellpos = plate_barcode + "_" + row_column;
    std::string compound_info = compound_map[barcode_wellpos]; 
    //std::cout << "compound_info: " << compound_info << "\n";
    
    mapped_str = key_str + "\t" + std::to_string(i5_plate) + "\t" + std::to_string(i5_quadrant) + "\t" +
        plate_barcode + "\t" + row_column + "\t" + compound_info;
    return mapped_str;
}


void combine_lanes::join_lanes() {

    std::ofstream summary_file(summary_file_str);

    for (const auto& e : count_file_map) {
        int lane = e.first;
        std::string lfile_str = e.second;
        std::cout << "lane: " << lane << " lfile: " << lfile_str << "\n"; 
        std::string ldelim = "/";
        std::string lpath = count_dir_str + ldelim + lfile_str;

        std::string line;
        std::ifstream lfile(lpath);
        if (lfile.is_open()) {
            // Ignore the first line as it is the header
            std::getline (lfile, line);
            while (std::getline (lfile, line)) {
                std::string key_str = std::to_string(lane) + "\t" + line;
                std::string mapped_str = get_mapped_str(key_str);
                summary_file << mapped_str << "\n";
            }
        }
        lfile.close();
        std::cout << "Finished reading " << lpath << "\n";
        
    }
  
    summary_file.close();
   
}

void combine_lanes::print_help() {
    std::cout << desc << "\n";
    std::cout << "Usage: combine_lanes --i5_plates <i5 plates> --i7_plates <i7 plates> "
        "--strains_map <strains map> --count_dir <count dir> --summary_file <summary_file>\n\n";
}

bool combine_lanes::parse_args(int argc, char* argv[]) {
    bool all_set = true;

    desc.add_options()
        ("help,h", "produce help message")
        ("i5_plates", po::value<std::string>(&i5_plates_str), "index5 plates file")    
        ("i7_plates", po::value<std::string>(&i7_plates_str), "index7 plates file")    
        ("plate_bcs", po::value<std::string>(&plate_bcs_str), "Plate barcodes file")
        ("comp_map", po::value<std::string>(&comp_map_str), "Compounds map file")
        ("strains_map", po::value<std::string>(&strains_map_str), "strains map file") 
        ("count_dir", po::value<std::string>(&count_dir_str), "directory for count files") 
        ("summary_file", po::value<std::string>(&summary_file_str), "summary_file")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);


    if (vm.count("help")) {
        //all_set = false;
        return 0;
    } else {
        //all_set = false;
    }

    if (vm.count("i5_plates")) {
        std::cout << "i5 plates file is set to: " << i5_plates_str << ".\n";
    } else {
        all_set = false;
        std::cout << "Error: i5 plates file is not set.\n";
    }
  
     if (vm.count("i7_plates")) {
        std::cout << "i7 plates file is set to: " << i7_plates_str << ".\n";
    } else {
        all_set = false;
        std::cout << "Error: i7 plates file is not set.\n";
    }
       
    if (vm.count("strains_map")) {
        std::cout << "strains map file is set to: " << strains_map_str << ".\n";
    } else {
        all_set = false;
        std::cout << "Error: strains map file is not set.\n";
    }

    if (vm.count("count_dir")) {
        std::cout << "count dir is set to: " << count_dir_str << ".\n";
    } else {
        all_set = false;
        std::cout << "Error: count_dir file is not set.\n";
    }

    if (vm.count("summary_file")) {
        std::cout << "summary file is set to: " << summary_file_str << ".\n";
    } else {
        all_set = false;
        std::cout << "Error: summary file is not set.\n";
    }

    return all_set;

}


int main(int argc, char* argv[]) {  
 
    combine_lanes cml; 
 
     
    bool all_set = true; 
    try { 
        all_set = cml.parse_args(argc, argv);    
        std::cout << "all_set: " << all_set << "\n";
    } catch(std::exception& e) { 
        std::cerr << "error: " << e.what() << "\n"; 
        //lbs.print_help(); 
        return 1; 
 
    } catch (...) { 
        //lbs.print_help(); 
        return 0; 
    } 
 
    if (!all_set) { 
        cml.print_help(); 
        return 0; 
    } 
    cml.get_count_file_map();
    cml.load_maps();
    cml.join_lanes();
    cml.print_some_maps();
 
         
    return 0; 
} 

