#include "cuda_runtime.h"

#define BLOCK_SIZE 32

#define BUF_SZ 5

typedef unsigned char uchar;

__global__ void cuDiff(const float *dataset, float *result_image, int w, int h)
{
	int tx = threadIdx.x;   int ty = threadIdx.y;
	int bx = blockIdx.x;	int by = blockIdx.y;

	int gx = bx * BLOCK_SIZE + tx;
	int gy = by * BLOCK_SIZE + ty;
	int s_idx = gy * w + gx;
	
	float sum = 0;
#pragma unroll
	for (int n = 0; n < BUF_SZ; n++) {
		sum += dataset[s_idx * BUF_SZ + n];
	}
	float avg = sum / BUF_SZ;

	float sum_diff_sq = 0;

	for (int n = 0; n < BUF_SZ; n++) {
		float diff = dataset[s_idx * BUF_SZ + n] - avg;
		sum_diff_sq += diff * diff;
	}

	float sdev = sqrtf(sum_diff_sq / BUF_SZ);

	result_image[s_idx] = fabsf(avg - dataset[s_idx * BUF_SZ + BUF_SZ-1]) > sdev ? 255.0f : 0.0f;
}

extern "C" bool cuImageProcessing(uchar *dataset, uchar *res, int w, int h)
{
	// convert to float
	float *pinned_dataset, *pinned_result_image;
	float *dev_dataset, *dev_res;

	cudaHostAlloc<float>((float**)&pinned_dataset, w * h * BUF_SZ * sizeof(float), cudaHostAllocDefault);
	cudaHostAlloc<float>((float**)&pinned_result_image, w * h * sizeof(float), cudaHostAllocDefault);

	// replace inner pixels with image data
	for (int y = 0; y < h; y++)
		for (int x = 0; x < w; x++)
			for (int n = 0; n < BUF_SZ; n++)
				pinned_dataset[(y * w + x) * BUF_SZ + n] = (float) dataset[(y * w + x) * BUF_SZ + n];

	cudaMalloc((void**)&dev_dataset, w * h * BUF_SZ * sizeof(float));
	cudaMalloc((void**)&dev_res, w * h * sizeof(float));

	cudaMemcpy(dev_dataset, pinned_dataset, w * h * BUF_SZ * sizeof(float), cudaMemcpyHostToDevice);

	dim3 dimGrid(w / BLOCK_SIZE, h / BLOCK_SIZE);
	dim3 dimBlock(BLOCK_SIZE, BLOCK_SIZE);

	cuDiff<<<dimGrid, dimBlock>>>(dev_dataset, dev_res, w, h);
	cudaDeviceSynchronize();

	cudaMemcpy(pinned_result_image, dev_res, w * h * sizeof(float), cudaMemcpyDeviceToHost);

	for (int i = 0; i < w * h; i++) {
		res[i] = (uchar)pinned_result_image[i];
	}

	cudaFree(dev_dataset);
	cudaFree(dev_res);
	cudaFreeHost(pinned_dataset);
	cudaFreeHost(pinned_result_image);

	return true;
}