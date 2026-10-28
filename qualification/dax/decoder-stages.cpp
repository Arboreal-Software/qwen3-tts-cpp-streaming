#include "audio_tokenizer_decoder.h"
#include <cstdio>
#include <vector>
int main(int argc,char**argv){
 if(argc!=2)return 2;
 qwen3_tts::AudioTokenizerDecoder d;
 if(!d.load_model(argv[1])){fprintf(stderr,"%s\n",d.get_error().c_str());return 3;}
 for(int frames:{6,14,24}){
  std::vector<int32_t>codes(frames*d.get_config().n_codebooks);
  for(size_t i=0;i<codes.size();++i)codes[i]=(i*13+7)%d.get_config().codebook_size;
  std::vector<float>audio;if(!d.decode(codes.data(),frames,audio))return 4;
  for(int repeat=0;repeat<3;++repeat){
   std::vector<qwen3_tts::decoder_profile_row>rows;
   if(!d.profile_decode(codes.data(),frames,rows)){fprintf(stderr,"%s\n",d.get_error().c_str());return 5;}
   for(auto&r:rows)printf("STAGE frames=%d repeat=%d name=%s cumulative_ms=%lld delta_ms=%lld bytes=%zu\n",frames,repeat,r.stage.c_str(),(long long)r.cumulative_ms,(long long)r.delta_ms,r.bytes);
   fflush(stdout);
  }
 }
}
