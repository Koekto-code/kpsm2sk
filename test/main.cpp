#include <iostream>
#include <string>
#include <cstdlib>
#include <utility>
#include <kpsm2sk.hpp>

std::mt19937 g_rgen;

float genFloat() {
	return (g_rgen() % 32768) * (1.f / 32768);
}

int main(int argc, char **argv)
{
	using namespace kpsm2sk;
	
	float wL = 0.5f;
	if (argc > 1) {
		wL = atof(argv[1]);
		if (!(wL >= 0.f && wL < 1.0e10f)) {
			std::cerr << "Insufficient learning modifier\n";
			return 1;
		}
	}
	
	float wW = 0.25f;
	if (argc > 2) {
		wW = atof(argv[2]);
		if (!(wW >= 0.f && wW <= 1.f)) {
			std::cerr << "Insufficient weight\n";
			return 1;
		}
	}
	
	float wD = 0.1f;
	if (argc > 3) {
		wD = atof(argv[3]);
		if (!(wD >= 0.f && wD <= 1.f)) {
			std::cerr << "Insufficient distribution\n";
			return 1;
		}
	}
	
	
	Network net;
	std::vector<size_t> cfg{8};
	std::cout << "> ";
	while (true) {
		int ll;
		std::cin >> ll;
		if (ll == 0) break;
		cfg.push_back(ll);
	}
	cfg.push_back(8);
	buildByConfig(net, cfg, wW, wD);
	
	// Learn the network outputting the same data
	std::vector<tuneSet> tset;
	for (int i = 0; i != 10; ++i)
	{
		std::vector<float> input(8);
		for (float &el: input)
			el = genFloat();
		std::vector<float> output(input);
		tset.push_back(tuneSet {std::move(input), std::move(output)});
	}
	
	tuneResult stat;
	
	for (int i = 0; i < 1000000; ++i)
	{
		tuneResult res;
		
		// res = net.tuneCoef(tset, wL);
		// stat.fails += res.fails;
		// stat.total += res.total;
		res = net.tuneWeights(tset, wL);
		stat.fails += res.fails;
		stat.total += res.total;
		
		if (1) {
			std::cout << i << ": " << net.calculateError(tset) <<
				"; " << stat.fails << " | " << stat.total << '\n';
			stat.fails = 0;
			stat.total = 0;
		}
	}
}