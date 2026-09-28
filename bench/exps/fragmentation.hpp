#pragma once 
#include <iostream>
#include <chrono>
#include <fstream>
#include <functional>
#include "../../include/alloc.hpp"

class FragmentationExperiment {
    int request_sizes [1] = {-1}; // -1 means random size between 16 and 4096
    size_t num_requests = 100; // number of allocation requests of each size
    std::vector<void*> allocated_ptrs;
    std::string output_dir = "bench/data/";
    std::string exp_name = "fragmentation";
    
    public:
    void run(std::string subject, std::ofstream& csv_file, std::function<void*(size_t)> alloc, std::function<void(void*)> free_fn) {     
        for (size_t i=0; i<num_requests; i++){
            int choice = rand() % 2; // Randomly choose between allocation and deallocation
            if(allocated_ptrs.size() == 0 || choice == 0) { // Allocate
                std::cout << "allocating..." << std::endl;
                size_t size = rand() % (4096 - 16 + 1) + 16; // random size between 16 and 4096
                allocated_ptrs.push_back(alloc(size)); 
            } else { // Deallocate randomly chosen pointer from allocated_ptrs
                std::cout << "deallocating..." << std::endl;
                int index = rand() % allocated_ptrs.size();
                void* ptr = allocated_ptrs[index];
                allocated_ptrs.erase(allocated_ptrs.begin() + index);
                free_fn(ptr);
            }
            Stats stats = get_stats();
            // Write to CSV
            csv_file << subject << "," << stats.total_mem << "," << stats.n_blocks_allocated << "," << stats.mem_size_allocated << "," << stats.mem_size_free << "," << stats.largest_free_block << std::endl;
        }
        for(auto ptr: allocated_ptrs){
            free_fn(ptr);
        }

    }

    void run_experiment() {
        std::ofstream csv_file;
        csv_file.open(output_dir + exp_name + ".csv");
        
        // Write CSV header
        csv_file << "subject,total_mem,n_blocks_allocated,mem_size_allocated,mem_size_free,largest_free_block" << std::endl;
        
        run("malloc", csv_file, malloc, free);
        run("first_fit", csv_file, mem_alloc, mem_free);
        run("best_fit", csv_file, mem_alloc_best_fit, mem_free);
        run("next_fit", csv_file, mem_alloc_next_fit, mem_free);


        csv_file.close();
    }
    
};