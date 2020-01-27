#include "Model.h"
#include "Model.h"

void Model::updateColorBuffer(float3 color) {
	ID3D11DeviceContext* gDeviceContext = Renderer::getDeviceContext();

	float4 col(color.x, color.y, color.z, 1);
	gDeviceContext->UpdateSubresource(gColorBuffer, 0, 0, &col, 0, 0);
}
