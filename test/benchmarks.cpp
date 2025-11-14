#include "../include/alignment.hpp"
#include "../include/alloc.hpp"
#include <chrono>
#include <cstddef>
#include <cstdlib> // for std::getenv
#include <filesystem>
#include <fstream>
#include <iostream>
#include <matplot/matplot.h>
#include <sstream>
#include <string>
#include <vector>

void successive_allocations_with_free() {
  size_t alloc_size = 64;
  size_t n_runs = 5000;

  std::vector<std::chrono::microseconds> memcpp_results;
  std::vector<std::chrono::microseconds> malloc_results;
  std::vector<std::chrono::microseconds> new_delete_results;

  // Helper to get home directory
  auto get_home_dir = []() -> std::string {
    const char *home = std::getenv("HOME");
    if (home)
      return std::string(home);
    return "."; // fallback
  };

  std::string documents_path = get_home_dir() + "/Documents";

  // Benchmark: Memcpp With alignment
  Alignment align = ALIGN_16;
  std::cout << "Benchmarking aligned allocations (memcpp)...\n";
  for (size_t i = 0; i < n_runs; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    void *addr = mem_alloc_align(alloc_size, align);
    auto end = std::chrono::high_resolution_clock::now();

    if (addr) {
      mem_free(addr); // Free memory
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      memcpp_results.push_back(duration);
    }
  }

  // Benchmark: malloc
  std::cout << "Benchmarking malloc...\n";
  for (size_t i = 0; i < n_runs; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    void *addr = malloc(alloc_size);
    auto end = std::chrono::high_resolution_clock::now();

    if (addr) {
      free(addr); // Free memory
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      malloc_results.push_back(duration);
    }
  }
  
  // Benchmark: new/delete
  std::cout << "Benchmarking new/delete...\n";
  for (size_t i = 0; i < n_runs; ++i) {
    auto start = std::chrono::high_resolution_clock::now();
    char *addr = new char[alloc_size]; // Fixed: allocate actual bytes
    auto end = std::chrono::high_resolution_clock::now();

    if (addr) {
      delete[] addr; // Fixed: use delete[] for array
      auto duration =
          std::chrono::duration_cast<std::chrono::microseconds>(end - start);
      new_delete_results.push_back(duration);
    }
  }
  
  // Ensure Documents directory exists
  if (!std::filesystem::exists(documents_path)) {
    std::cerr << "Warning: " << documents_path
              << " does not exist. Using current directory.\n";
    documents_path = ".";
  }

  // Extract values for plotting
  std::vector<double> allocation_numbers_memcpp, memcpp_times;
  std::vector<double> allocation_numbers_malloc, malloc_times;
  std::vector<double> allocation_numbers_new_delete, new_delete_times;

  // Memcpp results
  for (size_t i = 0; i < memcpp_results.size(); ++i) {
    allocation_numbers_memcpp.push_back(i + 1);
    memcpp_times.push_back(memcpp_results[i].count());
  }

  // Malloc results
  for (size_t i = 0; i < malloc_results.size(); ++i) {
    allocation_numbers_malloc.push_back(i + 1);
    malloc_times.push_back(malloc_results[i].count());
  }

  // New/delete results
  for (size_t i = 0; i < new_delete_results.size(); ++i) {
    allocation_numbers_new_delete.push_back(i + 1);
    new_delete_times.push_back(new_delete_results[i].count());
  }

  // Plotting with matplot
  using namespace matplot;

  // Plot for memcpp allocations
  figure();
  plot(allocation_numbers_memcpp, memcpp_times);
  title("Memcpp Allocation Times (64 bytes, ALIGN_16)");
  xlabel("Allocation Number");
  ylabel("Time (microseconds)");
  grid(on);
  std::string plot1_path = documents_path + "/memcpp_times.png";
  save(plot1_path);
  std::cout << "Memcpp plot saved to: " << plot1_path << "\n";

  // Plot for malloc allocations
  figure();
  plot(allocation_numbers_malloc, malloc_times);
  title("Malloc Allocation Times (64 bytes)");
  xlabel("Allocation Number");
  ylabel("Time (microseconds)");
  grid(on);
  std::string plot2_path = documents_path + "/malloc_times.png";
  save(plot2_path);
  std::cout << "Malloc plot saved to: " << plot2_path << "\n";

  // Plot for new/delete allocations
  figure();
  plot(allocation_numbers_new_delete, new_delete_times);
  title("New/Delete Allocation Times (64 bytes)");
  xlabel("Allocation Number");
  ylabel("Time (microseconds)");
  grid(on);
  std::string plot3_path = documents_path + "/new_delete_times.png";
  save(plot3_path);
  std::cout << "New/delete plot saved to: " << plot3_path << "\n";

  // Comparison plot (all three on same graph)
  figure();
  hold(on);
  plot(allocation_numbers_memcpp, memcpp_times)->display_name("Memcpp (aligned)");
  plot(allocation_numbers_malloc, malloc_times)->display_name("Malloc");
  plot(allocation_numbers_new_delete, new_delete_times)->display_name("New/Delete");
  hold(off);
  title("Allocation Time Comparison (64 bytes)");
  xlabel("Allocation Number");
  ylabel("Time (microseconds)");
  legend();
  grid(on);
  std::string plot4_path = documents_path + "/comparison_plot.png";
  save(plot4_path);
  std::cout << "Comparison plot saved to: " << plot4_path << "\n";
}

void successive_allocations_without_free();
void varying_alloc_sizes();
void concurrency_benchmark();

int main() { successive_allocations_with_free();}
