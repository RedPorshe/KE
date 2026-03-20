#pragma once

#include "KE/EngineObject.h"

struct GLFWwindow;

struct KE_API WindowExtent
	{
	uint32_t Width { 800 };
	uint32_t Height { 600 };
	float AspectRatio = static_cast< float >( Width / Height);
	};

class KE_API WindowSystem : public IEngineSystem
	{
	public:
		WindowSystem () = default;
		virtual ~WindowSystem () = default;

		bool PreInit () override;
		bool Init () override;
		void Shutdown () override;
		void Update ( float DeltaTime ) override;
		void SetTitle ( const std::string & inTitle ) { m_title = inTitle; }
		void SetFullscreen ( bool fullscreen ) { bIsFullscreen = fullscreen; }
		const std::string GetSystemName () const override;
		WindowExtent  GetExtent () const { return Extent; }
		float GetAspectRatio () const  { return Extent.AspectRatio; }
		void SetExtent ( uint32_t inWidth, uint32_t inHeight )
			{
			Extent.Width = inWidth;     
			Extent.Height = inHeight;
			Extent.AspectRatio = static_cast< float >( inWidth ) / static_cast< float >( inHeight );
			}
		GLFWwindow * GetWindowHandle () const { return m_WindowHandle; }
		void ShowCursor ( bool show );
		std::string GetTitle () const { return m_title; }
	private:
		GLFWwindow * m_WindowHandle = nullptr;
		bool bIsFullscreen { false };
		WindowExtent Extent {};
		std::string m_title { "Kuzbass Engine" };
	};