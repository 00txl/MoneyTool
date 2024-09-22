#pragma once
#include "Common.hpp"

// types
using u64 = uint64_t;
using i64 = int64_t;
using u32 = uint32_t;
using i32 = int32_t;
using u16 = uint16_t;
using i16 = int16_t;
using u8 = uint8_t;
using i8 = int8_t;
using cc = const char;
using db = double;
using s64 = i64;
using s32 = i32;
using s16 = i16;
using s8 = i8;
using ccp = const char*;
using u8byte = std::optional<u8>;
using bytes = std::vector<u8byte>;

namespace rage {
	inline constexpr char j_tolower(char const c) {
		return (c >= 'A' && c <= 'Z') ? c + ('a' - 'A') : c;
	}
	inline u32 joaat(std::string_view str, bool forceLowerCase = true) {
		u32 hash{};
		for (char c : str) {
			hash += forceLowerCase ? j_tolower(c) : c;
			hash += (hash << 10);
			hash ^= (hash >> 6);
		}
		hash += (hash << 3);
		hash ^= (hash >> 11);
		hash += (hash << 15);
		return hash;
	}
	template <typename t = std::string_view>
	inline consteval u32 constexprJoaat(t str, bool const forceLowerCase = true) {
		u32 hash{};
		while (*str != '\0') {
			hash += forceLowerCase ? j_tolower(*str) : *str, ++str;
			hash += hash << 10, hash ^= hash >> 6;
		}
		hash += (hash << 3), hash ^= (hash >> 11), hash += (hash << 15);
		return hash;
	}
	template <size_t s>
	inline constexpr u32 constexprJoaatFromArray(char const(&data)[s]) {
		u32 hash{};
		for (u64 i{}; i != s; ++i) {
			hash += j_tolower(*data);
			hash += hash << 10, hash ^= hash >> 6;
		}
		hash += (hash << 3), hash ^= (hash >> 11), hash += (hash << 15);
		return hash;
	}
}
inline consteval u32 operator ""_joaat(const char* str, size_t) { return rage::constexprJoaat(str); }
inline consteval u32 operator ""_joaat_normalised(const char* str, size_t) { return rage::constexprJoaat(str, false); }
namespace rage {
	struct atNonFinalHashString {
		u32 m_hash;
	};
	template <typename T>
	class atArray {
	public:
		T* begin() { return m_data; }
		T* end() { return m_data + m_size; }
		const T* begin() const { return m_data; }
		const T* end() const { return m_data + m_size; }
		T* data() { return m_data; }
		const T* data() const { return m_data; }
		u16 size() const { return m_size; }
		u16 capacity() const { return m_capacity; }
		T& operator[](u16 index) { return m_data[index]; }
		const T& operator[](u16 index) const { return m_data[index]; }
	public:
		T* m_data; //0x0000
		u16 m_size; //0x0008
		u16 m_capacity; //0x0010
	}; //Size: 0x0010
	static_assert(sizeof(rage::atArray<void*>) == 0x10);
#pragma pack(push, 8)
	template <typename T, size_t Count>
	class atFixedArray {
	public:
		T m_Elements[Count];
		int m_Count = Count;
		T& operator[](uint16_t index) { return m_Elements[index]; }
	};
#pragma pack(pop)
	class datBase {
	public:
		virtual ~datBase();
	};
}
namespace NetworkGameTransactions {
	struct InventoryItem {
		s32 m_itemId;
		union
		{
			s32 m_inventorySlotItemId;
			s32 m_quantity;
		};
		int m_price;
	};
	struct GameTransactionBase : rage::datBase
	{
		unsigned __int64 m_nounce;
	};
	struct GameTransactionItems : GameTransactionBase
	{
		bool m_evconly;
		int m_slot;
		rage::atArray<InventoryItem> m_items;
	};
	struct SpendEarnTransaction : GameTransactionItems
	{
		int m_bank;
		int m_wallet;
	};
	struct PlayerBalanceApplicationInfo
	{
		bool m_applicationSuccessful;
		bool m_dataReceived;
		__int64 m_bankCashDifference;
		__int64 m_walletCashDifference;
	};
	struct __declspec(align(8)) PlayerBalance
	{
		__int64 m_evc;
		__int64 m_pvc;
		__int64 m_bank;
		__int64 m_Wallets[5];
		long double m_pxr;
		long double m_usde;
		PlayerBalanceApplicationInfo m_applyDataToStatsInfo;
		bool m_deserialized;
		bool m_finished;
	};
	struct InventoryDataApplicationInfo
	{
		bool m_dataReceived;
		int m_numItems;
	};
	struct __declspec(align(8)) InventoryItemSet
	{
		bool m_deserialized;
		int m_slot;
		rage::atArray<InventoryItem> m_items;
		bool m_finished;
		InventoryDataApplicationInfo m_applyDataToStatsInfo;
	};
	struct TelemetryNonce
	{
		__int64 m_nonce;
	};
}
enum StatusCode : __int32
{
	NET_STATUS_NONE = 0x0,
	NET_STATUS_PENDING = 0x1,
	NET_STATUS_FAILED = 0x2,
	NET_STATUS_SUCCEEDED = 0x3,
	NET_STATUS_CANCELED = 0x4,
};
struct StatId
{
	u32 m_Id;
};
#define NET_SHOP_INVALID_TRANS_ID 0xFFFFFFFF 
#define NET_SHOP_INVALID_ITEM_ID  0xFFFFFFFF
typedef u32   NetShopItemId;           
class CNetShopItem
{
public:
	NetShopItemId     m_Id;
	NetShopItemId     m_ExtraInventoryId;
	int               m_Price;
	int               m_StatValue;
	u32               m_Quantity;

public:
	CNetShopItem() { Clear(); }
	CNetShopItem(const NetShopItemId id, const NetShopItemId catalogId, const int price, const int statValue, const u32 quantity = 1)
		: m_Id(id)
		, m_ExtraInventoryId(catalogId)
		, m_Price(price)
		, m_StatValue(statValue)
		, m_Quantity(quantity)
	{
		;
	}

