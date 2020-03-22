#include "VariableSyncer.h"
#include "ErrorLogger.h"

VariableSyncer VariableSyncer::m_this;
string VariableSyncer::m_prePathForStatic = "assets/FileSyncs/";

FileSyncer::FileSyncer(string path, SyncType type, void (*onLoad)(void)) {
	m_type = type;
	m_path = path;
	function_onLoad = onLoad;
}

void FileSyncer::readFile() {
	if (valid()) {
		fstream file;
		file.open(m_prePath + m_path, ios::in);
		if (file.is_open()) {
			string str = "";
			const size_t count = 100;
			char c_str[count];
			while (file.peek() != EOF) {
				// read line
				file.getline(c_str, count);
				str = c_str;
				// split string name:type
				size_t index = str.find(":");
				if (index != string::npos) {
					string varNameStr = str.substr(0, index);
					string varTypeStr = str.substr(index + 1, str.length() - (index + 1));
					if (varNameStr.length() > 0 && varTypeStr.length() > 0) {
						// find variable
						bool found = false;
						for (size_t i = 0; i < m_description.size(); i++) {
							if (varNameStr == m_description[i].m_varName) {
								found = true;
								// parse to data pointer
								parseToPointer(
									varTypeStr, m_description[i].m_varType, m_description[i].m_ptr);
								break;
							}
						}
						if (!found) {

						/*	ErrorLogger::logWarning(
								HRESULT(), "(VariableSyncer) At file: " + m_path +
											   "\nVariable name (" + varNameStr + ") not found");
						*/}
					}
					else {
						// corrupt line
						// ignore line
						ErrorLogger::logWarning(
							"(VariableSyncer) At file: " + m_path + "\nCorrupt line!");
					}
				}
				else {
					// variable was not found
					// ignore line
					ErrorLogger::logWarning("(VariableSyncer) At file: " + m_path +
											"\nLine in wrong format! (VarName:VarType)");
				}
			}
			file.close();
			onLoad();
		}
		else {
			writeFile();
		}
	}
	else {
		ErrorLogger::logWarning(
			"(VariableSyncer) Failed opening unvalid struct for file: " + m_path);
	}
}

void FileSyncer::writeFile() {
	if (valid()) {
		fstream file;
		file.open(m_prePath + m_path, ios::out);
		if (file.is_open()) {
			for (size_t i = 0; i < m_description.size(); i++) {
				file << m_description[i].m_varName << ":";
				file << typeToString(m_description[i].m_varType, m_description[i].m_ptr);
				file << '\n';
			}
			file.close();
			// update modification time
			m_timestamp = getModificationTime();
		}
		else {
			ErrorLogger::logWarning("(VariableSyncer) Failed at opening file: " + m_path);
		}
	}
	else {
		ErrorLogger::logWarning("(VariableSyncer) Failed opening unvalid file: " + m_path);
	}
}

bool FileSyncer::sync() {
	if (valid() && m_type == SyncType::state_liveFile) {
		// create if not already created
		if (!m_fileCreated) {
			m_fileCreated = true;
			writeFile();
		}
		// read from file if updated
		time_t time = getModificationTime();
		if (time > m_timestamp) {
			// update
			readFile();
			return true;
		}
		return false;
	}
	return false;
}

FileSyncer::VarTypes FileSyncer::getTypeFromString(string str_type) const {
	for (size_t i = 1; i < Count; i++) {
		if (str_type == m_typeInterpretations[i])
			return (VarTypes)i;
	}
	return type_noone;
}

size_t FileSyncer::getByteSizeFromType(VarTypes type) const {
	size_t size = 0;
	switch (type) {
	case FileSyncer::type_noone:
		break;
	case FileSyncer::type_double:
		size = sizeof(double);
		break;
	case FileSyncer::type_float:
		size = sizeof(float);
		break;
	case FileSyncer::type_vector2:
		size = sizeof(float) * 2;
		break;
	case FileSyncer::type_vector3:
		size = sizeof(float) * 3;
		break;
	case FileSyncer::type_vector4:
		size = sizeof(float) * 4;
		break;
	case FileSyncer::type_int:
		size = sizeof(int);
		break;
	case FileSyncer::type_string:
		size = sizeof(string);
		break;
	default:
		break;
	}
	return size;
}

void FileSyncer::bind(string description, void* ptr) {
	if (description != "") {
		size_t byteOffset = 0;
		char c;
		string str_temp;
		size_t length = description.length();
		for (size_t i = 0; i < length; i++) {
			c = description[i];
			if (c == ':') {
				m_description.push_back(FileVariable(str_temp));
				str_temp = "";
			}
			else if (c == '&' || i == length - 1) {
				if (i == length - 1)
					str_temp += c;
				VarTypes type = getTypeFromString(str_temp);
				size_t byteSize = getByteSizeFromType(type);
				m_description.back().m_varType = type;
				m_description.back().m_ptr = (char*)ptr + byteOffset;
				byteOffset += byteSize;
				str_temp = "";
			}
			else {
				str_temp += c;
			}
		}
	}
}

string FileSyncer::getPath() const { return m_path; }

FileSyncer::SyncType FileSyncer::getType() const { return m_type; }

bool FileSyncer::valid() { return (m_description.size() > 0 && m_path != ""); }

bool FileSyncer::fileCreated() const {
	fstream file;
	file.open(m_prePath + m_path, ios::in);
	if (file.is_open()) {
		file.close();
		return true;
	}
	else
		return false;
}

