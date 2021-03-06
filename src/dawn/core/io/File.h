/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2019 (git@dga.dev)
 */
#pragma once

#include "Path.h"
#include "InputStream.h"
#include "OutputStream.h"

namespace dw {

struct FileMode {
    enum {
        Read = 0b0001,
        Write = 0b0010,

        // Other options
        Append = 0b0100,

        ReadWrite = Read | Write
    };
};

class DW_API File : public Object, public InputStream, public OutputStream {
public:
    DW_OBJECT(File);

    File(Context* context);

    File(Context* context, const Path& path, int mode = FileMode::Read);
    ~File() override;

    usize readData(void* dest, usize size) override;
    void seek(usize position) override;

    usize writeData(const void* src, usize size) override;

    bool open(const Path& path, int mode);
    void close();

private:
    FILE* handle_;
    int mode_;

    String fileModeMapper(int mode);
};
}  // namespace dw
