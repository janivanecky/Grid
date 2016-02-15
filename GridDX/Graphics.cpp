#include "Graphics.h"
#include "DDSTextureLoader.h"
#include "DirectXMath.h"
#include <stdio.h>



void *WPReadFile(LPCWSTR fileName, DWORD *bytesRead)
{
	HANDLE fileHandle = 0;
	fileHandle = CreateFile2(fileName, GENERIC_READ, NULL, OPEN_EXISTING, NULL);
	void *result = NULL;
	if (fileHandle != INVALID_HANDLE_VALUE)
	{
		WIN32_FILE_ATTRIBUTE_DATA fileAttributes;
		if(GetFileAttributesExW(fileName, GetFileExInfoStandard, &fileAttributes))
		{
			uint32 fileSize = fileAttributes.nFileSizeLow;
			HANDLE heap = GetProcessHeap();
			result = HeapAlloc(heap, HEAP_ZERO_MEMORY, fileSize);
			if (result)
			{
				DWORD bytesReadFromFile = 0;
				if (ReadFile(fileHandle, result, fileSize, &bytesReadFromFile, NULL) &&
					bytesReadFromFile == fileSize)						
				{
					*bytesRead = bytesReadFromFile;
				}
				else
				{
					HeapFree(heap, NULL, result);
				}
			}
		}
		CloseHandle(fileHandle);
	}
	return result;
}

struct QuadVertexData
{
	DirectX::XMFLOAT3 position;
};


struct SpriteVertexData
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 texcoord;
};

void InitShaderPass(ID3D11Device *device, LPCWSTR vertexShaderFile, LPCWSTR pixelShaderFile, D3D11_INPUT_ELEMENT_DESC *inputDesc,
					uint32 inputElementsCount, ID3D11VertexShader **vertexShader, 
					ID3D11PixelShader **pixelShader, ID3D11InputLayout **inputLayout)
{
	DWORD bytesReadVertexShader = 0;
	void *vertexShaderData = WPReadFile(vertexShaderFile, &bytesReadVertexShader);
	if (!vertexShaderData)
	{
		return;
	}

	DWORD bytesReadPixelShader = 0;
	void *pixelShaderData = WPReadFile(pixelShaderFile, &bytesReadPixelShader);
	if (!pixelShaderData)
	{
		HANDLE heap = GetProcessHeap();
		HeapFree(heap, NULL, vertexShaderData);
		return;
	}

	HANDLE heap = GetProcessHeap();
	HRESULT hr = device->CreateVertexShader(vertexShaderData, bytesReadVertexShader, 
										 NULL, vertexShader);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating vertex shader\n");
	}

	
	hr = device->CreateInputLayout(inputDesc, inputElementsCount, 
										vertexShaderData, bytesReadVertexShader,
										inputLayout);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating input layout\n");
	}
	HeapFree(heap, NULL, vertexShaderData);

	hr = device->CreatePixelShader(pixelShaderData, bytesReadPixelShader,
										NULL, pixelShader);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating pixel shader\n");
	}
	HeapFree(heap, NULL, pixelShaderData);
}

