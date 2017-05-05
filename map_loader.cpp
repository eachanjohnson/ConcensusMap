#include <iostream>
#include <string>
#include <map>
#include "csv.h"


class MapLoader {
    public:
        MapLoader() = delete;
        MapLoader(std::string file_name, std::string delim): 
            file_name(file_name), delim(delim) {}

        template<typename T1, typename T2> 
        std::map<T1, T2> getMapComma(std::string head1, std::string head2) {
            T1 col1;
            T2 col2;
            io::CSVReader<2> csvr(file_name);
            std::map<T1, T2> lmap;
            csvr.read_header(io::ignore_extra_column, head1, head2);
            while(csvr.read_row(col1, col2)) {
                lmap[col1] = col2;
            }
            return lmap;
        }
        
        template<typename T1, typename T2> 
        std::map<T1, T2> getMapTab(std::string head1, std::string head2) {
            T1 col1;
            T2 col2;
            io::CSVReader<2, io::trim_chars<' '>, io::no_quote_escape<'\t'>> csvr(file_name);
            std::map<T1, T2> lmap;
            csvr.read_header(io::ignore_extra_column, head1, head2);
            while(csvr.read_row(col1, col2)) {
                lmap[col1] = col2;
            }
            return lmap;
        }

        template<typename T1, typename T2>
        std::map<T1, T2> getMap (std::string head1, std::string head2) {
            if (0 == delim.compare("comma")) {
                return getMapComma<T1, T2>(head1, head2);  
            } else if (0 == delim.compare("tab")) {
                return getMapTab<T1, T2>(head1, head2);
            }
        }
    private:
        std::string file_name;
        std::string delim;

};


int main() {
    std::string lfile = "i5_plates.csv" ;

    std::string ldelim = "comma";
    MapLoader mapl(lfile, ldelim);
    std::map<std::string, int> lmap = mapl.getMap<std::string, int>("i5_key", "i5_plate");
    for (auto val : lmap) {
        std::cout << "i5_key: " << val.first  << " i5_plate: " << val.second << "\n";
    }
}
