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

#define STANDARD_WIDTH 1280
#define STANDARD_HEIGHT 720
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