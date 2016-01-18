#pragma once

#include <atomic>
#include <map>
#include <stdint.h>

#include "Common/Variant.h"

namespace common
{
  typedef std::map< std::string, Variant > AdapterAttributeMap;

  class AdapterObject
  {
  public:
    AdapterObject(std::string const& name)
      : m_name(name)
      , m_id(GetNextObjectId())
    {
    }

    virtual ~AdapterObject() { }

    uint64_t GetId() const
      { return m_id; }

    std::string const& GetName() const
      { return m_name; }

    std::string const& GetDescription() const
      { return m_description; }

    void SetDescription(std::string const& desc)
      { m_description = desc; }

    AdapterAttributeMap const& GetAttributes() const
      { return m_attrs; }

    Variant GetAttribute(std::string const& name) const
    {
      auto itr = m_attrs.find(name);
      return itr != m_attrs.end() ? itr->second : Variant();
    }

    void AddAttribute(std::string const& name, Variant const& value)
      { m_attrs.insert(AdapterAttributeMap::value_type(name, value)); }

    void ClearAttributes()
      { m_attrs.clear(); }

  private:
    std::string           m_name;
    uint64_t              m_id;
    AdapterAttributeMap   m_attrs;
    std::string           m_description;

  private:
    uint64_t GetNextObjectId()
    {
      static std::atomic<uint64_t> s_id;
      return s_id++;
    }
  };
}