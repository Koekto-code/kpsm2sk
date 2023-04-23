#include <iostream>
#include <string>
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

int main()
{
	using namespace kpsm2sk;
	
	Network net;
	const std::vector<size_t> cfg{9, 7, 5, 3, 2};
	buildByConfig(net, cfg);
	
	// while (true) {
	// 	std::cout << "> ";
	// 	std::string cmd;
	// 	std::cin >> cmd;
	// 	if (cmd == "w") {
	// 		setWeight(net);
	// 	} else if (cmd == "i") {
	// 		inputData(net);
	// 	} else if (cmd == "r") {		
	// 		net.reset();
	// 		net.run();
	// 		for (auto &el: net.mat.back()) {
	// 			std::cout << "s: " << el.s << '\n';
	// 		}
	// 	} else {
	// 		std::cout << "Unknown command.\n";
	// 	}
	// }
	
	std::vector<float> inp0 {
		1.f, 1.f, 1.f,
		0.f, 0.f, 0.f,
		1.f, 1.f, 1.f
	};
	std::vector<float> inp1 {
		1.f, 0.f, 1.f,
		0.f, 1.f, 0.f,
		1.f, 0.f, 1.f
	};
	
	std::vector<float> outp0 {1.f, 0.f};
	std::vector<float> outp1 {0.f, 1.f};
	
	int currentSet = 0;
	
	tuneResult stat;
	
	for (int i = 0; i < 1000000; ++i) {
		if (i % 1000 == 0) {
			currentSet = i % 2000 == 0;
			if (currentSet == 0) {
				net.loadInput(inp0);
			} else {
				net.loadInput(inp1);
			}
		}
		
		tuneResult res;
		if (currentSet == 0) {
			res = net.tuneWeights(outp0, 1.f);
		} else {
			res = net.tuneWeights(outp1, 1.f);
		}
		stat.fails += res.fails;
		stat.total += res.total;
		
		if (i % 100 == 0) {
			net.reset();
			net.run();
			std::cout << i << ": tuned " << currentSet << "; " <<
				net.calculateError(currentSet ? outp1 : outp0) <<
				"; " << stat.fails << " | " << stat.total << '\n';
			stat.fails = 0;
			stat.total = 0;
		}
	}
}