#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <WRL/client.h>
#include <SimpleMath.h>

#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imgui_stdlib.h"
#include "implot.h"
#include "implot_internal.h"

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

#define GAME_VERSION 1
#define GAME_PATCH 1

#define DEBUG false

enum AreaTag { Forest, Plains, Desert, Volcano, NR_OF_AREAS };
enum FruitType { APPLE, BANANA, MELON, DRAGON, NR_OF_FRUITS };
enum TimeTargets { GOLD, SILVER, BRONZE, NR_OF_TIME_TARGETS };
enum Skillshot { SS_NOTHING, SS_BRONZE, SS_SILVER, SS_GOLD };

static string AreaTagToString(AreaTag tag) {
	static string str[AreaTag::NR_OF_AREAS] = { "Forest", "Plains", "Desert", "Volcano" };
	return str[tag];
}
static string FruitTypeToString(FruitType type) {
	static string str[FruitType::NR_OF_FRUITS] = { "Apple", "Banana", "Melon", "Dragon" };
	return str[type];
}
static string TimeTargetToString(TimeTargets target) {
	static string str[TimeTargets::NR_OF_TIME_TARGETS] = {"Gold", "Silver", "Bronze"};
	return str[target];
}

static std::wstring s2ws(const std::string& s) {
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
static string LPWSTR_to_STRING(LPWSTR str) {
	wstring ws(str);

	// convert from wide char to narrow char array
	char ch[256];
	char DefChar = ' ';
	WideCharToMultiByte(CP_ACP, 0, str, -1, ch, 256, &DefChar, NULL); // No error checking

	std::string sbuff = std::string(ch);

	return sbuff;
}

/* Create Pitch Yaw Roll matrix */
static float4x4 CreatePYRMatrix(float3 rotation) {
	return float4x4::CreateRotationZ(rotation.z) * float4x4::CreateRotationX(rotation.x) *
		   float4x4::CreateRotationY(rotation.y);
}

static string Seconds2DisplayableString(size_t timeSeconds) {
	size_t total = timeSeconds;
	size_t seconds = total % 60;
	size_t minutes = total / 60;
	return (minutes < 10 ? "0" : "") + to_string(minutes) + ":" + (seconds < 10 ? "0" : "") +
		   to_string(seconds);
}
static string Milliseconds2DisplayableString(size_t timeMs) {
	size_t total = timeMs;
	size_t milliseconds = total % 1000;
	size_t seconds = (total / 1000) % 60;
	size_t minutes = (total / 1000) / 60;
	return 
		(minutes < 10 ? "0" : "") + to_string(minutes) + ":" + 
		(seconds < 10 ? "0" : "") + to_string(seconds) + "." + 
		(milliseconds < 10 ? "00" : (milliseconds < 100 ? "0" : "")) + to_string(milliseconds);
}

// Helper Math functions
/* Generate a random float between low to high */
static float RandomFloat(float low = 0.f, float high = 1.f) {
	float randomCoefficent = (float)(rand() % RAND_MAX) / (float)RAND_MAX; // normalize
	return low + randomCoefficent * (high - low);
}

/* Rotate float2 value */
static float2 rotatef2(float2 v, float rad) {
	float c = cos(rad), s = sin(rad);
	return float2(v.x * c - v.y * s, v.x * s + v.y * c);
}
static float3 rotatef2X(float3 v, float rad) {
	float2 r = rotatef2(float2(v.z, v.y), rad);
	return float3(v.x, r.y, r.x);
}
static float3 rotatef2Y(float3 v, float rad) {
	float2 r = rotatef2(float2(v.x, v.z), rad);
	return float3(r.x, v.y, r.y);
}
static float3 rotatef2Z(float3 v, float rad) {
	float2 r = rotatef2(float2(v.x, v.y), rad);
	return float3(r.x, r.y, v.z);
}
static float3 rotatef3(float3 v, float3 rot) {
	float4x4 rotMat = float4x4::CreateRotationZ(rot.z) * float4x4::CreateRotationX(rot.x) *
					  float4x4::CreateRotationY(rot.y);
	return float3::Transform(v, rotMat);
}

/* Return length of direction until collision. Return 0 if no collision */
static float RayPlaneIntersection(float3 rayPoint, float3 rayDirection, float3 planePosition, float3 planeNormal) {
	float distanceTowardsNormal = rayDirection.Dot(planeNormal);
	if (distanceTowardsNormal == 0)
		return 0; // no intersection
	float distanceToPlane = planeNormal.Dot(planePosition - rayPoint);
	return distanceToPlane / distanceTowardsNormal;
}

/*  */
template <typename VECTOR> static VECTOR Normalize(VECTOR v) { 
	v.Normalize();
	return v; 
}

/* Map value from interval [low, high] to new value corresponding to interval [newLow, newHigh] */
static float Map(float low, float high, float newLow, float newHigh, float value) {
	float oldCoefficient = (value / (low + (high - low)));
	float newRange = (newHigh - newLow) + newLow;
	return oldCoefficient * newRange;
}

//static float Clamp(float val, float low, float high) { return min(max(val, low), high); }

template <typename TYPE> static TYPE Clamp(TYPE v, TYPE min, TYPE max) {
	return (v > max ? max : (v < min ? min : v));
}

/* Lerp between two values. Parameters require addition and multiplication operators! */
template <typename VARTYPE> static VARTYPE lerp(VARTYPE t1, VARTYPE t2, float mix) {
	return t1 * (1 - mix) + t2 * mix;
}

/* Modulus operation that also affects negative values */
static float mod(float v, float mod) { 
	v = fmod(v, mod);
	if (v < 0)
		v = mod + v;
	return v;
}

static float3 vector2Rotation(float3 direction) {
	float3 dir = direction;
	dir.Normalize();

	float2 mapY(dir.z, dir.x);
	float2 mapX(float2(dir.x, dir.z).Length(), dir.y);
	mapY.Normalize();
	mapX.Normalize();
	float rotY = (mapY.y >= 0) ? (acos(mapY.x)) : (-acos(mapY.x));
	float rotX = -((mapX.y >= 0) ? (acos(mapX.x)) : (-acos(mapX.x)));
	return float3(rotX, rotY, 0);
}

struct FrustumPlane {
	float3 m_position, m_normal;
	FrustumPlane(float3 position = float3(0, 0, 0), float3 normal = float3(0, 0, 0)) {
		m_position = position;
		m_normal = normal;
		m_normal.Normalize();
	}
};
struct CubeBoundingBox {
	// Position is the corner of the box. 
	// Position+Size is opposite corner of the box.
	float3 m_position, m_size;
	float3 getCenter() const { return m_position + m_size / 2.f; }
	CubeBoundingBox(float3 position = float3(0, 0, 0), float3 size = float3(0, 0, 0)) {
		m_position = position;
		m_size = size;
	}
	CubeBoundingBox(const vector<float3>& points) {
		bool setMin[3] = { 0, 0, 0 }, setMax[3] = { 0, 0, 0 }; // 0 = unset, 1 = set
		float min[3] = { 0 }, max[3] = { 0 };
		for (size_t i = 0; i < points.size(); i++) {
			float3 p = points[i];
			float ps[3] = { p.x, p.y, p.z };
			for (int j = 0; j < 3; j++) {
				if (setMin[j] == 0 || (ps[j] < min[j])) {
					setMin[j] = 1;
					min[j] = ps[j];
				}
				if (setMax[j] == 0 || (ps[j] > max[j])) {
					setMax[j] = 1;
					max[j] = ps[j];
				}
			}
		}
		m_position = float3(min[0], min[1], min[2]);
		m_size = float3(max[0] - min[0], max[1] - min[1], max[2] - min[2]);
	}
};