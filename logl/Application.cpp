#include "Application.h"
#include "../loglcore/graphic.h"
#include "../loglcore/CopyBatch.h"
#include "../loglcore/Mesh.h"
#include "../loglcore/Shader.h"
#include "../loglcore/GeometryGenerator.h"
#include "../loglcore/ConstantBuffer.h"
#include "../loglcore/TextureManager.h"
#include "../loglcore/DescriptorAllocator.h"

#include "../loglcore/SpriteRenderPass.h"
#include "../loglcore/PhongRenderPass.h"

#include "../loglcore/ModelManager.h"

#include "../loglcore/RenderObject.h"
#include "../loglcore/LightManager.h"

#include "InputBuffer.h"
#include "Timer.h"
#include "FPSCamera.h"
#include "FFTWave.h"

#include "Player.h"
#include "Bullet.h"

struct BoxConstantBuffer {

	Game::Mat4x4 world;
	Game::Mat4x4 transInvWorld;
};

SpriteData sdata[100];
SpriteRenderPass* srp;
Texture* face;

PhongRenderPass* prp;

std::unique_ptr<RenderObject> rp,bro[10];
std::unique_ptr<StaticMesh<MeshVertexNormal>> boxMesh;
std::unique_ptr<Player> player;
std::unique_ptr<BulletRenderPass> brp;
Texture* boxDiffuse,* boxNormal;

FFTWave wave;
extern bool Quit;

std::vector<Game::Vector3> bullets;

