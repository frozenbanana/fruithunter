#include "EffectSystem.h"
#include "SceneManager.h"

ShaderSet EffectSystem::m_shader;
bool EffectSystem::m_fetchedPresets = false;
vector<string> EffectSystem::m_presets;

int ES_compareParticles(const void* a, const void* b) {
	EffectSystem::ParticleBuffer *p1 = (EffectSystem::ParticleBuffer*)a,
								 *p2 = (EffectSystem::ParticleBuffer*)b;
	float3 target = SceneManager::getScene()->m_camera.getPosition();
	float p1L = (p1->position - target).LengthSquared();
	float p2L = (p2->position - target).LengthSquared();
	if (p1L > p2L)
		return -1;
	if (p1L == p2L)
		return 0;
	if (p1L < p2L)
		return 1;
}

string EffectSystem::asPath(string filename) { return PATH_PRESET + filename + "." + PRESET_END; }

void EffectSystem::fetchPresets() {
	SimpleFilesystem::readDirectory(PATH_PRESET, m_presets);
	SimpleFilesystem::filterByEnding(m_presets, PRESET_END);
	SimpleFilesystem::cutEndings(m_presets);
}

void EffectSystem::read_preset(ifstream& file) {
	m_selected = nullptr;

	size_t size = fileRead<size_t>(file);
	m_caches.resize(size);
	for (size_t i = 0; i < size; i++) {
		m_caches[i].source->read(file);
	}
}

void EffectSystem::write_preset(ofstream& file) {
	fileWrite<size_t>(file, m_caches.size());
	for (size_t i = 0; i < m_caches.size(); i++) {
		m_caches[i].source->write(file);
	}
}

void EffectSystem::update(float dt) {
	for (size_t i = 0; i < m_caches.size(); i++) {
		if (m_caches[i].source->getDescription().emitType != EmitterDescription::EmitType::Burst)
			m_caches[i].emitter.update(dt, *this); // bursting every frame will kill computer!
		m_caches[i].source->update(dt);
	}
}

size_t EffectSystem::getActiveParticleCount() const {
	size_t count = 0;
	for (size_t i = 0; i < m_caches.size(); i++) {
		count += m_caches[i].source->getActiveParticleCount();
	}
	return count;
}

void EffectSystem::draw() {
	for (size_t i = 0; i < m_caches.size(); i++) {
		m_caches[i].source->draw();
	}
}

void EffectSystem::setEmittingState(bool state) {
	for (size_t i = 0; i < m_caches.size(); i++) {
		m_caches[i].source->setEmittingState(state);
	}
}

void EffectSystem::burst() {
	for (size_t i = 0; i < m_caches.size(); i++) {
		m_caches[i].source->burst(*this);
	}
}

