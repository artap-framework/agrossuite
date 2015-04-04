// This file is part of Agros.
//
// Agros is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// Agros is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Agros.  If not, see <http://www.gnu.org/licenses/>.
//
//
// University of West Bohemia, Pilsen, Czech Republic
// Email: info@agros2d.org, home page: http://agros2d.org/

#include "info.hpp"

#include <sstream>  

#include "../paralution.hpp"
#include "../utils/def.hpp"

#ifdef SUPPORT_CUDA
#include "../base/gpu/backend_gpu.hpp"
#endif

#ifdef SUPPORT_OCL
#include "../base/ocl/backend_ocl.hpp"
#include "../base/ocl/ocl_utils.hpp"
#endif

#ifdef SUPPORT_MIC
#include "../base/mic/backend_mic.hpp"
#endif

using namespace paralution;

std::map<std::string, std::string> info_ocl()
{
    std::map<std::string, std::string> out;

    // opencl
    cl_uint ocl_freq;
    cl_ulong ocl_global_mem_size;
    cl_device_type ocl_typeDevice;

    char ocl_namePlatform[256];
    char ocl_nameDevice[256];
    char ocl_ver[256];
    std::string ocl_type;

    // Get and print OpenCL platform name
    clGetDeviceInfo(OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_devices[_get_backend_descriptor()->OCL_plat][_get_backend_descriptor()->OCL_dev], CL_DEVICE_NAME, sizeof(ocl_nameDevice), &ocl_nameDevice, NULL);
    // Get and print OpenCL device name
    clGetPlatformInfo(OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_platforms[_get_backend_descriptor()->OCL_plat], CL_PLATFORM_NAME, sizeof(ocl_namePlatform), &ocl_namePlatform, NULL);
    // Get and print OpenCL device global memory
    clGetDeviceInfo(OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_devices[_get_backend_descriptor()->OCL_plat][_get_backend_descriptor()->OCL_dev], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(ocl_global_mem_size), &ocl_global_mem_size, NULL);
    // Get and print OpenCL device clock frequency
    clGetDeviceInfo(OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_devices[_get_backend_descriptor()->OCL_plat][_get_backend_descriptor()->OCL_dev], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(ocl_freq), &ocl_freq, NULL);
    // Get and print OpenCL device version
    clGetDeviceInfo(OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_devices[_get_backend_descriptor()->OCL_plat][_get_backend_descriptor()->OCL_dev], CL_DEVICE_VERSION, sizeof(ocl_ver), &ocl_ver, NULL);

    // Get and print OpenCL device type
    clGetDeviceInfo((OCL_HANDLE(_get_backend_descriptor()->OCL_handle)->OCL_devices)[_get_backend_descriptor()->OCL_plat][_get_backend_descriptor()->OCL_dev], CL_DEVICE_TYPE, sizeof(ocl_typeDevice), &ocl_typeDevice, NULL);
    if (ocl_typeDevice & CL_DEVICE_TYPE_CPU) ocl_type = "CPU";
    if (ocl_typeDevice & CL_DEVICE_TYPE_GPU) ocl_type = "GPU";
    if (ocl_typeDevice & CL_DEVICE_TYPE_ACCELERATOR) ocl_type = "ACCELERATOR";
    if (ocl_typeDevice & CL_DEVICE_TYPE_DEFAULT) ocl_type = "DEFAULT";

    std::stringstream smemory;
    smemory << (ocl_global_mem_size >> 20);
    std::stringstream sfreq;
    sfreq << ocl_freq;
    
    out["platform"] = ocl_namePlatform;
    out["device"] = ocl_nameDevice;
    out["type"] = ocl_type;
    out["memory"] = smemory.str();
    out["clock_rate"] = sfreq.str();
    out["version"] = ocl_ver;
    
    return out;
}

std::map<std::string, std::string> info_gpu()
{
    std::map<std::string, std::string> out;

    std::stringstream smemory;
    smemory << 0; // (ocl_global_mem_size >> 20);
    std::stringstream sfreq;
    sfreq << 0; // ocl_freq;
    
    out["platform"] = ""; // ocl_namePlatform;
    out["device"] = ""; // ocl_nameDevice;
    out["type"] = ""; // ocl_type;
    out["memory"] = smemory.str();
    out["clock_rate"] = sfreq.str();
    out["version"] = ""; // ocl_ver;

    return out;
}
