#pragma once

#include <vector>
#include <cstdint>
#include "WaylandProtocol.hpp"
#include "ext-session-lock-v1.hpp"
#include "../helpers/signal/Signal.hpp"

class CMonitor;
class CSessionLock;
class CWLSurfaceResource;

class CSessionLockSurface {
  public:
    CSessionLockSurface(SP<CExtSessionLockSurfaceV1> resource_, SP<CWLSurfaceResource> surface_, PHLMONITOR pMonitor_, WP<CSessionLock> owner_);
    ~CSessionLockSurface();

    bool                   good();
    bool                   inert();
    PHLMONITOR             monitor();
    SP<CWLSurfaceResource> surface();

    struct {
        CSignalT<> map;
        CSignalT<> destroy;
        CSignalT<> commit;
    } m_events;

  private:
    SP<CExtSessionLockSurfaceV1> m_resource;
    WP<CSessionLock>             m_sessionLock;
    WP<CWLSurfaceResource>       m_surface;
    PHLMONITORREF                m_monitor;

    bool                         m_ackdConfigure = false;
    bool                         m_committed     = false;

    void                         sendConfigure();

    struct {
        CHyprSignalListener monitorMode;
        CHyprSignalListener surfaceCommit;
        CHyprSignalListener surfaceDestroy;
    } m_listeners;
};

class CSessionLock {
  public:
    CSessionLock(SP<CExtSessionLockV1> resource_);
    ~CSessionLock();

    bool good();
    void sendLocked();
    void sendDenied();

    struct {
        CSignalT<SP<CSessionLockSurface>> newLockSurface;
        CSignalT<>                        unlockAndDestroy;
        CSignalT<>                        destroyed; // fires regardless of whether there was a unlockAndDestroy or not.
    } m_events;

  private:
    SP<CExtSessionLockV1> m_resource;

    bool                  m_inert = false;

    friend class CSessionLockProtocol;
};

class CSessionLockProtocol : public IWaylandProtocol {
  public:
    CSessionLockProtocol(const wl_interface* iface, const int& ver, const std::string& name);

    virtual void bindManager(wl_client* client, void* data, uint32_t ver, uint32_t id);

    bool         isLocked();

    struct {
        CSignalT<SP<CSessionLock>> newLock;
    } m_events;

  private:
    void onManagerResourceDestroy(wl_resource* res);
    void destroyResource(CSessionLock* lock);
    void destroyResource(CSessionLockSurface* surf);
    void onLock(CExtSessionLockManagerV1* pMgr, uint32_t id);
    void onGetLockSurface(CExtSessionLockV1* lock, uint32_t id, wl_resource* surface, wl_resource* output);

    bool m_locked = false;

    //
    std::vector<UP<CExtSessionLockManagerV1>> m_managers;
    std::vector<SP<CSessionLock>>             m_locks;
    std::vector<SP<CSessionLockSurface>>      m_lockSurfaces;

    friend class CSessionLock;
    friend class CSessionLockSurface;
};

namespace PROTO {
    inline UP<CSessionLockProtocol> sessionLock;
};
