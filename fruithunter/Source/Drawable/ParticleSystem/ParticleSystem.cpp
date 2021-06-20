#include "ParticleSystem.h"
#include "Renderer.h"
#include "ErrorLogger.h"
#include "time.h"
#include "VariableSyncer.h"
#include "SceneManager.h"
#include "filesystemHelper.h"

ShaderSet ParticleSystem::m_shaderSetCircle;
ShaderSet ParticleSystem::m_shaderSetStar;
ShaderSet ParticleSystem::m_shaderSetSprite;

vector<shared_ptr<ParticleSystem::ParticleDescription>> ParticleSystem::m_descriptionList;

void ParticleSystem::load(string psFilename, float emitRate, size_t capacity) {
	setDesc(psFilename);
	setEmitRate(emitRate, false);
	setCapacity(capacity);
}

ParticleSystem::ParticleSystem()
	: Fragment(Fragment::Type::particleSystem), Transformation(float3(0.), float3(0.)) {
	createShaders();
	m_particle_description = make_shared<ParticleSystem::ParticleDescription>();
	m_tex_particle = TextureRepository::get("missing_texture.jpg");
}

void ParticleSystem::setParticle(size_t index) {
	Particle* part = &m_particles[index];
	part->isActive = true;
	// Position in world
	float3 position = getPosition();
	Matrix matRotation =
		Matrix::CreateFromYawPitchRoll(getRotation().y, getRotation().x, getRotation().z);
	float3 scale = getScale();
	// Positon in box
	float3 localSpawn = float3(RandomFloat(-scale.x / 2, scale.x / 2),
		RandomFloat(-scale.y / 2, scale.y / 2), RandomFloat(-scale.z / 2, scale.z / 2));
	part->position = position + float3::Transform(localSpawn, matRotation);
	// Color
	part->color = m_particle_description->colorVariety[rand() % 3];
	// Size
	part->size =
		RandomFloat(m_particle_description->size_interval.x, m_particle_description->size_interval.y);

	ParticleProperty* pp = &m_particleProperties[index];
	// Time alive
	pp->lifeTime = RandomFloat(
		m_particle_description->timeAlive_interval.x, m_particle_description->timeAlive_interval.y);
	pp->timeLeft = pp->lifeTime;
	pp->size = part->size;

	// velocity
	float randVeloX =
		RandomFloat(m_particle_description->velocity_min.x, m_particle_description->velocity_max.x);
	float randVeloY =
		RandomFloat(m_particle_description->velocity_min.y, m_particle_description->velocity_max.y);
	float randVeloZ =
		RandomFloat(m_particle_description->velocity_min.z, m_particle_description->velocity_max.z);
	float3 direction =
		float3::Transform(Normalize(float3(randVeloX, randVeloY, randVeloZ)), matRotation);
	float strength = RandomFloat(
		m_particle_description->velocity_interval.x, m_particle_description->velocity_interval.y);
	pp->velocity = direction * strength;
}

void ParticleSystem::syncSystemFromDescription() {
	// get texture
	m_tex_particle = TextureRepository::get(m_particle_description->str_sprite);
	// resize
	resizeBuffer();
}

void ParticleSystem::ReadDescriptionList() {
	// find files
	vector<string> m_descriptionsStr;
	read_directory(PATH_PSD, m_descriptionsStr);
	files_filterByEnding(m_descriptionsStr, PSD_END);
	size_t endLength = string(PSD_END).length() + 1;
	for (size_t i = 0; i < m_descriptionsStr.size(); i++)
		m_descriptionsStr[i] =
			m_descriptionsStr[i].substr(0, m_descriptionsStr[i].length() - endLength);

	// read files
	m_descriptionList.clear();
	m_descriptionList.resize(m_descriptionsStr.size());
	for (size_t i = 0; i < m_descriptionsStr.size(); i++) {
		m_descriptionList[i] = make_shared<ParticleDescription>();
		m_descriptionList[i]->load(m_descriptionsStr[i]);
	}
}

