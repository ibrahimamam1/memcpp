#include "exps/alloc_latency.hpp"
#include "exps/dealloc_latency.hpp"
#include "exps/alloc_latency_succesive.hpp"
#include "exps/fragmentation.hpp"

int main() {
    AllocLatencyExperiment experiment1;
    FreeLatencyExperiment experiment2;
    AllocLatencySuccesiveExperiment experiment3;
    FragmentationExperiment experiment4;
    experiment1.run_experiment();
    experiment2.run_experiment();
    experiment3.run_experiment();
    experiment4.run_experiment();
    std::cout << "Completed All Experiments\n";
    return 0;
}