void upload(){

	gLightManager.SetAmbientLight(Game::Vector3(.6, .6, .6));
	LightData light;
	light.fallStart = 0.;
	light.fallEnd = 20.;
	light.intensity = Game::Vector3(1.,1.,1.);
	light.direction = Game::normalize(Game::Vector3(0., 0.,1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);

}

#include "../web/WebClinet.h"
#include "logl.h"
#include "Config.h"

bool Application::initialize() {
	if (!gWebClinet.Connent(gConfig.GetValue<std::string>("ip").c_str(), gConfig.GetValue<int>("port"))) {
		MessageBeep(MB_ICONERROR);
		std::string message = "fail to connect to host ip:" + gConfig.GetValue<std::string>("ip") + " port:" + gConfig.GetValue<std::string>("port")
			+"\nYou can change the connection setting in file config.init";
		MessageBoxA(NULL, message.c_str(), "Error!", MB_OK | MB_ICONWARNING);
		return false;
	}

	upload();
	if (!wave.Initialize(512,512)) {
		return false;
	}
	srp = gGraphic.GetRenderPass<SpriteRenderPass>();
	prp = gGraphic.GetRenderPass<PhongRenderPass>();
	brp = std::make_unique<BulletRenderPass>();
	gGraphic.RegisterRenderPass(brp.get());
	
	UploadBatch up = UploadBatch::Begin();
	face = gTextureManager.loadTexture(L"../asserts/awesomeface.png", L"face",true,&up);
	face->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	Model* plane = gModelManager.loadModel("../asserts/spaceship/spaceship.obj", "plane",&up);
	rp = std::make_unique<RenderObject>(plane, Game::Vector3(0.,20.,5.),Game::Vector3(0.,0.,0.),Game::Vector3(.1,.1,.1));
	
	auto vertices = GeometryGenerator::Cube(1., 1., 1., GEOMETRY_FLAG_NONE);
	
	boxMesh = std::make_unique<StaticMesh<MeshVertexNormal>>(gGraphic.GetDevice(),
		vertices.size() / getVertexStrideByFloat<MeshVertexNormal>(),
		reinterpret_cast<MeshVertexNormal*>(vertices.data()),
		&up);

	boxDiffuse = gTextureManager.loadTexture(L"../asserts/brickwall.jpg", L"box_diffuse", true, &up);
	boxNormal = gTextureManager.loadTexture(L"../asserts/brickwall_normal.jpg", L"box_normal", true, &up);
	boxDiffuse->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());
	boxNormal->CreateShaderResourceView(gDescriptorAllocator.AllocateDescriptor());

	Mesh mesh;
	mesh.vbv = boxMesh->GetVBV();
	mesh.startIndex = 0;
	mesh.ibv = nullptr;
	mesh.indiceNum = boxMesh->GetVertexNum();

	SubMeshMaterial material;
	material.diffuse = Game::Vector3(1., 1., 1.);
	material.roughness = .9;
	material.specular = Game::Vector3(1., 1., 1.);
	material.textures[SUBMESH_MATERIAL_TYPE_BUMP] = boxNormal;
	material.textures[SUBMESH_MATERIAL_TYPE_DIFFUSE] = boxDiffuse;

	up.End();

	bro[0] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(0., 5. / 3., 10.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[1] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(32., 1. / 3., 33.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[2] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(6., 8. / 3., 17.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[3] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(-14.5, 11. / 3., -10.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[4] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(7., 5. / 3., -30.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[5] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(22., 5. / 3., -10.)* 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[6] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(33., 5. / 3., 30.)* 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[7] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(50., 5. / 3., 20.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[8] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(12.3, 5. / 3., -66.)* 3., Game::Vector3(), Game::Vector3(4., 10., 4.));
	bro[9] = std::make_unique<RenderObject>(mesh, material, Game::Vector3(-22., 5. / 3., 20.) * 3., Game::Vector3(), Game::Vector3(4., 10., 4.));

	player = std::make_unique<Player>(Game::Vector3(0., 20., 3.), Game::Vector3(.1, .1, .1),plane);

	LightData light;
	light.intensity = Game::Vector3(1., 1., 1.);
	light.direction = Game::normalize(Game::Vector3(0., -1., 1.));
	light.type = SHADER_LIGHT_TYPE_DIRECTIONAL;
	gLightManager.SetMainLightData(light);

	sdata[0].Color = Game::Vector4(1.,1.,1.,.5);
	sdata[0].Position = Game::Vector3(0., 0., .5);
	sdata[0].rotation = 0.;
	sdata[0].Scale = Game::Vector2(.3,.3);
	
	return true;
}


#include <sstream>

void splitstr(std::vector<std::string>& res, std::string& str, char dim) {
	std::istringstream iss(str);
	std::string temp;

	res.clear();
	while (std::getline(iss, temp, dim)) {
		res.push_back(temp);
	}
}

std::pair<Game::Vector3, Game::Vector3> UnpackPlayerState(std::string& cmd) {
	std::vector<std::string> data;
	splitstr(data, cmd, ',');
	Game::Vector3 Position, Rotation;

	Position[0] = (float)std::stoi(data[0]) / 10000.;
	Position[1] = (float)std::stoi(data[1]) / 10000.;
	Position[2] = (float)std::stoi(data[2]) / 10000.;
	Rotation[0] = (float)std::stoi(data[3]) / 10000.;
	Rotation[1] = (float)std::stoi(data[4]) / 10000.;
	Rotation[2] = (float)std::stoi(data[5]) / 10000.;

	return std::make_pair(Position,Rotation);
}

Game::Vector3 UnpackBulletState(std::string& cmd) {
	std::vector<std::string> data;
	splitstr(data, cmd, ',');

	Game::Vector3 Position;
	Position[0] = (float)std::stoi(data[0]) / 10000.;
	Position[1] = (float)std::stoi(data[1]) / 10000.;
	Position[2] = (float)std::stoi(data[2]) / 10000.;

	return Position;
}

std::string PackPlayerState(Game::Vector3 Position,Game::Vector3 Rotation) {
	std::string cmd = std::to_string((int)(Position[0] * 10000.)) + ",";
	cmd += std::to_string((int)(Position[1] * 10000.)) + ",";
	cmd += std::to_string((int)(Position[2] * 10000.)) + ",";
	cmd += std::to_string((int)(Rotation[0] * 10000.)) + ",";
	cmd += std::to_string((int)(Rotation[1] * 10000.)) + ",";
	cmd += std::to_string((int)(Rotation[2] * 10000.));

	return std::move(cmd);
}

constexpr float damage = 11.4514f;


void Application::update() {
	
	srp->DrawSpriteTransparent(1, sdata, face);
	rp->Render(prp);
	wave.Update(gTimer.DeltaTime());

	brp->UpdateBulletPositions(bullets.size(),bullets.data());
	bullets.clear();

	player->Update();
	
		ProtocolPost post;
		post.head = PROTOCOL_HEAD_CLINET_MESSAGE;
		post.protocolCommands.push_back({
			PROTOCOL_COMMAND_TYPE_PLAYER_POSITION,
			PackPlayerState(player->GetWorldPosition(),player->GetWorldRotation())
		});
		if (player->ShootSignal()) {
			auto[position, direction] = player->Bullet();
			post.protocolCommands.push_back(
				{
					PROTOCOL_COMMAND_TYPE_SHOOT,
					PackPlayerState(position,direction)
				}
			);
		}
		gWebClinet.Send(&post);
		gWebClinet.Receive(&post);

		for (auto cmd : post.protocolCommands) {
			if (cmd.type == PROTOCOL_COMMAND_TYPE_PLAYER_POSITION) {
				auto[position, rotation] = UnpackPlayerState(cmd.command);
				rp->SetWorldPosition(position);
				rp->SetWorldRotation(rotation);
			}
			else if (cmd.type == PROTOCOL_COMMAND_TYPE_BULLET_POSITION) {
				auto position = UnpackBulletState(cmd.command);
				bullets.push_back(position);
			}
			else if (cmd.type = PROTOCOL_COMMAND_TYPE_SHOOTED) {
				size_t count = std::stoi(cmd.command);
				player->Shooted(damage * count);
			}
		}


		if (player->GetHealth() < 0) {
			gWebClinet.Close();
			int rv = MessageBox(NULL, L"你死了，按确定复活", L"菜", MB_YESNO);
			if (rv) {
				gWebClinet.Connent(gConfig.GetValue<std::string>("ip").c_str(), gConfig.GetValue<int>("port"));
				player->SetHealth(100.f);
			}
			else {
				ApplicationQuit();
				return;
			}
		}
	for(size_t i = 0;i != 10;i++)
		bro[i]->Render(prp);
}

void Application::finalize() {
}

void Application::Quit() {
	
}