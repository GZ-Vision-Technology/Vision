//
// Copyright 2023 Pixar
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

#ifndef PXR_USD_IMAGING_USD_IMAGING_DATA_SOURCE_USD_PRIM_INFO_H
#define PXR_USD_IMAGING_USD_IMAGING_DATA_SOURCE_USD_PRIM_INFO_H

#include "pxr/imaging/hd/dataSource.h"
#include "pxr/usd/usd/prim.h"

PXR_NAMESPACE_OPEN_SCOPE

/// \class UsdImagingDataSourceUsdPrimInfo
///
/// A container data source containing metadata such as
/// the specifier of a prim of native instancing information.
class UsdImagingDataSourceUsdPrimInfo : public HdContainerDataSource
{
public:
    HD_DECLARE_DATASOURCE(UsdImagingDataSourceUsdPrimInfo);
    
    TfTokenVector GetNames() override;
    HdDataSourceBaseHandle Get(const TfToken &name) override;

    ~UsdImagingDataSourceUsdPrimInfo();
    
private:
    UsdImagingDataSourceUsdPrimInfo(UsdPrim usdPrim);

private:
    UsdPrim _usdPrim;
};

HD_DECLARE_DATASOURCE_HANDLES(UsdImagingDataSourceUsdPrimInfo);

PXR_NAMESPACE_CLOSE_SCOPE

#endif // PXR_USD_IMAGING_USD_IMAGING_DATA_SOURCE_USD_H
