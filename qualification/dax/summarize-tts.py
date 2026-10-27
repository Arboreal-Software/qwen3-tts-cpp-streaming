"""Summarize actual CLI receipts; fail on missing or unsuccessful synthesis."""
import argparse
import hashlib
import json
from pathlib import Path
import re
import wave

p=argparse.ArgumentParser();p.add_argument('directory',type=Path);args=p.parse_args()
rows=[]
for path in sorted(args.directory.glob('*.log')):
 text=path.read_text()
 final=re.findall(r'synthesis_completed wall_ms=(\d+) success=(\w+) audio_ms=([\d.]+)',text)
 if len(final)!=1 or final[0][1]!='yes':raise SystemExit(f'No unique successful synthesis receipt: {path.name}')
 wall,_,duration=final[0];wall=float(wall);duration=float(duration)
 wav=path.with_suffix('.wav')
 with wave.open(str(wav)) as f:
  actual_ms=1000*f.getnframes()/f.getframerate()
  if abs(actual_ms-duration)>1:raise SystemExit(f'WAV/receipt duration mismatch: {path.name}')
  fmt={'sample_rate':f.getframerate(),'channels':f.getnchannels(),'sample_width':f.getsampwidth()}
 def metric(pattern):
  match=re.search(pattern,text)
  if not match:raise SystemExit(f'Missing metric {pattern}: {path.name}')
  return float(match[1])
 rows.append({'name':path.stem,'synthesis_ms':wall,'audio_ms':actual_ms,
 'generation_rtf':wall/actual_ms,'first_pcm_ms':metric(r'first PCM ready:\s+(\d+)'),
 'max_window_gap_ms':metric(r'max window gap ms:\s+(\d+)'),
 'vocoder_ms':metric(r'Vocoder decode:\s+(\d+)'),
 'code_generation_ms':metric(r'Code generation:\s+(\d+)'),
 'wav_sha256':hashlib.sha256(wav.read_bytes()).hexdigest(),'format':fmt})
if not rows:raise SystemExit('No receipts found')
result={'scope':'Synthetic CLI synthesis only; no physical playback or live two-session capacity qualification','rows':rows}
if {'baseline','single'}<={r['name'] for r in rows}:
 byname={r['name']:r for r in rows}
 result['baseline_candidate_wav_equal']=byname['baseline']['wav_sha256']==byname['single']['wav_sha256']
 result['solo_speedup']=byname['baseline']['synthesis_ms']/byname['single']['synthesis_ms']
(args.directory/'summary.json').write_text(json.dumps(result,indent=2)+'\n')
print(json.dumps(result,indent=2))
