#include <iostream>
#include <string>
#include <cstdlib>
#include <kpsm2sk.hpp>

void inputData(kpsm2sk::Network &net) {
	std::cout << net.mat[0].size() << "n: ";
	for (auto &el: net.mat[0]) {
		std::cin >> el.s;
	}
}

void setWeight(kpsm2sk::Network &net) {
	std::cout << "pos: ";
	kpsm2sk::NodeAddr addr;
	std::cin >> addr.layer >> addr.node;
	size_t lnk;
	std::cin >> lnk;
	std::cin >> net[addr].links[lnk].k;
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
	std::vector<size_t> cfg{16};
	std::cout << "> ";
	while (true) {
		int ll;
		std::cin >> ll;
		if (ll == 0) break;
		cfg.push_back(ll);
	}
	cfg.push_back(3);
		
	buildByConfig(net, cfg, wW, wD);
	
	std::vector<float> inp0 {
		1, 1, 1, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 1, 1, 1
	};
	std::vector<float> inp1 {
		0, 0, 1, 0,
		0, 1, 1, 0,
		0, 0, 1, 0,
		0, 1, 1, 1
	};
	std::vector<float> inp2 {
		0, 1, 1, 1,
		0, 0, 0, 1,
		0, 0, 1, 0,
		0, 1, 1, 1
	};
	std::vector<float> inp3 {
		0, 1, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 1, 1, 1
	};
	std::vector<float> inp4 {
		0, 1, 0, 1,
		0, 1, 0, 1,
		0, 1, 1, 1,
		0, 0, 0, 1
	};
	std::vector<float> inp5 {
		0, 1, 1, 1,
		0, 1, 1, 0,
		0, 0, 0, 1,
		0, 1, 1, 1
	};
	std::vector<float> inp6 {
		0, 1, 1, 1,
		0, 1, 0, 0,
		0, 1, 1, 1,
		0, 1, 1, 1
	};
	std::vector<float> inp7 {
		0, 1, 1, 1,
		0, 0, 0, 1,
		0, 0, 1, 0,
		0, 0, 1, 0
	};
	
	std::vector<float> outp0 {0, 0, 0};
	std::vector<float> outp1 {0, 0, 1};
	std::vector<float> outp2 {0, 1, 0};
	std::vector<float> outp3 {0, 1, 1};
	std::vector<float> outp4 {1, 0, 0};
	std::vector<float> outp5 {1, 0, 1};
	std::vector<float> outp6 {1, 1, 0};
	std::vector<float> outp7 {1, 1, 1};
	
	std::vector<tuneSet> tset;
	tset.push_back(tuneSet {inp0, outp0});
	tset.push_back(tuneSet {inp1, outp1});
	tset.push_back(tuneSet {inp2, outp2});
	tset.push_back(tuneSet {inp3, outp3});
	tset.push_back(tuneSet {inp4, outp4});
	tset.push_back(tuneSet {inp5, outp5});
	tset.push_back(tuneSet {inp6, outp6});
	tset.push_back(tuneSet {inp7, outp7});
	
	tuneResult stat;
	
	for (int i = 0; i < 1000000; ++i) {
		
		tuneResult res = net.tuneCoef(tset, wL);
		stat.fails += res.fails;
		stat.total += res.total;
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