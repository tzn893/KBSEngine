#include <Windows.h>
#include "d3d12context.h"
#include "console.h"
#include <string>

HINSTANCE hinstance;
HWND winHandle;
int winWidth = 10, winHeight = 10;
bool quit = false;

D3D12Context d3d;

std::wstring ChooseFilePath(const wchar_t* filter);

LRESULT CALLBACK WinProc(HWND handle, UINT Msg,
	WPARAM wParam, LPARAM lParam);

bool D3DInitialize(D3D12Context* context);
bool D3DUpdate(D3D12Context* context);

std::wstring filepath,outputpath;
size_t outputTexSize2Pow = 10;
size_t outputTexSize = 1 << outputTexSize2Pow;
size_t outputMipNum = 5;

int main() {
	


	WNDCLASSEX wc;

	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	hinstance = GetModuleHandle(NULL);

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.hInstance = hinstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = "WindowClass";
	wc.lpfnWndProc = WinProc;

	RegisterClassEx(&wc);

	winHandle = CreateWindowEx(0,
		"WindowClass", "A",
		WS_OVERLAPPEDWINDOW,
		0, 0, winWidth, winHeight,
		NULL, NULL, hinstance, NULL);

	if (winHandle == NULL) {
		gConsole.Log("fail to initialize window").LineSwitch();
		return false;
	}

	//ShowWindow(winHandle, SW_SHOWDEFAULT);
	UpdateWindow(winHandle);


	RECT rect;
	::GetClientRect(winHandle, &rect);

	winHeight = rect.bottom - rect.top;
	winWidth = rect.right - rect.left;

	if (!d3d.Initialize(D3DInitialize,winHandle)) {
		gConsole.Log("fail to initialize d3d").LineSwitch();
		return false;
	}

	gConsole.FlushLog();

	filepath = ChooseFilePath(L"hdr\0*.HDR\0");

	
	if (filepath.empty()) {
		gConsole.WLog(L"fail to get the file name");
		return false;
	}
	
	//outputpath = ChooseFilePath(L"");


	gConsole.WLog(L"now loading file ", filepath, " to memory").LineSwitch();

	gConsole.FlushLog();

	MSG msg;
	while (!quit) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			quit = !d3d.Update(D3DUpdate);
		}
	}

	d3d.Finailize();

	return 0;
}


LRESULT CALLBACK WinProc(HWND handle, UINT Msg,
	WPARAM wParam, LPARAM lParam) {

	switch (Msg) {
	case WM_DESTROY:
		PostQuitMessage(0);
		quit = true;
		break;
	}

	return DefWindowProc(handle, Msg, wParam, lParam);
}


#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "../loglcore/Matrix.h"
#include "../loglcore/stb_image.h"
#include "../loglcore/stb_image_write.h"
#include "../loglcore/d3d12x.h"

#include "../loglcore/ConstantBuffer.h"

#include <filesystem>

#include <memory>

struct GenProj {
	Game::Mat4x4 mright;
	Game::Mat4x4 mleft;
	Game::Mat4x4 mup;
	Game::Mat4x4 mdown;
	Game::Mat4x4 mfront;
	Game::Mat4x4 mback;
};

std::unique_ptr<ConstantBuffer<Game::Mat4x4>> projConst;
std::unique_ptr<ConstantBuffer<GenProj>>      genProj;

std::vector<std::function<bool(D3D12Context*)>> taskQueue;

bool EndlessLoop(D3D12Context* con) {

	taskQueue.push_back(EndlessLoop);

	return true;
}


size_t hdrHandle = 0;
size_t hdrCubeRTV[] = {0,1,2,3,4,5};
size_t hdrCubeSrv = 1;
size_t irrMapRTV[] = {6,7,8,9,10,11};
size_t irrMapSrv = 2;

size_t specRTV[5][6] = { {12,13,14,15,16,17},
						 {18,19,20,21,22,23},
						 {24,25,26,27,28,29},
						 {30,31,32,33,34,35},
						 {36,37,38,39,40,41} };
size_t specSRV = 3;

struct Buffer {
	float* data;
	size_t size;
	size_t width;
	size_t height;
	size_t mipi;
	size_t arri;
};

