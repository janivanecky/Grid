#include "GridDX.h"
#include "Direct3DContentProvider.h"

using namespace Platform;
using namespace Microsoft::WRL;
using namespace Windows::Phone::Graphics::Interop;

GridDX::Direct3DBackground::Direct3DBackground() 
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	tickFrequency = (float)frequency.QuadPart;
	input1.touchCount = 0;
	input2.touchCount = 0;
	newInput = &input1;

	lastTicks = GetTicks();
}

IDrawingSurfaceBackgroundContentProvider^ GridDX::Direct3DBackground::CreateContentProvider()
{
    ComPtr<Direct3DContentProvider> provider = Make<Direct3DContentProvider>(this);
    return reinterpret_cast<IDrawingSurfaceBackgroundContentProvider^>(provider.Get());
}

HRESULT GridDX::Direct3DBackground::Connect(_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device)
{
	context.renderer.device = device;
	context.screenSize = Vector2(m_nativeResolution.Width, m_nativeResolution.Height);
	InitGraphics(&context);
    return S_OK;
}

void GridDX::Direct3DBackground::Disconnect()
{
}

HRESULT GridDX::Direct3DBackground::PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize)
{
	desiredRenderTargetSize->width = m_nativeResolution.Width;
	desiredRenderTargetSize->height = m_nativeResolution.Height;
    return S_OK;
}

HRESULT GridDX::Direct3DBackground::Draw(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* d3dcontext, _In_ ID3D11RenderTargetView* renderTargetView)
{
	context.renderer.context = d3dcontext;
	context.renderer.device = device;
	context.finalPass.renderTarget = renderTargetView;
	context.finalPass.renderTargetTexture.width = context.screenSize.x;
	context.finalPass.renderTargetTexture.height = context.screenSize.y;
	GameInput *oldInput = newInput;
	if (newInput == &input1)
		newInput = &input2;
	else
		newInput = &input1;
	uint64 newTicks = GetTicks();
	float dt = GetTimeFromTicks(newTicks - lastTicks);
	lastTicks = newTicks;
	Update(&context, oldInput, dt);
	oldInput->touchCount = 0;
	RequestAdditionalFrame();
    return S_OK;
}

void GridDX::Direct3DBackground::SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ inputHost)
{
	inputHost->PointerPressed +=
		ref new Windows::Foundation::TypedEventHandler<Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^, 
		Windows::UI::Core::PointerEventArgs^>(this, &Direct3DBackground::OnPointerPressed);

	inputHost->PointerMoved +=
		ref new Windows::Foundation::TypedEventHandler<Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^, 
		Windows::UI::Core::PointerEventArgs^>(this, &Direct3DBackground::OnPointerMoved);

	inputHost->PointerReleased +=
		ref new Windows::Foundation::TypedEventHandler<Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^, 
		Windows::UI::Core::PointerEventArgs^>(this, &Direct3DBackground::OnPointerReleased);

}

void GridDX::Direct3DBackground::OnPointerPressed(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, 
													   Windows::UI::Core::PointerEventArgs^ args)
{
	Vector2 touchPosition = Vector2(args->CurrentPoint->Position.Y, args->CurrentPoint->Position.X);
	newInput->touches[newInput->touchCount++] = touchPosition;
}

void GridDX::Direct3DBackground::OnPointerMoved(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, 
													 Windows::UI::Core::PointerEventArgs^ args)
{
}

void GridDX::Direct3DBackground::OnPointerReleased(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, 
														Windows::UI::Core::PointerEventArgs^ args)
{
}

bool GridDX::Direct3DBackground::OnBackButtonPressed()
{
	return true;
}

void GridDX::Direct3DBackground::NativeResolution::set(Windows::Foundation::Size nativeResolution)
{
	if (nativeResolution.Width != m_nativeResolution.Width ||
		nativeResolution.Height != m_nativeResolution.Height)
	{
		m_nativeResolution = nativeResolution;
	}
}

inline float GridDX::Direct3DBackground::GetTimeFromTicks(uint64 ticks)
{
	return (float)((double)ticks / (double)tickFrequency);
}

inline uint64 GridDX::Direct3DBackground::GetTicks()
{
	LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
	return ticks.QuadPart;
}