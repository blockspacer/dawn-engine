/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2019 (git@dga.dev)
 */
#pragma once

#include "InputStream.h"

namespace dw {
class DW_API StringInputStream : public InputStream {
public:
    StringInputStream(const String& data);
    ~StringInputStream() = default;

    usize readData(void* dest, usize size) override;
    void seek(usize position) override;

private:
    String data_;
};
}  // namespace dw
