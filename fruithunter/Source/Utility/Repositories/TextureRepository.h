#pragma once
#include <iostream>
#include <WICTextureLoader.h>
#include <vector>
#include <string>
#include <WRL/client.h>
#include "ErrorLogger.h"

using namespace std;
using namespace DirectX;
struct TextureSet {
	string filename = "";
	Microsoft::WRL::ComPtr<ID3D11Resource> resource;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> view;
	D3D11_TEXTURE2D_DESC description;
};
class TextureRepository {
public:
	enum Type {
		type_texture,
		type_heightmap,
		NR_OF_TYPES
	};

private:
	static TextureRepository m_this;
	vector<shared_ptr<TextureSet>> m_repositories[NR_OF_TYPES];
	shared_ptr<TextureSet> m_missingTexture = make_shared<TextureSet>();
	const string typeAsStr[NR_OF_TYPES] = {"Texture", "Heightmap"};
	const string paths[NR_OF_TYPES] = { "assets/Meshes/Textures/", "assets/TerrainHeightmap/" };

	shared_ptr<TextureSet> get_missingFile();
	HRESULT load(string path, string filename, shared_ptr<TextureSet>& set);
	bool add(string filename, Type type);
	shared_ptr<TextureSet> find(string filename, Type type);

	TextureRepository();
	~TextureRepository();

public:
	static TextureRepository* getInstance();

	static shared_ptr<TextureSet> get(string filename, Type type = type_texture);
};
