// Qualification probe, not a network service. One engine stays loaded across requests.
#include "offgrid_tts/Qwen3StreamingTts.h"
#include <chrono>
#include <cstdio>
#include <vector>
#include <cstring>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <cstdlib>
using Clock=std::chrono::steady_clock;
static double ms(Clock::time_point a,Clock::time_point b){return std::chrono::duration<double,std::milli>(b-a).count();}
int main(int argc,char**argv){
 if(argc!=3){fprintf(stderr,"usage: probe MODEL_DIR SPEAKER_EMBEDDING\n");return 2;}
 Qwen3StreamingTts engine;auto start=Clock::now();
 if(!engine.load(argv[1],"qwen3-tts-0.6b-f16")||!engine.load_speaker_embedding(argv[2]))return 3;
 printf("PERSISTENT load_ms=%.3f\n",ms(start,Clock::now()));
 TtsStreamOptions o;o.model_identifier="qwen3-tts-0.6b-f16";o.seed=42;o.play_streaming=false;o.print_progress=false;o.print_timing=true;o.paced_audio_delivery=false;
 std::vector<float> reference;
 for(int turn=0;turn<4;++turn){
  bool intermediate=turn==2;bool cancel=intermediate&&std::getenv("DAX_COMPLETE_INTERMEDIATE")==nullptr;int calls=0;bool cancel_requested=false;int after_cancel=0;
  double first=-1;Clock::time_point cancelled;std::vector<float> pcm;
  auto began=Clock::now();
  bool ok=engine.synthesize_streaming(intermediate?"This request should stop after the first audio chunk. The rest of this fairly long sentence must not keep speaking after interruption.":"Dax is ready for a voice test. This sentence checks clear speech and timing.",o,[&](const TtsStreamChunk& c){
   if(cancel_requested)++after_cancel;
   if(!c.samples.empty()){
    if(first<0)first=ms(began,Clock::now());
    pcm.insert(pcm.end(),c.samples.begin(),c.samples.end());++calls;
    if(cancel&&!cancel_requested){cancel_requested=true;cancelled=Clock::now();return false;}
   }
   return !cancel_requested;
  });
  auto ended=Clock::now();
  std::string output=std::getenv("DAX_PROBE_OUTPUT")?std::getenv("DAX_PROBE_OUTPUT"):"/work";
  std::ofstream dump(output+"/persistent-pcm-turn-"+std::to_string(turn)+".f32",std::ios::binary);
  dump.write(reinterpret_cast<const char*>(pcm.data()),pcm.size()*sizeof(float));dump.close();
  double max_difference=0,squared_error=0;size_t differing=0;
  if(!intermediate&&turn>0&&pcm.size()==reference.size())for(size_t i=0;i<pcm.size();++i){double d=double(pcm[i])-reference[i];max_difference=std::max(max_difference,std::abs(d));squared_error+=d*d;if(d!=0)++differing;}
  printf("PCM_DIFFERENCE turn=%d max_abs=%.12g rms=%.12g differing=%zu\n",turn,max_difference,pcm.empty()?0:std::sqrt(squared_error/pcm.size()),differing);
  bool equal=turn==0||intermediate||(pcm.size()==reference.size()&&memcmp(pcm.data(),reference.data(),pcm.size()*sizeof(float))==0);
  printf("PERSISTENT turn=%d success=%d first_ms=%.3f total_ms=%.3f samples=%zu calls=%d cancel_return_ms=%.3f callbacks_after_cancel=%d equal_reference=%d\n",turn,int(ok),first,ms(began,ended),pcm.size(),calls,cancel_requested?ms(cancelled,ended):-1,after_cancel,int(equal));fflush(stdout);
  if(turn==0)reference=pcm;
  if(!intermediate&&(!ok||pcm.empty()||!equal))return 4;
  if(cancel&&(!cancel_requested||after_cancel))return 5;
 }
 return 0;
}
