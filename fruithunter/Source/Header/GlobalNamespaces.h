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

#define STANDARD_WIDTH 1920
#define STANDARD_HEIGHT 1080
#define STANDARD_CORNER_X 0
#define STANDARD_CORNER_Y 0

#define NEAR_PLANE 0.1f
#define FAR_PLANE 100.f

enum FruitType { APPLE, BANANA, MELON, NR_OF_FRUITS };
enum TimeTargets { GOLD, SILVER, BRONZE, NR_OF_TIME_TARGETS };

static float RandomFloat(float low = 0.f, float high = 1.f) {
	float randomCoefficent = (float)(rand() % (int)100.f) / 100.f; // normalize
	return low + randomCoefficent * (high - low);
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
	CubeBoundingBox(float3 position = float3(0, 0, 0), float3 size = float3(0, 0, 0)) {
		m_position = position;
		m_size = size;
	}
	CubeBoundingBox(const vector<float3>& points) {
		bool setMin[3] = { 0, 0, 0 }, setMax[3] = { 0, 0, 0 }; // 0 = unset, 1 = set
		float min[3], max[3];
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