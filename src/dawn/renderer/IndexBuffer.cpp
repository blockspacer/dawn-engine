/*
 * Dawn Engine
 * Written by David Avedissian (c) 2012-2017 (git@dga.me.uk)
 */
#include "Common.h"
#include "renderer/IndexBuffer.h"

namespace dw {
IndexBuffer::IndexBuffer(Context* context) : Object(context) {
}

IndexBuffer::~IndexBuffer() {
}

// IndexBufferHandle IndexBuffer::internalHandle() const {
//    return handle_;
//}
}  // namespace dw