void EffectSystem::imgui_properties() {
	if (ImGui::BeginChild("sideWindow", ImVec2(350, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
		Transformation::imgui_properties();
		ImGui::Separator();
		ImGui::InputText("##86", &m_tempStoreString);
		ImGui::SameLine();
		if (ImGui::Button("Save Preset")) {
			storeAsPreset(m_tempStoreString);
		}
		if (ImGui::BeginCombo("Presets", m_previousPresetLoaded.c_str())) {
			for (size_t i = 0; i < m_presets.size(); i++) {
				bool selected = (m_presets[i] == m_previousPresetLoaded);
				if (ImGui::Selectable(m_presets[i].c_str(), &selected)) {
					loadFromPreset(m_presets[i]);
				}
			}
			ImGui::EndCombo();
		}
		ImGui::Separator();
		if (ImGui::Button("Burst")) {
			burst();
		}
		if (ImGui::Button("Add New Source")) {
			m_caches.push_back(SourceCache());
		}
		ImGui::SameLine();
		if (ImGui::Button("Delete Source")) {
			for (size_t i = 0; i < m_caches.size(); i++) {
				if (m_selected == m_caches[i].source.get()) {
					m_caches.erase(m_caches.begin() + i);
					m_selected = nullptr;
					break;
				}
			}
		}

		ImGuiWindowFlags windowFlags = ImGuiWindowFlags_None;
		if (ImGui::BeginChild("Cache Tree", ImVec2(0, 125), true, windowFlags)) {
			ImGuiTreeNodeFlags baseTreeFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick |
											   ImGuiTreeNodeFlags_OpenOnArrow |
											   ImGuiTreeNodeFlags_SpanAvailWidth;
			for (size_t i = 0; i < m_caches.size(); i++) {
				ParticleCache* source = m_caches[i].source.get();
				ImGuiTreeNodeFlags treeflags = baseTreeFlags;
				bool isSelected = (m_selected == source);
				if (isSelected)
					treeflags |= ImGuiTreeNodeFlags_Selected;
				if (!source->hasSubEmitters())
					treeflags |= ImGuiTreeNodeFlags_Leaf;
				string id = "Source[" + to_string(i) + "] (" +
							to_string(source->getActiveParticleCount()) + "/" +
							to_string(source->getParticleCount()) + ")";
				bool open = ImGui::TreeNodeEx((void*)source, treeflags, id.c_str());
				if (ImGui::IsItemClicked())
					m_selected = source;
				if (open) {
					source->imgui_tree(&m_selected);
					ImGui::TreePop();
				}
			}
		}
		ImGui::EndChild();

		if (m_selected != nullptr) {
			if (ImGui::BeginChild(
					"Properties", ImVec2(0, 0), true, ImGuiWindowFlags_AlwaysAutoResize)) {
				m_selected->imgui_properties();
			}
			ImGui::EndChild();
		}
	}
	ImGui::EndChild();
}

void EffectSystem::emit(size_t count) {
	for (size_t i = 0; i < m_caches.size(); i++)
		m_caches[i].source->emit(count, *this);
}

void EffectSystem::read(ifstream& file) {
	Transformation::stream_read(file);
	read_preset(file);
}

void EffectSystem::write(ofstream& file) {
	Transformation::stream_write(file);
	write_preset(file);
}

bool EffectSystem::loadFromPreset(string filename) {
	string path = asPath(filename);
	ifstream file;
	file.open(path, ios::binary);
	if (file.is_open()) {
		read_preset(file);
		file.close();
		m_previousPresetLoaded = filename;
		m_tempStoreString = filename;
		return true;
	}
	else
		ErrorLogger::logWarning("(EffectSystem::loadFromPreset) Couldnt find file: " + path);
	return false;
}

bool EffectSystem::storeAsPreset(string filename) {
	string path = asPath(filename);
	ofstream file;
	file.open(path, ios::binary);
	if (file.is_open()) {
		write_preset(file);
		file.close();
		fetchPresets(); // refetch presets
		return true;
	}
	return false;
}

bool EffectSystem::isEmitting() const {
	for (size_t i = 0; i < m_caches.size(); i++) {
		if (m_caches[i].source->isEmitting())
			return true;
	}
	return false;
}

EffectSystem::EffectSystem() : Fragment(Fragment::Type::effect, "") {
	if (!m_fetchedPresets) {
		// fetch presets
		m_fetchedPresets = true;
		fetchPresets();
	}
	if (!m_shader.isLoaded()) {
		D3D11_INPUT_ELEMENT_DESC inputLayout[] = {
			{
				"Position",					 // "semantic" name in shader
				0,							 // "semantic" index (not used)
				DXGI_FORMAT_R32G32B32_FLOAT, // size of ONE element (3 floats)
				0,							 // input slot
				0,							 // offset of first element
				D3D11_INPUT_PER_VERTEX_DATA, // specify data PER vertex
				0							 // used for INSTANCING (ignore)
			},
			{ "Rotation", 0, DXGI_FORMAT_R32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Size", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "IsActive", 0, DXGI_FORMAT_R32_SINT, 0, 36, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shader.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_sprite.hlsl",
			inputLayout, 5);
	}
}

string EffectSystem::EmitterDescription::toString(EmitType type) {
	static const string emitTypeStr[EmitType_Length]{ "Constant", "Distance", "Burst" };
	return emitTypeStr[type];
}

void EffectSystem::EmitterDescription::imgui_properties(SubEmitters type) {
	vector<EmitterDescription::EmitType> types;
	switch (type) {
	case EffectSystem::Trail:
		types.push_back(EmitType::Constant);
		types.push_back(EmitType::Distance);
		break;
	case EffectSystem::Death:
		types.push_back(EmitType::Burst);
		break;
	case EffectSystem::Collision:
		types.push_back(EmitType::Burst);
		break;
	case EffectSystem::Free:
		types.push_back(EmitType::Constant);
		types.push_back(EmitType::Distance);
		types.push_back(EmitType::Burst);
		break;
	}
	vector<string> emitTypeStr;
	emitTypeStr.resize(types.size());
	const char** emitTypeChar = new const char*[types.size()];
	int selectedEmitType = 0;
	for (size_t i = 0; i < types.size(); i++) {
		emitTypeStr[i] = EmitterDescription::toString(types[i]);
		emitTypeChar[i] = emitTypeStr[i].c_str();
		if (emitType == types[i])
			selectedEmitType = i;
	}
	if (ImGui::Combo("Emit Type", &selectedEmitType, emitTypeChar, types.size(), types.size())) {
		emitType = types[selectedEmitType];
	}
	delete[] emitTypeChar;

	switch (emitType) {
	case EffectSystem::EmitterDescription::Constant:
		ImGui::SliderFloat("Emit Rate", &emitRate, 0, 500, "%.1f");
		break;
	case EffectSystem::EmitterDescription::Distance:
		ImGui::SliderFloat("Emits Per Distance", &emitsPerDistance, 0, 100, "%.1f");
		break;
	case EffectSystem::EmitterDescription::Burst:
		ImGui::SliderInt("Burst Count", &burstCount, 0, 200, "%.1f");
		break;
	}
	if (ImGui::TreeNode("Colors")) {
		for (size_t i = 0; i < colors.size(); i++) {
			if (ImGui::Button(("Del##" + to_string(i)).c_str())) {
				colors.erase(colors.begin() + i);
				i--;
				continue;
			}
			ImGui::SameLine();
			ImGui::ColorEdit4(("Color[" + to_string(i) + "]").c_str(), (float*)&colors[i]);
		}
		if (ImGui::Button("Add Color")) {
			colors.push_back(Color(1, 1, 1, 1));
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Spawn Shape")) {
		ImGui::Combo("Shape", (int*)&spawnType, SpawnTypeStr, (int)SpawnType::SpawnType_Length,
			(int)SpawnType::SpawnType_Length);
		shapes[spawnType]->imgui_properties();
		ImGui::TreePop();
	}
	ImGui::DragFloatRange2(
		"Size Range", &sizeRange.x, &sizeRange.y, 0.01f, 0, 1, "Min: %.2f", "Max: %.2f");
	ImGui::Checkbox(randomRotation ? "Random Rotation" : "##54", &randomRotation);
	if (!randomRotation) {
		ImGui::SameLine();
		ImGui::SliderFloat("Rotation", &startRotation, 0, 2 * XM_PI, "Rad: %.2f");
	}
	ImGui::DragFloatRange2("Rotation Velocity Range", &rotationVelocity_range.x,
		&rotationVelocity_range.y, 0.01f, -10, 10, "Min: %.2f", "Max: %.2f");
	ImGui::DragFloatRange2("Time Alive Range", &lifetimeRange.x, &lifetimeRange.y, 0.1f, 0, 15,
		"Min: %.1f", "Max: %.1f");
	ImGui::DragFloatRange2("Velocity Radius Range", &velocityRadiusRange.x, &velocityRadiusRange.y,
		0.01f, 0, XM_PI, "%.2f Rad", "%.2f Rad");
	ImGui::DragFloatRange2("Velocity Intensity Range", &velocityIntensityRange.x,
		&velocityIntensityRange.y, 0.1f, 0, 500, "%.1f", "%.1f");
	ImGui::InputFloat3("Gravity", (float*)&gravity);
	ImGui::SliderFloat("Slowdown", &friction, 0, 1, "%.5f");
	ImGui::Checkbox("##52", &mapSizeToLifetime);
	ImGui::SameLine();
	if (ImGui::TreeNode("Map Size to Lifetime")) {
		ImGui::SliderFloat("Size Transition Edge", &mapSize_middleFactor, 0, 1);
		ImGui::TreePop();
	}
	ImGui::Checkbox("##53", &mapAlphaToLifetime);
	ImGui::SameLine();
	if (ImGui::TreeNode("Map Alpha to Lifetime")) {
		ImGui::SliderFloat("Alpha Transition Edge", &mapAlpha_middleFactor, 0, 1);
		ImGui::TreePop();
	}
	ImGui::Checkbox("##54", &collideWithTerrain);
	ImGui::SameLine();
	if (ImGui::TreeNode("Collide With Terrain")) {
		ImGui::SliderFloat("Bounce Reflection", &collisionBounceIntensity, 0, 1);
		ImGui::Checkbox("Destroy On Collision", &destroyOnCollision);
		ImGui::TreePop();
	}

	TextureRepository::Type sprType = TextureRepository::Type::type_particleSprite;
	static bool fetchedSprites = false;
	static vector<shared_ptr<Texture>> sprites;
	if (!fetchedSprites) {
		fetchedSprites = true;
		vector<string> temp_strs;
		SimpleFilesystem::readDirectory("assets/ParticleSystems/Sprites/", temp_strs);
		sprites.resize(temp_strs.size());
		for (size_t i = 0; i < temp_strs.size(); i++)
			sprites[i] = TextureRepository::get(temp_strs[i], sprType);
	}
	string texName = (texture.get() == nullptr ? "" : texture->getFilename());
	if (ImGui::BeginCombo("Sprite", texName.c_str())) {
		float cWidth = ImGui::CalcItemWidth();
		int itemCountOnWidth = 3;
		ImVec2 instSize = ImVec2(1, 1) * (cWidth / itemCountOnWidth);
		for (size_t i = 0; i < sprites.size(); i++) {
			ImGui::BeginGroup();
			ImGui::Text(sprites[i]->getFilename().c_str());
			if (ImGui::ImageButton(sprites[i]->getSRV().Get(), instSize)) {
				texture = sprites[i];
			}
			ImGui::EndGroup();
			if (texture.get() == sprites[i].get())
				ImGui::SetItemDefaultFocus();
			if ((i + 1) % itemCountOnWidth != 0)
				ImGui::SameLine();
		}
		ImGui::EndCombo();
	}
	static const char* modes[] = { "Opaque", "AlphaBlend", "NonPremultiplied", "Additive",
		"Subtractive", "Multiply" };
	ImGui::Combo("Draw Mode", (int*)&drawMode, modes, IM_ARRAYSIZE(modes));
	ImGui::Checkbox("Sort (Heavy Operation)", &sort);
}

void EffectSystem::EmitterDescription::read(ifstream& file) {
	fileRead(file, spawnType);
	shapes[spawnType]->read(file);
	fileRead(file, emitType);
	fileRead(file, emitRate);
	fileRead(file, emitsPerDistance);
	fileRead(file, burstCount);
	fileRead<Color>(file, colors);
	fileRead(file, randomRotation);
	fileRead(file, startRotation);
	fileRead(file, rotationVelocity_range);
	fileRead(file, sizeRange);
	fileRead(file, lifetimeRange);
	fileRead(file, velocityRadiusRange);
	fileRead(file, velocityIntensityRange);
	fileRead(file, gravity);
	fileRead(file, friction);
	fileRead(file, mapSizeToLifetime);
	fileRead(file, mapSize_middleFactor);
	fileRead(file, mapAlphaToLifetime);
	fileRead(file, mapAlpha_middleFactor);
	fileRead(file, sort);
	fileRead(file, drawMode);
	string tex;
	fileRead(file, tex);
	texture = TextureRepository::get(tex, TextureRepository::Type::type_particleSprite);
	fileRead(file, collideWithTerrain);
	fileRead(file, destroyOnCollision);
	fileRead(file, collisionBounceIntensity);
}

void EffectSystem::EmitterDescription::write(ofstream& file) {
	fileWrite(file, spawnType);
	shapes[spawnType]->write(file);
	fileWrite(file, emitType);
	fileWrite(file, emitRate);
	fileWrite(file, emitsPerDistance);
	fileWrite(file, burstCount);
	fileWrite<Color>(file, colors);
	fileWrite(file, randomRotation);
	fileWrite(file, startRotation);
	fileWrite(file, rotationVelocity_range);
	fileWrite(file, sizeRange);
	fileWrite(file, lifetimeRange);
	fileWrite(file, velocityRadiusRange);
	fileWrite(file, velocityIntensityRange);
	fileWrite(file, gravity);
	fileWrite(file, friction);
	fileWrite(file, mapSizeToLifetime);
	fileWrite(file, mapSize_middleFactor);
	fileWrite(file, mapAlphaToLifetime);
	fileWrite(file, mapAlpha_middleFactor);
	fileWrite(file, sort);
	fileWrite(file, drawMode);
	fileWrite(file, texture.get() == nullptr ? string("bananaFace.png") : texture->getFilename());
	fileWrite(file, collideWithTerrain);
	fileWrite(file, destroyOnCollision);
	fileWrite(file, collisionBounceIntensity);
}

EffectSystem::EmitterDescription::EmitterDescription() {
	texture = TextureRepository::get("dot.png", TextureRepository::Type::type_particleSprite);
}

bool EffectSystem::Emitter::isLinked() const { return system.get() != nullptr; }

void EffectSystem::Emitter::reset() {
	emitTimer = 0;
	previousPosition = float3(0.f);
	firstUpdate = true;
	system.reset();
}

void EffectSystem::Emitter::update(float dt, const Transformation& transform) {
	if (isLinked() && system->isEmitting()) {
		EmitterDescription desc = system->getDescription();
		if (firstUpdate)
			previousPosition = transform.getPosition();
		switch (desc.emitType) {
		case EmitterDescription::EmitType::Constant: {
			float rate = desc.emitRate;
			if (rate > 0.f) {
				emitTimer += dt;
				float emits = emitTimer * rate;
				size_t emitCount = (size_t)emits;
				if (emitCount > 0) {
					system->emit(emitCount, transform);
					emitTimer -= (float)emitCount / rate;
				}
			}
			break;
		}
		case EmitterDescription::EmitType::Distance: {
			float3 position = transform.getPosition();
			float3 forward = position - previousPosition;
			float distance = forward.Length();
			forward.Normalize();
			Transformation t = transform;
			t.setPosition(previousPosition);
			float unitPerEmit = 1.f / desc.emitsPerDistance;
			const float steps = distanceRest + distance;
			float step = 0;
			float rest = distanceRest;
			float accumulatedDistance = 0;
			while (step + unitPerEmit <= steps) {
				float desired = unitPerEmit - rest;
				rest = 0;
				step += unitPerEmit;

				accumulatedDistance += desired;
				t.move(forward * accumulatedDistance);
				system->emit(1, t);
			}
			distanceRest = fmod(steps, unitPerEmit); // leftover distance
			break;
		}
		case EmitterDescription::EmitType::Burst: {
			system->burst(transform);
			break;
		}
		}
		previousPosition = transform.getPosition();
		firstUpdate = false;
	}
}

void EffectSystem::Emitter::unlink() { system.reset(); }

void EffectSystem::Emitter::link(shared_ptr<ParticleCache> _system) { system = _system; }

void EffectSystem::ParticleCache::refreshParticle(size_t index, const Transformation& transform) {
	Matrix rotationMatrix = transform.getRotationMatrix();

	Particle& e = m_particles[index];
	e.vars.enabled = true;
	float3 localSpawn = m_description.shapes[m_description.spawnType]->getSpawn();
	float3 worldSpawn = float3::Transform(localSpawn, rotationMatrix);
	e.vars.position = transform.getPosition() + worldSpawn;
	if (m_description.randomRotation)
		e.vars.rotation = RandomFloat(0, 2 * XM_PI);
	else
		e.vars.rotation = m_description.startRotation;
	e.rotationVelocity =
		RandomFloat(m_description.rotationVelocity_range.x, m_description.rotationVelocity_range.y);
	e.startColor = (m_description.colors.size() == 0)
					   ? Color(1.f)
					   : m_description.colors[rand() % m_description.colors.size()];
	e.vars.color = e.startColor;
	e.startSize = RandomFloat(m_description.sizeRange.x, m_description.sizeRange.y);
	e.vars.size = e.startSize;
	e.lifetime = RandomFloat(m_description.lifetimeRange.x, m_description.lifetimeRange.y);
	e.timeleft = e.lifetime;
	float xRotation =
		RandomFloat(m_description.velocityRadiusRange.x, m_description.velocityRadiusRange.y);
	float3 preRot = rotatef2X(float3(0, 0, 1), xRotation);
	float zRotation = RandomFloat(0, 2.f * XM_PI);
	float3 rotBase = rotatef2Z(preRot, zRotation);
	float3 forward = float3::Transform(float3(0, 0, 1), rotationMatrix), right, up;
	if (forward == float3(0, 1, 0)) {
		right = float3(1, 0, 0);
		up = float3(0, 0, 1);
	}
	else {
		right = Normalize(forward.Cross(float3(0, 1, 0)));
		up = Normalize(forward.Cross(right));
	}
	Matrix matBase = Matrix(right, up, forward);
	float3 velocity = float3::Transform(rotBase, matBase);
	velocity *=
		RandomFloat(m_description.velocityIntensityRange.x, m_description.velocityIntensityRange.y);
	e.velocity = velocity;

	for (size_t i = 0; i < SubEmitter_Length; i++) {
		e.subEmitters[i].reset();
		e.subEmitters[i].link(m_subCaches[i]);
	}
}

void EffectSystem::ParticleCache::killParticle(size_t index, float dt) {
	Particle& e = m_particles[index];
	// kill
	e.vars.enabled = false;
	// on death
	Transformation t = Transformation(e.vars.position, float3(1.f), vector2Rotation(e.velocity));
	e.subEmitters[Death].update(dt, t);
}

void EffectSystem::ParticleCache::updateParticles(float dt) {
	if (m_buffer.size() != m_particles.size())
		m_buffer.resize(m_particles.size());
	for (size_t i = 0; i < m_particles.size(); i++) {
		Particle& e = m_particles[i];
		if (e.vars.enabled) {
			e.timeleft = max(0, e.timeleft - dt);
			if (e.timeleft == 0) {
				killParticle(i, dt);
			}
			else {
				float lifetimeFactor = 1 - (e.timeleft / e.lifetime); // 0 = begin, 1 = end
				if (m_description.mapSizeToLifetime) {
					float middle = m_description.mapSize_middleFactor;
					float factor = (lifetimeFactor < middle ? lifetimeFactor / middle
															: (1 - lifetimeFactor) / (1 - middle));
					e.vars.size = e.startSize * factor;
				}
				if (m_description.mapAlphaToLifetime) {
					float middle = m_description.mapAlpha_middleFactor;
					float factor = (lifetimeFactor < middle ? lifetimeFactor / middle
															: (1 - lifetimeFactor) / (1 - middle));
					e.vars.color.w = e.startColor.w * factor;
				}

				float3 previousPosition = e.vars.position;
				// update properties
				e.vars.rotation += e.rotationVelocity * dt;

				e.velocity += m_description.gravity * dt;

				// update collision
				if (m_description.collideWithTerrain) {
					float3 nextPosition = e.vars.position + e.velocity * dt;
					TerrainBatch* tb = &SceneManager::getScene()->m_terrains;
					if (tb->getHeightFromPosition(nextPosition) > nextPosition.y) {
						bool burst = (e.velocity.Length() > 5.f);
						// collision
						float3 normal = tb->getNormalFromPosition(nextPosition);
						e.velocity = float3::Reflect(e.velocity, normal) *
									 m_description.collisionBounceIntensity;

						// burst (only burst of high velocity)
						if (burst) {
							Transformation t(e.vars.position, float3(1.f), vector2Rotation(normal));
							e.subEmitters[Collision].update(dt, t);
						}

						// kill
						if (m_description.destroyOnCollision)
							killParticle(i, dt);
					}
				}

				e.vars.position += e.velocity * dt;
				e.velocity *= pow(1 - m_description.friction, dt);

				// update trail
				Transformation t =
					Transformation(e.vars.position, float3(1.f), vector2Rotation(e.velocity));
				e.subEmitters[Trail].update(dt, t);
			}
		}
		// fill buffer
		m_buffer[i] = e.vars;
	}
	if (m_description.sort)
		qsort(m_buffer.data(), m_buffer.size(), sizeof(ParticleBuffer), ES_compareParticles);
	m_buffer.syncBuffer();
}

bool EffectSystem::ParticleCache::adjustSize() {
	if (m_particles.size() > 100) {
		float factor = (float)getActiveParticleCount() / m_particles.size();
		if (factor <= 0.5f) {
			size_t splitIdx = 0.5f * m_particles.size();
			size_t stepIdx = 0;
			for (size_t i = splitIdx; i < m_particles.size(); i++) {
				if (m_particles[i].vars.enabled) {
					// move
					for (stepIdx; stepIdx < m_particles.size(); stepIdx++) {
						if (m_particles[stepIdx].vars.enabled == false) {
							// place
							m_particles[stepIdx] = m_particles[i];
							break;
						}
					}
				}
			}
			m_particles.resize(splitIdx);
			return true;
		}
	}
	return false;
}

void EffectSystem::ParticleCache::addSubCache(SubEmitters emitterType) {
	m_subCaches[emitterType] = make_shared<ParticleCache>();
	m_subCaches[emitterType]->setCacheType((SubEmitters)emitterType);
	m_subCaches[emitterType]->setParentCache(this);
}

void EffectSystem::ParticleCache::delSubCache(SubEmitters emitterType) {
	if (m_parentCache != nullptr)
		m_parentCache->unlinkParticlesToCache(emitterType);
	m_subCaches[emitterType].reset();
}

string EffectSystem::ParticleCache::toString(SubEmitters type) {
	static const string subCacheAsString[SubEmitter_Length] = { "Trail", "Death", "Collision" };
	return subCacheAsString[type];
}

EffectSystem::EmitterDescription EffectSystem::ParticleCache::getDescription() const {
	return m_description;
}

size_t EffectSystem::ParticleCache::getParticleCount() const { return m_particles.size(); }

size_t EffectSystem::ParticleCache::getActiveParticleCount() const {
	size_t count = 0;
	// local particles
	count += getLocalActiveParticleCount();
	// sub cache particles
	for (size_t i = 0; i < SubEmitter_Length; i++) {
		if (m_subCaches[i].get() != nullptr)
			count += m_subCaches[i]->getActiveParticleCount();
	}
	return count;
}

bool EffectSystem::ParticleCache::isEmitting() const { return m_isEmitting; }

void EffectSystem::ParticleCache::setEmittingState(bool state) { m_isEmitting = state; }

bool EffectSystem::ParticleCache::hasSubEmitters() const {
	for (size_t i = 0; i < SubEmitter_Length; i++) {
		if (m_subCaches[i].get() != nullptr)
			return true;
	}
	return false;
}

size_t EffectSystem::ParticleCache::getLocalActiveParticleCount() const {
	size_t count = 0;
	// local particles
	for (size_t i = 0; i < m_particles.size(); i++)
		count += (m_particles[i].vars.enabled == true);
	return count;
}

void EffectSystem::ParticleCache::setCacheType(SubEmitters type) {
	m_type = type;
	switch (type) {
	case EffectSystem::Trail:
		m_description.emitType = EmitterDescription::EmitType::Distance;
		break;
	case EffectSystem::Death:
		m_description.emitType = EmitterDescription::EmitType::Burst;
		break;
	case EffectSystem::Collision:
		m_description.emitType = EmitterDescription::EmitType::Burst;
		break;
	case EffectSystem::Free:
		m_description.emitType = EmitterDescription::EmitType::Constant;
		break;
	}
}

void EffectSystem::ParticleCache::setParentCache(ParticleCache* cache) { m_parentCache = cache; }

void EffectSystem::ParticleCache::emit(size_t count, const Transformation& transform) {
	if (m_isEmitting) {
		for (size_t i = 0; i < m_particles.size() && count > 0; i++) {
			if (!m_particles[i].vars.enabled) {
				refreshParticle(i, transform);
				count--;
			}
		}
		if (count > 0) {
			// increase size of array
			m_particles.resize(m_particles.size() + count);
			// try again to emit
			emit(count, transform);
		}
	}
}

void EffectSystem::ParticleCache::burst(const Transformation& transform) {
	if (m_description.emitType == EmitterDescription::EmitType::Burst)
		emit(m_description.burstCount, transform);
}

void EffectSystem::ParticleCache::unlinkParticlesToCache(SubEmitters emitTypeCache) {
	for (size_t i = 0; i < m_particles.size(); i++) {
		m_particles[i].subEmitters[emitTypeCache].reset();
	}
}

void EffectSystem::ParticleCache::update(float dt) {
	if (m_active) {
		// update particles
		updateParticles(dt);
		// update sub caches
		for (size_t i = 0; i < SubEmitter_Length; i++) {
			if (m_subCaches[i].get() != nullptr)
				m_subCaches[i]->update(dt);
		}
	}
}

void EffectSystem::ParticleCache::draw() {
	if (m_active && m_buffer.getBufferSize() > 0) {
		for (size_t i = 0; i < SubEmitter_Length; i++) {
			if (m_subCaches[i].get() != nullptr)
				m_subCaches[i]->draw();
		}

		auto deviceContext = Renderer::getDeviceContext();

		// bind
		m_shader.bindShadersAndLayout();
		bindVertexBuffer();
		if (m_description.texture.get() != nullptr)
			Renderer::getDeviceContext()->PSSetShaderResources(
				0, 1, m_description.texture->getSRV().GetAddressOf());

		// draw
		switch (m_description.drawMode) {
		case EmitterDescription::DrawMode::Opaque:
			Renderer::getInstance()->setBlendState_Opaque();
			break;
		case EmitterDescription::DrawMode::NonPremultiplied:
			Renderer::getInstance()->setBlendState_NonPremultiplied();
			break;
		case EmitterDescription::DrawMode::AlphaBlend:
			Renderer::getInstance()->setBlendState_AlphaBlend();
			break;
		case EmitterDescription::DrawMode::Additive:
			Renderer::getInstance()->setBlendState_Additive();
			Renderer::getInstance()->setDepthState_Read();
			break;
		case EmitterDescription::DrawMode::Subtractive:
			Renderer::getInstance()->setBlendState_Subtractive();
			Renderer::getInstance()->setDepthState_Read();
			break;
		case EmitterDescription::DrawMode::Multiply:
			Renderer::getInstance()->setBlendState_Multiply();
			Renderer::getInstance()->setDepthState_Read();
			break;
		}

		deviceContext->Draw((UINT)m_buffer.getBufferSize(), (UINT)0);

		Renderer::getInstance()->setBlendState_Opaque();
		Renderer::getInstance()->setDepthState_Default();
		ShaderSet::clearShaderBindings(); // removes bug of sprites not being able to be
										  // drawn(by removing geometry shade)
	}
}

void EffectSystem::ParticleCache::imgui_properties() {
	for (size_t i = 0; i < SubEmitters::SubEmitter_Length; i++) {
		if (m_subCaches[i].get() == nullptr) {
			if (ImGui::Button(("Add " + toString((SubEmitters)i)).c_str()))
				addSubCache((SubEmitters)i);
		}
		else {
			if (ImGui::Button(("Del " + toString((SubEmitters)i)).c_str()))
				delSubCache((SubEmitters)i);
		}
		if ((i + 1) < SubEmitter_Length)
			ImGui::SameLine();
	}
	m_description.imgui_properties(m_type);
}

void EffectSystem::ParticleCache::imgui_tree(ParticleCache** selected) {
	ImGuiTreeNodeFlags baseFlags = ImGuiTreeNodeFlags_OpenOnDoubleClick |
								   ImGuiTreeNodeFlags_OpenOnArrow |
								   ImGuiTreeNodeFlags_SpanAvailWidth;
	for (size_t i = 0; i < SubEmitter_Length; i++) {
		if (m_subCaches[i].get() != nullptr) {
			ImGuiTreeNodeFlags flags = baseFlags;
			string id = ParticleCache::toString((SubEmitters)i) + " (" +
						to_string(m_subCaches[i]->getLocalActiveParticleCount()) + "/" +
						to_string(m_subCaches[i]->getParticleCount()) + ")";
			bool isSelected = (*selected == m_subCaches[i].get());
			if (isSelected)
				flags |= ImGuiTreeNodeFlags_Selected;
			if (!m_subCaches[i]->hasSubEmitters())
				flags |= ImGuiTreeNodeFlags_Leaf;
			bool open = ImGui::TreeNodeEx(("##" + to_string(i)).c_str(), flags, id.c_str());
			if (ImGui::IsItemClicked())
				*selected = m_subCaches[i].get();
			if (open) {
				m_subCaches[i]->imgui_tree(selected);
				ImGui::TreePop();
			}
		}
	}
}

void EffectSystem::ParticleCache::read(ifstream& file) {
	clear();

	fileRead(file, m_type);
	m_description.read(file);
	for (size_t i = 0; i < SubEmitters::SubEmitter_Length; i++) {
		bool hasCache = fileRead<bool>(file);
		if (hasCache) {
			m_subCaches[i] = make_shared<ParticleCache>();
			m_subCaches[i]->read(file);
		}
	}
}

void EffectSystem::ParticleCache::write(ofstream& file) {
	fileWrite(file, m_type);
	m_description.write(file);
	for (size_t i = 0; i < SubEmitters::SubEmitter_Length; i++) {
		bool hasCache = (m_subCaches[i].get() != nullptr);
		fileWrite<bool>(file, hasCache); // has sub cache
		if (hasCache)
			m_subCaches[i]->write(file);
	}
}

void EffectSystem::ParticleCache::clear() {
	m_description = EmitterDescription();
	setCacheType(SubEmitters::Free);
	for (size_t i = 0; i < SubEmitters::SubEmitter_Length; i++) {
		m_subCaches->reset();
	}
	m_particles.clear();
	m_buffer.clear();
	m_buffer.Reset();
	m_active = true;
	m_emitTimer = 0;
	m_isEmitting = true;
}

EffectSystem::ParticleCache::ParticleCache() { setCacheType(SubEmitters::Free); }
