#include "Hooking.hpp"
#include "Game.hpp"
#include "Util.hpp"
#include "D3DRenderer.hpp"
#include <MinHook/MinHook.h>
#include "ImGui/imgui.h"

#pragma comment(lib, "MinHook.lib")
#pragma comment(lib, "winmm.lib")

namespace Dll
{
	LRESULT Hooks::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		g_D3DRenderer->WndProc(hWnd, msg, wParam, lParam);
		return static_cast<decltype(&WndProc)>(g_Hooking->m_OriginalWndProc)(hWnd, msg, wParam, lParam);
	}

	HRESULT Hooks::Present(IDXGISwapChain* dis, UINT syncInterval, UINT flags)
	{
		if (g_Running && g_Settings.m_LockMouse)
		{
			g_D3DRenderer->BeginFrame();
			ImGui::Begin("Money Tool");
			if (ImGui::Button("180K [Unlimited]"))
			{
				TansactionAdvanced("CATEGORY_SERVICE_WITH_THRESHOLD", "NET_SHOP_ACTION_EARN", "SERVICE_EARN_JUGGALO_STORY_MISSION", 180000, 4, 1);
			}
			if (ImGui::Button("3.6M Casino Heist"))
			{
				TansactionAdvanced("CATEGORY_SERVICE_WITH_THRESHOLD", "NET_SHOP_ACTION_EARN", "SERVICE_EARN_CASINO_HEIST_FINALE", 3619000, 4, 1);
			}
			if (ImGui::Button("7M Criminal Mastermind"))
			{
				TansactionAdvanced("CATEGORY_SERVICE_WITH_THRESHOLD", "NET_SHOP_ACTION_EARN", "SERVICE_EARN_GANGOPS_AWARD_MASTERMIND_4", 7000000, 4, 1);
			}
			if (ImGui::Button("15M Bend Job"))
			{
				TansactionAdvanced("CATEGORY_SERVICE_WITH_THRESHOLD", "NET_SHOP_ACTION_EARN", "SERVICE_EARN_BEND_JOB", 15000000, 4, 1);
			}
			ImGui::End();
			g_D3DRenderer->EndFrame();
		}

		return g_Hooking->m_D3DHook.GetOriginal<decltype(&Present)>(PresentIndex)(dis, syncInterval, flags);
	}

	HRESULT Hooks::ResizeBuffers(IDXGISwapChain* dis, UINT bufferCount, UINT width, UINT height, DXGI_FORMAT newFormat, UINT swapChainFlags)
	{
		if (g_Running)
		{
			g_D3DRenderer->PreResize();
			auto hr = g_Hooking->m_D3DHook.GetOriginal<decltype(&ResizeBuffers)>(ResizeBuffersIndex)(dis, bufferCount, width, height, newFormat, swapChainFlags);
			if (SUCCEEDED(hr))
			{
				g_D3DRenderer->PostResize();
			}

			return hr;
		}

		return g_Hooking->m_D3DHook.GetOriginal<decltype(&ResizeBuffers)>(ResizeBuffersIndex)(dis, bufferCount, width, height, newFormat, swapChainFlags);
	}

	int Hooks::GetBattleEyeInitState() 
	{
		return false;
	}

	bool Hooks::CanAccessNetworkFeatures(const eNetworkAccessArea accessArea, eNetworkAccessCode* nAccessCode) 
	{
		if (nAccessCode)
			*nAccessCode = eNetworkAccessCode::Access_Granted;
		return true;
	}

	bool Hooks::NetworkBail(const void* bailParams, bool bSendScriptEvent)
	{
		return false;
	}

	Hooking::Hooking():
		m_D3DHook(g_Pointers->m_Swapchain, 18)
	{
		MH_Initialize();
		MH_CreateHook(g_Pointers->m_WndProc, &Hooks::WndProc, &m_OriginalWndProc);
		MH_CreateHook(g_Pointers->m_GetBattleEyeInitState, &Hooks::GetBattleEyeInitState, &m_OriginalGetBattleEyeInitState);
		MH_CreateHook(g_Pointers->m_CanAccessNetworkFeatures, &Hooks::CanAccessNetworkFeatures, &m_OriginalCanAccessNetworkFeatures);
		MH_CreateHook(g_Pointers->m_NetworkBail, &Hooks::NetworkBail, &m_OriginalNetworkBail);

		m_D3DHook.Hook(&Hooks::Present, Hooks::PresentIndex);
		m_D3DHook.Hook(&Hooks::ResizeBuffers, Hooks::ResizeBuffersIndex);
	}

	Hooking::~Hooking() noexcept
	{
		MH_RemoveHook(g_Pointers->m_WndProc);
		MH_Uninitialize();
	}

	void Hooking::Hook()
	{
		m_D3DHook.Enable();
		MH_EnableHook(MH_ALL_HOOKS);
	}

	void Hooking::Unhook()
	{
		m_D3DHook.Disable();
		MH_DisableHook(MH_ALL_HOOKS);
	}
}
