#include "TextureHolder.h"

#include<assert.h>

TextureHolder* TextureHolder::m_s_Instance = nullptr;

TextureHolder::TextureHolder() {
	assert(m_s_Instance == nullptr);
	m_s_Instance = this;
}

Texture& TextureHolder::GetTexture(string const& filename) {
	// auto is equivalent to map<string, Texture> here
	auto& m = m_s_Instance->m_Textures;
	//creating a iterator
	auto keyValuePair = m.find(filename);

	if (keyValuePair != m.end()) {
		// return texture if key is found
		return keyValuePair->second;
	}
	else {
		// if not found then add it to the texture holder
		auto& texture = m[filename];
		// texture is the reference of texture in the newly created string-texture pair
		texture.loadFromFile(filename);
		return texture;
	}
}