void Graphics::InitRenderPass(Renderer *renderer, RenderPass *renderPass, LPCWSTR vertexShader, LPCWSTR pixelShader, 
							  float renderTargetWidth, float renderTargetHeight)
{ 
	ID3D11Device *device = renderer->device;
	D3D11_INPUT_ELEMENT_DESC positionDescSprite = {};
	positionDescSprite.SemanticName = "POSITION";
	positionDescSprite.SemanticIndex = 0;
	positionDescSprite.Format = DXGI_FORMAT_R32G32B32_FLOAT;
	positionDescSprite.InputSlot = 0;
	positionDescSprite.AlignedByteOffset = 0;
	positionDescSprite.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	positionDescSprite.InstanceDataStepRate = 0;

	D3D11_INPUT_ELEMENT_DESC texcoordDescSprite = {};
	texcoordDescSprite.SemanticName = "TEXCOORD";
	texcoordDescSprite.SemanticIndex = 0;
	texcoordDescSprite.Format = DXGI_FORMAT_R32G32_FLOAT;
	texcoordDescSprite.InputSlot = 0;
	texcoordDescSprite.AlignedByteOffset = 12;
	texcoordDescSprite.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	texcoordDescSprite.InstanceDataStepRate = 0;

	D3D11_INPUT_ELEMENT_DESC vertexDescSprite[] = { positionDescSprite, texcoordDescSprite };

	InitShaderPass(device, vertexShader, pixelShader, vertexDescSprite, 2, &renderPass->vertexShader,
				   &renderPass->pixelShader, &renderPass->inputLayout);


	static const SpriteVertexData spriteQuad[] = 
	{
		{ DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 1.0f) },
		{ DirectX::XMFLOAT3(1.0f, 0.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 1.0f) },
		{ DirectX::XMFLOAT3(1.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(1.0f, 0.0f) },
		{ DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f), DirectX::XMFLOAT2(0.0f, 0.0f) },
	};

	D3D11_SUBRESOURCE_DATA vertexBufferDataSprite = {};
	vertexBufferDataSprite.pSysMem = spriteQuad;

	CD3D11_BUFFER_DESC vertexBufferDescSprite(sizeof(spriteQuad), D3D11_BIND_VERTEX_BUFFER);
	HRESULT hr = device->CreateBuffer(&vertexBufferDescSprite, &vertexBufferDataSprite, &renderPass->vertexBuffer);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating vertex buffer\n");
	}
	renderPass->vertexStride = sizeof(SpriteVertexData);

	static const unsigned short spriteQuadIndicesCounterClockwise[] = 
	{
		2, 1, 0,
		0, 3, 2
	};

	static const unsigned short spriteQuadIndicesClockwise[] = 
	{
		0, 1, 2,
		2, 3, 0
	};

#define spriteQuadIndices spriteQuadIndicesCounterClockwise
	renderPass->indexCount = ARRAYSIZE(spriteQuadIndices);

	D3D11_SUBRESOURCE_DATA indexBufferDataSprite = {0};
	indexBufferDataSprite.pSysMem = spriteQuadIndices;
	CD3D11_BUFFER_DESC indexBufferDescSprite(sizeof(spriteQuadIndices), D3D11_BIND_INDEX_BUFFER);

	hr = device->CreateBuffer(&indexBufferDescSprite, &indexBufferDataSprite, &renderPass->indexBuffer);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating index buffer\n");
	}

	CD3D11_BUFFER_DESC perSceneBufferDesc(sizeof(PerSceneBufferData), D3D11_BIND_CONSTANT_BUFFER);
	hr = device->CreateBuffer(&perSceneBufferDesc, NULL, &renderPass->perSceneBuffer);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating constant buffer\n");
	}

	CD3D11_BUFFER_DESC perModelBufferDesc(sizeof(PerModelBufferData), D3D11_BIND_CONSTANT_BUFFER);
	hr = device->CreateBuffer(&perModelBufferDesc, NULL, &renderPass->perModelBuffer);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating constant buffer\n");
	}

	if (renderTargetHeight > 0 && renderTargetWidth > 0)
	{
		D3D11_TEXTURE2D_DESC textureDesc = { 0 };
		textureDesc.Width = (UINT)renderTargetWidth;
		textureDesc.Height = (UINT)renderTargetHeight;
		textureDesc.MipLevels = 1;
		textureDesc.ArraySize = 1;
		textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		textureDesc.SampleDesc.Count = 1;
		textureDesc.SampleDesc.Quality = 0;
		textureDesc.Usage = D3D11_USAGE_DEFAULT;
		textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		ID3D11Texture2D *texture;
		hr = device->CreateTexture2D(&textureDesc, NULL, &texture);
		if (hr == S_OK)
		{
			D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
			renderTargetViewDesc.Format = textureDesc.Format;
			renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
			renderTargetViewDesc.Texture2D.MipSlice = 0;
			hr = device->CreateRenderTargetView(texture, &renderTargetViewDesc, &renderPass->renderTarget);
			if (hr != S_OK)
			{
				OutputDebugStringA("Error creating render target\n");
			}

			D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
			shaderResourceViewDesc.Format = textureDesc.Format;
			shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
			shaderResourceViewDesc.Texture2D.MipLevels = 1;

			hr = device->CreateShaderResourceView(texture, &shaderResourceViewDesc, &renderPass->renderTargetTexture.data);
			if (hr == S_OK)
			{
				renderPass->renderTargetTexture.width = renderTargetWidth;
				renderPass->renderTargetTexture.height = renderTargetHeight;
			}
			else
			{
				OutputDebugStringA("Error creating render target texture\n");
			}
		}
	}
}

