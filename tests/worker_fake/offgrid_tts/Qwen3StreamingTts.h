#pragma once
// Protocol test double only. No production engine/GPU qualification.
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <stdexcept>
struct TtsStreamChunk {std::vector<float> samples;int sample_rate=24000;};
struct TtsStreamOptions {std::string model_identifier;int seed;bool play_streaming,print_progress,print_timing,paced_audio_delivery;};
struct Qwen3StreamingTts {
 bool load(const char*,const char*){return true;}
 bool load_speaker_embedding(const char*){return true;}
 template<class Callback> bool synthesize_streaming(const std::string& text,const TtsStreamOptions&,Callback callback){
  if(text=="fail")return false;if(text=="throw")throw std::runtime_error("fake native failure");
  if(text=="empty")return true;
  for(int i=0;i<100;++i){
   TtsStreamChunk chunk;chunk.samples.assign(text=="backpressure"?65536:32,text=="second"?.2f:.1f);
   if(!callback(chunk))return false;
   std::this_thread::sleep_for(std::chrono::milliseconds(2));
  }return true;
 }
};
