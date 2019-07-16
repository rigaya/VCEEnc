// TypeIn
// TypeOut
// IMAGE_SRC
// IMAGE_DST
// in_bit_depth
// out_bit_depth

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE | CLK_ADDRESS_NONE | CLK_FILTER_NEAREST;

#ifndef __OPENCL_VERSION__
#define __kernel 
#define __global
#define __local
#define __read_only
#define __write_only
#define image2d_t void*
#define uchar unsigned char
#endif

#define BIT_DEPTH_CONV(x) (TypeOut)((out_bit_depth == in_bit_depth) \
    ? (x) \
    : ((out_bit_depth > in_bit_depth) \
        ? ((int)(x) << (out_bit_depth - in_bit_depth)) \
        : ((int)(x) >> (in_bit_depth - out_bit_depth))))

#define BIT_DEPTH_CONV_FLOAT(x) (TypeOut)((out_bit_depth == in_bit_depth) \
    ? (x) \
    : ((out_bit_depth > in_bit_depth) \
        ? ((x) * (float)(1 << (out_bit_depth - in_bit_depth))) \
        : ((x) * (float)(1.0f / (1 << (in_bit_depth - out_bit_depth))))))

#define BIT_DEPTH_CONV_AVG(a, b) (TypeOut)((out_bit_depth == in_bit_depth + 1) \
    ? ((int)(a) + (int)(b) + 1) \
    : ((out_bit_depth > in_bit_depth + 1) \
        ? (((int)(a) + (int)(b) + 1) << (out_bit_depth - in_bit_depth - 1)) \
        : (((int)(a) + (int)(b) + (1 << (in_bit_depth - out_bit_depth))) >> (in_bit_depth + 1 - out_bit_depth))))

#define BIT_DEPTH_CONV_3x1_AVG(a, b) (TypeOut)((out_bit_depth == in_bit_depth + 2) \
    ? ((int)(a) * 3 + (int)(b) + 2) \
    : ((out_bit_depth > in_bit_depth + 2) \
        ? (((int)(a) * 3 + (int)(b) + 2) << (out_bit_depth - in_bit_depth - 2)) \
        : (((int)(a) * 3 + (int)(b) + (2 << (in_bit_depth - out_bit_depth))) >> (in_bit_depth + 2 - out_bit_depth))))

#if IMAGE_SRC
#define LOAD(src, x, y) (TypeIn)(read_imageui((src), sampler, (int2)((x), (y))).x)
#define LOAD_NV12_UV(src, src_u, src_v, x, y, cropX, cropY) { \
	uint4 ret = read_imageui((src), sampler, (int2)((x) + ((cropX)>>1), (y) + ((cropY)>>1))); \
    (src_u) = (TypeIn)ret.x; \
    (src_v) = (TypeIn)ret.w; \
}
#else
#define LOAD(src, x, y) *(__global TypeIn *)(&(src)[(y) * srcPitch + (x) * sizeof(TypeIn)])
#define LOAD_NV12_UV(src, src_u, src_v, x, y, cropX, cropY) { \
    (src_u) = LOAD((src), ((x)<<1) + 0 + (cropX), (y) + ((cropY)>>1)); \
    (src_v) = LOAD((src), ((x)<<1) + 1 + (cropX), (y) + ((cropY)>>1)); \
}
#endif

#if IMAGE_DST
#define STORE(dst, x, y, val) write_imageui((dst), (int2)((x), (y)), (val))
#define STORE_NV12_UV(dst, x, y, val_u, val_v) { \
	uint4 val = (uint4)(val_u, val_v, val_v, val_v); \
	STORE((dst), (x), (y), val); \
}
#else
#define STORE(dst, x, y, val)  { \
	__global TypeOut *ptr = (__global TypeOut *)(&(dst)[(y) * dstPitch + (x) * sizeof(TypeOut)]); \
	ptr[0] = (TypeOut)(val); \
}
#define STORE_NV12_UV(dst, x, y, val_u, val_v) { \
	STORE(dst, ((x) << 1) + 0, (y), val_u); \
	STORE(dst, ((x) << 1) + 1, (y), val_v); \
}
#endif

__kernel void kernel_copy_plane(
#if IMAGE_DST
    __write_only image2d_t dst,
#else
    __global uchar *dst,
#endif
    int dstPitch,
#if IMAGE_SRC
    __read_only image2d_t src,
#else
    __global uchar *src,
#endif
    int srcPitch,
    int width,
    int height,
    int cropX,
    int cropY
) {
    const int x = get_global_id(0);
    const int y = get_global_id(1);
    
    if (x < width && y < height) {
        TypeIn pixSrc = LOAD(src, x + cropX, y + cropY);
		TypeOut out = BIT_DEPTH_CONV(pixSrc);
		STORE(dst, x, y, out);
    }
}

__kernel void kernel_crop_nv12_yv12(
#if IMAGE_DST
    __write_only image2d_t dstU,
    __write_only image2d_t dstV,
#else
    __global uchar *dstU,
    __global uchar *dstV,
#endif
    int dstPitch,
#if IMAGE_SRC
    __read_only image2d_t src,
#else
    __global uchar *src,
#endif
    int srcPitch,
    int uvWidth,
    int uvHeight,
    int cropX,
    int cropY
) {
    const int uv_x = get_global_id(0);
    const int uv_y = get_global_id(1);
    if (uv_x < uvWidth && uv_y < uvHeight) {
		TypeIn pixSrcU, pixSrcV;
		LOAD_NV12_UV(src, pixSrcU, pixSrcV, uv_x, uv_y, cropX, cropY);
        TypeOut pixDstU = BIT_DEPTH_CONV(pixSrcU);
        TypeOut pixDstV = BIT_DEPTH_CONV(pixSrcV);
		STORE(dstU, uv_x, uv_y, pixDstU);
		STORE(dstV, uv_x, uv_y, pixDstV);
    }
}

__kernel void kernel_crop_yv12_nv12(
#if IMAGE_DST
    __write_only image2d_t dst,
#else
    __global uchar *dst,
#endif
    int dstPitch,
#if IMAGE_SRC
    __read_only image2d_t srcU,
    __read_only image2d_t srcV,
#else
    __global uchar *srcU,
    __global uchar *srcV,
#endif
    int srcPitch,
    int uvWidth,
    int uvHeight,
    int cropX,
    int cropY
) {
    const int uv_x = get_global_id(0);
    const int uv_y = get_global_id(1);
    
    if (uv_x < uvWidth && uv_y < uvHeight) {
        TypeIn pixSrcU = LOAD(srcU, uv_x + (cropX>>1), uv_y + (cropY>>1));
        TypeIn pixSrcV = LOAD(srcV, uv_x + (cropX>>1), uv_y + (cropY>>1));
        TypeOut pixDstU = BIT_DEPTH_CONV(pixSrcU);
        TypeOut pixDstV = BIT_DEPTH_CONV(pixSrcV);
		STORE_NV12_UV(dst, uv_x, uv_y, pixDstU, pixDstV);
    }
}
