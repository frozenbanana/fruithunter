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
#define STANDARD_CORNER_X 100
#define STANDARD_CORNER_Y 100

#define NEAR_PLANE 0.05f
#define FAR_PLANE 100.f

enum FruitType { APPLE, BANANA, MELON, NR_OF_FRUITS };