void Graphics::InitRenderer(Renderer *renderer)
{
	ID3D11Device *device = renderer->device;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;
	HRESULT hr = device->CreateSamplerState(&samplerDesc, &renderer->sampler);

	D3D11_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = { 0 };
	renderTargetBlendDesc.BlendEnable = true;
	renderTargetBlendDesc.SrcBlend = D3D11_BLEND_SRC_ALPHA;
	renderTargetBlendDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	renderTargetBlendDesc.BlendOp = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
	renderTargetBlendDesc.DestBlendAlpha = D3D11_BLEND_ONE;
	renderTargetBlendDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	renderTargetBlendDesc.RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	D3D11_BLEND_DESC blendDesc = { 0 };
	blendDesc.RenderTarget[0] = renderTargetBlendDesc;

	renderer->device->CreateBlendState(&blendDesc, &renderer->blendState);

	// Debug PASS
	
	D3D11_INPUT_ELEMENT_DESC debugPositionDesc = {};
	debugPositionDesc.SemanticName = "POSITION";
	debugPositionDesc.SemanticIndex = 0;
	debugPositionDesc.Format = DXGI_FORMAT_R32G32_FLOAT;
	debugPositionDesc.InputSlot = 0;
	debugPositionDesc.AlignedByteOffset = 0;
	debugPositionDesc.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	debugPositionDesc.InstanceDataStepRate = 0;

	InitShaderPass(device, L"DebugVertexShader.cso", L"DebugPixelShader.cso", &debugPositionDesc, 1, 
				   &renderer->debugPass.vertexShader, &renderer->debugPass.pixelShader, &renderer->debugPass.inputLayout);

	CD3D11_BUFFER_DESC debugVertexBufferDesc(sizeof(Vector2) * 10000, D3D11_BIND_VERTEX_BUFFER, D3D11_USAGE_DYNAMIC,
											 D3D11_CPU_ACCESS_WRITE);
	hr = renderer->device->CreateBuffer(&debugVertexBufferDesc, NULL, &renderer->debugPass.vertexBuffer);
	if (hr != S_OK)
	{
		OutputDebugStringA("Error creating vertex buffer\n");
	}
	renderer->debugPass.vertexStride = sizeof(Vector2);

}


void Graphics::DrawDebugData(Renderer *renderer, uint32 vertexCount)
{
	ID3D11DeviceContext *context = renderer->context;
	context->VSSetShader(renderer->debugPass.vertexShader, NULL, 0);
	context->IASetInputLayout(renderer->debugPass.inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
	context->PSSetShader(renderer->debugPass.pixelShader, NULL, 0);
	UINT stride = renderer->debugPass.vertexStride;
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &renderer->debugPass.vertexBuffer, &stride, &offset);

	context->Draw(vertexCount, 0);
}

void Graphics::DrawRectangle(Renderer *renderer, RenderPass *renderPass, Vector2 position, float width, float height, Color color)
{
	ID3D11DeviceContext *context = renderer->context;
	PerModelBufferData modelBuffer;
	modelBuffer.model = Math::Transpose(Math::GetTranslation(position.x, position.y, 0) * 
										Math::GetScale(width, height, 0));
	modelBuffer.color = color;
	context->IASetIndexBuffer(renderPass->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
	context->VSSetShader(renderPass->vertexShader, NULL, 0);
	context->UpdateSubresource(renderPass->perModelBuffer, 0, NULL, &modelBuffer, 0, 0);
	context->VSSetConstantBuffers(1, 1, &renderPass->perModelBuffer);
	context->IASetInputLayout(renderPass->inputLayout);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->PSSetShader(renderPass->pixelShader, NULL, 0);
	UINT stride = renderPass->vertexStride;
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &renderPass->vertexBuffer, &stride, &offset);

	context->DrawIndexed(renderPass->indexCount,0,0);
}

