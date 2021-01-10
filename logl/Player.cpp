#include "Player.h"
#include "../loglcore/graphic.h"
#include "InputBuffer.h"
#include "Timer.h"
#include "logl.h"



extern std::vector<Game::Vector3> bullets;
Game::Vector2 pos;
void Player::Update() {
	PhongRenderPass* rp = gGraphic.GetRenderPass<PhongRenderPass>();
	ro->Render(rp);

	static int tri = 3;

	if (tri > 0) {
		mousePos = gInput.MousePosition();
		tri--;
	}
	Game::Vector2 deltaPos = gInput.MousePosition() - mousePos;
	
	camBeta += deltaPos.x * gTimer.DeltaTime() * camSpeed;
	camTheta += deltaPos.y * gTimer.DeltaTime() * camSpeed;
	camBeta = clamp(45., -45., camBeta);
	camTheta = clamp(45., -45., camTheta);

	camera.setPosition(vecForward() * -cameraDis + GetWorldPosition() + Game::Vector3(0.,.3,0.));
	camera.look(GetWorldPosition() + Game::Vector3(0., .3, 0.));

	if (gInput.KeyHold(InputBuffer::SHIFT)) {
		speedLerp = Game::fmin(speedLerp + speedAcc * gTimer.DeltaTime(), 1.);
	}
	else {
		speedLerp = Game::fmax(speedLerp - (speedFast * 4.) * gTimer.DeltaTime(),0.);
	}

	float speed = speedSlow * (1. - speedLerp) + speedFast * speedLerp;
	moveCamera.walk(speed * gTimer.DeltaTime());


	if (gInput.KeyDown(InputBuffer::ESCAPE)) {
		ApplicationQuit();
	}
	
	if (gInput.KeyHold(InputBuffer::D)) {
		moveCamera.rotateX(-rotateSpeedX * gTimer.DeltaTime());
	}
	if (gInput.KeyHold(InputBuffer::A)) {
		moveCamera.rotateX(rotateSpeedX * gTimer.DeltaTime());
	}

	float deltaY = 0.;
	if (gInput.KeyHold(InputBuffer::W)) {
		deltaY  = rotateSpeedY * gTimer.DeltaTime();
	}
	if (gInput.KeyHold(InputBuffer::S)) {
		deltaY = -rotateSpeedY * gTimer.DeltaTime();
	}

	if (deltaY != 0.) {
		if (rangeY > accY && -rangeY < accY) {
			accY += deltaY;
			moveCamera.rotateY(deltaY);
		}
	}
	else {
		float drawbackDelta = (accY * drawBackSpeed - accY) * gTimer.DeltaTime();
		moveCamera.rotateY(drawbackDelta);
		accY += drawbackDelta;
	}

	Shoot();
	UpdateTransform();

	if (recoverTimer > recoverTime) {
		health = fmin(100., health + recoverSpeed * gTimer.DeltaTime());
	}
	else {
		recoverTimer += gTimer.DeltaTime();
	}

	postEffect->SetHealthFactor(health);
}

void Player::UpdateTransform() {
	Game::Mat4x4 transform = moveCamera.GetLookAtMat();
	Game::Vector3 Position, Rotation, trash;
	Game::UnpackTransfrom(transform, Position, Rotation, trash);
	SetWorldPosition(Position);
	SetWorldRotation(Rotation);
}

Player::Player(Game::Vector3 Position,Game::Vector3 Scale,
	Model* model){

	Game::Vector2 center = Game::Vector2(GetWinWidth(), GetWinHeight()) * .5;
	gInput.LockCursor();

	ro = std::make_unique<RenderObject>(model, Position, Game::Vector3(0., 0., 0.), Scale);
	camera.attach(gGraphic.GetMainCamera());
	camera.setPosition(vecForward() * -cameraDis + Position);
	camera.look(Position);

	moveCamera.attach(GetWorldPosition(), GetWorldRotation());

	gInput.HideCursor();
	health = 100.;

	postEffect = std::make_unique<PlayerPostEffect>();
	gGraphic.RegisterRenderPass(postEffect.get());
}

Game::Vector3 Player::vecForward() {
	float theta = camTheta * PI / 180.;
	float beta = camBeta * PI / 180.;
	Game::Vector3 rv = Game::Vector3(cos(theta) * sin(beta),sin(theta),cos(theta) * cos(beta));
	Game::Mat4x4 rotation = Game::MatrixRotation(GetWorldRotation());
	rv = Game::Vector3(Game::mul(rotation, Game::Vector4(rv, 0.)));
	return rv;
}

Game::Vector3 Player::GetWorldPosition() {
	return ro->GetWorldPosition();
}
Game::Vector3 Player::GetWorldRotation() {
	return ro->GetWorldRotation();
}
Game::Vector3 Player::GetWorldScale() {
	return ro->GetWorldScale();
}

