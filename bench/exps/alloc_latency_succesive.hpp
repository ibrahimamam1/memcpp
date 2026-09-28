#pragma once 
#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>
#include "../../include/alloc.hpp"

class AllocLatencySuccesiveExperiment {
    int request_sizes [1] = {-1}; // -1 means random size between 16 and 4096
    size_t num_requests = 1000; // number of allocation requests of each size

    std::string output_dir = "bench/data/";
    std::string exp_name = "alloc_latency_successive";
    
    public:
    void run(std::string subject, std::ofstream& csv_file, std::function<void*(size_t)> alloc, std::function<void(void*)> free_fn) {
        std::vector<void*> allocated_ptrs;
        for (size_t i=0; i<num_requests; i++){
            std::cout << "ALLOC LATENCY SUCCESSIVE EXP: " << subject << " " << i+1 << "/" << num_requests  << std::endl;
            size_t size = rand() % (4096 - 16 + 1) + 16; // random size between 16 and 4096
            const auto start_time{std::chrono::high_resolution_clock::now()};
            void* ptr = alloc(size);
            const auto end_time{std::chrono::high_resolution_clock::now()};
            std::chrono::duration<double, std::micro> latency = end_time - start_time;
            allocated_ptrs.push_back(ptr);
            // Write to CSV
            csv_file << subject << "," << size << "," << start_time.time_since_epoch().count() << "," << end_time.time_since_epoch().count() << "," << latency.count() << std::endl;
        }
        for(auto ptr : allocated_ptrs){
            free_fn(ptr);
        }

    }

    void run_experiment() {
        std::ofstream csv_file;
        csv_file.open(output_dir + exp_name + ".csv");
        
        // Write CSV header
        csv_file << "subject,request_size,start_time,end_time,latency" << std::endl;
        
        run("malloc", csv_file, malloc, free);
        run("first_fit", csv_file, mem_alloc, mem_free);
        run("best_fit", csv_file, mem_alloc_best_fit, mem_free);
        run("next_fit", csv_file, mem_alloc_next_fit, mem_free);
        
        csv_file.close();
    }
    
};