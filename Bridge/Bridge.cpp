#include "Bridge.h"
#include "Common/Log.h"
#include "Bridge/BridgeDevice.h"

#include <alljoyn/Init.h>

namespace
{
  DSB_DECLARE_LOGNAME(DeviceSystemBridge);

  shared_ptr<Bridge::DeviceSystemBridge> g_Instance;

  int32_t const kWaitTimeoutForAdapterOperation = 20000; //ms
  std::string const kDeviceArrivalSignal = "Device_Arrival";
  std::string const kDeviceArriaveDeviceHandle = "Device_Handle";
  std::string const kDeviceRemovalSignal = "Device_Removal";
  std::string const kDeviceRemovalDeviceHandle = "Device_Handle";
}

Bridge::DeviceSystemBridge::DeviceSystemBridge(shared_ptr<IAdapter> const& adapter)
  : m_alljoynInitialized(false)
  , m_adapter(adapter)
{
  g_Instance = shared_from_this();
  m_adapterSignaListener = shared_ptr<AdapterSignalListener>(new AdapterSignalListener(shared_from_this()));
}

Bridge::DeviceSystemBridge::~DeviceSystemBridge()
{
  Shutdown();
  m_adapter.reset();
}

QStatus Bridge::DeviceSystemBridge::Initialize()
{
  QStatus st = ER_OK;

  DSBLOG_DEBUG(__FUNCTION__);
  if (!m_alljoynInitialized)
  {
    st = AllJoynInit();
    if (st != ER_OK)
    {
      DSBLOG_WARN("Failed to initialize AllJoyn: 0x%x", st);
      goto Leave;
    }
  }

  st = InitializeInternal();
  if (st != ER_OK)
    goto Leave;

Leave:
  if (st != ER_OK)
    Shutdown();

  return st;
}

QStatus Bridge::DeviceSystemBridge::InitializeInternal()
{
  QStatus st = ER_OK;

  // st = m_configManager.Initialize(this);
  if (st != ER_OK)
    goto Leave;

  st = InitializeAdapter();
  if (st != ER_OK)
  {
    DSBLOG_WARN("Failed to intialize adapter: 0x%x", st);
    goto Leave;
  }

  st = InitializeDevices();
  if (st != ER_OK)
  {
    DSBLOG_WARN("Failed to initialize devices: 0x%x", st);
    goto Leave;
  }

  st = RegisterAdapterSignalHandlers(true);
  if (st != ER_OK)
  {
    DSBLOG_WARN("Failed to register adapter signal handlers: 0x%x", st);
  }

Leave:
  return st;
}

QStatus Bridge::DeviceSystemBridge::Shutdown()
{
  QStatus st = ER_OK;

  st = ShutdownInternal();
  if (st != ER_OK)
  {
    DSBLOG_WARN("failed to shutdown internal: 0x%x", st);
    return st;
  }

  if (m_alljoynInitialized)
  {
    AllJoynShutdown();
    m_alljoynInitialized = false;
  }

  return st;
}

QStatus Bridge::DeviceSystemBridge::InitializeAdapter()
{
  if (!m_adapter)
  {
    DSBLOG_ERROR("can't initialize null adapter");
    return ER_FAIL;
  }
  return m_adapter->Initialize();
}

QStatus Bridge::DeviceSystemBridge::InitializeDevices(bool isUpdate)
{
  AdapterDeviceVector deviceList;
  shared_ptr<IAdapterIoRequest> request;

  EnumDeviceOptions opts = EnumDeviceOptions::CacheOnly;
  if (isUpdate)
    opts = EnumDeviceOptions::ForceRefresh;

  QStatus st = m_adapter->EnumDevices(opts, deviceList, &request);
  if (st == ER_OK)
  {
    if ((st = request->Wait(kWaitTimeoutForAdapterOperation)) != ER_OK)
      DSBLOG_WARN("timeout waiting for EnumDevices to complete");
  }

  if (st != ER_OK)
    goto Leave;

  for (AdapterDeviceVector::iterator itr = deviceList.begin(); itr != deviceList.end(); ++itr)
  {
    // TODO
  }

Leave:
  return st;
}

void Bridge::DeviceSystemBridge::OnAdapterSignal(IAdapterSignal const& signal, void* argp)
{
  // TODO
  shared_ptr<IAdapterDevice> adapterDevice;

  std::string const name = signal.GetName();
  if (name == kDeviceArrivalSignal || name ==kDeviceArriaveDeviceHandle)
  {
    AdapterValueVector params = signal.GetParams();
    for (AdapterValueVector::const_iterator itr = params.begin(); itr != params.end(); ++itr)
    {
      if ((*itr)->GetName() == kDeviceArriaveDeviceHandle)
      {
        // TODO: figure out where this signal is generated and figure out how we're 
        // going to put an arbitrary pointer inside.
        //
        // adapterDevice = param->GetData();
      }

      if (!adapterDevice)
        goto Leave;
    }
  }

  if (name == kDeviceArrivalSignal)
  {
  }
  else if (name == kDeviceRemovalSignal)
  {
    UpdateDevice(adapterDevice, false);
  }
Leave:
  return;
}

QStatus Bridge::DeviceSystemBridge::RegisterAdapterSignalHandlers(bool isRegister)
{
  QStatus ret = ER_OK;

  shared_ptr<IAdapterSignalListener> listener =
    dynamic_pointer_cast<IAdapterSignalListener>(g_Instance);

  AdapterSignalVector signals = m_adapter->GetSignals();
  for (AdapterSignalVector::const_iterator itr = signals.begin(); itr != signals.end(); ++itr)
  {
    if (isRegister)
    {
      QStatus st = m_adapter->RegisterSignalListener(*itr, listener, NULL);
      if (st != ER_OK)
      {
        DSBLOG_WARN("failed to register signal listener on adapter: 0x%x", st);
        if (ret == ER_OK)
          ret = st;
      }
    }
    else
    {
      QStatus st = m_adapter->UnregisterSignalListener(*itr, listener);
      if (st != ER_OK)
      {
        DSBLOG_WARN("failed to unregister signal listener on adapter: 0x%x", st);
        if (ret == ER_OK)
          ret = st;
      }
    }
  }

  return ret;
}

QStatus Bridge::DeviceSystemBridge::ShutdownInternal()
{
  // m_configManager.Shutdown();

  if (m_adapter)
  {
    RegisterAdapterSignalHandlers(false);
    m_adapter->Shutdown();
  }

  for (BridgeDeviceList::iterator itr = m_deviceList.begin(); itr != m_deviceList.end(); ++itr)
    itr->second->Shutdown();

  m_deviceList.clear();

  return ER_OK;
}



