#include "../loglcore/d3dcommon.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/ShaderDataStruct.h"

#include "../loglcore/RenderPass.h"
#include "../loglcore/Shader.h"
#include "../loglcore/ConstantBuffer.h"


#include "../loglcore/Complex.h"
#include "../loglcore/Texture.h"
#include "../loglcore/ComputeCommand.h"

class FFTWave;

class FFTWaveRenderPass : public RenderPass {
public:

	virtual size_t GetPriority() { return 10; }

	virtual bool   Initialize(UploadBatch* batch = nullptr) override;
	virtual void   Render(Graphic* graphic, RENDER_PASS_LAYER layer) override;

	virtual void   finalize() override;

	void Attach(FFTWave* wave) {
		this->wave = wave;
	}
	void SetWorldTransform(Game::Vector3 position,Game::Vector3 rotation,Game::Vector3 scale);

	void UpdateTime(float time) {
		currentTime = time;
	}

	void SetWaveLength(float len) {
		OceanLength = len;
	}

	void SetWaveColor(Game::Vector3 dcolor,Game::Vector3 scolor) {
		objectPass->GetBufferPtr()->DepthColor = Game::Vector4(dcolor, 1.);
		objectPass->GetBufferPtr()->SwallowColor = Game::Vector4(scolor, 1.);
	}
private:
	void UpdateWaveConstant();
	void ComputeFFT(std::unique_ptr<Texture>& tex,size_t index,ComputeCommand& cc);

	FFTWave* wave;
	Shader* fftWaveShader;

	struct WaveConstant {
		Game::Vector4 WindAndSeed;		//风和随机种子 xy为风, zw为两个随机种子

		int N;					//fft纹理大小
		float OceanLength;		//海洋长度
		float A;				//phillips谱参数，影响波浪高度
		float Time;				//时间
		int Ns;					//Ns = pow(2,m-1); m为第几阶段
		float Lambda;			//偏移影响
		float HeightScale;		//高度影响
		float BubblesScale;	    //泡沫强度
		float BubblesThreshold; //泡沫阈值
		int   Ended;
	};

	struct FFTWaveObjectPass {
		Game::Mat4x4 world;
		Game::Mat4x4 transInvWorld;
		Game::Vector4 DepthColor;
		Game::Vector4 SwallowColor;
	};
	std::unique_ptr<ConstantBuffer<FFTWaveObjectPass>> objectPass;
	std::unique_ptr<ConstantBuffer<WaveConstant>> waveConstant;

	std::unique_ptr<DescriptorHeap> mHeap;

	std::unique_ptr<Texture> mRandomMap;
	std::unique_ptr<Texture> mHeightMap, mGradientX, mGradientZ;
	std::unique_ptr<Texture> mSpareTexture;

	int NPow = 9;
	int N;
	float windScale = 20.;
	Game::Vector2 windDir = Game::Vector2(0.,1.);
	Game::Vector2 randSeed;
	float currentTime;
	float Lambda  = 0.;			
	float HeightScale = 8.;		
	float BubblesScale = 28.;	 
	float BubblesThreshold = 1.;
	float OceanLength = 512.;
	float A = 256.;
	
};

class FFTWave {
	friend class FFTWaveRenderPass;
public:
	bool Initialize(float width, float height);
	void Update(float deltaTime);

	void SetPosition(Game::Vector3 position) { this->position = position; transformUpdated = true; }
	void SetRotation(Game::Vector3 rotation) { this->rotation = rotation; transformUpdated = true;}
	void SetScale(Game::Vector3 scale) { this->scale = scale; transformUpdated = true;}

	FFTWaveRenderPass* GetRenderPass() { return mRenderPass.get(); }
	void SetColor(Game::Vector3 dcolor,Game::Vector3 scolor) { mRenderPass->SetWaveColor(dcolor,scolor); }
private:
	//static constexpr size_t rowNum = 64;
	std::unique_ptr<StaticMesh<MeshVertex>> mMesh;
	std::unique_ptr<FFTWaveRenderPass> mRenderPass;

	float currentTime;

	Game::Vector3 position;
	Game::Vector3 rotation;
	Game::Vector3 scale;
	bool transformUpdated = false;
};