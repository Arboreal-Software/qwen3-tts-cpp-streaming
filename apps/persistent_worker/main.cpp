// Linux process-owned streaming worker. Private pipe protocol; no network ingress.
#include "offgrid_tts/Qwen3StreamingTts.h"
#include <atomic>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <csignal>
#include <cstdlib>
#include <fcntl.h>
#include <mutex>
#include <poll.h>
#include <thread>
#include <unistd.h>

namespace {
[[noreturn]] void fail() { _exit(70); } // Poisoned/unknown state is never reused.
bool read_all(void* p, size_t n) {
    auto* b=static_cast<unsigned char*>(p);const size_t wanted=n;
    while(n) { auto k=read(STDIN_FILENO,b,n); if(k<0&&errno==EINTR)continue;
        if(k<=0){if(n!=wanted)fail();return false;} b+=k;n-=size_t(k); }
    return true;
}
uint64_t unpack(const unsigned char* b,int n) {
    uint64_t v=0;for(int i=0;i<n;++i)v|=uint64_t(b[i])<<(8*i);return v;
}
void pack(unsigned char* b,uint64_t v,int n) {for(int i=0;i<n;++i)b[i]=static_cast<unsigned char>(v>>(8*i));}
void write_all(int fd,const void* p,size_t n) {
    auto* b=static_cast<const unsigned char*>(p);
    auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(n) {
        if(std::chrono::steady_clock::now()>=deadline)fail();
        auto k=write(fd,b,n);
        if(k>0){b+=k;n-=size_t(k);continue;}
        if(k<0&&errno==EINTR)continue;
        if(k<0&&(errno==EAGAIN||errno==EWOULDBLOCK)){
            pollfd event{fd,POLLOUT,0};if(poll(&event,1,20)<0&&errno!=EINTR)fail();continue;
        }
        fail();
    }
}
void emit(int fd,char type,uint64_t id,const void* body=nullptr,uint32_t size=0) {
    unsigned char h[13];h[0]=type;pack(h+1,id,8);pack(h+9,size,4);
    write_all(fd,h,sizeof h);if(size)write_all(fd,body,size);
}
}
int main(int argc,char** argv) {
    if(argc!=4)return 2;
    char* end=nullptr;long parsed=strtol(argv[3],&end,10);
    if(!end||*end||parsed<3||parsed>1024)return 2;
    int output=int(parsed);int flags=fcntl(output,F_GETFL);
    if(flags<0||fcntl(output,F_SETFL,flags|O_NONBLOCK)<0)return 2;
    signal(SIGPIPE,SIG_IGN);
    Qwen3StreamingTts engine;
    if(!engine.load(argv[1],"qwen3-tts-0.6b-f16")||!engine.load_speaker_embedding(argv[2]))return 3;
    std::mutex mutex;std::condition_variable wake;
    bool busy=false,pending=false,closing=false;uint64_t current=0,last=0;
    std::string text;std::atomic<bool> cancelled{false};
    emit(output,'R',0);
    std::thread worker([&]{
        try {
            for(;;) {
                std::unique_lock<std::mutex> lock(mutex);
                wake.wait(lock,[&]{return pending||closing;});if(closing&&!pending)return;
                auto id=current;auto request=text;pending=false;lock.unlock();
                TtsStreamOptions options;options.model_identifier="qwen3-tts-0.6b-f16";
                options.seed=42;options.play_streaming=false;options.print_progress=false;
                options.require_natural_eos=true;
                // Opt-in diagnostics stay on the bounded supervisor log pipe,
                // never the binary audio/control pipe. Include engine EOS versus
                // safety-limit termination when investigating incomplete speech.
                const char* diagnostics=std::getenv("ARBOREAL_TTS_DIAGNOSTICS");
                options.print_timing=diagnostics&&std::string(diagnostics)=="1";
                options.paced_audio_delivery=false;
                size_t emitted=0;
                bool ok=engine.synthesize_streaming(request,options,[&](const TtsStreamChunk& chunk){
                    if(cancelled.load())return false;
                    if(chunk.sample_rate!=24000||chunk.samples.size()>65536)fail();
                    for(float v:chunk.samples)if(!std::isfinite(v))fail();
                    if(!chunk.samples.empty()){emit(output,'A',id,chunk.samples.data(),uint32_t(chunk.samples.size()*sizeof(float)));emitted+=chunk.samples.size();}
                    return !cancelled.load();
                });
                lock.lock();
                if((!ok||!emitted)&&!cancelled.load())fail();
                // Busy remains true until the terminal event is on the pipe.
                emit(output,cancelled.load()?'C':'D',id);busy=false;
            }
        } catch(...) {fail();}
    });
    for(;;) {
        unsigned char h[13];if(!read_all(h,sizeof h))break;
        const auto id=unpack(h+1,8);const auto size=unpack(h+9,4);
        if(size>6000)fail();
        std::string body(size,'\0');if(size&&!read_all(body.data(),size))fail();
        std::lock_guard<std::mutex> lock(mutex);
        if(h[0]=='S') {
            if(busy||id==0||id<=last||body.empty()||body.find('\0')!=std::string::npos)fail();
            current=last=id;text=std::move(body);cancelled=false;busy=pending=true;wake.notify_one();
        } else if(h[0]=='C') {
            if(size||id==0||id!=current)fail();
            // The controller may send cancellation while the terminal event is
            // in flight. Its exact most-recent request is already settled; do
            // not emit another terminal or poison the next request.
            if(busy)cancelled=true;
        } else {fail();}
    }
    {std::lock_guard<std::mutex> lock(mutex);closing=true;cancelled=true;wake.notify_one();}
    worker.join();return 0;
}
