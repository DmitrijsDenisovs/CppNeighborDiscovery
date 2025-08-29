#pragma once
#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <string>
#include <functional>
#include <atomic>

namespace Processes {
    //singleton class to handle process daemonization and running
    class Process {
    private:
        const bool cyclic;
        unsigned int iterationPeriodS;
        std::string logPath;
        std::function<void()> processIterationFunction;

        Process(bool cyclic, unsigned int iterationPeriodS, const std::string& logPath, const std::function<void()>& processIterationFunction) 
            : cyclic{cyclic}, iterationPeriodS{iterationPeriodS}, logPath{logPath}, processIterationFunction{processIterationFunction} {}

    public:
        Process(const Process&) = delete;
        Process& operator=(const Process&) = delete;

        static Process& create(bool cyclic, unsigned int iterationPeriodS, const std::string& logPath, const std::function<void()>& processIterationFunction) {
            static Process instance(cyclic, iterationPeriodS, logPath, processIterationFunction);
            return instance;
        }

        //daemonizes process, redirects std::cout to file specified in log Path
        bool daemonize();
        //runs the passed function every passed period
        void run();
    };
}

#endif