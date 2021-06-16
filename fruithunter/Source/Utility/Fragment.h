#pragma once
#include <iostream>
typedef size_t FragmentID;
class Fragment {
public:
	enum Type { entity, terrain, particleSystem, sea, size };
	std::string m_typeAsString[Type::size] = { "Entity", "Terrain", "ParticleSystem", "Sea" };

private:
	static size_t m_id_counter;
	size_t m_id;
	std::string m_description;
	Type m_type;

public:
	Fragment(Type type, std::string description = "");
	Fragment(const Fragment& other);
	virtual ~Fragment();

	Fragment& operator=(const Fragment& other);

	void rename(std::string description);
	void changeType(Type type);

	FragmentID getID() const;
	std::string getDescription() const;
	Type getType() const;
	std::string getTypeAsString() const;
	std::string getFullDescription() const;

	virtual void imgui_properties();

private:
	size_t nextID();
};
