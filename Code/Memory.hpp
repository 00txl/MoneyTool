#pragma once
#include "Common.hpp"
#define EnableIf std::enable_if_t
#define IsPtr std::is_pointer_v
#define IsValRef std::is_lvalue_reference_v
#define IsSame std::is_same_v
#define AddPtr std::add_pointer_t
#define RemoveRef std::remove_reference_t
#define SimpleTemplate template <typename T>
#define Sleep(time) std::this_thread::sleep_for(time)

using u8byte = std::optional<u8>;
using Bytes = std::vector<u8byte>;

namespace Dll
{
    namespace Scanner
    {
        inline int GTotalPtrs{};

        class Mem {
        public:
            Mem(void* p = nullptr) : m_ptr(p) {}
            Mem(u64 p) : Mem((void*)p) {}
        public:
            SimpleTemplate EnableIf<IsPtr<T>, T> As() { return static_cast<T>(m_ptr); }
            SimpleTemplate EnableIf<IsValRef<T>, T> As() { return *static_cast<AddPtr<RemoveRef<T>>>(m_ptr); }
            SimpleTemplate EnableIf<IsSame<T, u64>, T> As() { return (T)m_ptr; }
        public:
            Mem Add(u64 v) { return Mem(As<u64>() + v); }
            Mem Sub(u64 v) { return Mem(As<u64>() - v); }
            Mem Rip() { return Add(As<i32&>()).Add(4); }
            Mem Mov() { return Add(3).Rip(); }
            Mem Cmp() { return Add(3).Rip(); }
            Mem Lea() { return Add(2).Rip(); }
            Mem Call() { return Add(1).Rip(); }
        public:
            void* m_ptr{};
        };

        class ModuleWrapper {
        public:
            ModuleWrapper(std::string name = "") : m_base(GetModuleHandleA(name.empty() ? NULL : name.data())) {
                m_size = m_base.Add(m_base.As<IMAGE_DOS_HEADER*>()->e_lfanew).As<IMAGE_NT_HEADERS*>()->OptionalHeader.SizeOfImage;
            }
        public:
            Mem m_base{};
            i64 m_size{};
        };

        inline u8byte CharToHex(cc c) {
            if (c >= 'a' && c <= 'f') return static_cast<u8>(static_cast<i32>(c) - 87);
            if (c >= 'A' && c <= 'F') return static_cast<u8>(static_cast<i32>(c) - 55);
            if (c >= '0' && c <= '9') return static_cast<u8>(static_cast<i32>(c) - 48);
            return {};
        }

        inline Bytes CreateBytesFromString(const std::string& str) {
            Bytes result{};
            for (u64 i{}; i != str.size() - 1; ++i) {
                if (str[i] == ' ') continue;
                if (str[i] != '?') {
                    if (u8byte hex1{ CharToHex(str[i]) }; hex1) {
                        if (u8byte hex2{ CharToHex(str[i + 1]) }; hex2) {
                            result.emplace_back(static_cast<u8>((*hex1 * 0x10) + *hex2));
                        }
                    }
                }
                else {
                    result.push_back({});
                }
            }
            return result;
        }

        inline bool DoesMemMatch(u8* target, std::optional<u8> const* sig, u64 len) {
            for (u64 i{}; i != len; ++i) {
                if (sig[i] && *sig[i] != target[i]) {
                    return false;
                }
            }
            return true;
        }

        inline std::vector<u64> GetAllResults(std::string ptr, ModuleWrapper module = {}) {
            const Bytes bytes{ CreateBytesFromString(ptr) };
            std::vector<u64> results{};
            for (u64 i{}; i != module.m_size - bytes.size(); ++i) {
                if (DoesMemMatch(module.m_base.Add(i).As<u8*>(), bytes.data(), bytes.size())) {
                    results.push_back(module.m_base.Add(i).As<u64>());
                }
            }
            return results;
        }

        inline Mem FindPattern(const char* pattern, bool log = false, ModuleWrapper module = {}) {
            const Bytes bytes{ CreateBytesFromString(pattern) };
            i64 maxShift{ static_cast<i64>(bytes.size()) }, maxIdx{ maxShift - 1 }, shiftTable[UINT8_MAX + 1]{}, wildCardIdx{ -1 };
            for (i64 i{ maxIdx - 1 }; i >= 0; --i) {
                if (!bytes[i]) {
                    maxShift = maxIdx - i;
                    wildCardIdx = i;
                    break;
                }
            }
            for (i64 i{}; i <= UINT8_MAX; ++i) shiftTable[i] = maxShift;
            for (i64 i{ wildCardIdx + 1 }; i != maxIdx; ++i) shiftTable[*bytes[i]] = maxIdx - i;
            for (i64 curIdx{}; curIdx != module.m_size - static_cast<i64>(bytes.size());) {
                for (i64 sigIdx{ maxIdx }; sigIdx >= 0; --sigIdx) {
                    if (bytes[sigIdx].has_value() && *module.m_base.Add(curIdx + sigIdx).As<u8*>() != bytes[sigIdx].value()) {
                        curIdx += shiftTable[*module.m_base.Add(curIdx + maxIdx).As<u8*>()];
                        break;
                    }
                    else if (sigIdx == 0) {
                        if (module.m_base.Add(curIdx).As<u64>()) {
                            if (log) {
                                printf("found %s\n", pattern);
                            }
                            GTotalPtrs++;
                            return module.m_base.Add(curIdx).As<u64>();
                        }
                        else {
                            return nullptr;
                        }
                    }
                }
            }
            return nullptr;
        }

        inline int SpacesUntilWildcard(const std::string& str) {
            std::istringstream stream(str);
            std::string segment;
            int count{};
            while (stream >> segment) {
                if (segment == "?") {
                    return count;
                }
                count++;
            }
            return -1;
        }

        inline Mem FindVariable(const char* pattern) {
            return FindPattern(pattern).Add(SpacesUntilWildcard(pattern)).Rip();
        }
    }
}
