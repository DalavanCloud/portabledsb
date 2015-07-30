#pragma once

#include "Common/defines.h"

#include <alljoyn/BusAttachment.h>
#include <alljoyn/ProxyBusObject.h>
#include <alljoyn/SessionListener.h>

#include <map>

class AllJoynBusObject;
class AllJoynProvider;

class AllJoynService : public enable_shared_from_this<AllJoynService>
{
public:
  static shared_ptr<AllJoynService> Create(shared_ptr<AllJoynProvider> const& provider,
    std::string const& serviceName, ajn::SessionPort port)
  {
    return shared_ptr<AllJoynService>(new AllJoynService(provider, serviceName, port));
  }

public:
  AllJoynService(shared_ptr<AllJoynProvider> const& provider, std::string const& serviceName, ajn::SessionPort port);
  ~AllJoynService();

  void Initialize(ajn::MsgArg const& aboutDataArg, ajn::MsgArg const& objectDescriptionArg);
  void Shutdown();

  shared_ptr<AllJoynBusObject> GetBusObject(std::string const& path);

  shared_ptr<AllJoynProvider> GetProvider() const
    { return m_provider.lock(); }

  ajn::BusAttachment* GetBusAttachment() const;

  inline ajn::SessionId GetSessionId() const
    { return m_sessionId; }

  inline std::string GetName() const
    { return m_name; }

  inline uint16_t GetSessionPort() const
    { return m_sessionPort; }

private:
  class SessionListener : public ajn::SessionListener
  {
  public:
    SessionListener(AllJoynService* parent)
      : m_parent(parent) { }
    virtual ~SessionListener() { }
    virtual void SessionLost(ajn::SessionId sessionId, SessionLostReason reason);
    virtual void SessionMemberAdded(ajn::SessionId sessionId, char const* uniqueName);
    virtual void SessionMemberRemoved(ajn::SessionId sessionId, char const* uniqueName);
  private:
    AllJoynService* m_parent;
  };

  void ParseObjectData();
  void OnSessionLost(ajn::SessionId sessionId, ajn::SessionListener::SessionLostReason reason);
  void OnSessionMemberAdded(ajn::SessionId sessionId, char const* uniqueName);
  void OnSessionMemberRemoved(ajn::SessionId sessionId, char const* uniqueName);

private:
  typedef std::map<std::string, weak_ptr<AllJoynBusObject> > object_map_type;

  weak_ptr<AllJoynProvider>     m_provider;
  std::string                   m_name;
  ajn::SessionPort              m_sessionPort;
  shared_ptr<AllJoynBusObject>  m_busObject;
  ajn::SessionId                m_sessionId;
  object_map_type               m_objectsMap;
  pthread_mutex_t               m_mutex;
  SessionListener*              m_sessionListener;
  ajn::MsgArg                   m_aboutDataArg;
  ajn::MsgArg                   m_objectDescriptionArg;
};

