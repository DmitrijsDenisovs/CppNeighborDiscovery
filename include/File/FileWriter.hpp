#pragma once
#ifndef FILEWRITER_HPP
#define FILEWRITER_HPP

#include "Utility/FunctionReturn.hpp"

#include <string>
#include <vector>
#include <cstdint>

using Utility::FunctionReturn;

namespace File {
    class FileWriter {
    private:
        std::string path;
    public:
        FileWriter() = delete;
        FileWriter(const std::string path) : path{path} {}
        FunctionReturn<std::vector<std::uint8_t>> read();
    };
}

#endif