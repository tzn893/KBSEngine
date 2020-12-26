#pragma once
//#include <d3d12.h>
#include "d3dcommon.h"
#include <vector>
#include <wrl.h>

namespace Game {

	#define ROOT_PARAM_TYPE_INVAILD (D3D12_ROOT_PARAMETER_TYPE)-1

	class RootSignatureParam {
		friend class RootSignature;
	public:


		RootSignatureParam(){
			ZeroMemory(&m_Param,sizeof(D3D12_ROOT_PARAMETER));
			m_Param.ParameterType = ROOT_PARAM_TYPE_INVAILD;
		}

		bool IsVaild() { return m_Param.ParameterType != ROOT_PARAM_TYPE_INVAILD; }

		~RootSignatureParam() { Reset(); }

		void Reset() {
			if (m_Param.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE) {
				delete m_Param.DescriptorTable.pDescriptorRanges;
			}
			m_Param.ParameterType = ROOT_PARAM_TYPE_INVAILD;
		}

		void initAsConstants(size_t Slot,size_t Space,UINT Value) {
			m_Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
			m_Param.Constants.RegisterSpace = Space;
			m_Param.Constants.Num32BitValues = Value;
			m_Param.Constants.ShaderRegister = Slot;
		}

		void initAsConstantBuffer(size_t Slot,size_t Space,
			D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL) {
			m_Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
			m_Param.Descriptor.RegisterSpace = Space;
			m_Param.Descriptor.ShaderRegister = Slot;
			m_Param.ShaderVisibility = Visibility;
		}

		void initAsShaderResource(size_t Slot,size_t Space,
			D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL) {
			m_Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
			m_Param.Descriptor.RegisterSpace = Space;
			m_Param.Descriptor.ShaderRegister = Slot;
			m_Param.ShaderVisibility = Visibility;
		}

		void initAsUnorderedAccess(size_t Slot,size_t Space,
			D3D12_SHADER_VISIBILITY Visibilty = D3D12_SHADER_VISIBILITY_ALL) {
			m_Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
			m_Param.Descriptor.RegisterSpace = Space;
			m_Param.Descriptor.ShaderRegister = Slot;
			m_Param.ShaderVisibility = Visibilty;
		}

		//在绝大多数情况下一般都是直接使用Table下面一个Range,不会使用多个Range
		void initAsDescriptorTable(size_t Slot,size_t Space,size_t DescriptorNum,
			D3D12_DESCRIPTOR_RANGE_TYPE Type,
			D3D12_SHADER_VISIBILITY Visibility = D3D12_SHADER_VISIBILITY_ALL) {
			m_Param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
			m_Param.ShaderVisibility = Visibility;
			m_Param.DescriptorTable.NumDescriptorRanges = 1;
			
			D3D12_DESCRIPTOR_RANGE* range = new  D3D12_DESCRIPTOR_RANGE();
			range->BaseShaderRegister = Slot;
			range->NumDescriptors = DescriptorNum;
			range->OffsetInDescriptorsFromTableStart = 0;
			range->RangeType = Type;

			m_Param.DescriptorTable.pDescriptorRanges = range;
		}

		D3D12_ROOT_PARAMETER_TYPE GetParameterType() const {
			return m_Param.ParameterType;
		}
	private:
		D3D12_ROOT_PARAMETER m_Param;
	};


	class RootSignature {
		friend class DynamicDescriptorHeap;
	public:
		RootSignature(){	
			ResetRoot();
			m_ParameterNum = 0;
			m_SamplerSize = 0;
		}

		RootSignature(size_t paramNum, size_t samplerNum):
		m_Params(paramNum),m_Samplers(samplerNum){
			ResetRoot();
			m_ParameterNum = paramNum;
			m_SamplerSize = samplerNum;
		}

		void Reset(size_t paramNum,size_t samplerNum) {
			ResetRoot();

			m_Params.resize(paramNum),m_Samplers.resize(samplerNum);
			m_ParameterNum = paramNum;
			m_SamplerSize = samplerNum;
		}

		void InitializeSampler(size_t RegisterSlot,
			D3D12_STATIC_SAMPLER_DESC samplerDesc,
			D3D12_SHADER_VISIBILITY Visibilty = D3D12_SHADER_VISIBILITY_ALL);

		bool EndEditingAndCreate(
			ID3D12Device* device,
			D3D12_ROOT_SIGNATURE_FLAGS flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		);

		bool RootIsCreated() { return IsCreated; }

		RootSignatureParam& operator[](size_t index) {
			return m_Params[index];
		}

		ID3D12RootSignature* GetRootSignature() {
			return m_RootSignature.Get();
		}
	private:
		inline void ResetRoot() {
			m_RootSignature = nullptr;
			memset(m_DescriptorTableSize, -1, sizeof(m_DescriptorTableSize));
			IsCreated = false;
			m_InitializeSampler = 0;
		}


		ComPtr<ID3D12RootSignature> m_RootSignature;
		std::vector<RootSignatureParam> m_Params;
		std::vector<D3D12_STATIC_SAMPLER_DESC> m_Samplers;
		uint32_t m_DescriptorTableBitMap;
		size_t m_DescriptorTableSize[32];
		size_t m_ParameterNum;
		size_t m_InitializeSampler;
		size_t m_SamplerSize;
		bool IsCreated;
	};
}