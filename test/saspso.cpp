#include <memory>
#include <chrono>

#include "SASPSO/SASPSO.hpp"
#include "TestProblems.hpp"
#include "OptimizerFactory.hpp"

using namespace type_traits;

int serial_parallel_test() {
	// Initialize problem and solver parameters
	int iter = 5000;
	int particles = 300;
	auto problem = TestProblems::create_problem<2>(TestProblems::GOMEZ_LEVY);

	std::cout << "Problem: Gomez Levy" << std::endl;
	std::cout << "Max iterations: " << iter << std::endl;
	std::cout << "Num particles: " << particles << std::endl;

	// Test the serial version
	std::cout << "--- Serial optimizer testing ---" << std::endl;
	std::unique_ptr<Optimizer<2>> opt = std::make_unique<SASPSO<2>>(problem, particles, iter, 1e-10);
	opt->initialize();
	auto t1 = std::chrono::high_resolution_clock::now();
	opt->optimize();
	auto t2 = std::chrono::high_resolution_clock::now();
	opt->print_results();
	std::cout << "Absolute error: " << std::abs(TestProblems::get_exact_value<2>(TestProblems::GOMEZ_LEVY) - opt->get_global_best_value()) << std::endl;
	std::cout << "Absolute distance: " << (TestProblems::get_exact_position<2>(TestProblems::GOMEZ_LEVY) - opt->get_global_best_position()).norm() << std::endl;
	std::cout << "Elapsed optimization time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << std::endl;

	// Yest the parallel version
	std::cout << "--- Parallel optimizer testing ---" << std::endl;
	std::unique_ptr<Optimizer<2>> opt_p = std::make_unique<SASPSO<2>>(problem, particles, iter, 1e-10);
	auto saspso_ptr = dynamic_cast<SASPSO<2>*>(opt_p.get());
    if (!saspso_ptr)
	{
		std::cerr << "Error: dynamic_cast failed" << std::endl;
		return 1;
	}
	saspso_ptr->initialize_parallel();
	t1 = std::chrono::high_resolution_clock::now();
	saspso_ptr->optimize_parallel();
	t2 = std::chrono::high_resolution_clock::now();
	saspso_ptr->print_results();
	std::cout << "Absolute error: " << std::abs(TestProblems::get_exact_value<2>(TestProblems::GOMEZ_LEVY) - opt->get_global_best_value()) << std::endl;
	std::cout << "Absolute distance: " << (TestProblems::get_exact_position<2>(TestProblems::GOMEZ_LEVY) - opt->get_global_best_position()).norm() << std::endl;
	std::cout << "Elapsed optimization time: " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << std::endl;
	return 0;
}


int main(int argc, char ** argv)
{
	// Check the number of arguments
	if (argc != 2)
	{
		std::cout << "Usage: ./test-saspso test_name" << std::endl;
		std::cout << "Available tests for SASPSO algorithm: error_iterations, serial_parallel, time_numparticles" << std::endl;
		return -1;
	}
	// Get from command line the required test
	std::string test = argv[1];
	if (test == "error_iterations")
		serial_parallel_test();
	else if (test == "serial_parallel")
		serial_parallel_test();
	else if (test == "time_numparticles")
		serial_parallel_test();
	else
	{
		std::cout << "Usage: ./test-saspso test_name" << std::endl;
		std::cout << "Available tests for SASPSO algorithm: error_iterations, serial_parallel, time_numparticles" << std::endl;
		return -1;
	}
	return 0;
}