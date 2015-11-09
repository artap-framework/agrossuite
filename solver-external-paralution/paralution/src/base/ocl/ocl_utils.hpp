// **************************************************************************
//
//    PARALUTION   www.paralution.com
//
//    Copyright (C) 2015  PARALUTION Labs UG (haftungsbeschränkt) & Co. KG
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRA 706051
//                        Vertreten durch:
//                        PARALUTION Labs Verwaltungs UG (haftungsbeschränkt)
//                        Am Hasensprung 6, 76571 Gaggenau
//                        Handelsregister: Amtsgericht Mannheim, HRB 721277
//                        Geschäftsführer: Dimitar Lukarski, Nico Trost
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// **************************************************************************



// PARALUTION version 1.0.0 


#ifndef PARALUTION_OCL_OCL_UTILS_HPP_
#define PARALUTION_OCL_OCL_UTILS_HPP_

#include "../backend_manager.hpp"
#include "../../utils/log.hpp"
#include "backend_ocl.hpp"

#if defined(__APPLE__) && defined(__MACH__)
  #include <OpenCL/cl.h>
#else
  #include <CL/cl.h>
#endif

#define OCL_HANDLE(handle) static_cast<oclHandle_t*>(handle)

static const char *OCL_ERROR[] = {
  "CL_SUCCESS",
  "CL_DEVICE_NOT_FOUND",
  "CL_DEVICE_NOT_AVAILABLE",
  "CL_COMPILER_NOT_AVAILABLE",
  "CL_MEM_OBJECT_ALLOCATION_FAILURE",
  "CL_OUT_OF_RESOURCES",
  "CL_OUT_OF_HOST_MEMORY",
  "CL_PROFILING_INFO_NOT_AVAILABLE",
  "CL_MEM_COPY_OVERLAP",
  "CL_IMAGE_FORMAT_MISMATCH",
  "CL_IMAGE_FORMAT_NOT_SUPPORTED",
  "CL_BUILD_PROGRAM_FAILURE",
  "CL_MAP_FAILURE",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "",
  "CL_INVALID_VALUE",
  "CL_INVALID_DEVICE_TYPE",
  "CL_INVALID_PLATFORM",
  "CL_INVALID_DEVICE",
  "CL_INVALID_CONTEXT",
  "CL_INVALID_QUEUE_PROPERTIES",
  "CL_INVALID_COMMAND_QUEUE",
  "CL_INVALID_HOST_PTR",
  "CL_INVALID_MEM_OBJECT",
  "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR",
  "CL_INVALID_IMAGE_SIZE",
  "CL_INVALID_SAMPLER",
  "CL_INVALID_BINARY",
  "CL_INVALID_BUILD_OPTIONS",
  "CL_INVALID_PROGRAM",
  "CL_INVALID_PROGRAM_EXECUTABLE",
  "CL_INVALID_KERNEL_NAME",
  "CL_INVALID_KERNEL_DEFINITION",
  "CL_INVALID_KERNEL",
  "CL_INVALID_ARG_INDEX",
  "CL_INVALID_ARG_VALUE",
  "CL_INVALID_ARG_SIZE",
  "CL_INVALID_KERNEL_ARGS",
  "CL_INVALID_WORK_DIMENSION",
  "CL_INVALID_WORK_GROUP_SIZE",
  "CL_INVALID_WORK_ITEM_SIZE",
  "CL_INVALID_GLOBAL_OFFSET",
  "CL_INVALID_EVENT_WAIT_LIST",
  "CL_INVALID_EVENT",
  "CL_INVALID_OPERATION",
  "CL_INVALID_GL_OBJECT",
  "CL_INVALID_BUFFER_SIZE",
  "CL_INVALID_MIP_LEVEL",
  "CL_INVALID_GLOBAL_WORK_SIZE"
};

#define CHECK_OCL_ERROR(err_t, file, line) {                                                    \
  if ( err_t != CL_SUCCESS ) {                                                                  \
    LOG_INFO("OPENCL ERROR: " <<OCL_ERROR[-err_t]);                                             \
    LOG_INFO("File: " << file << "; line: " << line);                                           \
    paralution_stop_ocl();                                                                      \
    exit(1);                                                                                    \
  }                                                                                             \
}


#endif // PARALUTION_OCL_OCL_UTILS_HPP_
