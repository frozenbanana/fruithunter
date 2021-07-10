#pragma once
#include <fstream>
#include <string>
#include <vector>

template <typename VAR> static void fileWrite(ofstream& file, const VAR& variable) {
	file.write((char*)&variable, sizeof(VAR));
}
template <typename VAR> static void fileRead(ifstream& file, VAR& variable) {
	file.read((char*)&variable, sizeof(VAR));
}
template <typename VAR> static VAR fileRead(ifstream& file) {
	VAR ret;
	file.read((char*)&ret, sizeof(VAR));
	return ret;
}
static void fileWrite(ofstream& file, const string& str) {
	size_t length = str.length();
	fileWrite<size_t>(file, length);
	file.write(str.c_str(), length);
}
static void fileRead(ifstream& file, string& str) {
	size_t length = fileRead<size_t>(file);
	char* text = new char[length];
	file.read(text, length);
	str.clear();
	str.insert(0, text, length);
	delete[] text;
}

template <typename VAR> static void fileWrite(ofstream& file, const vector<VAR>& vector) {
	fileWrite<size_t>(file, vector.size());
	for (size_t i = 0; i < vector.size(); i++)
		fileWrite<VAR>(file, vector[i]);
}
template <typename VAR> static void fileRead(ifstream& file, vector<VAR>& vector) {
	size_t size = fileRead<size_t>(file);
	vector.resize(size);
	for (size_t i = 0; i < size; i++)
		fileRead<VAR>(file, vector[i]);
}