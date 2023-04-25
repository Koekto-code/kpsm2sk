#ifndef KPSM2SK_HPP
#define KPSM2SK_HPP

#include <cstddef>
#include <vector>
#include <cassert>
#include <iostream>
#include <random>

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
		float w;
		NodeAddr addr;
	};
	
	struct Node
	{
		float s;
		std::vector<Connection> links;
	};
	
	typedef struct {
		size_t fails;
		size_t total;
	} tuneResult;
	
	typedef struct {
		std::vector<float> input;
		std::vector<float> output;
	} tuneSet;
	
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
		inline void flow(size_t nLayer) {
			for (auto &node: mat[nLayer]) {
				for (auto &lnk: node.links) {
					float &s = (*this)[lnk.addr].s;
					s *= lnk.k + (node.s * lnk.w) - 2.f * (node.s * lnk.w * lnk.k);
				}
			}
		}
		inline void flow() {
			for (size_t i = 0; i + 1 < mat.size(); ++i) {
				flow(i);
			}
		}
		inline void run() {
			reset();
			flow();
		}		
		
		inline float calculateTuningInfluence(size_t layer) {
			// float infl = 1.f;
			// for (size_t i = 0; i <= layer; ++i) {
			// 	infl *= mat[i].size();
			// }
			// return 1.f / infl;
			return 0.07f;
		}
		
		inline void loadInput(const std::vector<float> &input) {
			for (size_t i = 0; i != input.size(); ++i) {
				mat[0][i].s = input[i];
			}
		}
		
		inline float calculateError(const std::vector<tuneSet> &tune)
		{
			float err = 0.f;
			
			for (size_t i = 0; i != tune.size(); ++i)
			{
				loadInput(tune[i].input);
				run();
				for (size_t n = 0; n != mat.back().size(); ++n)
				{
					float diff = mat.back()[n].s - tune[i].output[n];
					// @todo compare with abs(diff) in learning efficiency
					
					err += diff < 0.f ? -diff : diff;
				}
			}
			return err;
		}
		
		inline tuneResult tuneCoef(const std::vector<tuneSet> &tuneData, float learnMul)
		{
			float currentErr = calculateError(tuneData);
			size_t fails = 0;
			size_t total = 0;
			std::mt19937 rgen;
			
			for (size_t i = mat.size() - 1; i != 0; --i)
			{
				float infl = calculateTuningInfluence(i) * learnMul;
				for (auto &node: mat[i])
				{
					for (auto &lnk: node.links)
					{
						++total;
						float prevK = lnk.k;
						float err;
						
						if (infl != 0.0f)
						{
							if (lnk.k < 1.f)
							{
								lnk.k = prevK + infl;
								if (lnk.k > 1.f)
									lnk.k = 1.f;
								err = calculateError(tuneData);
								if (err < currentErr)
									continue;
							}
							
							if (lnk.k > 0.f)
							{
								lnk.k = prevK - infl;
								if (lnk.k < 0.f)
									lnk.k = 0.f;
								err = calculateError(tuneData);
								if (err < currentErr)
									continue;
							}
						}
						else
						{
							lnk.k = (rgen() % 32768) * (1.f / 32768);
							err = calculateError(tuneData);
							if (err < currentErr)
								continue;
						}
						
						++fails;
						lnk.k = prevK;
					}
				}
			}
			return {fails, total};
		}
		inline tuneResult tuneWeights(const std::vector<tuneSet> &tuneData, float learnMul)
		{
			float currentErr = calculateError(tuneData);
			size_t fails = 0;
			size_t total = 0;
			std::mt19937 rgen;
			
			for (size_t i = mat.size() - 1; i != 0; --i)
			{
				float infl = calculateTuningInfluence(i) * learnMul;
				for (auto &node: mat[i])
				{
					for (auto &lnk: node.links)
					{
						++total;
						float prevWeight = lnk.w;
						float err;
						
						if (infl != 0.0f)
						{
							if (lnk.w < 1.f)
							{
								lnk.w = prevWeight + infl;
								if (lnk.w > 1.f)
									lnk.w = 1.f;
								err = calculateError(tuneData);
								if (err < currentErr)
									continue;
							}
							
							if (lnk.w > 0.f)
							{
								lnk.w = prevWeight - infl;
								if (lnk.w < 0.f)
									lnk.w = 0.f;
								err = calculateError(tuneData);
								if (err < currentErr)
									continue;
							}
						}
						else
						{
							lnk.w = (rgen() % 32768) * (1.f / 32768);
							err = calculateError(tuneData);
							if (err < currentErr)
								continue;
						}
						
						++fails;
						lnk.w = prevWeight;
					}
				}
			}
			return {fails, total};
		}
	};
	
	inline void buildFlatFrustum(Network &net, size_t input, size_t output)
	{
		assert(input > output);
		assert(output > 0);
		
		// number of "thinking" layers, excluding output layer
		const size_t layers = input - output;
		net.mat.resize(layers + 1);
		
		for (size_t nLayer = 0; nLayer <= layers; ++nLayer)
		{
			const size_t nlNodes = input - nLayer;
			net.mat[nLayer].resize(nlNodes);
			
			// no need to create connections for output layer
			if (nLayer == layers) break;
			
			for (auto &node: net.mat[nLayer])
			{
				// insert links to all nodes in next layer
				for (size_t i = 0; i + 1 < nlNodes; ++i)
				{
					node.links.push_back(Connection {0.25f, 1.f, NodeAddr {nLayer + 1, i}});
				}
			}
		}
	}
	
	inline void buildByConfig(
		Network &net,
		std::vector<size_t> config,
		float w = 0.25f, // initial weight
		float d = 0.1f // weight distribution width
	) {
		const size_t layers = config.size();
		net.mat.resize(layers);
		std::mt19937 rgen;
		
		for (size_t nLayer = 0; nLayer < layers; ++nLayer)
		{
			net.mat[nLayer].resize(config[nLayer]);
			
			if (nLayer + 1 == layers) break;
			
			for (auto &node: net.mat[nLayer])
			{
				// insert links to all nodes in next layer
				for (size_t i = 0; i < config[nLayer + 1]; ++i)
				{
					float r = (rgen() % 32768) * (1.f / 32768);
					// at w = 0.5, d = 0.1, r is uniformly distributed 0.4f ... 0.6f
					r = w - d * 0.5f + r * d;
					node.links.push_back(Connection {r, 1.f, NodeAddr {nLayer + 1, i}});
				}
			}
		}
	}
}

#endif