void Graphics::RenderTexture(Renderer *renderer, RenderPass *renderPass, Texture *texture, 
							 Vector2 position, Vector2 origin, Color color, float scale)
{
	if (texture)
	{
		Vector2 worldSize = Vector2(texture->width * scale,
									texture->height * scale);
		Vector2 worldPosition = position - Vector2(origin.x * worldSize.x, origin.y * worldSize.y);

		PerModelBufferData modelData;
		modelData.model = Math::Transpose(
			Math::GetTranslation(worldPosition.x, worldPosition.y, 0) *
			Math::GetScale(worldSize.x, worldSize.y, 1.0f));
		modelData.color = color;

		ID3D11DeviceContext *context = renderer->context;
		context->IASetIndexBuffer(renderPass->indexBuffer, DXGI_FORMAT_R16_UINT, 0);
		context->PSSetShader(renderPass->pixelShader, NULL, 0);
		context->PSSetSamplers(0, 1, &renderer->sampler);

		context->VSSetShader(renderPass->vertexShader, NULL, 0);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(renderPass->inputLayout);
		UINT stride = renderPass->vertexStride;
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, &renderPass->vertexBuffer, &stride, &offset);

		context->UpdateSubresource(renderPass->perModelBuffer, 0, NULL, &modelData, 0, 0);
		context->VSSetConstantBuffers(1, 1, &renderPass->perModelBuffer);
		context->PSSetShaderResources(0, 1, &texture->data);
		context->DrawIndexed(renderPass->indexCount, 0, 0);
	}
}

void Graphics::LoadTexture(Renderer *renderer, Texture *texture, char *fileName, char *path, char *extension)
{
	wchar_t wideChar[100];
	swprintf(wideChar, 100, L"%hs%hs.%hs", path, fileName, extension);
	ID3D11Resource *res;
	HRESULT hr = DirectX::CreateDDSTextureFromFile(renderer->device, wideChar, &res, &texture->data);
	if (hr == S_OK)
	{
		D3D11_TEXTURE2D_DESC textureDesc;	
		ID3D11Texture2D *tex = (ID3D11Texture2D *)res;
		tex->GetDesc(&textureDesc);
		texture->width = (float)textureDesc.Width;
		texture->height = (float)textureDesc.Height;
	}
	else
	{
		OutputDebugStringA("Error creating texture from file\n");
		texture->data = NULL;
		texture->width = 0;
		texture->height = 0;
	}
	
}

void Graphics::StartPass(Renderer *renderer, RenderPass *renderPass, Color clearColor)
{
	ID3D11DeviceContext *context = renderer->context;
	float color[4] = {clearColor.r, clearColor.g, clearColor.b, clearColor.a};
	context->ClearRenderTargetView(renderPass->renderTarget, color);
	CD3D11_VIEWPORT viewport(0.0f,0.0f,renderPass->renderTargetTexture.width, renderPass->renderTargetTexture.height);
	context->RSSetViewports(1, &viewport);
	context->OMSetRenderTargets(1, &renderPass->renderTarget, NULL);
	
	context->VSSetConstantBuffers(0, 1, &renderPass->perSceneBuffer);
}

void Graphics::SetColumnMajorProjectionView(Renderer *renderer, RenderPass *renderPass, Matrix4x4 projectionMatrix,
											  Matrix4x4 viewMatrix)
{
	PerSceneBufferData sceneBuffer;
	sceneBuffer.projection = Math::Transpose(projectionMatrix);
	sceneBuffer.view = Math::Transpose(viewMatrix);
	renderer->context->UpdateSubresource(renderPass->perSceneBuffer, 0, NULL, &sceneBuffer, 0, 0);
}

void Graphics::UpdateBlendState(Renderer *renderer)
{
	float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	renderer->context->OMSetBlendState(renderer->blendState, NULL, 0xffffffff);

}