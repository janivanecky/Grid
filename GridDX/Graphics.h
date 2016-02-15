#pragma once
#include "GridMath.h"
#include <D3D11.h>

struct Color
{
	float r;
	float g;
	float b;
	float a;

	Color()
	{
		this->r = 1;
		this->g = 1;
		this->b = 1;
		this->a = 1;
	}

	Color(float r, float g, float b)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = 1;
	}

	Color(float r, float g, float b, float a)
	{
		this->r = r;
		this->g = g;
		this->b = b;
		this->a = a;
	}

	const Color operator*(float x)
	{
		return Color(this->r * x, this->g * x, this->b * x, this->a * x);
	}

	const Color operator+(Color c)
	{
		return Color(this->r + c.r, this->g + c.g, this->b + c.b, this->a + c.a);
	}

	const Color operator+(Color c) const
	{
		return Color(this->r + c.r, this->g + c.g, this->b + c.b, this->a + c.a);
	}

	static Color ByteColor(byte r, byte g, byte b)
	{
		return Color((float)r / 255.0f, (float)g / 255.0f, (float)b / 255.0f, 1.0f);
	}
};

struct PerSceneBufferData
{
	Matrix4x4 projection;
	Matrix4x4 view;
};

struct PerModelBufferData
{
	Matrix4x4 model;
	Color color;
};

struct Texture
{
	ID3D11ShaderResourceView *data;
	float width;
	float height;
};

struct RenderPass
{
	ID3D11VertexShader *vertexShader;
	ID3D11InputLayout *inputLayout;
	ID3D11PixelShader *pixelShader;
	ID3D11Buffer *vertexBuffer;
	ID3D11Buffer *indexBuffer;
	ID3D11Buffer *perSceneBuffer;
	ID3D11Buffer *perModelBuffer;

	ID3D11RenderTargetView *renderTarget;
	Texture renderTargetTexture;
	uint32 vertexStride;
	uint32 indexCount;
};

struct DebugPass
{
	ID3D11VertexShader *vertexShader;
	ID3D11InputLayout *inputLayout;
	ID3D11PixelShader *pixelShader;
	ID3D11Buffer *vertexBuffer;

	uint32 vertexStride;
};

struct Renderer
{
	ID3D11Device *device;
	ID3D11DeviceContext *context;
	ID3D11BlendState *blendState; 
	ID3D11SamplerState *sampler;
	DebugPass debugPass;

	ID3D11RenderTargetView *renderTarget;
};


namespace Graphics
{
	void InitRenderer(Renderer *renderer);
	void InitRenderPass(Renderer *renderer, RenderPass *renderPass, LPCWSTR vertexShader, LPCWSTR pixelShader,
						float renderTargetWidth, float renderTargetHeight);
	void SetColumnMajorProjectionView(Renderer *renderer, RenderPass *renderPass, Matrix4x4 projectionMatrix,
										Matrix4x4 viewMatrix);
	void StartPass(Renderer *renderer, RenderPass *renderPass, Color color);
	void DrawRectangle(Renderer *renderer, RenderPass *renderPass, Vector2 position, float width, float height, Color color);
	void DrawDebugData(Renderer *renderer, uint32 vertexCount);
	void LoadTexture(Renderer *renderer, Texture *texture, char *fileName, char *path, char *extension);
	void RenderTexture(Renderer *renderer, RenderPass *renderPass, Texture *texture, Vector2 position, Vector2 origin, Color color, float size);
	void UpdateBlendState(Renderer *renderer);
}