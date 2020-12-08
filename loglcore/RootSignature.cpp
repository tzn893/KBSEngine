#include "RootSignature.h"

using namespace Game;
using namespace std;


void RootSignature::InitializeSampler(size_t RegisiterSlot,
	D3D12_STATIC_SAMPLER_DESC samplerDesc,
	D3D12_SHADER_VISIBILITY Visibility) {
	
	D3D12_STATIC_SAMPLER_DESC& desc = m_Samplers[m_InitializeSampler++];
	desc = samplerDesc;
	desc.ShaderRegister = RegisiterSlot;
	desc.RegisterSpace = 0;
	desc.ShaderVisibility = Visibility;
}

bool RootSignature::EndEditingAndCreate(
	ID3D12Device* device,D3D12_ROOT_SIGNATURE_FLAGS flag) {
	if (IsCreated) return true;

	D3D12_ROOT_SIGNATURE_DESC desc;
	desc.pStaticSamplers = m_Samplers.data();
	desc.NumStaticSamplers = m_InitializeSampler;
	desc.NumParameters = m_ParameterNum;
	desc.pParameters = reinterpret_cast<D3D12_ROOT_PARAMETER*>(m_Params.data());
	desc.Flags = flag;

	m_DescriptorTableBitMap = 0;

	//纪录每个DescriptorHeap的所在位置以及其大小
	for (int ParamIndex = 0; ParamIndex != m_ParameterNum; ParamIndex++) {
		const RootSignatureParam& param = m_Params[ParamIndex];
		if (param.GetParameterType() == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
			m_DescriptorTableSize[ParamIndex] = param.m_Param.DescriptorTable.pDescriptorRanges->NumDescriptors;
			m_DescriptorTableBitMap |= 1 << ParamIndex;
		}
	}

	ComPtr<ID3DBlob> byteCode, errorCode;
	
	HRESULT hr = D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		&byteCode, &errorCode);

	if (FAILED(hr)) {
		printf("Fail to serialize root signature\n");
		return false;
	}

	device->CreateRootSignature(0, byteCode->GetBufferPointer(),
		byteCode->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));

	IsCreated = m_RootSignature != nullptr;
	return m_RootSignature != nullptr;
}