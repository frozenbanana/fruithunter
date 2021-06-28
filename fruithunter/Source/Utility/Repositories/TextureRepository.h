#pragma once
#include <iostream>
#include <WICTextureLoader.h>
#include <vector>
#include <string>
#include <WRL/client.h>
#include "ErrorLogger.h"

#include "Texture.h"

using namespace std;
using namespace DirectX;

class TextureRepository {
public:
	enum Type {
		type_texture,
		type_heightmap,
		type_particleSprite,
		type_sprites,
		NR_OF_TYPES
	};

private:
	static TextureRepository m_this;
	vector<shared_ptr<Texture>> m_repositories[NR_OF_TYPES];
	shared_ptr<Texture> m_missingTexture = make_shared<Texture>();
	const string typeAsStr[NR_OF_TYPES] = {"Texture", "Heightmap", "ParticleSprite", "Sprite"};
	const string paths[NR_OF_TYPES] = { "assets/Meshes/Textures/", "assets/TerrainHeightmap/" , "assets/ParticleSystems/Sprites/", "assets/sprites/"};

	shared_ptr<Texture> get_missingFile();
	bool add(string filename, Type type);
	shared_ptr<Texture> find(string filename, Type type);

	TextureRepository();
	~TextureRepository();

public:
	static TextureRepository* getInstance();

	static shared_ptr<Texture> get(string filename, Type type = type_texture);
};