vector<shared_ptr<ParticleSystem::ParticleDescription>>* ParticleSystem::GetDescriptionList() {
	return &m_descriptionList;
}

bool ParticleSystem::GetDesc(string psdName, shared_ptr<ParticleDescription>& pointer) {
	for (size_t i = 0; i < m_descriptionList.size(); i++) {
		if (m_descriptionList[i]->identifier == psdName) {
			pointer = m_descriptionList[i];
			return true;
		}
	}
	return false;
}

void ParticleSystem::setEmitingState(bool state) { m_isEmitting = state; }

void ParticleSystem::setActiveState(bool state) { m_isActive = state; }

void ParticleSystem::emit(size_t count) {
	for (size_t i = 0; i < m_particles.size() && count > 0; i++) {
		if (m_particles[i].isActive == 0) {
			setParticle(i);
			count--;
		}
	}
}

void ParticleSystem::updateEmits(float dt) {
	m_emitTimer += dt;
	float rate = m_emitRate;
	if (rate > 0.f) {
		float emits = m_emitTimer * rate;
		size_t emitCount = (size_t)emits;
		if (emitCount > 0) {
			emit(emitCount);
			m_emitTimer -= (float)emitCount / rate;
		}
	}
}

void ParticleSystem::updateParticles(float dt) {
	Environment* environment =
		SceneManager::getScene()->m_terrains.getTerrainFromPosition(getPosition());
	for (size_t i = 0; i < m_particles.size(); i++) {
		if (m_particles[i].isActive != 0) {
			m_particleProperties[i].timeLeft -= dt;
			float lifeTime = m_particleProperties[i].lifeTime;
			float timeLeft = m_particleProperties[i].timeLeft;
			float size = m_particleProperties[i].size;
			float fadeStart = m_particle_description->fadeInterval_start;
			float fadeEnd = m_particle_description->fadeInterval_end;
			if (timeLeft < fadeStart)
				m_particles[i].size = (timeLeft / fadeStart) * size;
			else if ((lifeTime - timeLeft) < fadeEnd)
				m_particles[i].size = ((lifeTime - timeLeft) / fadeEnd) * size;
			else
				m_particles[i].size = size;

			if (m_particleProperties[i].timeLeft <= 0.f) {
				// Inactivate particles when lifetime is over
				m_particles[i].isActive = false;
			}
			else {
				float density = 1; // air
				float dragCoefficient = 1;

				float r = m_particles[i].size / 2.f;
				float area = 3.1415f * (float)pow(r, 2);
				float mass = 3.1415f * (4.f / 3.f) * (float)pow(r, 3);
				// get wind
				float3 acceleration = m_particle_description->acceleration;
				if (environment != nullptr && m_affectedByWind) {
					float3 wind = environment->getWindStatic();
					float3 v_relative =
						(m_particleProperties[i].velocity - wind); // velocity relative wind
					float v_length = v_relative.Length();
					float Fd =
						0.5f * density * (float)pow(v_length, 2) * area * dragCoefficient; // drag from wind
					acceleration += (-Fd / mass) * Normalize(v_relative);
				}
				// update velocity and position
				m_particleProperties[i].velocity += acceleration * dt;
				m_particles[i].position += m_particleProperties[i].velocity * dt;
				m_particleProperties[i].velocity *= pow(m_particle_description->slowdown, dt);
			}
		}
	}
}

void ParticleSystem::update(float dt) {
	if (m_isActive) {
		if (m_isEmitting) {
			updateEmits(dt);
		}
		updateParticles(dt);
	}
}

void ParticleSystem::createShaders() {
	if (!m_shaderSetCircle.isLoaded() || !m_shaderSetStar.isLoaded()) {
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
			{ "Color", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Size", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "IsActive", 0, DXGI_FORMAT_R32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		m_shaderSetCircle.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_circle.hlsl",
			inputLayout, 4);
		m_shaderSetStar.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_star.hlsl",
			inputLayout, 4);
		m_shaderSetSprite.createShaders(L"VertexShader_particleSystem.hlsl",
			L"GeometryShader_particleSystem.hlsl", L"PixelShader_particleSystem_sprite.hlsl",
			inputLayout, 4);
	}
}

