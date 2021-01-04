#pragma once

#include "RootSignature.h"
#include <vector>

namespace Game {
	class GraphicPSO {
		template<typename T>
		using ComPtr = Microsoft::WRL::ComPtr<T>;
	public:
		GraphicPSO() {
			Reset();
		}
		GraphicPSO& operator=(GraphicPSO& pso) {
			memcpy(&m_PSODesc, &pso.m_PSODesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
			m_InputLayouts = pso.m_InputLayouts;
		}
		
		void Reset() { 
			m_Pso = nullptr; 
			memset(&m_PSODesc,0,sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC)); 
		}

		void SetBlendState(D3D12_BLEND_DESC desc) {	m_PSODesc.BlendState = desc; }
		void SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC desc) { m_PSODesc.DepthStencilState = desc; }
		void SetDepthStencilViewFomat(DXGI_FORMAT format) { m_PSODesc.DSVFormat = format; }
		void SetFlag(D3D12_PIPELINE_STATE_FLAGS flag) { m_PSODesc.Flags = flag; }
		void SetNodeMask(UINT nodeMask) { m_PSODesc.NodeMask = nodeMask; }
		void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE top) { m_PSODesc.PrimitiveTopologyType = top; }
		void SetRenderTargetFormat(DXGI_FORMAT format) { SetRenderTargetFormat(1, &format); }
		void SetRenderTargetFormat(UINT num, const DXGI_FORMAT* formats);
		
		inline void SetRootSignature(ID3D12RootSignature* rootSig) {
			m_PSODesc.pRootSignature = rootSig;
		}
		void SetRootSignature(RootSignature* rootSig);

		void SetRasterizerState(D3D12_RASTERIZER_DESC state) { m_PSODesc.RasterizerState = state; }
		void SetSampleDesc(UINT count, UINT quality) { m_PSODesc.SampleDesc = { count,quality }; }
		void SetSampleMask(UINT mask) { m_PSODesc.SampleMask = mask; }
		
		void SetInputElementDesc(std::vector<D3D12_INPUT_ELEMENT_DESC>& desc) {
			m_InputLayouts = desc;
		}
		void PushBackInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc) {
			m_InputLayouts.push_back(desc);
		}

		void SetVertexShader(const void* bytecode, UINT size) { m_PSODesc.VS = { bytecode,size }; }
		void SetPixelShader(const void* bytecode, UINT size) { m_PSODesc.PS = { bytecode,size }; }
		void SetGeometryShader(const void* bytecode, UINT size) { m_PSODesc.GS = { bytecode,size }; }
		void SetHullingShader(const void* bytecode, UINT size) { m_PSODesc.HS = { bytecode,size }; }
		void SetDomainShader(const void* bytecode, UINT size) { m_PSODesc.DS = { bytecode,size }; }

		bool Create(ID3D12Device* device);

		//Default的PSO仍然需要初始化Shader,RootSignature等等这些东西
		void LazyBlendDepthRasterizeDefault();

		ID3D12PipelineState* GetPSO();

	private:
		D3D12_GRAPHICS_PIPELINE_STATE_DESC m_PSODesc;
		ComPtr<ID3D12PipelineState> m_Pso;
		std::vector<D3D12_INPUT_ELEMENT_DESC> m_InputLayouts;
	};

	class GraphicPSORP : public GraphicPSO{
	public:
		/*
		void SetBlendState(D3D12_BLEND_DESC desc);
		void LazyBlendDepthRasterizeDefault();
		void SetDepthStencilState(D3D12_DEPTH_STENCIL_DESC desc);
		void SetFlag(D3D12_PIPELINE_STATE_FLAGS flag);
		void SetNodeMask(UINT nodeMask);
		void SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE top);
		void SetRasterizerState(D3D12_RASTERIZER_DESC state);
		*/
		static GraphicPSORP Default() {
			Game::GraphicPSORP mPso;
			mPso.LazyBlendDepthRasterizeDefault();
			mPso.SetNodeMask(0);
			mPso.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
			mPso.SetFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
			return mPso;
		}
	private:
		void SetRootSignature(ID3D12RootSignature* rootSig) = delete;
		void SetRootSignature(RootSignature* rootSig) = delete;

		void SetSampleDesc(UINT count, UINT quality) = delete;
		void SetSampleMask(UINT mask) = delete;

		void SetInputElementDesc(std::vector<D3D12_INPUT_ELEMENT_DESC>& desc) = delete;
		void PushBackInputElementDesc(D3D12_INPUT_ELEMENT_DESC desc) = delete;
		void SetDepthStencilViewFomat(DXGI_FORMAT format) = delete;
		void SetRenderTargetFormat(DXGI_FORMAT format) = delete;
		void SetRenderTargetFormat(UINT num, const DXGI_FORMAT* formats) = delete;

		void SetVertexShader(const void* bytecode, UINT size) = delete;
		void SetPixelShader(const void* bytecode, UINT size) = delete;
		void SetGeometryShader(const void* bytecode, UINT size) = delete;
		void SetHullingShader(const void* bytecode, UINT size) = delete; 
		void SetDomainShader(const void* bytecode, UINT size) = delete;

		bool Create(ID3D12Device* device) = delete;

		ID3D12PipelineState* GetPSO() = delete;
	};

	class ComputePSO {
	public:

		void SetComputePipelineStateFlag(D3D12_PIPELINE_STATE_FLAGS flag) { cPso.Flags = flag; }
		void SetComputePipelineStateNodeMask(UINT mask) { cPso.NodeMask = mask; }
		void SetComputePipelineStateRootSignature(RootSignature* rootSig) {
			if (rootSig != nullptr && rootSig->GetRootSignature() != nullptr)
				cPso.pRootSignature = rootSig->GetRootSignature();
		}
		void SetComputePipelineStateRootSignature(ID3D12RootSignature* rootSig) {
			cPso.pRootSignature = rootSig;
		}
		void SetComputePipelineStateComputeShader(void* CSCode, size_t CSSize) {
			cPso.CS = { CSCode,CSSize };
		}

		static ComputePSO Default() {
			ComputePSO dpso;
			dpso.SetComputePipelineStateFlag(D3D12_PIPELINE_STATE_FLAG_NONE);
			dpso.SetComputePipelineStateNodeMask(0);
			return dpso;
		}

		bool Create(ID3D12Device* device) {
			HRESULT hr = device->CreateComputePipelineState(&cPso, IID_PPV_ARGS(&mPso));
			if (FAILED(hr)) {
				return false;
			}
			created = true;
			return true;
		}

		ID3D12PipelineState* GetPSO() {
			return mPso.Get();
		}

	private:
		D3D12_COMPUTE_PIPELINE_STATE_DESC cPso;
		bool created = false;
		ComPtr<ID3D12PipelineState> mPso;
	};
}