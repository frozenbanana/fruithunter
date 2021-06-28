#include "TextureRepository.h"
#include "ErrorLogger.h"
#include "Renderer.h"

TextureRepository TextureRepository::m_this;

shared_ptr<Texture> TextureRepository::get_missingFile() {
	if (!m_missingTexture->isLoaded()) {
		bool res = m_missingTexture->load(paths[type_texture] + "missing_texture.jpg");
	}
	return m_missingTexture;
}

bool TextureRepository::add(string filename, Type type) {
	shared_ptr<Texture> set = make_shared<Texture>();
	bool res = set->load(paths[type] + filename);
	if (!res) {
		return false;
	}
	else {
		m_repositories[type].push_back(set);
		ErrorLogger::log("Loaded " + typeAsStr[type] + ": " + filename);
		return true;
	}
}

shared_ptr<Texture> TextureRepository::find(string filename, Type type) {
	// find texture set
	for (size_t i = 0; i < m_repositories[type].size(); i++) {
		if (m_repositories[type][i]->getFilename() == filename)
			return m_repositories[type][i];
	}
	return shared_ptr<Texture>();
}

TextureRepository::TextureRepository() {}

TextureRepository::~TextureRepository() {}

TextureRepository* TextureRepository::getInstance() { return &m_this; }

shared_ptr<Texture> TextureRepository::get(string filename, Type type) {
	TextureRepository* tr = TextureRepository::getInstance();
	if (filename != "") {
		shared_ptr<Texture> set = tr->find(filename, type);
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
