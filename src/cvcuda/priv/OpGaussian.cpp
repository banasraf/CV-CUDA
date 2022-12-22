/*
 * SPDX-FileCopyrightText: Copyright (c) 2022 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "OpGaussian.hpp"

#include "legacy/CvCudaLegacy.h"
#include "legacy/CvCudaLegacyHelpers.hpp"

#include <nvcv/Exception.hpp>
#include <util/CheckError.hpp>

namespace cvcuda::priv {

namespace legacy = nvcv::legacy::cuda_op;

Gaussian::Gaussian(nvcv::Size2D maxKernelSize, int maxBatchSize)
{
    legacy::DataShape maxIn, maxOut; //maxIn/maxOut not used by op.
    m_legacyOp         = std::make_unique<legacy::Gaussian>(maxIn, maxOut, maxKernelSize);
    m_legacyOpVarShape = std::make_unique<legacy::GaussianVarShape>(maxIn, maxOut, maxKernelSize, maxBatchSize);
}

void Gaussian::operator()(cudaStream_t stream, const nvcv::ITensor &in, const nvcv::ITensor &out,
                          nvcv::Size2D kernelSize, double2 sigma, NVCVBorderType borderMode) const
{
    auto *inData = dynamic_cast<const nvcv::ITensorDataStridedCuda *>(in.exportData());
    if (inData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Input must be cuda-accessible, pitch-linear tensor");
    }

    auto *outData = dynamic_cast<const nvcv::ITensorDataStridedCuda *>(out.exportData());
    if (outData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Output must be cuda-accessible, pitch-linear tensor");
    }

    NVCV_CHECK_THROW(m_legacyOp->infer(*inData, *outData, kernelSize, sigma, borderMode, stream));
}

void Gaussian::operator()(cudaStream_t stream, const nvcv::IImageBatchVarShape &in, nvcv::IImageBatchVarShape &out,
                          const nvcv::ITensor &kernelSize, const nvcv::ITensor &sigma, NVCVBorderType borderMode) const
{
    auto *inData = dynamic_cast<const nvcv::IImageBatchVarShapeDataStridedCuda *>(in.exportData(stream));
    if (inData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Input must be cuda-accessible, varshape pitch-linear image batch");
    }

    auto *outData = dynamic_cast<const nvcv::IImageBatchVarShapeDataStridedCuda *>(out.exportData(stream));
    if (outData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Output must be cuda-accessible, varshape pitch-linear image batch");
    }

    auto *kernelSizeData = dynamic_cast<const nvcv::ITensorDataStridedCuda *>(kernelSize.exportData());
    if (kernelSizeData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Kernel size must be cuda-accessible, pitch-linear tensor");
    }

    auto *sigmaData = dynamic_cast<const nvcv::ITensorDataStridedCuda *>(sigma.exportData());
    if (sigmaData == nullptr)
    {
        throw nvcv::Exception(nvcv::Status::ERROR_INVALID_ARGUMENT,
                              "Kernel sigma must be cuda-accessible, pitch-linear tensor");
    }

    NVCV_CHECK_THROW(m_legacyOpVarShape->infer(*inData, *outData, *kernelSizeData, *sigmaData, borderMode, stream));
}

} // namespace cvcuda::priv