void Player::SetWorldPosition(Game::Vector3 position) {
	ro->SetWorldPosition(position);
}

void Player::SetWorldRotation(Game::Vector3 rotation) {
	ro->SetWorldRotation(rotation);
}

void Player::SetWorldScale(Game::Vector3 scale) {
	ro->SetWorldScale(scale);
}
/*
void Player::Shoot() {
	bullets.push_back(GetWorldPosition());
}
*/
void Player::Shoot() {
	if (shootTimer > shootCD) {
		if (gInput.KeyHold(InputBuffer::MOUSE_LEFT)) {
			shootTimer = 0.f;
			shootSignal = true;
		}
	}
	else{
		shootTimer += gTimer.DeltaTime();
	}
}

bool Player::ShootSignal() {
	bool rv = shootSignal;
	shootSignal = false;
	return rv;
}

std::pair<Game::Vector3, Game::Vector3> Player::Bullet() {
	Game::Vector3 Position, Direction;
	Direction = moveCamera.GetViewDir();
	Position = Direction * shootOffset + GetWorldPosition();
	return std::make_pair(Position,Direction);
}

void Player::Shooted(float damage) {
	health -= damage;
	recoverTimer = 0.f;
}

bool PlayerPostEffect::Initialize(UploadBatch* batch){
	Game::RootSignature rootSig(2, 0);
	rootSig[0].initAsDescriptorTable(0, 0, 1, D3D12_DESCRIPTOR_RANGE_TYPE_UAV);
	rootSig[1].initAsConstantBuffer(0, 0);

	if (!gGraphic.CreateRootSignature(L"player",&rootSig)) {
		OUTPUT_DEBUG_STRING("fail to create root signature for player's post effect\n");
		return false;
	}

	ComputeShader* CS = gShaderManager.loadComputeShader(L"../shader/Custom/RedPostEffect.hlsl",
		"main", L"player", L"player", nullptr);
	if (CS == nullptr) {
		OUTPUT_DEBUG_STRING("fail to create compute shader for player's post effect\n");
		return false;
	}

	Game::ComputePSO cPso = Game::ComputePSO::Default();
	if (!gGraphic.CreateComputePipelineStateObject(CS,&cPso,L"player")) {
		OUTPUT_DEBUG_STRING("fail to create compute pipeline state object for player's post effect\n");
		return false;
	}

	texture = std::make_unique<Texture>(GetWinWidth(), GetWinHeight(), TEXTURE_FORMAT_RGBA, TEXTURE_FLAG_ALLOW_UNORDERED_ACCESS,
		D3D12_RESOURCE_STATE_COPY_SOURCE);
	mHeap = std::make_unique<DescriptorHeap>(1);
	texture->CreateUnorderedAccessView(mHeap->Allocate());

	mBuffer = std::make_unique<ConstantBuffer<TextureConstantBuffer>>(gGraphic.GetDevice());
	mBuffer->GetBufferPtr()->healthFactor = 0.;
	mBuffer->GetBufferPtr()->height = (float)GetWinHeight();
	mBuffer->GetBufferPtr()->width = (float)GetWinWidth();

	return true;
}

#include "../loglcore/ComputeCommand.h"

void PlayerPostEffect::PostProcess(ID3D12Resource* res) {


	{
		ComputeCommand cc = ComputeCommand::Begin();
		cc.ResourceTrasition(texture->GetResource(), D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		cc.ResourceTrasition(res, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cc.ResourceCopy(texture->GetResource(), res);
		cc.ResourceTrasition(texture->GetResource(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		
		
		cc.BindDescriptorHeap(mHeap->GetHeap());
		cc.SetCPSOAndRootSignature(L"player", L"player");
		cc.BindConstantBuffer(1, mBuffer->GetADDR());
		cc.BindDescriptorHandle(0, texture->GetUnorderedAccessViewGPU());
		size_t Width = GetWinWidth(), Height = GetWinHeight();
		size_t x = Width / 16, y = Height / 16;
		cc.Dispatch(x, y, 1);


		cc.ResourceTrasition(texture->GetResource(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
		cc.ResourceTrasition(res, D3D12_RESOURCE_STATE_COPY_SOURCE, D3D12_RESOURCE_STATE_COPY_DEST);
		cc.ResourceCopy(res, texture->GetResource());
		cc.ResourceTrasition(res, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_RENDER_TARGET);

		cc.End();
	}
}

void PlayerPostEffect::SetHealthFactor(float health) {
	mBuffer->GetBufferPtr()->healthFactor = 1. - (health / 100.);
}

void PlayerPostEffect::finalize() {
	mBuffer.release();
	mHeap.release();
	texture.release();
}