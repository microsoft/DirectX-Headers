// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#pragma once

#ifndef __cplusplus
#error "This header requires C++"
#endif

template <typename T> GUID uuidof() {
    return __uuidof(T);
}
template <typename T> GUID uuidof(T*) { return uuidof<T>(); }
template <typename T> GUID uuidof(T**) { return uuidof<T>(); }
template <typename T> GUID uuidof(T&) { return uuidof<T>(); }

