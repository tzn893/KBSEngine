#include "TransitionBatch.h"
#include "graphic.h"

TransitionBatch TransitionBatch::Begin() {
	TransitionBatch batch;
	if (gGraphic.state != Graphic::BEGIN_COMMAND_RECORDING) {
		batch.valid = false;
	}
	batch.valid = true;

	return batch;
}

void TransitionBatch::Transition(ID3D12Resource* Resource, D3D12_RESOURCE_STATES initState,
	D3D12_RESOURCE_STATES finalState) {
	if (!valid) return;
	res.push_back(Resource);
	initStates.push_back(initState);
	finalStates.push_back(finalState);
}

void TransitionBatch::End() {
	if (valid && !res.empty() && gGraphic.state == Graphic::BEGIN_COMMAND_RECORDING) {
		gGraphic.ResourceTransition(res.data(),
			initStates.data(), finalStates.data(),
			res.size());
	}
	res.clear();
	initStates.clear();
	finalStates.clear();
	valid = false;
}

