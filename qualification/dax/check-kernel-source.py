"""Prevent the standalone copied-kernel test silently testing stale source."""
from pathlib import Path
import hashlib
import json
root = Path(__file__).resolve().parents[2]
manifest = json.loads((root / 'qualification/dax/provenance.json').read_text())
actual = hashlib.sha256((root / 'engine/ggml/src/ggml-cuda/conv-transpose-1d.cu').read_bytes()).hexdigest()
if actual != manifest['modified_cuda_source_sha256']:
    raise SystemExit('CUDA source changed: update reference/equivalence coverage before refreshing provenance.')
print('CUDA source matches qualified standalone kernel fixture.')
