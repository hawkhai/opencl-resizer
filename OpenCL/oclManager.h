//
// Created by Vaclav Samec on 4/28/15 AD.
// Copyright (c) 2015 Venca. All rights reserved.
//

#pragma once

#define __CL_ENABLE_EXCEPTIONS
#if defined(__APPLE__) || defined(__MACOSX)
#include "cl.hpp"
#else
#include <CL/cl.h>
#include <CL/opencl.hpp>
#endif

#include <string>

class Image;
class oclManager
{
public:

    enum DeviceType
    {
        CPU = CL_DEVICE_TYPE_CPU,
        GPU = CL_DEVICE_TYPE_GPU
    };

    bool createContext(DeviceType type);

    bool addKernelProgram(const std::string& kernel);

    void resizeImage(const Image& in, Image& out, float ratio, const std::string& programEntry,
        unsigned long& time1,
        unsigned long& time2,
         unsigned long& time3
        );

protected:

    void setupPlatform(DeviceType type);

    static char *getCLErrorString(cl_int err);

	cl::ImageFormat getImageFormat(const Image&) const;

	static const std::string preferredDeviceVendors[];

	cl::Platform        m_platform;
	cl::Device          m_device;
    cl::Context         m_context;
    cl::Program         m_program;
    cl::CommandQueue    m_queue;
};
