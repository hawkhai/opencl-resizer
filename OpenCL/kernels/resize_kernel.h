#pragma once

// names of kernel entry points
static const std::vector<std::string> entries = { "resize_nn", "resize_linear", "resize_bicubic", "resize_supersample" };

// kernel with 4 resize methods
static const std::string resizeKernel =

"__constant sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_CLAMP_TO_EDGE | CLK_FILTER_NEAREST;\n"
"\n"
"//#pragma OPENCL EXTENSION cl_amd_printf : enable\n"
"//int printf(const char *restrict format, ...);\n"
"\n"
"struct CLImage\n"
"{\n"
"   uint Width;\n"
"   uint Height;\n"
"};\n"
"\n"
"//  nearest neighbour interpolation resizing\n"
"// ----------------------------------------------------------------------------------\n"
"//\n"
"__kernel void resize_nn(__read_only image2d_t source, __write_only image2d_t dest, struct CLImage src_img, struct CLImage dst_img, float ratioX, float ratioY)\n"
"{\n"
"   const int gx = get_global_id(0);\n"
"   const int gy = get_global_id(1);\n"
"   const int2 pos = { gx, gy };\n"
"\n"
"   if (pos.x >= dst_img.Width || pos.y >= dst_img.Height)\n"
"      return;\n"
"\n"
"   float2 srcpos = {(pos.x + 0.4995f) / ratioX, (pos.y + 0.4995f) / ratioY};\n"
"   int2 SrcSize = (int2)(src_img.Width, src_img.Height);\n"
"\n"
"   float4 value;\n"
"\n"
"   int2 ipos = convert_int2(srcpos);\n"
"   if (ipos.x < 0 || ipos.x >= SrcSize.x || ipos.y < 0 || ipos.y >= SrcSize.y)\n"
"      value = 0;\n"
"   else\n"
"      value = read_imagef(source, sampler, ipos);\n"
"\n"
"   write_imagef(dest, pos, value);\n"
"}\n"
"\n"
"//  linear interpolation resizing\n"
"// ----------------------------------------------------------------------------------\n"
"//\n"
"__kernel void resize_linear(__read_only image2d_t source, __write_only image2d_t dest, struct CLImage src_img, struct CLImage dst_img, float ratioX, float ratioY)\n"
"{\n"
"   const int gx = get_global_id(0);\n"
"   const int gy = get_global_id(1);\n"
"   const int2 pos = { gx, gy };\n"
"\n"
"   if (pos.x >= dst_img.Width || pos.y >= dst_img.Height)\n"
"      return;\n"
"\n"
"   float2 srcpos = {(pos.x + 0.4995f) / ratioX, (pos.y + 0.4995f) / ratioY};\n"
"   int2 SrcSize = { (int)src_img.Width, (int)src_img.Height };\n"
"\n"
"   float4 value;\n"
"\n"
"   if ((int)(srcpos.x + .5f) == SrcSize.x)\n"
"      srcpos.x = SrcSize.x - 0.5001f;\n"
"\n"
"   if ((int)(srcpos.y + .5f) == SrcSize.y)\n"
"      srcpos.y = SrcSize.y - 0.5001f;\n"
"\n"
"   srcpos -= (float2)(0.5f, 0.5f);\n"
"\n"
"   if (srcpos.x < -0.5f || srcpos.x >= SrcSize.x - 1 || srcpos.y < -0.5f || srcpos.y >= SrcSize.y - 1)\n"
"      value = 0;\n"
"\n"
"   int x1 = (int)(srcpos.x);\n"
"   int x2 = (int)(srcpos.x + 1);\n"
"   int y1 = (int)(srcpos.y);\n"
"   int y2 = (int)(srcpos.y + 1);\n"
"\n"
"   float factorx1 = 1 - (srcpos.x - x1);\n"
"   float factorx2 = 1 - factorx1;\n"
"   float factory1 = 1 - (srcpos.y - y1);\n"
"   float factory2 = 1 - factory1;\n"
"\n"
"   float4 f1 = factorx1 * factory1;\n"
"   float4 f2 = factorx2 * factory1;\n"
"   float4 f3 = factorx1 * factory2;\n"
"   float4 f4 = factorx2 * factory2;\n"
"\n"
"   const int2 pos0 = { x1, y1 };\n"
"   const int2 pos1 = { x2, y1 };\n"
"   const int2 pos2 = { x1, y2 };\n"
"   const int2 pos3 = { x2, y2 };\n"
"\n"
"   float4 v1 = read_imagef(source, sampler, pos0);\n"
"   float4 v2 = read_imagef(source, sampler, pos1);\n"
"   float4 v3 = read_imagef(source, sampler, pos2);\n"
"   float4 v4 = read_imagef(source, sampler, pos3);\n"
"   value =  v1 * f1 + v2 * f2 + v3 * f3 + v4 * f4;\n"
"\n"
"   write_imagef(dest, pos, value);\n"
"}\n"
"\n"
"\n"
"//  bicubic interpolation resizing\n"
"// ----------------------------------------------------------------------------------\n"
"//\n"
"float4 sample_bicubic_border(__read_only image2d_t source, float2 pos, int2 SrcSize)\n"
"{\n"
"   int2 isrcpos = convert_int2(pos);\n"
"   float dx = pos.x - isrcpos.x;\n"
"   float dy = pos.y - isrcpos.y;\n"
"\n"
"   float4 C[4] = {0, 0, 0, 0};\n"
"\n"
"   if (isrcpos.x < 0 || isrcpos.x >= SrcSize.x)\n"
"      return 0;\n"
"\n"
"   if (isrcpos.y < 0 || isrcpos.y >= SrcSize.y)\n"
"      return 0;\n"
"\n"
"   for (int i = 0; i < 4; i++)\n"
"   {\n"
"      int y = isrcpos.y - 1 + i;\n"
"      if (y < 0)\n"
"         y = 0;\n"
"\n"
"      if (y >= SrcSize.y)\n"
"         y = SrcSize.y - 1;\n"
"\n"
"      int Middle = clamp(isrcpos.x, 0, SrcSize.x - 1);\n"
"\n"
"      const int2 pos0 = { Middle, y };\n"
"      float4 center = read_imagef(source, sampler, pos0);\n"
"\n"
"      float4 left = 0, right1 = 0, right2 = 0;\n"
"      if (isrcpos.x - 1 >= 0)\n"
"      {\n"
"         const int2 pos1 = { isrcpos.x - 1, y };\n"
"         left = read_imagef(source, sampler, pos1);\n"
"      }\n"
"      else\n"
"      {\n"
"         left = center;\n"
"      }\n"
"\n"
"      if (isrcpos.x + 1 < SrcSize.x)\n"
"      {\n"
"         const int2 pos2 = { isrcpos.x + 1, y };\n"
"         right1 = read_imagef(source, sampler, pos2);\n"
"      }\n"
"      else\n"
"      {\n"
"         right1 = center;\n"
"      }\n"
"\n"
"      if (isrcpos.x + 2 < SrcSize.x)\n"
"      {\n"
"         const int2 pos3 = { isrcpos.x + 2, y };\n"
"         right2 = read_imagef(source, sampler, pos3);\n"
"      }\n"
"      else\n"
"      {\n"
"         right2 = right1;\n"
"      }\n"
"\n"
"      float4 a0 = center;\n"
"      float4 d0 = left - a0;\n"
"      float4 d2 = right1 - a0;\n"
"      float4 d3 = right2 - a0;\n"
"\n"
"      float4 a1 = -1.0f / 3 * d0 + d2 - 1.0f / 6 * d3;\n"
"      float4 a2 =  1.0f / 2 * d0 + 1.0f / 2 * d2;\n"
"      float4 a3 = -1.0f / 6 * d0 - 1.0f / 2 * d2 + 1.0f / 6 * d3;\n"
"      C[i] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;\n"
"   }\n"
"\n"
"   float4 d0 = C[0] - C[1];\n"
"   float4 d2 = C[2] - C[1];\n"
"   float4 d3 = C[3] - C[1];\n"
"   float4 a0 = C[1];\n"
"   float4 a1 = -1.0f / 3 * d0 + d2 -1.0f / 6 * d3;\n"
"   float4 a2 = 1.0f / 2 * d0 + 1.0f / 2 * d2;\n"
"   float4 a3 = -1.0f / 6 * d0 - 1.0f / 2 * d2 + 1.0f / 6 * d3;\n"
"\n"
"   return a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy;\n"
"}\n"
"\n"
"__kernel void resize_bicubic(__read_only image2d_t source, __write_only image2d_t dest, struct CLImage src_img, struct CLImage dst_img, float ratioX, float ratioY)\n"
"{\n"
"   const int gx = get_global_id(0);\n"
"   const int gy = get_global_id(1);\n"
"   const int2 pos = { gx, gy };\n"
"\n"
"   if (pos.x >= dst_img.Width || pos.y >= dst_img.Height)\n"
"      return;\n"
"\n"
"   float2 srcpos = {(pos.x + 0.4995f) / ratioX, (pos.y + 0.4995f) / ratioY};\n"
"   int2 SrcSize = { (int)src_img.Width, (int)src_img.Height };\n"
"\n"
"   float4 value;\n"
"\n"
"   srcpos -= (float2)(0.5f, 0.5f);\n"
"\n"
"   int2 isrcpos = convert_int2(srcpos);\n"
"   float dx = srcpos.x - isrcpos.x;\n"
"   float dy = srcpos.y - isrcpos.y;\n"
"\n"
"   if (isrcpos.x <= 0 || isrcpos.x >= SrcSize.x - 2)\n"
"      value = sample_bicubic_border(source, srcpos, SrcSize);\n"
"\n"
"   if (isrcpos.y <= 0 || isrcpos.y >= SrcSize.y - 2)\n"
"      value = sample_bicubic_border(source, srcpos, SrcSize);\n"
"\n"
"   float4 C[4] = {0, 0, 0, 0};\n"
"\n"
"   for (int i = 0; i < 4; i++)\n"
"   {\n"
"      const int y = isrcpos.y - 1 + i;\n"
"\n"
"      const int2 pos0 = { isrcpos.x, y };\n"
"      const int2 pos1 = { isrcpos.x - 1, y };\n"
"      const int2 pos2 = { isrcpos.x + 1, y };\n"
"      const int2 pos3 = { isrcpos.x + 2, y };\n"
"\n"
"      float4 a0 = read_imagef(source, sampler, pos0);\n"
"      float4 d0 = read_imagef(source, sampler, pos1) - a0;\n"
"      float4 d2 = read_imagef(source, sampler, pos2) - a0;\n"
"      float4 d3 = read_imagef(source, sampler, pos3) - a0;\n"
"\n"
"      float4 a1 = -1.0f / 3 * d0 + d2 - 1.0f / 6 * d3;\n"
"      float4 a2 =  1.0f / 2 * d0 + 1.0f / 2 * d2;\n"
"      float4 a3 = -1.0f / 6 * d0 - 1.0f / 2 * d2 + 1.0f / 6 * d3;\n"
"      C[i] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;\n"
"   }\n"
"\n"
"   float4 d0 = C[0] - C[1];\n"
"   float4 d2 = C[2] - C[1];\n"
"   float4 d3 = C[3] - C[1];\n"
"   float4 a0 = C[1];\n"
"   float4 a1 = -1.0f / 3 * d0 + d2 -1.0f / 6 * d3;\n"
"   float4 a2 = 1.0f / 2 * d0 + 1.0f / 2 * d2;\n"
"   float4 a3 = -1.0f / 6 * d0 - 1.0f / 2 * d2 + 1.0f / 6 * d3;\n"
"   value = a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy;\n"
"\n"
"   write_imagef(dest, pos, value);\n"
"}\n"
"\n"
"\n"
"//  super-sample interpolation resizing\n"
"// ----------------------------------------------------------------------------------\n"
"//\n"
"float4 supersample_border(__read_only image2d_t source, float2 pos, int2 SrcSize, float2 ratio)\n"
"{\n"
"   float4 sum = 0;\n"
"   float factor_sum = 0;\n"
"\n"
"   float2 start = pos - ratio / 2;\n"
"   float2 end = start + ratio;\n"
"   int2 istart = convert_int2(start);\n"
"   int2 length = convert_int2(end - convert_float2(istart)) + 1;\n"
"\n"
"   float2 factors = 1.f / ratio;\n"
"\n"
"   for (int iy = 0; iy < length.y; iy++)\n"
"   {\n"
"      int y = istart.y + iy;\n"
"\n"
"      if (y < 0 || y >= SrcSize.y)\n"
"         continue;\n"
"\n"
"      float factor_y = factors.y;\n"
"      if (y < start.y)\n"
"         factor_y = factors.y * (y + 1 - start.y);\n"
"\n"
"      if (y + 1 > end.y)\n"
"         factor_y = factors.y * (end.y - y);\n"
"\n"
"      for (int ix = 0; ix < length.x; ix++)\n"
"      {\n"
"         int x = istart.x + ix;\n"
"\n"
"         if (x < 0 || x >= SrcSize.x)\n"
"            continue;\n"
"\n"
"         float factor_x = factors.x;\n"
"         if (x < start.x)\n"
"            factor_x = factors.x * (x + 1 - start.x);\n"
"\n"
"         if (x + 1 > end.x)\n"
"            factor_x = factors.x * (end.x - x);\n"
"\n"
"         float factor = factor_x * factor_y;\n"
"\n"
"         const int2 pos0 = { x, y };\n"
"\n"
"         sum += read_imagef(source, sampler, pos0) * factor;\n"
"\n"
"         factor_sum += factor;\n"
"      }\n"
"   }\n"
"\n"
"   sum /= factor_sum;\n"
"\n"
"   return sum;\n"
"}\n"
"\n"
"kernel void resize_supersample(__read_only image2d_t source, __write_only image2d_t dest, struct CLImage src_img, struct CLImage dst_img, float ratioX, float ratioY)\n"
"{\n"
"   const int gx = get_global_id(0);\n"
"   const int gy = get_global_id(1);\n"
"   const int2 pos = { gx, gy };\n"
"\n"
"   if (pos.x >= dst_img.Width || pos.y >= dst_img.Height)\n"
"      return;\n"
"\n"
"   ratioX = 1.0f / ratioX;\n"
"   ratioY = 1.0f / ratioY;\n"
"\n"
"   float2 srcpos = {(pos.x + 0.4995f) * ratioX, (pos.y + 0.4995f) * ratioY};\n"
"   int2 SrcSize = { (int)src_img.Width, (int)src_img.Height };\n"
"\n"
"   float4 value;\n"
"\n"
"   float2 ratio = (float2)(ratioX, ratioY);\n"
"\n"
"   float4 sum = 0;\n"
"\n"
"   float2 start = srcpos - ratio / 2;\n"
"   float2 end = start + ratio;\n"
"   int2 istart = convert_int2(start);\n"
"   int2 length = convert_int2(end - convert_float2(istart)) + 1;\n"
"\n"
"   float2 factors = 1.f / ratio;\n"
"\n"
"   if (start.x < 0 || start.y < 0 || end.x > SrcSize.x || end.y > SrcSize.y)\n"
"      value = supersample_border(source, srcpos, SrcSize, ratio);\n"
"\n"
"   for (int iy = 0; iy < length.y; iy++)\n"
"   {\n"
"      int y = istart.y + iy;\n"
"\n"
"      float factor_y = factors.y;\n"
"      if (y < start.y)\n"
"         factor_y = factors.y * (y + 1 - start.y);\n"
"\n"
"      if (y + 1 > end.y)\n"
"         factor_y = factors.y * (end.y - y);\n"
"\n"
"      for (int ix = 0; ix < length.x; ix++)\n"
"      {\n"
"         int x = istart.x + ix;\n"
"\n"
"         float factor_x = factors.x;\n"
"         if (x < start.x)\n"
"            factor_x = factors.x * (x + 1 - start.x);\n"
"\n"
"         if (x + 1 > end.x)\n"
"            factor_x = factors.x * (end.x - x);\n"
"\n"
"         float factor = factor_x * factor_y;\n"
"\n"
"         const int2 pos0 = { x, y };\n"
"\n"
"         sum += read_imagef(source, sampler, pos0) * factor;\n"
"      }\n"
"   }\n"
"\n"
"   value = sum;\n"
"\n"
"   write_imagef(dest, pos, value);\n"
"}\n"
"\n"
"kernel void test(__read_only image2d_t source, __write_only image2d_t outputImage)\n"
"{\n"
"// get id of element in array\n"
"   int x = get_global_id(0);\n"
"   int y = get_global_id(1);\n"
"   int w = get_global_size(0);\n"
"   int h = get_global_size(1);\n"
"   \n"
"   float4 result = (float4)(0.0f,0.0f,0.0f,1.0f);\n"
"   float MinRe = -2.0f;\n"
"   float MaxRe = 1.0f;\n"
"   float MinIm = -1.5f;\n"
"   float MaxIm = MinIm+(MaxRe-MinRe)*h/w;\n"
"   float Re_factor = (MaxRe-MinRe)/(w-1);\n"
"   float Im_factor = (MaxIm-MinIm)/(h-1);\n"
"   float MaxIterations = 250;\n"
"\n"
"   //C imaginary\n"
"   float c_im = MaxIm - y*Im_factor;\n"
"\n"
"   //C real\n"
"   float c_re = MinRe + x*Re_factor;\n"
"\n"
"   //Z real\n"
"   float Z_re = c_re, Z_im = c_im;\n"
"\n"
"   bool isInside = true;\n"
"   bool col2 = false;\n"
"   bool col3 = false;\n"
"   int iteration =0;\n"
"\n"
"   for(int n=0; n<MaxIterations; n++)\n"
"   {\n"
"      // Z - real and imaginary\n"
"      float Z_re2 = Z_re*Z_re, Z_im2 = Z_im*Z_im;\n"
"            \n"
"      //if Z real squared plus Z imaginary squared is greater than c squared\n"
"      if(Z_re2 + Z_im2 > 4)\n"
"      {\n"
"         if(n >= 0 && n <= (MaxIterations/2-1))\n"
"         {\n"
"            col2 = true;\n"
"            isInside = false;\n"
"            break;\n"
"         }\n"
"         else if(n >= MaxIterations/2 && n <= MaxIterations-1)\n"
"         {\n"
"            col3 = true;\n"
"            isInside = false;\n"
"            break;\n"
"         }\n"
"      }\n"
"      Z_im = 2*Z_re*Z_im + c_im;\n"
"      Z_re = Z_re2 - Z_im2 + c_re;\n"
"      iteration++;\n"
"   }\n"
"   if(col2) \n"
"   { \n"
"      result = (float4)(iteration*0.05f,0.0f, 0.0f, 1.0f);\n"
"   }\n"
"   else if(col3)\n"
"   {\n"
"      result = (float4)(255, iteration*0.05f, iteration*0.05f, 1.0f);\n"
"   }\n"
"   else if(isInside)\n"
"   {\n"
"      result = (float4)(0.0f, 0.0f, 0.0f, 1.0f);\n"
"   }\n"
"\n"
"   write_imagef(outputImage, (int2)(x, y), result);\n"
"}\n"
"";
