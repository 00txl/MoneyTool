#pragma once
#include "Types.hpp"

namespace rage
{
	struct scrNativeRegistration;
}

namespace Types {
	using NetShopTransactionBasketAdd = bool(*)(CNetShopTransactionBasket* _this, const CNetShopItem& item);
	using NetShopTransactionBaseRegisterPendingCash = bool(*)(CNetShopTransactionBase* _this);
	using NetworkShoppingMgrFindBasket = CNetShopTransactionBase * (*)(CNetworkShoppingMgr* _this, NetShopTransactionId& id);
	using NetworkShoppingMgrCreateBasket = bool(*)(CNetworkShoppingMgr* pTransactionMgr, NetShopTransactionId& pId, u32 Category, u32 Action, u32 Flags);
	using NetworkShoppingMgrCreateFreeSpaces = bool(*)(CNetworkShoppingMgr* _this, bool removeAlsofailed);
	using NetworkShoppingMgrAppendNewNode = bool(*)(CNetworkShoppingMgr* _this, CNetShopTransactionBase* transaction);
	using ConstructNetShopTransactionBasket = CNetShopTransactionBasket * (*)(CNetShopTransactionBasket* _this);
	using PendingCashReductionsHelperRegister = void(*)(PendingCashReductionsHelper* _this, const int id, const s64 wallet, const s64 bank, const bool evconly);
	using NetworkShoppingMgrIsBasketFull = bool(*)(CNetworkShoppingMgr* _this);
}

namespace Dll
{

	class Pointers
	{
	public:
		explicit Pointers();
		~Pointers() noexcept = default;
		Pointers(Pointers const&) = delete;
		Pointers(Pointers&&) = delete;
		Pointers& operator=(Pointers const&) = delete;
		Pointers& operator=(Pointers&&) = delete;

		using IsDlcPresent = bool(std::uint32_t hash);
		IsDlcPresent* m_IsDlcPresent;

		using WndProc = LRESULT(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
		WndProc* m_WndProc;

		using GetEventData = bool(std::int32_t eventGroup, std::int32_t eventIndex, std::int64_t* args, std::uint32_t argCount);
		GetEventData* m_GetEventData;

		using GetLabelText = const char*(void* unk, const char* label);
		GetLabelText* m_GetLabelText;

		Types::NetShopTransactionBasketAdd m_NetShopTransactionBasketAdd{};
		Types::NetShopTransactionBaseRegisterPendingCash m_NetShopTransactionBaseRegisterPendingCash{};
		Types::NetworkShoppingMgrCreateBasket m_NetworkShoppingMgrCreateBasket{};


		std::uint32_t* m_GameState;
		std::uint32_t* m_FrameCount;
		HWND m_GameWindow;
		IDXGISwapChain* m_Swapchain;
		rage::scrNativeRegistration** m_NativeRegistrations;
		std::uint64_t** m_GlobalBase;
		char* m_GameBuild;

		CNetworkShoppingMgr** m_ShoppingMgr{};
		Types::PendingCashReductionsHelperRegister m_PendingCashReductionsHelperRegister{};

		PVOID m_GetBattleEyeInitState{};
		PVOID m_CanAccessNetworkFeatures{};
		PVOID m_NetworkBail{};
	};

	inline std::unique_ptr<Pointers> g_Pointers;

	bool TansactionAdvanced(std::string category, std::string action, std::string id, int amount, int flag, u32 quanity);
}
