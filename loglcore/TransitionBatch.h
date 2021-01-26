#pragma once
#include "d3dcommon.h"

class TransitionBatch {
public:
	static TransitionBatch Begin();
	void   End();

	void   Transition(ID3D12Resource* Resource,D3D12_RESOURCE_STATES initState,
		D3D12_RESOURCE_STATES finalState);

	TransitionBatch() { valid = false; }

	~TransitionBatch() { End(); }
private:
	std::vector<D3D12_RESOURCE_STATES> initStates;
	std::vector<ID3D12Resource*> res;
	std::vector<D3D12_RESOURCE_STATES> finalStates;

	bool valid;
};