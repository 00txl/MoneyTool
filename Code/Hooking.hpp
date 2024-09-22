#pragma once
#include "VMTHook.hpp"

namespace Dll
{
	struct Hooks
	{
		static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

		static constexpr auto PresentIndex = 8;
		static HRESULT Present(IDXGISwapChain* dis, UINT syncInterval, UINT flags);

		static constexpr auto ResizeBuffersIndex = 13;
		static HRESULT ResizeBuffers(IDXGISwapChain* dis, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags);

		static int GetBattleEyeInitState();
		static bool CanAccessNetworkFeatures(const eNetworkAccessArea accessArea, eNetworkAccessCode* nAccessCode);
		static bool NetworkBail(const void* BailParams, bool bSendScriptEvent);
	};

	class Hooking
	{
	public:
		explicit Hooking();
		~Hooking() noexcept;
		Hooking(Hooking const&) = delete;
		Hooking(Hooking&&) = delete;
		Hooking& operator=(Hooking const&) = delete;
		Hooking& operator=(Hooking&&) = delete;

		void Hook();
		void Unhook();

		friend struct Hooks;
	private:
		void* m_OriginalWndProc{};

		void* m_OriginalGetBattleEyeInitState{};
		void* m_OriginalCanAccessNetworkFeatures{};
		void* m_OriginalNetworkBail{};

		VMTHook m_D3DHook;
	};

	inline std::unique_ptr<Hooking> g_Hooking;
}
