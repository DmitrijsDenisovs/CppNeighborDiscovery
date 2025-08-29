#include "Process.hpp"

#include <unistd.h> 
#include <stdlib.h> 
#include <fcntl.h>     
#include <sys/stat.h>

#include <string>
#include <csignal>  
#include <ctime>
#include <chrono>
#include <iostream>

using Processes::Process;

bool Process::daemonize() {
    if (::daemon(0, 1) < 0) {
        return false;
    }

    if (!this->logPath.empty()) {
        int fd = ::open(this->logPath.c_str(), O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd < 0) {
            //redirect output to dev null if logPath failed
            fd = ::open("/dev/null", O_RDWR);
            if (fd < 0) {
                return false;
            }
        }

        // redirect stdin, stdout, stderr
        if (::dup2(fd, STDIN_FILENO) < 0) {
            return false;
        }
        if (::dup2(fd, STDOUT_FILENO) < 0) {
            return false;
        }
        if (::dup2(fd, STDERR_FILENO) < 0) {
            return false;
        }

        if (fd > STDERR_FILENO) {
            if (::close(fd) < 0) {
                //nonfatal for daemonization
            }
        }
    }

    return true;
}

void Process::run() {
    //exit from loop happens externally by OS (Ctr+C, kill etc.)
    if (this->processIterationFunction) {
        do {
            auto start = std::chrono::high_resolution_clock::now();

            this->processIterationFunction();

            auto end = std::chrono::high_resolution_clock::now();

            auto secondsToWait = iterationPeriodS - std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

            //not sure of std::this_thread usage
            //std::this_thread::sleep_for(std::chrono::seconds(secondsToWait);
            if (this->cyclic && (secondsToWait > 0)) {
                ::sleep(secondsToWait);
            }

        } while(this->cyclic);
    }
}