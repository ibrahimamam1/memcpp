#include "exps/alloc_latency.hpp"
#include "exps/dealloc_latency.hpp"
#include "exps/alloc_latency_succesive.hpp"
#include "exps/fragmentation.hpp"
#include <string>

int main(int argc, char* argv[]) {
    std::string exp = (argc > 1) ? argv[1] : "all";

    if (exp == "all" || exp == "alloc") {
        AllocLatencyExperiment experiment1;
        experiment1.run_experiment();
    }
    if (exp == "all" || exp == "dealloc") {
        FreeLatencyExperiment experiment2;
        experiment2.run_experiment();
    }
    if (exp == "all" || exp == "successive") {
        AllocLatencySuccesiveExperiment experiment3;
        experiment3.run_experiment();
    }
    if (exp == "all" || exp == "frag" || exp == "fragmentation") {
        FragmentationExperiment experiment4;
        experiment4.run_experiment();
    }

    std::cout << "Completed Experiments\n";
    return 0;
}