char* convert_fimage_to_cimage(Buffer& buffer){

	char* data = reinterpret_cast<char*>(malloc(buffer.width * buffer.height * 3));

	float* p = buffer.data;

	auto castF = [](float f) {
		int res = static_cast<int>(f * 256.f) - 1;
		if (res < 0) res = 0;
		if (res > 255) res = 255;
		return static_cast<char>(res);
	};

	for (size_t i = 0; i != buffer.height;i++) {
		for (size_t j = 0; j != buffer.width; j++) {
			data[i * buffer.width * 3 + j * 3] = castF(*p);
			data[i * buffer.width * 3 + j * 3 + 1] = castF(*(p + 1));
			data[i * buffer.width * 3 + j * 3 + 2] = castF(*(p + 2));

			p += 4;
		}
	}

	return data;
}

std::vector<Buffer> irrData;
std::vector<Buffer> specData;

bool D3DInitialize(D3D12Context* context) {
	//create resources
	{
		D3D12_RESOURCE_DESC rDesc;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 6;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		rDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rDesc.Height = outputTexSize;
		rDesc.Width = outputTexSize;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc = { 1,0 };


		ComPtr<ID3D12Resource> hdrCube;
		context->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATE_COMMON, nullptr,
			IID_PPV_ARGS(&hdrCube)
		);

		context->resources["hdr_cube"] = hdrCube;

		for (size_t i = 0; i != 6; i++) {
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;

			context->device->CreateRenderTargetView(hdrCube.Get(), &rtvDesc, context->GetRtvDescriptor(hdrCubeRTV[i]));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = -1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		context->device->CreateShaderResourceView(hdrCube.Get(), &srvDesc, context->GetSrvCpuDescriptor(hdrCubeSrv));
	}

	{
		D3D12_RESOURCE_DESC rDesc;
		rDesc.DepthOrArraySize = 6;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		rDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rDesc.Height = outputTexSize;
		rDesc.Width = outputTexSize;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.MipLevels = 1;
		rDesc.SampleDesc = { 1,0 };
		rDesc.Alignment = 0;

		ComPtr<ID3D12Resource> res;
		context->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATE_RENDER_TARGET, nullptr,
			IID_PPV_ARGS(&res)
		);

		context->resources["irradiance"] = res;
		
		res->SetName(L"hdr_irradiance");

		for (size_t i = 0; i != 6; i++) {
			D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
			rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
			rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
			rtvDesc.Texture2DArray.ArraySize = 1;
			rtvDesc.Texture2DArray.FirstArraySlice = i;
			rtvDesc.Texture2DArray.MipSlice = 0;
			rtvDesc.Texture2DArray.PlaneSlice = 0;

			context->device->CreateRenderTargetView(res.Get(), &rtvDesc, context->GetRtvDescriptor(irrMapRTV[i]));
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = 1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		context->device->CreateShaderResourceView(res.Get(), &srvDesc, context->GetSrvCpuDescriptor(irrMapSrv));
	}

	{
		D3D12_RESOURCE_DESC rDesc;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 6;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		rDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		rDesc.Height = outputTexSize;
		rDesc.Width = outputTexSize;
		rDesc.MipLevels = outputMipNum;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.SampleDesc = { 1,0 };

		ComPtr<ID3D12Resource> res;

		context->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc, D3D12_RESOURCE_STATE_RENDER_TARGET,
			nullptr, IID_PPV_ARGS(&res)
		);

		res->SetName(L"hdr_specular");


		for (size_t mipi = 0; mipi != outputMipNum; mipi++) {
			for (size_t arri = 0; arri != 6; arri++) {
				D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
				rtvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
				rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
				rtvDesc.Texture2DArray.ArraySize = 1;
				rtvDesc.Texture2DArray.FirstArraySlice = arri;
				rtvDesc.Texture2DArray.MipSlice = mipi;
				rtvDesc.Texture2DArray.PlaneSlice = 0;

				context->device->CreateRenderTargetView(
					res.Get(), &rtvDesc, context->GetRtvDescriptor(specRTV[mipi][arri])
				);
			}
		}

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = -1;
		srvDesc.TextureCube.MostDetailedMip = 0;
		srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;

		context->device->CreateShaderResourceView(
			res.Get(), &srvDesc, context->GetSrvCpuDescriptor(specSRV)
		);

		context->resources["hdr_spec"] = res;
	}
	
	
	{
		
		CD3DX12_ROOT_PARAMETER params[2];

		CD3DX12_DESCRIPTOR_RANGE tex;
		tex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		params[0].InitAsDescriptorTable(1, &tex);
		params[1].InitAsConstantBufferView(0);

		if (!context->CreateRootSignature(std::vector<CD3DX12_ROOT_PARAMETER>(params, params + 2),
			{ CD3DX12_STATIC_SAMPLER_DESC(0) }, "HDR2Cube")) {
			return false;
		}

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		CD3DX12_RASTERIZER_DESC rdDesc(D3D12_DEFAULT);
		rdDesc.CullMode = D3D12_CULL_MODE_NONE;
		psoDesc.RasterizerState = rdDesc;
		CD3DX12_DEPTH_STENCIL_DESC dsDesc(D3D12_DEFAULT);
		dsDesc.DepthEnable = false;
		dsDesc.StencilEnable = false;
		psoDesc.DepthStencilState = dsDesc;

		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		psoDesc.InputLayout = {nullptr , 0};

		DXGI_FORMAT rtvFormats[6] = { DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT ,DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT,
		DXGI_FORMAT_R32G32B32A32_FLOAT,DXGI_FORMAT_R32G32B32A32_FLOAT };

		psoDesc.NodeMask = 0;
		psoDesc.NumRenderTargets = 6;
		memcpy(psoDesc.RTVFormats, rtvFormats, sizeof(rtvFormats));

		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.pRootSignature = context->rootSigs["HDR2Cube"].Get();
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		psoDesc.SampleMask = UINT_MAX;

		auto vsb = context->CompileShader(L"../shader/Util/GenerateCubeForHDRUtil.hlsl", "vs_5_0", "VS"),
			 psb = context->CompileShader(L"../shader/Util/GenerateCubeForHDRUtil.hlsl", "ps_5_0", "PS");

		psoDesc.VS = { vsb->GetBufferPointer(),vsb->GetBufferSize() };
		psoDesc.PS = { psb->GetBufferPointer(),psb->GetBufferSize() };

		if (!context->CreatePipelineState(psoDesc,"HDR2Cube")) {
			return false;
		}

		genProj = std::make_unique<ConstantBuffer<GenProj>>(context->device.Get(), 1);
		projConst = std::make_unique<ConstantBuffer<Game::Mat4x4>>(context->device.Get(),6);

		GenProj* irr = genProj->GetBufferPtr();
		irr->mright = Game::Mat4x4(
			0, 0, 0, 1,
			0, 1, 0, 0,
			-1, 0, 0, 0,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(0) = irr->mright;
		irr->mleft = Game::Mat4x4(
			0, 0, 0, -1,
			0, 1, 0, 0,
			1, 0, 0, 0,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(1) = irr->mleft;
		irr->mup = Game::Mat4x4(
			-1, 0, 0, 0,
			0, 0, 0, 1,
			0, 1, 0, 0,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(2) = irr->mup;
		irr->mdown = Game::Mat4x4(
			1, 0, 0, 0,
			0, 0, 0, -1,
			0, 1, 0, 0,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(3) = irr->mdown;
		irr->mfront = Game::Mat4x4(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, 1,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(4) = irr->mfront;
		irr->mback = Game::Mat4x4(
			-1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 0, -1,
			0, 0, 0, 0).T();
		*projConst->GetBufferPtr(5) = irr->mback;
	}

	
	taskQueue.push_back([&](D3D12Context* context) {

		FILE* file;
		auto err = _wfopen_s(&file, filepath.c_str(), L"rb");

		stbi_set_flip_vertically_on_load(true);

		int height, width;
		void* hdr_data = stbi_loadf_from_file(file, &width, &height, nullptr, 3);
		fclose(file);

		D3D12_RESOURCE_DESC desc;
		desc.Alignment = 0;
		desc.DepthOrArraySize = 1;
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;
		desc.Width = width;
		desc.Height = height;
		desc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.MipLevels = 1;
		desc.SampleDesc = { 1,0 };

		ComPtr<ID3D12Resource> res;

		context->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&desc, D3D12_RESOURCE_STATE_COMMON,
			nullptr, IID_PPV_ARGS(&res)
		);

		context->resources["hdr_tex"] = res;

		size_t copyResSize = GetRequiredIntermediateSize(res.Get(), 0, 1);

		ComPtr<ID3D12Resource> tmpRes;
		context->device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(copyResSize),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr, IID_PPV_ARGS(&tmpRes)
		);

		context->resources["hdr_tex_upload"] = tmpRes;

		D3D12_SUBRESOURCE_DATA data;
		data.pData = hdr_data;
		data.RowPitch = width * 3 * 4;
		data.SlicePitch = data.RowPitch * height;

		UpdateSubresources(context->cmdList.Get(),
			res.Get(), tmpRes.Get(), 0,
			0, 1, &data);

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = DXGI_FORMAT_R32G32B32_FLOAT;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = -1;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.PlaneSlice = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		context->device->CreateShaderResourceView(res.Get(), &srvDesc,
			context->GetSrvCpuDescriptor(hdrHandle)
		);

		free(hdr_data);

		gConsole.Log("load the hdr file ").WLog(filepath, L" successfully!").LineSwitch().FlushLog();

		return true;
	});

	taskQueue.push_back([&](D3D12Context* context) {
		
		ComPtr<ID3D12Resource> cube = context->resources["hdr_cube"], res = context->resources["hdr_tex"];

		D3D12_CPU_DESCRIPTOR_HANDLE handles[6];
		for (size_t i = 0; i != 6; i++) {
			handles[i] = context->GetRtvDescriptor(i);
		}

		std::vector<D3D12_RESOURCE_BARRIER> barrier = {
			CD3DX12_RESOURCE_BARRIER::Transition(cube.Get(),D3D12_RESOURCE_STATE_COMMON,D3D12_RESOURCE_STATE_RENDER_TARGET),
			CD3DX12_RESOURCE_BARRIER::Transition(cube.Get(),D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_COMMON)
		};

		ID3D12GraphicsCommandList* cl = context->cmdList.Get();

		cl->ResourceBarrier(1, barrier.data());

		cl->SetPipelineState(context->psos["HDR2Cube"].Get());
		cl->SetGraphicsRootSignature(context->rootSigs["HDR2Cube"].Get());

		cl->OMSetRenderTargets(6, handles, false, nullptr);


		UINT tarHeight = cube->GetDesc().Height, tarWidth = cube->GetDesc().Width;

		D3D12_RECT sissor;
		sissor.bottom = tarHeight;
		sissor.top = 0;
		sissor.left = 0;
		sissor.right = tarWidth;

		D3D12_VIEWPORT viewPort;
		viewPort.Width = tarWidth;
		viewPort.Height = tarHeight;
		viewPort.TopLeftX = 0;
		viewPort.TopLeftY = 0;
		viewPort.MinDepth = 0;
		viewPort.MaxDepth = 1.0f;

		cl->RSSetScissorRects(1, &sissor);
		cl->RSSetViewports(1, &viewPort);

		cl->SetGraphicsRootConstantBufferView(1, genProj->GetADDR());
		cl->SetGraphicsRootDescriptorTable(0, context->GetGpuDescriptor(hdrHandle));

		cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		
		cl->DrawInstanced(6, 1, 0, 0);

		cl->ResourceBarrier(1, barrier.data() + 1);

		gConsole.Log("bake the hdr to cube map successfully!").LineSwitch().FlushLog();

		return true;
	});
	

	{
		CD3DX12_ROOT_PARAMETER params[2];

		CD3DX12_DESCRIPTOR_RANGE tex;
		tex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		params[0].InitAsDescriptorTable(1, &tex);
		params[1].InitAsConstantBufferView(0);

		context->CreateRootSignature(std::vector<CD3DX12_ROOT_PARAMETER>(params, params + 2),
			{ CD3DX12_STATIC_SAMPLER_DESC(0) }, "bake_irradiance");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC pipeDesc{};

		D3D12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		dsDesc.DepthEnable = false;
		dsDesc.StencilEnable = false;
		pipeDesc.DepthStencilState = dsDesc;

		pipeDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		pipeDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		pipeDesc.InputLayout = { nullptr,0 };
		pipeDesc.NodeMask = 0;
		pipeDesc.NumRenderTargets = 1;
		pipeDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		pipeDesc.pRootSignature = context->rootSigs["bake_irradiance"].Get();

		D3D12_RASTERIZER_DESC rd = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		rd.CullMode = D3D12_CULL_MODE_NONE;

		pipeDesc.RasterizerState = rd;

		pipeDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		pipeDesc.NumRenderTargets = 1;
		pipeDesc.SampleDesc = {1,0};
		pipeDesc.SampleMask = UINT_MAX;

		ComPtr<ID3DBlob> vs = context->CompileShader(L"../shader/Util/PBRIrradianceBakingUtil.hlsl", "vs_5_0", "VS")
		,ps = context->CompileShader(L"../shader/Util/PBRIrradianceBakingUtil.hlsl", "ps_5_0", "PS");

		pipeDesc.PS = { ps->GetBufferPointer(),ps->GetBufferSize() };
		pipeDesc.VS = { vs->GetBufferPointer(),vs->GetBufferSize() };

		if (!context->CreatePipelineState(pipeDesc,"bake_irradiance")) {
			return false;
		}
	}

	size_t blockNum = 4;
	size_t rtNum = 6;

	
	for (size_t i = 0; i != rtNum; i++) {
		for (size_t j = 0;j != blockNum * blockNum; j++) {
			taskQueue.push_back([&,i,j,blockNum,rtNum](D3D12Context* context) {
				ComPtr<ID3D12Resource> target = context->resources["irradiance"],
					src = context->resources["hdr_cube"];

				context->cmdList->SetPipelineState(context->psos["bake_irradiance"].Get());
				context->cmdList->SetGraphicsRootSignature(context->rootSigs["bake_irradiance"].Get());

				context->cmdList->OMSetRenderTargets(1, &context->GetRtvDescriptor(irrMapRTV[i]), false, nullptr);

				context->cmdList->SetGraphicsRootConstantBufferView(1, projConst->GetADDR(i));
				context->cmdList->SetGraphicsRootDescriptorTable(0, context->GetGpuDescriptor(hdrCubeSrv));

				UINT tarHeight = target->GetDesc().Height, tarWidth = target->GetDesc().Width;

				size_t xj = j / blockNum, yj = j % blockNum;

				size_t tarXBlock = tarWidth / blockNum, tarYBlock = tarHeight / blockNum;

				D3D12_RECT sissor;
				sissor.bottom = xj * tarXBlock + tarXBlock;
				sissor.top = xj * tarXBlock;
				sissor.left = yj * tarYBlock;
				sissor.right = yj * tarYBlock + tarYBlock;

				D3D12_VIEWPORT viewPort;
				viewPort.Width = tarWidth;
				viewPort.Height = tarHeight;
				viewPort.TopLeftX = 0;
				viewPort.TopLeftY = 0;
				viewPort.MinDepth = 0;
				viewPort.MaxDepth = 1.0f;

				context->cmdList->RSSetScissorRects(1, &sissor);
				context->cmdList->RSSetViewports(1, &viewPort);

				context->cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

				context->cmdList->DrawInstanced(6, 1, 0, 0);

				gConsole.ClearLog().LogProcess("baking irradiance map process:", (float)(j + i * blockNum * blockNum + 1) / (blockNum * blockNum * rtNum), "")
					.Log("(", i * blockNum * blockNum + j + 1, "/", (blockNum * blockNum * rtNum), ")");

				if (j == blockNum * blockNum - 1 && i == rtNum - 1) {
					gConsole.LineSwitch(2).FlushLog();
				}

				return true;
			});
		}
	}
	


	{
		CD3DX12_ROOT_PARAMETER params[3];

		CD3DX12_DESCRIPTOR_RANGE tex;
		tex.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

		params[0].InitAsDescriptorTable(1, &tex);
		params[1].InitAsConstantBufferView(0);
		params[2].InitAsConstants(1, 1);
			
		context->CreateRootSignature(std::vector<CD3DX12_ROOT_PARAMETER>(params, params + _countof(params)),
			{ CD3DX12_STATIC_SAMPLER_DESC(0) }, "bake_specular");

		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
		D3D12_DEPTH_STENCIL_DESC dsDesc = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		dsDesc.DepthEnable = false;
		psoDesc.DepthStencilState = dsDesc;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
		psoDesc.InputLayout = { nullptr,0 };
		psoDesc.NodeMask = 0;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.pRootSignature = context->rootSigs["bake_specular"].Get();
		
		CD3DX12_RASTERIZER_DESC rDesc(D3D12_DEFAULT);
		rDesc.CullMode = D3D12_CULL_MODE_NONE;
		
		psoDesc.RasterizerState = rDesc;
		psoDesc.SampleDesc = { 1,0 };
		psoDesc.SampleMask = UINT_MAX;
		
		ComPtr<ID3DBlob> vs = context->CompileShader(L"../shader/Util/PBRPrefliteringEnvorimentUtil.hlsl", "vs_5_0", "VS"),
			ps = context->CompileShader(L"../shader/Util/PBRPrefliteringEnvorimentUtil.hlsl", "ps_5_0", "PS");
		psoDesc.VS = { vs->GetBufferPointer(),vs->GetBufferSize() };
		psoDesc.PS = { ps->GetBufferPointer(),ps->GetBufferSize() };


		if (!context->CreatePipelineState(psoDesc,"bake_specular")) {
			return false;
		}
	}


	size_t mipnum = outputMipNum;
	size_t arrnum = 6;
	blockNum = 4;

	
	for (size_t mipi = 0; mipi != mipnum;mipi++) {
		for (size_t arri = 0; arri != arrnum;arri++) {
			for (size_t bi = 0; bi != blockNum * blockNum;bi++) {
				taskQueue.push_back([&, arri, mipi, arrnum, mipnum,blockNum,bi](D3D12Context* context) {

					ID3D12GraphicsCommandList* cl = context->cmdList.Get();

					ID3D12Resource* res = context->resources["hdr_cube"].Get(),
						*target = context->resources["hdr_spec"].Get();

					cl->SetPipelineState(context->psos["bake_specular"].Get());
					cl->SetGraphicsRootSignature(context->rootSigs["bake_specular"].Get());

					cl->OMSetRenderTargets(1, &context->GetRtvDescriptor(specRTV[mipi][arri]), false, nullptr);

					cl->SetGraphicsRootConstantBufferView(1, projConst->GetADDR(arri));
					float r = (float)mipi / (float)(mipnum - 1);
					cl->SetGraphicsRoot32BitConstant(2, *reinterpret_cast<UINT*>(&r), 0);
					cl->SetGraphicsRootDescriptorTable(0, context->GetGpuDescriptor(hdrCubeSrv));

					size_t tarWidth = (target->GetDesc().Width >> mipi) / blockNum, 
						tarHeight = (target->GetDesc().Height >> mipi) / blockNum;
					size_t bx = bi % blockNum, by = bi / blockNum;

					D3D12_RECT rect;
					rect.left = tarWidth * bx;
					rect.top = tarHeight * by;
					rect.right = tarWidth * bx + tarWidth;
					rect.bottom = tarHeight* by + tarWidth;

					D3D12_VIEWPORT viewPort;
					viewPort.Width = target->GetDesc().Width >> mipi;
					viewPort.Height = target->GetDesc().Height >> mipi;
					viewPort.TopLeftX = 0;
					viewPort.TopLeftY = 0;
					viewPort.MinDepth = 0;
					viewPort.MaxDepth = 1.0f;

					cl->RSSetScissorRects(1, &rect);
					cl->RSSetViewports(1, &viewPort);

					cl->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					cl->DrawInstanced(6, 1, 0, 0);


					gConsole.ClearLog().LogProcess("baking specular map process:", (float)(bi + (arri + mipi * arrnum) * blockNum * blockNum + 1) / (blockNum * blockNum * arrnum * mipnum), "").
						Log("(", (bi + (arri + mipi * arrnum) * blockNum * blockNum + 1), "/", (blockNum * blockNum * arrnum * mipnum),")");

					if (bi == blockNum * blockNum - 1 && arri == arrnum - 1 && mipi == mipnum - 1) {
						gConsole.LineSwitch(2).FlushLog();
					}

					return true;
				});
			}
		}
	}
	
	
	{
		D3D12_RESOURCE_DESC rDesc;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 1;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.Height = 1;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.SampleDesc = { 1,0 };
		rDesc.MipLevels = 1;

		for (size_t i = 0; i != 5;i++) {

			size_t size = GetRequiredIntermediateSize(context->resources["hdr_spec"].Get(),
				i, 1);

			rDesc.Width = size;

			ComPtr<ID3D12Resource> res;
			context->device->CreateCommittedResource(
				&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_READBACK),
				D3D12_HEAP_FLAG_NONE,
				&rDesc, D3D12_RESOURCE_STATE_COPY_DEST,
				nullptr, IID_PPV_ARGS(&res)
			);

			context->resources["readback" + std::to_string(i)] = res;

			std::wstring name = L"readback" + std::to_wstring(i);
			 
			res->SetName(name.c_str());
		}
	}

	auto CaptureTexture = [&](const char* src,size_t subres,size_t mipi) {
		return [&,src,subres,mipi](D3D12Context* context) {
			ID3D12Resource* srcTex = context->resources[src].Get();
			std::string tarName = "readback" + std::to_string(mipi);
			ID3D12Resource* tarTex = context->resources[tarName].Get();

			std::vector<D3D12_RESOURCE_BARRIER> barrier = {
				CD3DX12_RESOURCE_BARRIER::Transition(srcTex,D3D12_RESOURCE_STATE_RENDER_TARGET,D3D12_RESOURCE_STATE_COPY_SOURCE),
				CD3DX12_RESOURCE_BARRIER::Transition(srcTex,D3D12_RESOURCE_STATE_COPY_SOURCE,D3D12_RESOURCE_STATE_RENDER_TARGET)
			};

			ID3D12GraphicsCommandList* cl = context->cmdList.Get();

			cl->ResourceBarrier(1, barrier.data());

			D3D12_RESOURCE_DESC srcDesc = srcTex->GetDesc();
			size_t mipnum = srcDesc.MipLevels;

			D3D12_TEXTURE_COPY_LOCATION srcLoc;
			srcLoc.SubresourceIndex = mipnum * subres + mipi;
			srcLoc.pResource = srcTex;
			srcLoc.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;

			size_t srcRowSize = 0;
			context->device->GetCopyableFootprints(&srcDesc, mipnum * subres + mipi, 1, 0, nullptr, nullptr, &srcRowSize, nullptr);

			D3D12_TEXTURE_COPY_LOCATION dstLoc{};
			dstLoc.pResource = tarTex;
			dstLoc.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			dstLoc.PlacedFootprint.Offset = 0;
			dstLoc.PlacedFootprint.Footprint.Width = srcDesc.Width >> mipi;
			dstLoc.PlacedFootprint.Footprint.Height = srcDesc.Height >> mipi;
			dstLoc.PlacedFootprint.Footprint.Format = srcDesc.Format;
			dstLoc.PlacedFootprint.Footprint.RowPitch = srcRowSize;
			dstLoc.PlacedFootprint.Footprint.Depth = 1;

			cl->CopyTextureRegion(&dstLoc, 0, 0, 0, &srcLoc, nullptr);

			cl->ResourceBarrier(1,barrier.data() + 1);

			return true;
		};
	};

	auto LoadCapturedTexture = [&](std::vector<Buffer>* output,size_t mipi,size_t arri,size_t width,size_t height) {
		return [&,output,mipi,arri,width,height](D3D12Context* context) {
			std::string resName = "readback" + std::to_string(mipi);
			ID3D12Resource* res = context->resources[resName].Get();
			
			size_t bufferSize = GetRequiredIntermediateSize(res,0,1);

			Buffer buf;
			buf.size = bufferSize;
			buf.mipi = mipi;
			buf.arri = arri;
			buf.width = width;
			buf.height = height;
			void* pbuf = malloc(bufferSize);

			void* pReadbackBuf = nullptr;

			if (FAILED(res->Map(0, nullptr, &pReadbackBuf))) {
				free(pbuf);
				gConsole.Log("fail to map readback buffer from gpu to cpu").LineSwitch();

				return false;
			}
			memcpy(pbuf, pReadbackBuf, bufferSize);

			res->Unmap(0, nullptr);

			buf.data = reinterpret_cast<float*>(pbuf);

			output->emplace_back(buf);

			return true;
		};
	};


	for (size_t i = 0; i != 6;i++) {
		taskQueue.push_back(CaptureTexture("irradiance", i, 0));
		taskQueue.push_back(LoadCapturedTexture(&irrData,0,i,outputTexSize,outputTexSize));
	}

	for (size_t mipi = 0; mipi != outputMipNum; mipi++) {
		for (size_t arri = 0; arri != 6; arri++) {
			taskQueue.push_back(CaptureTexture("hdr_spec",arri,mipi));
			taskQueue.push_back(LoadCapturedTexture(&specData,mipi,arri,
					outputTexSize >> mipi,outputTexSize >> mipi));
		}

	}

	taskQueue.push_back([&](D3D12Context* con) {

		std::sort(irrData.begin(), irrData.end(), [](const Buffer& lhs, const Buffer& rhs) {
			if (lhs.arri < rhs.arri) {
				return true;
			}
			return false;
		});

		std::sort(specData.begin(), specData.end(), [](const Buffer& lhs, const Buffer& rhs) {
			if (lhs.mipi < rhs.mipi) return true;
			else if (lhs.mipi > rhs.mipi) return false;
			else {
				if (lhs.arri < rhs.arri) return true;
				else return false;
			}
		});

		std::filesystem::path p(filepath);
		auto dir = p.parent_path();

		dir /= "pbr_bake";

		const char* filenames[] = { "right.bmp","left.bmp","up.bmp","down.bmp","front.bmp","back.bmp" };

		if (!std::filesystem::exists(dir)) {
			CreateDirectory(dir.string().c_str(), NULL);
		}

		auto irra_dir = dir / "irradiance";
		if (!std::filesystem::exists(irra_dir)) {
			CreateDirectory(irra_dir.string().c_str(), NULL);
		}

		for (size_t i = 0; i != 6; i++) {
			char* rawData = convert_fimage_to_cimage(irrData[i]);
			stbi_write_bmp((irra_dir / filenames[i]).string().c_str(), irrData[i].width, irrData[i].height, 3, rawData);
			free(rawData);

			gConsole.Log("irradiance map has been written to ", (irra_dir / filenames[i]).string()).LineSwitch();
		}

		auto spec_dir = dir / "specular";
		if (!std::filesystem::exists(spec_dir)) {
			CreateDirectory(spec_dir.string().c_str(), NULL);
		}

		size_t mipnum = specData.size() / 6;

		for (size_t mipi = 0; mipi < mipnum;mipi++) {
			
			auto spec_mip_dir = spec_dir / ("mip" + std::to_string(mipi)).c_str();
			if (!std::filesystem::exists(spec_mip_dir)) {
				CreateDirectory(spec_mip_dir.string().c_str(), NULL);
			}

			for (size_t arri = 0; arri != 6;arri ++) {
				Buffer& buf = specData[arri + mipi * 6];
				
				char* rawData = convert_fimage_to_cimage(buf);
				stbi_write_bmp((spec_mip_dir / filenames[arri]).string().c_str(),
						buf.width,buf.height,3,rawData);
				free(rawData);

				gConsole.Log("specular map has been written to ", (spec_mip_dir / filenames[arri]).string()).LineSwitch();
			}
		}

		return true;
	});


	taskQueue.push_back([](D3D12Context* context) {
		gConsole.Log("done! program exists").LineSwitch();
		return false;
	});

	return true;
}


bool D3DUpdate(D3D12Context* context) {

	if (taskQueue.empty()) return false;

	bool rv = taskQueue.front()(context);
	taskQueue.erase(taskQueue.begin());

	return rv;
}


std::wstring ChooseFilePath(const wchar_t* filter) {
	OPENFILENAMEW ofn;
	char szFile[300];

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = (LPWSTR)szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter;
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn)) {
		return ofn.lpstrFile;
	}
	else {
		return L"";
	}
}


