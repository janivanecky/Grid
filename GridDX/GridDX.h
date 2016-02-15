#pragma once
#include <DrawingSurfaceNative.h>
#include <queue>
#include "main.h"

namespace GridDX
{
    public delegate void RequestAdditionalFrameHandler();
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Direct3DBackground sealed: public Windows::Phone::Input::Interop::IDrawingSurfaceManipulationHandler
    {
    public:
        Direct3DBackground();
        Windows::Phone::Graphics::Interop::IDrawingSurfaceBackgroundContentProvider^ CreateContentProvider();
        event RequestAdditionalFrameHandler^ RequestAdditionalFrame;

		virtual void SetManipulationHost(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ manipulationHost);

		
		property Windows::Foundation::Size NativeResolution
		{
			Windows::Foundation::Size get(){ return m_nativeResolution; }
			void set(Windows::Foundation::Size nativeResolution);
		}

		bool OnBackButtonPressed();
    internal:
        HRESULT Connect(_In_ IDrawingSurfaceRuntimeHostNative* host, _In_ ID3D11Device1* device);
        void Disconnect();

        HRESULT PrepareResources(_In_ const LARGE_INTEGER* presentTargetTime, _Inout_ DrawingSurfaceSizeF* desiredRenderTargetSize);
        HRESULT Draw(_In_ ID3D11Device1* device, _In_ ID3D11DeviceContext1* context, _In_ ID3D11RenderTargetView* renderTargetView);
	private:
		void OnPointerPressed(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerMoved(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		void OnPointerReleased(Windows::Phone::Input::Interop::DrawingSurfaceManipulationHost^ sender, Windows::UI::Core::PointerEventArgs^ args);
		float GetTimeFromTicks(uint64 ticks);
		uint64 GetTicks();
		
		uint64 lastTicks;
		float tickFrequency;
		GameInput input1;
		GameInput input2;
		GameInput *newInput;
		Context context;
		Windows::Foundation::Size m_nativeResolution;
    };
}
