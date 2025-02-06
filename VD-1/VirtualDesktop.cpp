
#include "VirtualDesktop.h"
#include <optional>
#include <memory>
#include <windows.h>
#include <strsafe.h>


// Non-Localizable strings
namespace NonLocalizable
{
    const wchar_t RegCurrentVirtualDesktop[] = L"CurrentVirtualDesktop";
    const wchar_t RegVirtualDesktopIds[] = L"VirtualDesktopIDs";
    const wchar_t RegKeyVirtualDesktops[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\VirtualDesktops";
    const wchar_t RegKeyVirtualDesktopsFromSession[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\SessionInfo\\%d\\VirtualDesktops";
}

VirtualDesktop::VirtualDesktop()
{

}

VirtualDesktop::~VirtualDesktop()
{

}

VirtualDesktop& VirtualDesktop::instance()
{
    static VirtualDesktop self;
    return self;
}

HKEY OpenVirtualDesktopsRegKey()
{
    HKEY hKey{ nullptr };
    if (RegOpenKeyEx(HKEY_CURRENT_USER, NonLocalizable::RegKeyVirtualDesktops, 0, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS)
    {
        return hKey;
    }
    return nullptr;
}

HKEY GetVirtualDesktopsRegKey()
{
    //static wil::unique_hkey virtualDesktopsKey{ OpenVirtualDesktopsRegKey() };
    //return virtualDesktopsKey.get();
    static HKEY virtualDesktopsKey = OpenVirtualDesktopsRegKey();
    return virtualDesktopsKey;
}

std::optional<GUID> NewGetCurrentDesktopId()
{
    //wil::unique_hkey key{};
    HKEY key{ nullptr };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, NonLocalizable::RegKeyVirtualDesktops, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
        GUID value{};
        DWORD size = sizeof(GUID);
        if (RegQueryValueExW(key, NonLocalizable::RegCurrentVirtualDesktop, 0, nullptr, reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS)
        {
            RegCloseKey(key);
            return value;
        }
    }
    RegCloseKey(key);
    return std::nullopt;
}

std::optional<GUID> GetDesktopIdFromCurrentSession()
{
    DWORD sessionId;
    if (!ProcessIdToSessionId(GetCurrentProcessId(), &sessionId))
    {
        return std::nullopt;
    }

    wchar_t sessionKeyPath[256]{};
    HRESULT hr = StringCchPrintfW(sessionKeyPath, ARRAYSIZE(sessionKeyPath), NonLocalizable::RegKeyVirtualDesktopsFromSession, sessionId);
    if (FAILED(hr))
    {
        return std::nullopt;
    }

    //wil::unique_hkey key{};
    HKEY key{ nullptr };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, sessionKeyPath, 0, KEY_ALL_ACCESS, &key) == ERROR_SUCCESS)
    {
        GUID value{};
        DWORD size = sizeof(GUID);
        if (RegQueryValueExW(key, NonLocalizable::RegCurrentVirtualDesktop, 0, nullptr, reinterpret_cast<BYTE*>(&value), &size) == ERROR_SUCCESS)
        {
            RegCloseKey(key);
            return value;
        }
    }
    RegCloseKey(key);

    return std::nullopt;
}

GUID VirtualDesktop::GetCurrentVirtualDesktopIdFromRegistry() const
{
    // On newer Windows builds, the current virtual desktop is persisted to
    // a totally different reg key. Look there first.
    std::optional<GUID> desktopId = NewGetCurrentDesktopId();
    if (desktopId.has_value())
    {
        return desktopId.value();
    }

    // Explorer persists current virtual desktop identifier to registry on a per session basis, but only
    // after first virtual desktop switch happens. If the user hasn't switched virtual desktops in this
    // session, value in registry will be empty.
    desktopId = GetDesktopIdFromCurrentSession();
    if (desktopId.has_value())
    {
        return desktopId.value();
    }

    // Fallback scenario is to get array of virtual desktops stored in registry, but not kept per session.
    // Note that we are taking first element from virtual desktop array, which is primary desktop.
    // If user has more than one virtual desktop, previous function should return correct value, as desktop
    // switch occurred in current session.
    else
    {
        auto ids = GetVirtualDesktopIdsFromRegistry();
        if (ids.has_value() && ids->size() > 0)
        {
            return ids->at(0);
        }
    }

    return GUID_NULL;
}
std::optional<std::vector<GUID>> VirtualDesktop::GetVirtualDesktopIdsFromRegistry(HKEY hKey) const
{
    if (!hKey)
    {
        return std::nullopt;
    }

    DWORD bufferCapacity;
    // request regkey binary buffer capacity only
    if (RegQueryValueExW(hKey, NonLocalizable::RegVirtualDesktopIds, 0, nullptr, nullptr, &bufferCapacity) != ERROR_SUCCESS)
    {
        return std::nullopt;
    }

    std::unique_ptr<BYTE[]> buffer = std::make_unique<BYTE[]>(bufferCapacity);
    // request regkey binary content
    if (RegQueryValueExW(hKey, NonLocalizable::RegVirtualDesktopIds, 0, nullptr, buffer.get(), &bufferCapacity) != ERROR_SUCCESS)
    {
        return std::nullopt;
    }

    const size_t guidSize = sizeof(GUID);
    std::vector<GUID> temp;
    temp.reserve(bufferCapacity / guidSize);
    for (size_t i = 0; i < bufferCapacity; i += guidSize)
    {
        GUID* guid = reinterpret_cast<GUID*>(buffer.get() + i);
        temp.push_back(*guid);
    }

    return temp;
}

std::optional<std::vector<GUID>> VirtualDesktop::GetVirtualDesktopIdsFromRegistry() const
{
    return GetVirtualDesktopIdsFromRegistry(GetVirtualDesktopsRegKey());
}
