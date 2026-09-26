#include "exps/alloc_latency.hpp"
#include "exps/dealloc_latency.hpp"
#include "exps/alloc_latency_succesive.hpp"
#include "exps/fragmentation.hpp"

int main() {
    AllocLatencyExperiment experiment1;
    FreeLatencyExperiment experiment2;
    AllocLatencySuccesiveExperiment experiment3;
    FragmentationExperiment experiment4;
    std::cout << "Running Alloc Latency Experiment..." << std::endl;
    experiment1.run_experiment();
    std::cout << "Running Free Latency Experiment..." << std::endl;
    experiment2.run_experiment();
    std::cout << "Running Alloc Latency Successive Experiment..." << std::endl;
    experiment3.run_experiment();
    std::cout << "Running Fragmentation Experiment..." << std::endl;
    experiment4.run_experiment();
    return 0;
}