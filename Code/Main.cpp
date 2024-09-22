#include "Game.hpp"
#include "Hooking.hpp"
#include "D3DRenderer.hpp"

BOOL DllMain(HINSTANCE hInstance, DWORD reason, LPVOID)
{
	using namespace Dll;

	if (reason == DLL_PROCESS_ATTACH)
	{
		g_Module = hInstance;
		CreateThread(nullptr, 0, [](LPVOID) -> DWORD
		{
			g_Logger = std::make_unique<Logger>();
			g_Logger->Info("MoneyTool injected.");

			g_Pointers = std::make_unique<Pointers>();
			g_D3DRenderer = std::make_unique<D3DRenderer>();
			g_Hooking = std::make_unique<Hooking>();
			g_Hooking->Hook();

			g_Logger->Info("MoneyTool loaded.");

			while (g_Running)
			{
				if (IsKeyPressed(VK_DELETE))
					g_Running = false;

				std::this_thread::sleep_for(3ms);
				std::this_thread::yield();
			}

			std::this_thread::sleep_for(500ms);
			g_Hooking->Unhook();
			g_D3DRenderer.reset();
			std::this_thread::sleep_for(500ms);
			g_Hooking.reset();
			g_Pointers.reset();
			g_Logger.reset();
			FreeLibraryAndExitThread(g_Module, 0);
		}, nullptr, 0, nullptr);
	}

	return true;
}
