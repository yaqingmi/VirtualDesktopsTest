#pragma once
#include <windows.h>
#include <shobjidl_core.h>
#include <shobjidl.h>
#include <optional>
#include <vector>

class VirtualDesktop
{
public:
    static VirtualDesktop& instance();



    // registry
    GUID GetCurrentVirtualDesktopIdFromRegistry() const;
    std::optional<std::vector<GUID>> GetVirtualDesktopIdsFromRegistry() const;

private:
    VirtualDesktop();
    ~VirtualDesktop();

    IVirtualDesktopManager* m_vdManager{ nullptr };

    std::optional<std::vector<GUID>> GetVirtualDesktopIdsFromRegistry(HKEY hKey) const;
};