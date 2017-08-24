#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}


std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}


std::string add_elements(std::vector<std::string> vec1, std::vector<std::string> vec2, int pos) {

    std::string val1_str = vec1[pos];
    std::string val2_str = vec2[pos];
    unsigned int val1 = std::stoi(val1_str);
    unsigned int val2 = std::stoi(val2_str);
    unsigned int total = val1 + val2;
    std::string total_str = std::to_string(total);
    return total_str;
}

std::string get_outstring(std::vector<std::string> lvec, std::string out_delim) {
    std::string lout;
    for (int j = 0; j < lvec.size(); j++) {
        if (j == 0) {
            lout = lvec[j];
        } else {
            lout += out_delim + lvec[j];
        }
    }
    return lout;
}

void combine_files(std::string file1_str, std::string file2_str, std::string outfile_str) {
    std::ifstream file1(file1_str);
    std::ifstream file2(file2_str);
    std::ofstream outfile(outfile_str, std::ofstream::out);

    std::string line1;
    std::string line2;
    std::string head1;
    std::string head2;
    std::getline(file1, head1);
    std::getline(file2, head2);
    outfile << head1 << "\n";
    while(std::getline(file1, line1)) {
        if (!std::getline(file2, line2)) { break; }

        std::vector<std::string> parts1 =  split(line1, ',');
        std::vector<std::string> parts2 =  split(line2, ',');
        std::vector<std::string> final_vec;

        for (size_t j = 0; j < parts1.size() - 3; j++) {
            final_vec.push_back(parts1[j]);
        }

        int exact_pos = parts1.size() -3;
        std::string exact_str = add_elements(parts1, parts2, exact_pos);
        final_vec.push_back(exact_str);

        int nonexact_pos = parts1.size() -2;
        std::string nonexact_str = add_elements(parts1, parts2, nonexact_pos);
        final_vec.push_back(nonexact_str);

        int all_pos = parts1.size() -1;
        std::string all_str = add_elements(parts1, parts2, all_pos);
        final_vec.push_back(all_str);
    
        std::string out_str = get_outstring(final_vec, ",");
        outfile << out_str << "\n";
    }

    file1.close();
    file2.close();
    outfile.close();

}


int main(int argc, char** argv) {
   std::string file1_str(argv[1]);
   std::string file2_str(argv[2]);
   std::string outfile_str(argv[3]);
   combine_files(file1_str, file2_str, outfile_str); 
}