void FileSyncer::parseToPointer(string str, VarTypes type, void* ptr_data) {
	void* data = ptr_data;
	size_t split1, split2, split3;
	switch (type) {
	case FileSyncer::type_noone:
		break;
	case FileSyncer::type_double:
		*((double*)data) = std::stod(str);
		break;
	case FileSyncer::type_float:
		*((float*)data) = std::stof(str);
		break;
	case FileSyncer::type_vector2:
		split1 = str.find(" ", 0);
		((float*)data)[0] = std::stof(str.substr(0, split1));
		((float*)data)[1] = std::stof(str.substr(split1 + 1, str.length() - (split1 + 1)));
		break;
	case FileSyncer::type_vector3:
		split1 = str.find(" ", 0);
		split2 = str.find(" ", split1 + 1);
		((float*)data)[0] = std::stof(str.substr(0, split1));
		((float*)data)[1] = std::stof(str.substr(split1 + 1, split2 - (split1 + 1)));
		((float*)data)[2] = std::stof(str.substr(split2 + 1, str.length() - (split2 + 1)));
		break;
	case FileSyncer::type_vector4:
		split1 = str.find(" ", 0);
		split2 = str.find(" ", split1 + 1);
		split3 = str.find(" ", split2 + 1);
		((float*)data)[0] = std::stof(str.substr(0, split1));
		((float*)data)[1] = std::stof(str.substr(split1 + 1, split2 - (split1 + 1)));
		((float*)data)[2] = std::stof(str.substr(split2 + 1, split3 - (split2 + 1)));
		((float*)data)[3] = std::stof(str.substr(split3 + 1, str.length() - (split3 + 1)));
		break;
	case FileSyncer::type_int:
		*((int*)data) = std::stoi(str);
		break;
	case FileSyncer::type_string:
		*((string*)data) = str;
		break;
	}
}

string FileSyncer::typeToString(VarTypes type, void* ptr_data) {
	void* data = ptr_data;
	string ret = "";
	switch (type) {
	case FileSyncer::type_noone:
		break;
	case FileSyncer::type_double:
		ret = to_string(*(double*)data);
		break;
	case FileSyncer::type_float:
		ret = to_string(*(float*)data);
		break;
	case FileSyncer::type_vector2:
		ret += to_string(((float*)data)[0]);
		ret += " ";
		ret += to_string(((float*)data)[1]);
		break;
	case FileSyncer::type_vector3:
		ret += to_string(((float*)data)[0]);
		ret += " ";
		ret += to_string(((float*)data)[1]);
		ret += " ";
		ret += to_string(((float*)data)[2]);
		break;
	case FileSyncer::type_vector4:
		ret += to_string(((float*)data)[0]);
		ret += " ";
		ret += to_string(((float*)data)[1]);
		ret += " ";
		ret += to_string(((float*)data)[2]);
		ret += " ";
		ret += to_string(((float*)data)[3]);
		break;
	case FileSyncer::type_int:
		ret = to_string(*(int*)data);
		break;
	case FileSyncer::type_string:
		ret = *(string*)data;
		break;
	}
	return ret;
}

time_t FileSyncer::getModificationTime() const {
	struct stat fileDesc;
	if (stat((m_prePath + m_path).c_str(), &fileDesc) != 0) {
		// error
	}
	return fileDesc.st_mtime;
}

void FileSyncer::onLoad() {
	// update timestamp
	time_t time = getModificationTime();
	m_timestamp = time;
	// call onload function
	if (function_onLoad != nullptr)
		(*function_onLoad)();
}

VariableSyncer::VariableSyncer() {}
VariableSyncer::~VariableSyncer() {}

VariableSyncer* VariableSyncer::getInstance() { return &m_this; }

bool VariableSyncer::bind(string path, string description, void* data_ptr) {
	for (size_t i = 0; i < files.size(); i++) {
		if (path == files[i].getPath()) {
			files[i].bind(description, data_ptr);
			return true;
		}
	}
	return false; // not found
}

FileSyncer* VariableSyncer::create(string path, FileSyncer::SyncType type, void (*onLoad)(void)) {
	bool found = false;
	for (size_t i = 0; i < files.size(); i++) {
		if (path == files[i].getPath()) {
			return &files[i]; // file already exists
		}
	}
	// file doesnt exists yet
	files.push_back(FileSyncer(path, type, onLoad));
	return &files.back();
}

void VariableSyncer::sync() {
	clock_t time = clock();
	if ((time - m_latestSyncTime) / 1000.f > m_syncInterval) {
		m_latestSyncTime = time;
		// sync
		for (size_t i = 0; i < files.size(); i++) {
			files[i].sync();
		}
	}
}

void VariableSyncer::saveAll() {
	for (size_t i = 0; i < files.size(); i++) {
		if (files[i].getType() == FileSyncer::SyncType::state_saveFile)
			files[i].writeFile();
	}
}

bool VariableSyncer::writeToFile(string path, void* ptr, size_t byteSize) {
	fstream file;
	file.open(m_prePathForStatic+path, ios::binary | ios::out);
	if (file.is_open()) {
		file.write((char*)ptr, byteSize);
		file.close();
		return true;
	}
	else {
		return false;
	}
}

bool VariableSyncer::readFromFile(string path, void* ptr, size_t byteSize) {
	fstream file;
	file.open(m_prePathForStatic+path, ios::binary | ios::in);
	if (file.is_open()) {
		file.read((char*)ptr, byteSize);
		file.close();
		return true;
	}
	else {
		return false;
	}
}

FileSyncer::FileVariable::FileVariable(string name) { m_varName = name; }
