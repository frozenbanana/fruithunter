#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <WRL/client.h>
#include <SimpleMath.h>

using namespace std;
using namespace DirectX;
using namespace SimpleMath;

using float2 = DirectX::SimpleMath::Vector2;
using float3 = DirectX::SimpleMath::Vector3;
using float4 = DirectX::SimpleMath::Vector4;
using float4x4 = DirectX::SimpleMath::Matrix;

enum FruitType { APPLE, BANANA, MELON, NR_OF_FRUITS };
enum TimeTargets { GOLD, SILVER, BRONZE, NR_OF_TIME_TARGETS };
enum Skillshot { SS_NOTHING, SS_BRONZE, SS_SILVER, SS_GOLD };

// Helper Math functions
/* Generate a random float between low to high */
static float RandomFloat(float low = 0.f, float high = 1.f) {
	float randomCoefficent = (float)(rand() % (int)100.f) / 100.f; // normalize
	return low + randomCoefficent * (high - low);
}

/* Map value from interval [low, high] to new value corresponding to interval [newLow, newHigh] */
static float Map(float low, float high, float newLow, float newHigh, float value) {
	float oldCoefficient = (value / (low + (high - low)));
	float newRange = (newHigh - newLow) + newLow;
	return oldCoefficient * newRange;
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