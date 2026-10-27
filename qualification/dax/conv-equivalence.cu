#include <cuda_runtime.h>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#define GGML_UNUSED_VARS(...) ((void)0)
#define CHECK(x) do { auto err=(x); if(err!=cudaSuccess){fprintf(stderr,"%s:%d %s\n",__FILE__,__LINE__,cudaGetErrorString(err));exit(1);} } while(0)
static  __global__ void conv_transpose_1d_kernel(
        const int s0, const int p0, const int d0, const int output_size,
        const int src0_ne0, const int src0_ne1, const int src0_ne2, const int src0_ne3,
        const int src1_ne0, const int src1_ne1, const int src1_ne2, const int src1_ne3, const int64_t src1_nb2,
        const int dst_ne0, const int dst_ne1, const int dst_ne2, const int dst_ne3,
        const float * src0, const float * src1,  float * dst) {
    int global_index = threadIdx.x + blockIdx.x * blockDim.x;
    if (global_index >= output_size) {
        return;
    }

    const int out_channel = (global_index / dst_ne0) % dst_ne1;
    const int batch = (global_index / (dst_ne0 * dst_ne1)) % dst_ne2;

    float accumulator = 0;

    for (int c = 0; c < src0_ne2; c++) {
        int idx = global_index % dst_ne0;

        int kernel_offset = (src0_ne0 * src0_ne1 * c) + (out_channel * src0_ne0);
        int64_t input_offset = batch * src1_nb2 + src1_ne0 * c;

        for (int i = 0; i < src1_ne0; i++) {
            if (!(idx >= i*s0 && idx < i*s0 + src0_ne0)) {
                continue;
            }
            int weight_idx = idx - i*s0;

            float kernel_weight = src0[kernel_offset + weight_idx];
            float input_value =  src1[input_offset+i];

            accumulator += kernel_weight * input_value;
        }
    }
    dst[global_index] = accumulator;
    GGML_UNUSED_VARS(p0, d0, src0_ne3, src1_ne3, dst_ne3, src1_ne2);
}

static  __global__ void bounded_kernel(
        const int s0, const int p0, const int d0, const int output_size,
        const int src0_ne0, const int src0_ne1, const int src0_ne2, const int src0_ne3,
        const int src1_ne0, const int src1_ne1, const int src1_ne2, const int src1_ne3, const int64_t src1_nb2,
        const int dst_ne0, const int dst_ne1, const int dst_ne2, const int dst_ne3,
        const float * src0, const float * src1,  float * dst) {
    int global_index = threadIdx.x + blockIdx.x * blockDim.x;
    if (global_index >= output_size) {
        return;
    }

    const int out_channel = (global_index / dst_ne0) % dst_ne1;
    const int batch = (global_index / (dst_ne0 * dst_ne1)) % dst_ne2;

    float accumulator = 0;

    for (int c = 0; c < src0_ne2; c++) {
        int idx = global_index % dst_ne0;

        int kernel_offset = (src0_ne0 * src0_ne1 * c) + (out_channel * src0_ne0);
        int64_t input_offset = batch * src1_nb2 + src1_ne0 * c;

        const int first = max(0, (idx - src0_ne0 + s0) / s0);
        const int last = min(src1_ne0 - 1, idx / s0);
        for (int i = first; i <= last; i++) {
            int weight_idx = idx - i*s0;

            float kernel_weight = src0[kernel_offset + weight_idx];
            float input_value =  src1[input_offset+i];

            accumulator += kernel_weight * input_value;
        }
    }
    dst[global_index] = accumulator;
    GGML_UNUSED_VARS(p0, d0, src0_ne3, src1_ne3, dst_ne3, src1_ne2);
}


int main(){
 int tests=0; size_t compared=0;
 for(int len: {1,3,17,64,257}) for(int k:{1,3,8,16}) for(int stride:{1,2,5,8}) {
  int ci=5,co=7,batch=2,out=(len-1)*stride+k,n=out*co*batch;
  std::vector<float>w(k*co*ci),x(len*ci*batch),ref(n),got(n);
  for(size_t i=0;i<w.size();++i)w[i]=float(int(i*17%101)-50)/37;
  for(size_t i=0;i<x.size();++i)x[i]=float(int(i*13%97)-48)/31;
  float *dw,*dx,*dr,*dg;CHECK(cudaMalloc(&dw,w.size()*4));CHECK(cudaMalloc(&dx,x.size()*4));CHECK(cudaMalloc(&dr,n*4));CHECK(cudaMalloc(&dg,n*4));
  CHECK(cudaMemcpy(dw,w.data(),w.size()*4,cudaMemcpyHostToDevice));CHECK(cudaMemcpy(dx,x.data(),x.size()*4,cudaMemcpyHostToDevice));
  conv_transpose_1d_kernel<<<(n+255)/256,256>>>(stride,0,1,n,k,co,ci,1,len,ci,batch,1,len*ci,out,co,batch,1,dw,dx,dr);
  CHECK(cudaGetLastError());
  bounded_kernel<<<(n+255)/256,256>>>(stride,0,1,n,k,co,ci,1,len,ci,batch,1,len*ci,out,co,batch,1,dw,dx,dg);
  CHECK(cudaGetLastError());CHECK(cudaDeviceSynchronize());
  CHECK(cudaMemcpy(ref.data(),dr,n*4,cudaMemcpyDeviceToHost));CHECK(cudaMemcpy(got.data(),dg,n*4,cudaMemcpyDeviceToHost));
  if(memcmp(ref.data(),got.data(),n*4)){fprintf(stderr,"Mismatch len=%d k=%d stride=%d\n",len,k,stride);return 2;}
  // Independent scalar reference tolerates CPU/GPU fused-arithmetic differences.
  for(int ba=0;ba<batch;++ba)for(int oc=0;oc<co;++oc)for(int t=0;t<out;++t){
   double sum=0;for(int c=0;c<ci;++c)for(int i=0;i<len;++i){int wi=t-i*stride;if(wi>=0&&wi<k)sum+=double(w[(c*co+oc)*k+wi])*x[(ba*ci+c)*len+i];}
   float actual=got[(ba*co+oc)*out+t];if(std::abs(sum-actual)>1e-4*(1+std::abs(sum))){fprintf(stderr,"CPU mismatch\n");return 3;}
  }
  compared+=n;++tests;CHECK(cudaFree(dw));CHECK(cudaFree(dx));CHECK(cudaFree(dr));CHECK(cudaFree(dg));
 }
 printf("PASS %d GPU shapes, %zu output samples; bitwise original/candidate equality and scalar reference tolerance\n",tests,compared);
}
