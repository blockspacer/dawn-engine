/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2016 (git@davedissian.com)
 */
#pragma once

#include "io/InputStream.h"
#include "io/OutputStream.h"

namespace dw {

enum ConfigNodeType { NT_NULL, NT_SCALAR, NT_SEQUENCE, NT_MAP };

class ConfigNode;

struct ConfigNodeData {
    String scalar;
    Vector<ConfigNode> sequence;
    Map<String, ConfigNode> keymap;
};

template <class T> struct Converter {
    static ConfigNode encode(Context* context, const T& value);
    static bool decode(Log& logger, const ConfigNode& node, T& value);
};

class DW_API ConfigNode : public Object {
public:
    DW_OBJECT(ConfigNode);

    template <class T> friend struct Converter;

    ConfigNode(Context* context);
    ConfigNode(Context* context, const ConfigNode& rhs);
    template <class T> ConfigNode(Context* context, const T& s);
    ~ConfigNode();

    void load(InputStream& src);
    void save(OutputStream& dst);

    // Stream operators
    friend DW_API std::istream& operator>>(std::istream& stream, ConfigNode& node);
    friend DW_API std::ostream& operator<<(std::ostream& stream, const ConfigNode& node);

    template <class T> T as() const {
        T out;
        Converter<T>::decode(getLog(), *this, out);
        return out;
    }

    template <class T> T as(const T& defaultValue) const {
        T out;
        if (Converter<T>::decode(getLog(), *this, out))
            return out;
        else
            return defaultValue;
    }

    ConfigNodeType getNodeType() const;
    bool isScalar() const;
    bool isSequence() const;
    bool isMap() const;
    uint size() const;

    // Sequence operations
    void push(const ConfigNode& v);
    ConfigNode& operator[](uint i);
    const ConfigNode& operator[](uint i) const;

    Vector<ConfigNode>::iterator seq_begin();
    Vector<ConfigNode>::const_iterator seq_begin() const;
    Vector<ConfigNode>::iterator seq_end();
    Vector<ConfigNode>::const_iterator seq_end() const;

    // Map operations
    void insert(const Pair<String, ConfigNode>& p);
    bool contains(const String& key) const;
    ConfigNode& operator[](const String& key);
    const ConfigNode& operator[](const String& key) const;

    Map<String, ConfigNode>::iterator map_begin();
    Map<String, ConfigNode>::const_iterator map_begin() const;
    Map<String, ConfigNode>::iterator map_end();
    Map<String, ConfigNode>::const_iterator map_end() const;

private:
    ConfigNodeType mType;
    ConfigNodeData mData;
};
}

#include "ConfigNode.i.h"
