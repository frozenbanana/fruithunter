#include "TextureRepository.h"
#include "ErrorLogger.h"
#include "Renderer.h"

TextureRepository TextureRepository::m_this;

shared_ptr<TextureSet> TextureRepository::get_missingFile() { 
	if (m_missingTexture->filename == "") {
		HRESULT hr = load(paths[type_texture], "missing_texture.jpg", m_missingTexture);
		if (FAILED(hr)) {
			ErrorLogger::logError("FAILED LOADING MISSINGTEXTURE!", hr);
			return shared_ptr<TextureSet>();
		}
		else {
			ErrorLogger::log("Loaded texture: missing_texture.jpg");
			return m_missingTexture;
		}
	}
	else {
		return m_missingTexture;
	}
}

HRESULT TextureRepository::load(string path, string filename, shared_ptr<TextureSet>& set) {
	set->filename = filename;
	string filepath = path + filename;
	wstring wstr = s2ws(filepath);
	const wchar_t* fp = wstr.c_str();
	HRESULT hr = CreateWICTextureFromFile(
		Renderer::getDevice(), fp, set->resource.GetAddressOf(), set->view.GetAddressOf());
	if (!FAILED(hr)) {
		ID3D11Texture2D* tex = nullptr;
		hr = set->resource->QueryInterface(&tex);
		tex->GetDesc(&set->description);
		tex->Release();
	}
	return hr;
}

bool TextureRepository::add(string filename, Type type) {
	shared_ptr<TextureSet> set = make_shared<TextureSet>();
	HRESULT hr = load(paths[type], filename, set);
	if (FAILED(hr)) {
		ErrorLogger::logError("Failed loading " + typeAsStr[type] + "! path: " + paths[type]+filename, hr);
		return false;
	}
	else {
		m_repositories[type].push_back(set);
		ErrorLogger::log("Loaded "+typeAsStr[type]+": " + filename);
		return true;
	}
}

shared_ptr<TextureSet> TextureRepository::find(string filename, Type type) {
	// find texture set
	for (size_t i = 0; i < m_repositories[type].size(); i++) {
		if (m_repositories[type][i]->filename == filename)
			return m_repositories[type][i];
	}
	return shared_ptr<TextureSet>();
}

TextureRepository::TextureRepository() {}

TextureRepository::~TextureRepository() {}

TextureRepository* TextureRepository::getInstance() { return &m_this; }

shared_ptr<TextureSet> TextureRepository::get(string filename, Type type) {
	TextureRepository* tr = TextureRepository::getInstance();
	if (filename != "") {
		shared_ptr<TextureSet> set = tr->find(filename, type);
		if (set.get() == nullptr) {
			if (tr->add(filename, type)) {
				return tr->m_repositories[type].back();
			}
			else
				return tr->get_missingFile(); // Plan B
		}
		else
			return set;
	}
	else
		return tr->get_missingFile(); // Plan B
}