void ParticleSystem::resizeBuffer() {
	auto device = Renderer::getDevice();
	auto deviceContext = Renderer::getDeviceContext();
	// calc size
	size_t size = m_capacity;
	if (m_capacity == 0)
		size = (size_t)round(m_emitRate * m_particle_description->timeAlive_interval.y);
	if (size != m_particles.size()) {
		// resize buffers
		m_particles.resize(size);
		m_particleProperties.resize(size);

		//  Buffer for particle data
		m_vertexBuffer.Reset();
		if (size != 0) {
			D3D11_BUFFER_DESC buffDesc;
			memset(&buffDesc, 0, sizeof(buffDesc));
			buffDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			buffDesc.Usage = D3D11_USAGE_DEFAULT;
			buffDesc.ByteWidth = (UINT)(sizeof(Particle) * size);

			HRESULT check = device->CreateBuffer(&buffDesc, NULL, m_vertexBuffer.GetAddressOf());

			if (FAILED(check))
				ErrorLogger::logError("(ParticleSystem) Failed creating vertex buffer!\n", check);
		}
	}
	else {
		// already the correct size
	}
}

void ParticleSystem::bindVertexBuffer() {
	auto deviceContext = Renderer::getDeviceContext();
	if (m_vertexBuffer.Get() != nullptr) {
		deviceContext->UpdateSubresource(m_vertexBuffer.Get(), 0, 0, m_particles.data(), 0, 0);

		UINT strides = sizeof(Particle);
		UINT offset = 0;
		deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &strides, &offset);
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	}
}

void ParticleSystem::draw(bool alpha) {
	if (m_isActive && m_particles.size() > 0) {
		auto deviceContext = Renderer::getDeviceContext();

		// bind
		switch (m_particle_description->shape) {
		case ParticleDescription::Shape::Circle:
			m_shaderSetCircle.bindShadersAndLayout();
			break;
		case ParticleDescription::Shape::Star:
			m_shaderSetStar.bindShadersAndLayout();
			break;
		case ParticleDescription::Shape::Sprite:
			m_shaderSetSprite.bindShadersAndLayout();
			break;
		default:
			m_shaderSetCircle.bindShadersAndLayout();
			break;
		}
		bindVertexBuffer();
		Renderer::getDeviceContext()->PSSetShaderResources(
			0, 1, m_tex_particle->view.GetAddressOf());

		// draw
		if (alpha) {
			switch (m_particle_description->drawMode) {
			case ParticleDescription::DrawMode::Normal:
				Renderer::getInstance()->setBlendState_NonPremultiplied();
				break;
			case ParticleDescription::DrawMode::Add:
				Renderer::getInstance()->setBlendState_Additive();
				Renderer::getInstance()->setDepthState_Read();
				break;
			case ParticleDescription::DrawMode::Sub:
				Renderer::getInstance()->setBlendState_NonPremultiplied();
				break;
			default:
				break;
			}
	
			deviceContext->Draw((UINT)m_particles.size(), (UINT)0);

			Renderer::getInstance()->setBlendState_Opaque();
			Renderer::getInstance()->setDepthState_Default();
			ShaderSet::clearShaderBindings(); // removes bug of sprites not being able to be
											  // drawn(by removing geometry shade)
		}
		else {
			deviceContext->Draw((UINT)m_particles.size(), (UINT)0);
		}
	}
}

bool ParticleSystem::setDesc(string psdName) {
	for (size_t i = 0; i < m_descriptionList.size(); i++) {
		if (m_descriptionList[i]->identifier == psdName) {
			m_particle_description = m_descriptionList[i];
			syncSystemFromDescription();
			return true;
		}
	}
	return false;
}

