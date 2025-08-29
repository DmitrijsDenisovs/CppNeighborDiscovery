#pragma once
#ifndef FILEREADER_HPP
#define FILEREADER_HPP

#include "Utility/FunctionReturn.hpp"

#include <string>
#include <vector>
#include <cstdint>

using Utility::FunctionReturn;

namespace File {
    class FileReader {
    private:
        std::string path;
    public:
        FileReader() = delete;
        FileReader(const std::string path) : path{path} {}
        FunctionReturn<std::vector<std::uint8_t>> read();
    };
}

#endif