	void Clear();
	bool IsValid() const { return (m_Id != NET_SHOP_INVALID_ITEM_ID); }

	const CNetShopItem& operator=(const CNetShopItem& other)
	{
		m_Id = other.m_Id;
		m_ExtraInventoryId = other.m_ExtraInventoryId;
		m_Price = other.m_Price;
		m_StatValue = other.m_StatValue;
		m_Quantity = other.m_Quantity;

		return *this;
	}
};
class netStatus
{
public:
	enum StatusCode
	{
		NET_STATUS_NONE,
		NET_STATUS_PENDING,
		NET_STATUS_FAILED,
		NET_STATUS_SUCCEEDED,
		NET_STATUS_CANCELED,
		NET_STATUS_COUNT
	};
	netStatus()
		: m_Status(NET_STATUS_NONE)
		, m_ResultCode(0)
	{
	}
	~netStatus()
	{
	}
	void Reset()
	{
	}
	StatusCode GetStatus() const
	{
		return m_StatusCode;
	}
	int GetResultCode() const
	{
		return m_ResultCode;
	}
	void SetStatus(const StatusCode code)
	{
		switch (code)
		{
		case NET_STATUS_PENDING:
		case NET_STATUS_SUCCEEDED:
			SetStatus(code, 0);
			break;
		case NET_STATUS_FAILED:
		case NET_STATUS_CANCELED:
		default:
			SetStatus(code, -1);
			break;
		}
	}
	void SetStatus(const StatusCode code, const int resultCode)
	{

	}
	void SetPending()
	{
		this->SetStatus(NET_STATUS_PENDING);
	}
	void SetSucceeded(const int resultCode = 0)
	{
		this->SetStatus(NET_STATUS_SUCCEEDED, resultCode);
	}
	void SetFailed(const int resultCode = -1)
	{
		this->SetStatus(NET_STATUS_FAILED, resultCode);
	}
	void SetCanceled()
	{
		this->SetStatus(NET_STATUS_CANCELED);
	}
	void ForceSucceeded(const int resultCode = 0)
	{

	}
	void ForceFailed(const int resultCode = -1)
	{

	}
	bool Pending() const
	{
		return NET_STATUS_PENDING == m_Status;
	}
	bool Succeeded() const
	{
		return NET_STATUS_SUCCEEDED == m_Status;
	}
	bool Failed() const
	{
		return NET_STATUS_FAILED == m_Status;
	}
	bool Canceled() const
	{
		return NET_STATUS_CANCELED == m_Status;
	}
	bool Finished() const
	{
		return Succeeded() || Failed() || Canceled();
	}
	bool None() const
	{
		return NET_STATUS_NONE == m_Status;
	}
	static const char* GetStatusCodeString(const StatusCode statusCode);
	static const char* GetStatusCodeString(const netStatus& status) { return GetStatusCodeString(status.m_StatusCode); }
	const char* c_str() const { return GetStatusCodeString(*this); }

private:
	union
	{
		unsigned m_Status;
		StatusCode m_StatusCode;
	};
	int m_ResultCode;
};
struct CNetShopTransactionBase
{
	virtual ~CNetShopTransactionBase() = 0;
	virtual void             Init() = 0;
	virtual void           Cancel() = 0;
	virtual void         Shutdown() = 0;
	virtual void           Update() = 0;
	virtual void            Clear() = 0;
	virtual void     ProcessSuccess() = 0;
	virtual bool    ProcessingStart() = 0;
	virtual bool  GetTransactionObj(NetworkGameTransactions::SpendEarnTransaction& transactionObj) const = 0;
	virtual void     ProcessFailure() = 0;
	virtual void       SpewItems() const = 0;
	virtual int GetServiceId() const { return 0; };
	virtual void  GrcDebugDraw();
	unsigned int m_Id;
	unsigned int m_Type;
	unsigned __int64 m_RequestId;
	unsigned int m_Category;
	netStatus m_Status;
	unsigned int m_Action;
	int m_Flags;
	bool m_Checkout;
	bool m_NullTransaction;
	bool m_IsProcessing;
	unsigned int m_TimeEnd;
	NetworkGameTransactions::PlayerBalance m_PlayerBalanceResponse;
	NetworkGameTransactions::InventoryItemSet m_InventoryItemSetResponse;
	NetworkGameTransactions::TelemetryNonce m_TelemetryNonceResponse;
	unsigned int m_attempts;
	unsigned int m_FrameStart;
	unsigned int m_FrameEnd;
	unsigned int m_TimeStart;
};
struct CNetTransactionItemKey
{
	rage::atNonFinalHashString m_key;
};
namespace rage {
	template <typename t, typename t2>
	struct atDNode : rage::datBase
	{
		t Data;
		rage::atDNode<t, t2>* Next;
		rage::atDNode<t, t2>* Prev;
	};
	template <typename t>
	struct atDList
	{
		t* Head;
		t* Tail;
	};
}
struct __declspec(align(8)) CashAmountHelper
{
	int m_id;
	__int64 m_bank;
	__int64 m_wallet;
	bool m_EvcOnly;
};
struct PendingCashReductionsHelper
{
	rage::atArray<CashAmountHelper> m_transactions;
	__int64 m_totalbank;
	__int64 m_totalwallet;
	__int64 m_totalbankEvcOnly;
	__int64 m_totalwalletEvcOnly;
};
#define align(a) __declspec(align(a))
namespace rage {
	class datResourceChunk {
	public:
		void* SrcAddr;
		void* DestAddr;
		unsigned __int64 Size;
	};
	class datResourceMap {
	public:
		u8 VirtualCount;
		u8 PhysicalCount;
		u8 RootVirtualChunk;
		u8 DisableMerge;
		u8 HeaderType;
		void* VirtualBase;
		datResourceChunk Chunks[128];
		int LastSrc;
		int LastDest;
	};
	class sysMemDistribution {
	public:
		u32 UsedBySize[32];
		u32 FreeBySize[32];
	};
	class align(8) sysMemDefragmentationNode {
	public:
		void* From;
		void* To;
		unsigned int Size;
	};
	class sysMemDefragmentation {
	public:
		int Count;
		sysMemDefragmentationNode Nodes[32];
	};
	class sysMemDefragmentationFree {
	public:
		int Count;
		void* Nodes[4];
	};
	class sysMemAllocator {
	public:
		virtual ~sysMemAllocator();
		virtual void SetQuitOnFail(const bool);
		virtual void Allocate(unsigned __int64, unsigned __int64, int);
		virtual void TryAllocate(unsigned __int64, unsigned __int64, int);
		virtual void Free(const void*);
		virtual void DeferredFree(const void*);
		virtual void Resize(const void*, unsigned __int64);
		virtual void GetAllocator(int);
		virtual void GetAllocator(int) const;
		virtual void GetPointerOwner(const void*);
		virtual void GetSize(const void*);
		virtual void GetMemoryUsed(int);
		virtual void GetMemoryAvailable();
		virtual void GetLargestAvailableBlock();
		virtual void GetLowWaterMark(bool);
		virtual void GetHighWaterMark(bool);
		virtual void UpdateMemorySnapshot();
		virtual void GetMemorySnapshot(int);
		virtual void IsTallied();
		virtual void BeginLayer();
		virtual void EndLayer(const char*, const char*);
		virtual void BeginMemoryLog(const char*, bool);
		virtual void EndMemoryLog();
		virtual void IsBuildingResource();
		virtual void HasMemoryBuckets();
		virtual void SanityCheck();
		virtual void IsValidPointer(const void*);
		virtual void SupportsAllocateMap();
		virtual void AllocateMap(rage::datResourceMap*);
		virtual void FreeMap(const rage::datResourceMap*);
		virtual void GetSizeWithOverhead(const void*);
		virtual void GetHeapSize();
		virtual void GetHeapBase();
		virtual void SetHeapBase(void*);
		virtual void IsRootResourceAllocation();
		virtual void GetCanonicalBlockPtr(const void*);
		virtual void TryLockBlock(const void*, unsigned int);
		virtual void UnlockBlock(const void*, unsigned int);
		virtual void GetUserData(const void*);
		virtual void SetUserData(const void*, unsigned int);
		virtual void GetMemoryDistribution(rage::sysMemDistribution*);
		virtual void Defragment(rage::sysMemDefragmentation*, rage::sysMemDefragmentationFree*, unsigned __int64);
		virtual void GetFragmentation();
	};
}
class CNetworkShoppingMgr {
public:
	virtual ~CNetworkShoppingMgr() = default;

