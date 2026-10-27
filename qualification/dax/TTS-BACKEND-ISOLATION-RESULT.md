# Component backend isolation — initial result, 2026-10-27

Candidate runtime files at b8875c89ba287721bb82405eda86830388fca21e,
built from preserved fork7a686960 plus recorded runtime patch. Default timing
configuration; timing-enabled repair remains unqualified. Candidate build and
wrapper/kernel checks passed. Isolation is opt-in with literal
`ARBOREAL_ISOLATED_BACKENDS=1`; other values retain shared backend behavior.

Twelve fresh-process trials completed successfully (three per combination),
F32 transpose enabled, fixed public sentence/seed42, 2CPU/6GB container limits,
GTX1080, no competing containers. Generated11.497seconds of audio each.

| Backend | Decode | Median synthesis | Median first PCM | Sampled GPU peak |
|---|---|---:|---:|---:|
| Shared | Async | 7.512s | 337ms | 2447MiB |
| Shared | Sync | 7.541s | 328ms | 2447MiB |
| Separate | Async | 7.465s | 338ms | 2449MiB |
| Separate | Sync | 7.544s | 327ms | 2449MiB |

All twelve WAV files have the same SHA256. This establishes exact output
consistency for this fixture across these trials, not general perceptual quality.
Memory sampled every100ms is a lower bound on instantaneous peak. The2MiB
observed difference is small; a three-repeat comparison is insufficient to
claim a meaningful speed improvement. Main purpose is removal of shared mutable
component backend state, not speed.

Evidence: results/backend-isolation-20261027T064831Z/ and
results/backend-comparison.tgz. Earlier attempt064656Z exited127 before synthesis
because the runner omitted /build library paths; preserved separately, excluded
from these numbers. Initial source review found a concurrency hazard; the earlier
illegal access remains unlocalized and this experiment does not prove causation.

Still required: timing-enabled build, sustained repeated/variable-length requests,
cancellation/error/teardown, concurrent independent users and ASR coexistence.
No live deployment or default change is authorized by this result alone.
