#ifdef NDEBUG
#undef NDEBUG
#endif
// Actual GGML attention contraction, deterministic data, no model or audio device.
#include "ggml.h"
#include "ggml-backend.h"
#include "ggml-cpu.h"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <vector>

std::vector<float> attention(int capacity,bool active_only,bool poison_tail) {
    constexpr int D=64,H=2,valid=2,queries=2;
    auto backend=ggml_backend_cpu_init();assert(backend);
    auto ctx=ggml_init({4*1024*1024,nullptr,true});assert(ctx);
    auto Kcache=ggml_new_tensor_3d(ctx,GGML_TYPE_F16,D,H,capacity);
    auto Vcache=ggml_new_tensor_3d(ctx,GGML_TYPE_F16,D,H,capacity);
    auto Qcur=ggml_new_tensor_3d(ctx,GGML_TYPE_F32,D,H,queries);
    const int extent=active_only?valid:capacity;
    // Preserve physical cache strides, changing only valid read extent.
    auto K=ggml_view_3d(ctx,Kcache,D,H,extent,Kcache->nb[1],Kcache->nb[2],0);
    auto V=ggml_view_3d(ctx,Vcache,D,H,extent,Vcache->nb[1],Vcache->nb[2],0);
    auto Q=ggml_permute(ctx,Qcur,0,2,1,3);
    K=ggml_permute(ctx,K,0,2,1,3);V=ggml_permute(ctx,V,0,2,1,3);
    auto scores=ggml_mul_mat(ctx,K,Q);
    scores=ggml_scale(ctx,scores,1.0f/std::sqrt(float(D)));
    scores=ggml_diag_mask_inf(ctx,scores,0);
    auto probabilities=ggml_soft_max(ctx,scores);
    V=ggml_cont(ctx,ggml_transpose(ctx,V));
    auto result=ggml_mul_mat(ctx,V,probabilities);
    auto graph=ggml_new_graph_custom(ctx,256,false);ggml_build_forward_expand(graph,result);
    auto buffer=ggml_backend_alloc_ctx_tensors(ctx,backend);assert(buffer);
    std::vector<ggml_fp16_t> keys(D*H*capacity,ggml_fp32_to_fp16(0));
    std::vector<ggml_fp16_t> values(D*H*capacity,ggml_fp32_to_fp16(poison_tail?NAN:0));
    std::vector<float> query(D*H*queries);
    for(int t=0;t<valid;t++)for(int h=0;h<H;h++)for(int d=0;d<D;d++) {
        const int i=(t*H+h)*D+d;
        keys[i]=ggml_fp32_to_fp16(.07f*t+.03f*h+.001f*d);
        values[i]=ggml_fp32_to_fp16(.1f*t-.1f*h+.0001f*d);
    }
    for(int i=0;i<int(query.size());i++)query[i]=.01f*(i%7-3);
    ggml_backend_tensor_set(Kcache,keys.data(),0,keys.size()*sizeof(keys[0]));
    ggml_backend_tensor_set(Vcache,values.data(),0,values.size()*sizeof(values[0]));
    ggml_backend_tensor_set(Qcur,query.data(),0,query.size()*sizeof(float));
    assert(ggml_backend_graph_compute(backend,graph)==GGML_STATUS_SUCCESS);
    std::vector<float> output(ggml_nelements(result));
    ggml_backend_tensor_get(result,output.data(),0,output.size()*sizeof(float));
    ggml_backend_buffer_free(buffer);ggml_free(ctx);ggml_backend_free(backend);
    return output;
}
int main() {
    const auto reference=attention(16,true,false);
    assert(std::all_of(reference.begin(),reference.end(),[](float x){return std::isfinite(x);}));
    for(int capacity:{16,511,512,513,3839,3840,3841}) {
        const auto fixed=attention(capacity,true,true);
        const auto zero=attention(capacity,false,false);
        for(size_t i=0;i<reference.size();i++) {
            assert(std::isfinite(fixed[i]));assert(std::fabs(fixed[i]-reference[i])<1e-6f);
            assert(std::isfinite(zero[i]));assert(std::fabs(zero[i]-reference[i])<1e-5f);
        }
    }
    const auto poisoned_full=attention(512,false,true);
    const bool contamination=std::any_of(poisoned_full.begin(),poisoned_full.end(),[](float x){return !std::isfinite(x);});
    assert(contamination); // Demonstrate the original full-capacity read hazard.
    std::puts("KV tail poison: full-capacity attention contaminated; active-prefix attention invariant across7capacities");
}
