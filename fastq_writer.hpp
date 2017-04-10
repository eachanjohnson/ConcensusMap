#ifndef _FASTQ_WRITER_HPP
#define _FASTQ_WRITER_HPP
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <iostream>
#include <fstream>
namespace bio = boost::iostreams;


class fastq_writer {
    public:
    fastq_writer(std::string& outfile_str, 
        std::ios_base::openmode mode = std::ios_base::out|std::ios_base::trunc|std::ios_base::binary) {

        this -> outfile_str = outfile_str;
        outfile = std::ofstream(outfile_str, mode);

        if (has_suffix(outfile_str, ".gz")) {
            out.push(bio::gzip_compressor());
        } 
        out.push(outfile);
    }

    bool putline(std::string& line) {
        out << line << "\n";
        return true;
    }


    bool has_suffix(const std::string &str, const std::string &suffix) {
        return str.size() >= suffix.size() &&
            str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
    }

    ~fastq_writer() {
        out.pop();
    }

    private:
    std::string outfile_str;
    bio::filtering_ostream out;
    std::ofstream outfile;


};
#endif
