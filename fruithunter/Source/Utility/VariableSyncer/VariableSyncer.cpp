#include "VariableSyncer.h"
#include "ErrorLogger.h"

VariableSyncer VariableSyncer::m_this;

VariableSyncer::FileSyncer::FileSyncer(string path, void (*onLoad)(void)) {
	m_path = path;
	function_onLoad = onLoad;
}

void VariableSyncer::FileSyncer::readFile() {
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
						if (!found)
							ErrorLogger::logWarning(
								HRESULT(), "(VariableSyncer) At file: " + m_path +
											   "\nVariable name (" + varNameStr + ") not found");
					}
					else {
						// corrupt line
						// ignore line
						ErrorLogger::logWarning(
							HRESULT(), "(VariableSyncer) At file: " + m_path + "\nCorrupt line!");
					}
				}
				else {
					// variable was not found
					// ignore line
					ErrorLogger::logWarning(
						HRESULT(), "(VariableSyncer) At file: " + m_path +
									   "\nLine in wrong format! (VarName:VarType)");
				}
			}
			file.close();
			onLoad();
		}
		else {
			ErrorLogger::logWarning(
				HRESULT(), "(VariableSyncer) Failed at opening file: " + m_path);
		}
	}
	else {
		ErrorLogger::logWarning(
			HRESULT(), "(VariableSyncer) Failed opening unvalid struct for file: " + m_path);
	}
}

void VariableSyncer::FileSyncer::writeFile() {
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
			ErrorLogger::logWarning(
				HRESULT(), "(VariableSyncer) Failed at opening file: " + m_path);
		}
	}
	else {
		ErrorLogger::logWarning(
			HRESULT(), "(VariableSyncer) Failed opening unvalid file: " + m_path);
	}
}

bool VariableSyncer::FileSyncer::sync() {
	if (valid()) {
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

VariableSyncer::FileSyncer::VarTypes VariableSyncer::FileSyncer::getTypeFromString(
	string str_type) const {
	for (size_t i = 1; i < Count; i++) {
		if (str_type == m_typeInterpretations[i])
			return (VarTypes)i;
	}
	return type_noone;
}

size_t VariableSyncer::FileSyncer::getByteSizeFromType(VarTypes type) const {
	size_t size = 0;
	switch (type) {
	case VariableSyncer::FileSyncer::type_noone:
		break;
	case VariableSyncer::FileSyncer::type_float:
		size = sizeof(float);
		break;
	case VariableSyncer::FileSyncer::type_double:
		size = sizeof(double);
		break;
	case VariableSyncer::FileSyncer::type_vector3:
		size = sizeof(float) * 3;
		break;
	case VariableSyncer::FileSyncer::type_int:
		size = sizeof(int);
		break;
	case VariableSyncer::FileSyncer::type_string:
		size = sizeof(string);
		break;
	default:
		break;
	}
	return size;
}

void VariableSyncer::FileSyncer::bind(string description, void* ptr) {
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

string VariableSyncer::FileSyncer::getPath() const { return m_path; }

bool VariableSyncer::FileSyncer::valid() { return (m_description.size() > 0 && m_path != ""); }

void VariableSyncer::FileSyncer::parseToPointer(string str, VarTypes type, void* ptr_data) {
	void* data = ptr_data;
	size_t split1, split2;
	switch (type) {
	case VariableSyncer::FileSyncer::type_noone:
		break;
	case VariableSyncer::FileSyncer::type_float:
		*((float*)data) = std::stof(str);
		break;
	case VariableSyncer::FileSyncer::type_double:
		*((double*)data) = std::stod(str);
		break;
	case VariableSyncer::FileSyncer::type_vector3:
		split1 = str.find(" ", 0);
		split2 = str.find(" ", split1 + 1);
		((float*)data)[0] = std::stof(str.substr(0, split1));
		((float*)data)[1] = std::stof(str.substr(split1 + 1, split2 - (split1 + 1)));
		((float*)data)[2] = std::stof(str.substr(split2 + 1, str.length() - (split2 + 1)));
		break;
	case VariableSyncer::FileSyncer::type_int:
		*((int*)data) = std::stoi(str);
		break;
	case VariableSyncer::FileSyncer::type_string:
		*((string*)data) = str;
		break;
	}
}

string VariableSyncer::FileSyncer::typeToString(VarTypes type, void* ptr_data) {
	void* data = ptr_data;
	string ret = "";
	switch (type) {
	case VariableSyncer::FileSyncer::type_noone:
		break;
	case VariableSyncer::FileSyncer::type_float:
		ret = to_string(*(float*)data);
		break;
	case VariableSyncer::FileSyncer::type_double:
		ret = to_string(*(double*)data);
		break;
	case VariableSyncer::FileSyncer::type_vector3:
		ret += to_string(((float*)data)[0]);
		ret += " ";
		ret += to_string(((float*)data)[1]);
		ret += " ";
		ret += to_string(((float*)data)[2]);
		break;
	case VariableSyncer::FileSyncer::type_int:
		ret = to_string(*(int*)data);
		break;
	case VariableSyncer::FileSyncer::type_string:
		ret = *(string*)data;
		break;
	}
	return ret;
}

time_t VariableSyncer::FileSyncer::getModificationTime() const {
	struct stat fileDesc;
	if (stat((m_prePath + m_path).c_str(), &fileDesc) != 0) {
		// error
	}
	return fileDesc.st_mtime;
}

void VariableSyncer::FileSyncer::onLoad() {
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

bool VariableSyncer::create(string path, void (*onLoad)(void)) {
	bool found = false;
	for (size_t i = 0; i < files.size(); i++) {
		if (path == files[i].getPath()) {
			return false; // file already exists
		}
	}
	// file doesnt exists yet
	files.push_back(FileSyncer(path, onLoad));
	return true;
}

void VariableSyncer::sync() {
	clock_t time = clock();
	if ((time - m_latestSyncTime) / 1000.f > m_syncInterval) {
		m_latestSyncTime = time;
		// sync
		ErrorLogger::log("log");
		for (size_t i = 0; i < files.size(); i++) {
			files[i].sync();
		}
	}
}

void VariableSyncer::writeAll() {
	for (size_t i = 0; i < files.size(); i++) {
		files[i].writeFile();
	}
}

VariableSyncer::FileSyncer::FileVariable::FileVariable(string name) { m_varName = name; }
