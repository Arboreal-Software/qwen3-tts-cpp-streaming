"""Exercise the actual worker process with a deterministic engine double."""
import os, select, struct, subprocess, sys, unittest
HEADER=struct.Struct('<cQI')
BINARY=sys.argv.pop()
class Worker:
    def __init__(self):
        self.read,self.write=os.pipe()
        self.p=subprocess.Popen([BINARY,'models','speaker',str(self.write)],stdin=subprocess.PIPE,
            stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL,pass_fds=(self.write,))
        os.close(self.write)
        assert self.event()[:2]==(b'R',0)
    def exact(self,n):
        out=b''
        while len(out)<n:
            if not select.select([self.read],[],[],4)[0]:raise TimeoutError('No worker event')
            data=os.read(self.read,n-len(out))
            if not data:raise EOFError('Worker closed event pipe')
            out+=data
        return out
    def event(self):
        kind,identity,n=HEADER.unpack(self.exact(13))
        assert n<=262144
        return kind,identity,self.exact(n)
    def send(self,kind,identity,body=b''):
        self.p.stdin.write(HEADER.pack(kind,identity,len(body))+body);self.p.stdin.flush()
    def close(self):
        if self.p.poll() is None:self.p.kill()
        self.p.wait(timeout=4);self.p.stdin.close();os.close(self.read)
class Protocol(unittest.TestCase):
    def setUp(self):self.w=Worker()
    def tearDown(self):self.w.close()
    def test_cancel_then_reuse_has_no_old_audio(self):
        w=self.w;w.send(b'S',1,b'first');self.assertEqual(w.event()[:2],(b'A',1));w.send(b'C',1)
        while True:
            event=w.event();self.assertEqual(event[1],1)
            if event[0]==b'C':break
            self.assertEqual(event[0],b'A') # Already queued audio must be dropped by caller.
        w.send(b'S',2,b'second');count=0
        while True:
            kind,identity,data=w.event();self.assertEqual(identity,2)
            if kind==b'D':break
            self.assertEqual(kind,b'A');self.assertAlmostEqual(struct.unpack('<f',data[:4])[0],.2);count+=1
        self.assertEqual(count,100)
    def test_cancel_after_completion_does_not_poison_reuse(self):
        w=self.w;w.send(b'S',1,b'first')
        while w.event()[0]!=b'D':pass
        w.send(b'C',1);w.send(b'S',2,b'second')
        while True:
            kind,identity,_=w.event();self.assertEqual(identity,2)
            if kind==b'D':break
            self.assertEqual(kind,b'A')

    def test_cancel_for_older_request_is_fatal(self):
        w=self.w;w.send(b'S',1,b'first')
        while w.event()[0]!=b'D':pass
        w.send(b'S',2,b'second');w.send(b'C',1)
        self.assertEqual(w.p.wait(timeout=4),70)

    def test_duplicate_request_is_fatal(self):
        self.w.send(b'S',1,b'first');self.w.send(b'S',1,b'first')
        self.assertEqual(self.w.p.wait(timeout=4),70)
    def test_backpressure_is_bounded(self):
        self.w.send(b'S',1,b'backpressure');self.assertEqual(self.w.p.wait(timeout=4),70)
    def test_native_failure_is_fatal(self):
        self.w.send(b'S',1,b'fail');self.assertEqual(self.w.p.wait(timeout=4),70)
    def test_native_exception_is_fatal(self):
        self.w.send(b'S',1,b'throw');self.assertEqual(self.w.p.wait(timeout=4),70)
    def test_empty_success_is_fatal(self):
        self.w.send(b'S',1,b'empty');self.assertEqual(self.w.p.wait(timeout=4),70)
    def test_partial_header_is_fatal(self):
        self.w.p.stdin.write(b'S');self.w.p.stdin.close();self.assertEqual(self.w.p.wait(timeout=4),70)
if __name__=='__main__':unittest.main()
