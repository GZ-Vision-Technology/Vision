//
// Copyright 2016 Pixar
//
// Licensed under the Apache License, Version 2.0 (the "Apache License")
// with the following modification; you may not use this file except in
// compliance with the Apache License and the following modification to it:
// Section 6. Trademarks. is deleted and replaced with:
//
// 6. Trademarks. This License does not grant permission to use the trade
//    names, trademarks, service marks, or product names of the Licensor
//    and its affiliates, except as required to comply with Section 4(c) of
//    the License and to reproduce the content of the NOTICE file.
//
// You may obtain a copy of the Apache License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the Apache License with the above modification is
// distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
// KIND, either express or implied. See the Apache License for the specific
// language governing permissions and limitations under the Apache License.
//
#ifndef PXR_BASE_VT_HASH_H
#define PXR_BASE_VT_HASH_H

#include "pxr/pxr.h"
#include "pxr/base/vt/api.h"
#include "pxr/base/tf/hash.h"
#include <typeinfo>
#include <utility>

PXR_NAMESPACE_OPEN_SCOPE

namespace Vt_HashDetail {

// Issue a coding error when we attempt to hash a t.
VT_API void _IssueUnimplementedHashError(std::type_info const &t);

// A constexpr function that determines hashability.
template <class T, class = decltype(TfHash()(std::declval<T>()))>
constexpr bool _IsHashable(long) { return true; }
template <class T>
constexpr bool _IsHashable(...) { return false; }

// Hash implementations -- We're using an overload resolution ordering trick
// here (long vs ...) so that we pick TfHash() if possible, otherwise
// we issue a runtime error.
template <class T, class = decltype(TfHash()(std::declval<T>()))>
inline size_t
_HashValueImpl(T const &val, long)
{
    return TfHash()(val);
}

template <class T>
inline size_t
_HashValueImpl(T const &val, ...)
{
    Vt_HashDetail::_IssueUnimplementedHashError(typeid(T));
    return 0;
}

} // Vt_HashDetail


/// A constexpr function that returns true if T is hashable via VtHashValue,
/// false otherwise.  This is true if we can invoke TfHash()() on a T instance.
template <class T>
constexpr bool
VtIsHashable() {
    return Vt_HashDetail::_IsHashable<T>(0);
}

/// Compute a hash code for \p val by invoking TfHash()(val) or when not
/// possible issue a coding error and return 0.
template <class T>
size_t VtHashValue(T const &val)
{
    return Vt_HashDetail::_HashValueImpl(val, 0);
}

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_BASE_VT_HASH_H