	char pad_0008[24]; //0x0008
	rage::atDList<rage::atDNode<CNetShopTransactionBase*, rage::datBase>> m_TransactionList; //0x0020
	rage::sysMemAllocator* m_Allocator;
	bool m_LoadCatalogFromCache;
	bool m_transactionInProgress;
	PendingCashReductionsHelper m_cashreductions;
};
typedef u64   NetShopRequestId;  
typedef u32   NetShopItemId;        
typedef u32   NetShopCategory;         
typedef u32   NetShopTransactionId;     
typedef u32   NetShopTransactionType;   
typedef u32   NetShopTransactionAction; 
#pragma pack(push, 1)
class CNetShopTransactionBasket : public CNetShopTransactionBase {
public:
	CNetShopItem   m_Items[71];
	u8             m_Size;
}; //Size: 0x0670
#pragma pack(pop)


enum eNetworkAccessArea
{
	AccessArea_Invalid,
	AccessArea_First = 0,
	AccessArea_Landing = AccessArea_First,
	AccessArea_Multiplayer,
	AccessArea_MultiplayerEnter,
	AccessArea_Num,
};

enum eNetworkAccessCode
{
	Access_BankDefault = -2,
	Access_Invalid = -1,
	Access_Granted,
	Access_Deprecated_TunableNotFound,
	Access_Deprecated_TunableFalse,
	Access_Denied_MultiplayerLocked,
	Access_Denied_InvalidProfileSettings,
	Access_Denied_PrologueIncomplete,
	Access_Denied_NotSignedIn,
	Access_Denied_NotSignedOnline,
	Access_Denied_NoOnlinePrivilege,
	Access_Denied_NoRosCredentials,
	Access_Denied_NoRosPrivilege,
	Access_Denied_MultiplayerDisabled,
	Access_Denied_NoTunables,
	Access_Denied_NoBackgroundScript,
	Access_Denied_NoNetworkAccess,
	Access_Denied_RosBanned,
	Access_Denied_RosSuspended,
};