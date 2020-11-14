#pragma once
#include <iostream>
#include <fstream>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <cerrno>
#include <cstring>
#include <vector>

using namespace std;

struct FileSyncer {
public:
	enum SyncType { state_saveFile, state_liveFile };

private:
	enum VarTypes {
		type_noone,
		type_double,
		type_float,
		type_vector2,
		type_vector3,
		type_vector4,
		type_int,
		type_string,
		type_bool,
		Count
	};
	const string m_typeInterpretations[Count] = { "0", "d", "f", "v2", "v3", "v4", "i", "s", "b" };
	const string m_prePath = "assets/FileSyncs/";

	bool m_fileCreated = false;
	string m_path;
	struct FileVariable {
		string m_varName;
		VarTypes m_varType = type_noone;
		void* m_ptr = nullptr;
		FileVariable(string name = "");
	};
	vector<FileVariable> m_description;
	void (*function_onLoad)(void);

	time_t m_timestamp;

	SyncType m_type;

	VarTypes getTypeFromString(string str_type) const;
	size_t getByteSizeFromType(VarTypes type) const;

	bool valid(string path);

	void parseToPointer(string str, VarTypes type, void* ptr_data);
	string typeToString(VarTypes type, void* ptr_data);

	time_t getModificationTime() const;
	void onLoad();

public:
	string getPath() const;
	SyncType getType() const;
	bool fileCreated() const;

	void clear();
	void readFile(string overridePath = "");
	void writeFile(string overridePath = "");
	bool sync();
	void bind(string description, void* ptr);
	void connect(string path);

	static bool fileExists(string path);

	FileSyncer(
		string path = "", SyncType type = SyncType::state_liveFile, void (*onLoad)(void) = nullptr);
};

class VariableSyncer {
private:
	static VariableSyncer m_this;
	static string m_prePathForStatic;

	const float m_syncInterval = 0.5f; // time until sync interval occurs, in seconds
	clock_t m_latestSyncTime;
	vector<FileSyncer> files;

	// --Functions--
	VariableSyncer();
	~VariableSyncer();

public:
	static VariableSyncer* getInstance();
	/*
	 *	types - f = float, d = double, i = int, s = string, v2 = float2, v3 = float3, v4 = float4
	 *	description = "name1:type1&name2:type2"
	*/
	bool bind(string path, string description, void* data_ptr);
	FileSyncer* create(string path, FileSyncer::SyncType type = FileSyncer::SyncType::state_liveFile,
		void (*onLoad)(void) = nullptr);
	void sync();
	void saveAll();

	static bool writeToFile(string file, void* ptr, size_t byteSize);
	static bool readFromFile(string file, void* ptr, size_t byteSize);
};