bool ParticleSystem::setDesc(size_t index) { 
	if (index < m_descriptionList.size()) {
		m_particle_description = m_descriptionList[index];
		syncSystemFromDescription();
		return true;
	}
	return false;
}

string ParticleSystem::asPath(string psFilename) { return PATH_PSD + psFilename + "." + PSD_END; }

void ParticleSystem::setAffectedByWindState(bool state) { m_affectedByWind = state; }

bool ParticleSystem::isActive() const { return m_isActive; }

size_t ParticleSystem::getActiveParticleCount() const {
	size_t sum = 0;
	for (size_t i = 0; i < m_particles.size(); i++) {
		sum += (m_particles[i].isActive == 1);
	}
	return sum;
}

bool ParticleSystem::isEmiting() const { return m_isEmitting; }

bool ParticleSystem::isAffectedByWind() const { return m_affectedByWind; }

void ParticleSystem::setEmitRate(float emitRate, bool resize) {
	m_emitRate = emitRate;
	if (resize)
		resizeBuffer();
}

void ParticleSystem::setCapacity(size_t capacity) {
	m_capacity = capacity;
	resizeBuffer();
}

float ParticleSystem::getEmitRate() const { return m_emitRate; }

size_t ParticleSystem::getCapacity() const { return m_capacity; }

shared_ptr<ParticleSystem::ParticleDescription> ParticleSystem::getDesc() {
	return m_particle_description;
}

void ParticleSystem::setCustomDescription(ParticleSystem::ParticleDescription& desc) {
	*m_particle_description.get() = desc;
	syncSystemFromDescription();
}

bool ParticleSystem::ParticleDescription::save(string psdName) const { 
	if (psdName != "") {
		string path = asPath(psdName);
		ofstream file;
		file.open(path, ios::binary);
		if (file.is_open()) {
			// write
			write(file);
			file.close();
			ErrorLogger::log(
				"(ParticleDescription::save) Saved particlesystem description: " +
							 psdName + "." + PSD_END);
			return true;
		}
		else {
			ErrorLogger::logWarning(
				"(ParticleDescription::save) Failed opening to file: " + path);
			return false;
		}
	}
}

ParticleSystem::ParticleDescription::ParticleDescription() {}

void ParticleSystem::imgui_properties() { 
	Transformation::imgui_properties(); 
	if (ImGui::BeginCombo("Particle Description", m_particle_description->identifier.c_str())) {
		for (size_t i = 0; i < m_descriptionList.size(); i++) {
			if (ImGui::Selectable(m_descriptionList[i]->identifier.c_str()))
				setDesc(i);
		}
		ImGui::EndCombo();
	}
	// Emit Rate
	if (ImGui::SliderFloat("Emit Rate", &m_emitRate, 0, 500)) {
		setEmitRate(max(0, m_emitRate));
	}
	// Capacity
	if (ImGui::InputScalar("Capacity", ImGuiDataType_U32, (unsigned int*)&m_capacity)) {
		setCapacity(max(0, m_capacity));
	}
	// Wind
	if (ImGui::Checkbox("Wind", &m_affectedByWind)) {
		setAffectedByWindState(m_affectedByWind);
	}
	// Manual Emit
	ImGui::Separator();
	static int emitCount = 0;
	if (ImGui::Button("Emit")) {
		emit(emitCount);
	}
	ImGui::SameLine();
	ImGui::InputInt("Count", &emitCount);
}

