#ifndef _UTILC_HPP
#define _UTILC_HPP
#include <iostream>
#include <map>
#include <unordered_map>

class UtilC {
    public:
        template <typename T1, typename T2>
        static void print_map(std::map<T1, T2> lmap) {
            for (const auto & e : lmap) {
                auto key = e.first;
                auto val = e.second;
                std::cout << "key: " << key << " val: " << val << "\n";
            }
        }     
        template <typename T1, typename T2>
        static void print_map(std::unordered_map<T1, T2> lmap) {
            for (const auto & e : lmap) {
                auto key = e.first;
                auto val = e.second;
                std::cout << "key: " << key << " val: " << val << "\n";
            }
        }     

};

template <>
void UtilC::print_map<std::string, int>(std::map<std::string, int> lmap) {
    for (const auto & e : lmap) {
        auto key = e.first;
        auto val = e.second;
        std::cout << "key: " << key << " val: " << 
            std::to_string(val) << "\n";
    }
}

template <>
void UtilC::print_map<std::string, int>(std::unordered_map<std::string, int> lmap) {
    for (const auto & e : lmap) {
        auto key = e.first;
        auto val = e.second;
        std::cout << "key: " << key << " val: " << 
            std::to_string(val) << "\n";
    }
}
#endif
