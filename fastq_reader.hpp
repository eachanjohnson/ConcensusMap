#ifndef _FASTQ_READER_HPP
#define _FASTQ_READER_HPP
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <iostream>
#include <fstream>
namespace bio = boost::iostreams;


class fastq_reader {
    public:
    fastq_reader(std::string& infile_str, 
        std::ios_base::openmode mode = std::ios_base::in | std::ios_base::binary) {

        this -> infile_str = infile_str;
        file = std::ifstream(infile_str, mode);
        if (has_suffix(infile_str, ".gz")) {
            in.push(bio::gzip_decompressor());
        } 
        in.push(file);
    }

    bool getline(std::string& line) {
      if(std::getline(in, line)) {
          return true;
      } else {
          return false;
      }
    }


   bool has_suffix(const std::string &str, const std::string &suffix) {
       return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
   }

    private:
    std::string infile_str;
    bio::filtering_istream in;
    std::ifstream file;


};
#endif
