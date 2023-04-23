#ifndef KPSM2SK_HPP
#define KPSM2SK_HPP

#include <cstddef>
#include <vector>
#include <cassert>

namespace kpsm2sk
{
	struct NodeAddr
	{
		size_t layer;
		size_t node;
	};
	
	struct Connection
	{
		float k;
		NodeAddr addr;
	};
	
	struct Node
	{
		float s;
		std::vector<Connection> links;
	};
	
	class Network
	{
	public:
		std::vector<std::vector<Node>> mat;
		
		inline Node &operator [](NodeAddr i) {
			return mat[i.layer][i.node];
		}
		inline const Node &operator [](NodeAddr i) const {
			return mat[i.layer][i.node];
		}
			
		inline void reset() {
			for (size_t i = 1; i < mat.size(); ++i) {
				for (auto &node: mat[i]) {
					node.s = 1.f;
				}
			}
		}
		inline void run(size_t nLayer) {
			for (auto &node: mat[nLayer]) {
				for (auto &lnk: node.links) {
					(*this)[lnk.addr].s *= lnk.k + node.s - 2.f * (node.s * lnk.k);
				}
			}
		}
		inline void run() {
			for (size_t i = 0; i + 1 < mat.size(); ++i) {
				run(i);
			}		
		}
		inline float calculateError(const std::vector<float> &dout) {
			float err = 0.f;
			for (size_t n = 0; n != dout.size(); ++n) {
				float diff = mat.back()[n].s - dout[n];
				// @todo compare with abs(diff) in learning efficiency
				err += diff * diff;
			}
			return err;
		}
		inline float calculateTuningInfluence(size_t layer) {
			float infl = 1.f;
			for (size_t i = 0; i <= layer; ++i) {
				infl *= mat[i].size();
			}
			return 1.f / infl;
		}
		inline void loadInput(const std::vector<float> &input) {
			for (size_t i = 0; i != input.size(); ++i) {
				mat[0][i].s = input[i];
			}
		}
		// Learning algorithm
		inline size_t tuneWeights(const std::vector<float> &dout, float learnMul = 0.5f)
		{
			float currentErr = calculateError(dout);
			size_t fails = 0;
			
			for (size_t i = mat.size() - 1; i != 0; --i)
			{
				float infl = calculateTuningInfluence(i) * learnMul;
				for (auto &node: mat[i])
				{
					for (auto &lnk: node.links)
					{
						float prevWeight = lnk.k;
						float err;
						
						lnk.k = prevWeight + infl;
						reset();
						run();
						err = calculateError(dout);
						if (err < currentErr)
							continue;
						
						lnk.k = prevWeight - infl;
						reset();
						run();
						err = calculateError(dout);
						if (err < currentErr)
							continue;
						
						++fails;
						lnk.k = prevWeight;
					}
				}
			}
			return fails;
		}	
	};
	
	inline void buildFlatFrustum(Network &net, size_t input, size_t output)
	{
		assert(input > output);
		assert(output > 0);
		
		// number of "thinking" layers, excluding output layer
		size_t layers = input - output;
		net.mat.resize(layers + 1);
		
		for (size_t nLayer = 0; nLayer <= layers; ++nLayer)
		{
			const size_t nlNodes = input - nLayer;
			net.mat[nLayer].resize(nlNodes);
			
			for (auto &node: net.mat[nLayer])
			{
				// insert links to all nodes in next layer
				for (size_t i = 0; i + 1 < nlNodes; ++i)
				{
					node.links.push_back(Connection {0.25f, NodeAddr {nLayer + 1, i}});
				}
			}
		}
	}
}

#endif
