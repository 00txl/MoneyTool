#include "Game.hpp"

namespace Dll
{
	Pointers::Pointers() :
		m_GameState(Scanner::FindPattern("83 3D ? ? ? ? ? 75 17 8B 43 20 25").Add(2).Rip().Add(1).As<decltype(m_GameState)>()),
		m_FrameCount(Scanner::FindPattern("8B 15 ? ? ? ? 41 FF CF").Add(2).Rip().As<decltype(m_FrameCount)>()),
		m_GameWindow(FindWindowA("grcWindow", "Grand Theft Auto V")),
		m_Swapchain(Scanner::FindPattern("48 8B 0D ? ? ? ? 48 8B 01 44 8D 43 01 33 D2 FF 50 40 8B C8").Add(3).Rip().As<decltype(m_Swapchain)&>()),
		m_NativeRegistrations(Scanner::FindPattern("48 83 EC 20 48 8D 0D ? ? ? ? E8 ? ? ? ? 0F B7 15 ? ? ? ? 33 FF").Add(7).Rip().As<decltype(m_NativeRegistrations)>()),
		m_GlobalBase(Scanner::FindPattern("4C 8D 4D 08 48 8D 15 ? ? ? ? 4C 8B C0").Add(7).Rip().As<decltype(m_GlobalBase)>()),
		m_IsDlcPresent(Scanner::FindPattern("84 C0 74 0D 8D 53 0C").Sub(63).As<decltype(m_IsDlcPresent)>()),
		m_WndProc(Scanner::FindPattern("48 8B 0D ? ? ? ? 48 8D 55 EF FF 15").Sub(901).As<decltype(m_WndProc)>()),
		m_GetEventData(Scanner::FindPattern("48 85 C0 74 14 4C 8B 10").Sub(28).As<decltype(m_GetEventData)>()),
		m_GetLabelText(Scanner::FindPattern("75 ? E8 ? ? ? ? 8B 0D ? ? ? ? 65 48 8B 04 25 ? ? ? ? BA ? ? ? ? 48 8B 04 C8 8B 0C 02 D1 E9").Sub(19).As<decltype(m_GetLabelText)>()),
		m_NetShopTransactionBasketAdd(Scanner::FindPattern("83 79 1C 00 75 50 8A 81 ? ? ? ? 3C 47 73 46 0F B6 C0 4C 8D 04 80").As<decltype(m_NetShopTransactionBasketAdd)>()),
		m_NetShopTransactionBaseRegisterPendingCash(Scanner::FindPattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8B EC 48 83 EC 60 8B 41 24 48 8B D9 40 B7 01").As<decltype(m_NetShopTransactionBaseRegisterPendingCash)>()),
		m_NetworkShoppingMgrCreateBasket(Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 48 89 78 20 41 54 41 56 41 57 48 83 EC 30 48 8B F1").As<decltype(m_NetworkShoppingMgrCreateBasket)>()),
		m_ShoppingMgr(Scanner::FindVariable("48 8B 0D ? ? ? ? 89 74 24 30").As<decltype(m_ShoppingMgr)>()),
		m_PendingCashReductionsHelperRegister(Scanner::FindPattern("48 8B C4 48 89 58 08 48 89 68 10 48 89 70 18 57 48 83 EC 50 83 60 C8 00").As<decltype(m_PendingCashReductionsHelperRegister)>()),
		m_GetBattleEyeInitState(Scanner::FindPattern("E9 ? ? ? ? CC 4A 68 E4 E8 A6 3F").Add(1).Rip().As<decltype(m_GetBattleEyeInitState)>()),
		m_CanAccessNetworkFeatures(Scanner::FindPattern("E8 ? ? ? ? 8B 54 24 30 89 13").Add(1).Rip().As<decltype(m_CanAccessNetworkFeatures)>()),
		m_NetworkBail(Scanner::FindPattern("E8 ? ? ? ? 32 C0 EB 11").Add(1).Rip().As<decltype(m_NetworkBail)>())
	{
		auto sig = Scanner::FindPattern("48 83 EC 60 48 8D 0D ? ? ? ? E8").Sub(17);
		m_GameBuild = sig.Add(265 + 3).Rip().As<decltype(m_GameBuild)>();
	}


	typedef unsigned long _DWORD;
	typedef unsigned char _BYTE;
	typedef unsigned long long _QWORD;


	 CNetShopTransactionBase* NetworkShoppingMgrFindBasket(NetShopTransactionId& id) {
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* pNode{ (*g_Pointers->m_ShoppingMgr)->m_TransactionList.Head };
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* pNextNode{};
		id = NET_SHOP_INVALID_TRANS_ID;
		while (pNode) {
			pNextNode = pNode->Next;
			CNetShopTransactionBase* pEvent{ pNode->Data };
			if (pEvent->m_Type == "NET_SHOP_TTYPE_BASKET"_joaat) {
				id = pEvent->m_Id;
				return pEvent;
			}
			pNode = pNextNode;
		}
		return 0;
	}

	 bool NetGameServerBasketIsActive() {
		NetShopTransactionId transid{ NET_SHOP_INVALID_TRANS_ID };
		CNetShopTransactionBase* basket{ NetworkShoppingMgrFindBasket(transid) };
		return (NULL != basket);
	}

	 bool NetworkShoppingMgrAddItem(CNetShopItem& item) {
		bool result{};
		NetShopTransactionId id{ NET_SHOP_INVALID_TRANS_ID };
		CNetShopTransactionBase* pEvent{ NetworkShoppingMgrFindBasket(id) };
		if (pEvent) {
			if (pEvent->m_Status.None()) {
				CNetShopTransactionBasket* transaction{ (CNetShopTransactionBasket*)pEvent };
				if (transaction) {
					result = g_Pointers->m_NetShopTransactionBasketAdd(transaction, item);
					if (result) {
						if ("NET_SHOP_ACTION_BUY_PROPERTY"_joaat != transaction->m_Action && "NET_SHOP_ACTION_BUY_WAREHOUSE"_joaat != transaction->m_Action) {
							g_Pointers->m_NetShopTransactionBaseRegisterPendingCash(transaction);
						}
					}
				}
			}
		}
		return result;
	}

	 bool NetworkShoppingMgrStartCheckout(const NetShopTransactionId id) {
		bool result{};
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* pNode{ (*g_Pointers->m_ShoppingMgr)->m_TransactionList.Head };
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* pNextNode{};
		while (pNode) {
			pNextNode = pNode->Next;
			CNetShopTransactionBase* pEvent{ pNode->Data };
			if (id == pEvent->m_Id) {
				if (!pEvent->m_Status.Pending() && !pEvent->m_Status.Succeeded() && !pEvent->m_Status.Canceled() && !pEvent->m_Checkout) {
					pEvent->m_Checkout = true;
					pEvent->m_Status.Reset();
					pEvent->m_TimeEnd = 0;
					bool isPropertyTradeCredit{};
					if (pEvent->m_Type == "NET_SHOP_TTYPE_BASKET"_joaat) {
						const CNetShopTransactionBasket* transaction = static_cast<const CNetShopTransactionBasket*> (pEvent);
						if (transaction) {
							int numProperties{};
							bool hasOverrideItem{};
							//transaction->GetTotalPrice(isPropertyTradeCredit, numProperties, hasOverrideItem);
						}
					}
					if (!isPropertyTradeCredit) {
						g_Pointers->m_NetShopTransactionBaseRegisterPendingCash(pEvent);
					}
					result = true;
				}
				pNextNode = 0;
			}
			pNode = pNextNode;
		}
		return result;
	}

	 bool DeleteBasket() {
		bool result{};
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* node{ (*g_Pointers->m_ShoppingMgr)->m_TransactionList.Head };
		rage::atDNode<CNetShopTransactionBase*, rage::datBase>* nextNode{};
		while (node) {
			nextNode = node->Next;
			CNetShopTransactionBase* event{ node->Data };
			if (event->m_Type == "NET_SHOP_TTYPE_BASKET"_joaat) {
				result = (!event->m_Status.Pending());
				if (result) {
					delete event;
					(*g_Pointers->m_ShoppingMgr)->m_TransactionList.Head = 0;
					(*g_Pointers->m_ShoppingMgr)->m_TransactionList.Tail = 0;
					delete node;
					break;
				}
			}
			node = nextNode;
		}
		return result;
	}

	bool TansactionAdvanced(std::string category, std::string action, std::string id, int amount, int flag, u32 quanity) {
		NetShopTransactionId transactionId{ NET_SHOP_INVALID_TRANS_ID };
		CNetworkShoppingMgr* shoppingMgr{ *g_Pointers->m_ShoppingMgr };
		if (!shoppingMgr->m_transactionInProgress) {
		    DeleteBasket();
			if (g_Pointers->m_NetworkShoppingMgrCreateBasket(shoppingMgr, transactionId, rage::joaat(category), rage::joaat(action), flag)) {
				CNetShopItem item{ rage::joaat(id), 1, amount, 0, quanity };
				NetworkShoppingMgrAddItem(item);
				if (NetworkShoppingMgrStartCheckout(transactionId)) {
					g_Logger->Info("Added %i", amount);
					return true;
				}
			}
		}
		else {
			g_Logger->Info("Transaction already in progress");
			return false;
		}
		g_Logger->Error("Failed to add money");
		return false;
	}
}
