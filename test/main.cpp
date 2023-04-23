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
	buildFlatFrustum(net, 9, 2);
	
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
	
	for (int i = 0; i < 10; ++i) {
		std::cout << "Iter " << i << '\n';
		if (i % 10 == 0) {
			currentSet = i % 20 == 0;
			if (currentSet == 0) {
				net.loadInput(inp0);
			} else {
				net.loadInput(inp1);
			}
		}
		
		if (currentSet == 0) {
			net.tuneWeights(outp0);
		} else {
			net.tuneWeights(outp1);
		}
		
		net.reset();
		net.run();
		std::cout << "Tuned " << currentSet <<
			"\n\tError 0: " << net.calculateError(outp0) <<
			"\n\tError 1: " << net.calculateError(outp1) << '\n';
	}
}