bool ParticleSystem::ParticleDescription::imgui_properties() {
	bool update = false;
	for (size_t i = 0; i < 3; i++)
		ImGui::ColorEdit4(("Color["+to_string(i)+"]").c_str(), (float*)&colorVariety[i]);
	ImGui::DragFloatRange2("Size Range", &size_interval.x, &size_interval.y, 0.01f, 0,1, "Min: %.2f", "Max: %.2f");
	if (ImGui::DragFloatRange2("Time Alive Range", &timeAlive_interval.x, &timeAlive_interval.y,
			0.1f, 0, 15, "Min: %.1f", "Max: %.1f"))
		update = true;
	ImGui::InputFloat3("Velocity Min", (float*)&velocity_min);
	ImGui::InputFloat3("Velocity Max", (float*)&velocity_max);
	ImGui::DragFloatRange2("Velocity Intensity Range", &velocity_interval.x, &velocity_interval.y, 0.01f, 0, 50, "Min: %.2f","Max: %.2f");
	ImGui::InputFloat3("Gravity", (float*)&acceleration);
	ImGui::SliderFloat("Slowdown", &slowdown, 0, 1);
	ImGui::SliderFloat("Fade Begin", &fadeInterval_start, 0, 1);
	ImGui::SliderFloat("Fade End", &fadeInterval_end, 0, 1);
	static const char* shapes[] = { "Circle", "Star", "Sprite" };
	ImGui::Combo("Shape", (int*)&shape, shapes, IM_ARRAYSIZE(shapes));
	static const shared_ptr<TextureSet> sprites[] = { 
		TextureRepository::get("bananaFace.png"),
		TextureRepository::get("cuteFace.png"),
		TextureRepository::get("dragonFace.png")
	};
	static int spriteSelected = 0;
	if (ImGui::BeginCombo("Sprite", str_sprite.c_str())) {
		float cWidth = ImGui::CalcItemWidth();
		int itemCountOnWidth = 3;
		ImVec2 instSize = ImVec2(1, 1) * (cWidth / itemCountOnWidth);
		for (size_t i = 0; i < IM_ARRAYSIZE(sprites); i++) {
			ImGui::BeginGroup();
			ImGui::Text(sprites[i]->filename.c_str());
			if (ImGui::ImageButton(sprites[i]->view.Get(), instSize)) {
				spriteSelected = i;
				str_sprite = sprites[i]->filename;
				update = true;
			}
			ImGui::EndGroup();
			if ((i + 1) % itemCountOnWidth != 0)
				ImGui::SameLine();
		}
		ImGui::EndCombo();
	}
	static const char* modes[] = { "Normal", "Add", "Sub" };
	ImGui::Combo("Draw Mode", (int*)&drawMode, modes, IM_ARRAYSIZE(modes));

	return update;
}

void ParticleSystem::ParticleDescription::write(ofstream& file) const {
	if (file.is_open()) {
		for (size_t i = 0; i < 3; i++)
			fileWrite(file, colorVariety[i]);
		fileWrite(file, size_interval);
		fileWrite(file, timeAlive_interval);
		fileWrite(file, velocity_min);
		fileWrite(file, velocity_max);
		fileWrite(file, velocity_interval);
		fileWrite(file, acceleration);
		fileWrite(file, slowdown);
		fileWrite(file, fadeInterval_start);
		fileWrite(file, fadeInterval_end);
		fileWrite(file, str_sprite);
		fileWrite(file, shape);
		fileWrite(file, drawMode);
	}
}

void ParticleSystem::ParticleDescription::read(ifstream& file) {
	if (file.is_open()) {
		for (size_t i = 0; i < 3; i++)
			fileRead(file, colorVariety[i]);
		fileRead(file, size_interval);
		fileRead(file, timeAlive_interval);
		fileRead(file, velocity_min);
		fileRead(file, velocity_max);
		fileRead(file, velocity_interval);
		fileRead(file, acceleration);
		fileRead(file, slowdown);
		fileRead(file, fadeInterval_start);
		fileRead(file, fadeInterval_end);
		fileRead(file, str_sprite);
		fileRead(file, shape);
		fileRead(file, drawMode);
	}
}

bool ParticleSystem::ParticleDescription::load(string psdName) {
	string path = asPath(psdName);
	ifstream file;
	file.open(path, ios::binary);
	if (file.is_open()) {
		identifier = psdName;
		read(file);
		file.close();
		return true;
	}
	else {
		ErrorLogger::logWarning("(ParticleDescription::load) Failed opening file: " + path);
		return false;
	}
}

bool ParticleSystem::ParticleDescription::save() const { return save(identifier); }
