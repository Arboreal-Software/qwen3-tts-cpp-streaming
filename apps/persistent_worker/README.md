# Experimental persistent pipe worker

Build with `-DARBOREAL_BUILD_PERSISTENT_WORKER=ON`, target
`arboreal_tts_worker`. Linux x86-64 only (PCM is native little-endian float32).
Start with `MODEL_DIR SPEAKER_EMBEDDING EVENT_FD`. The supervisor passes a private
writable pipe FD >=3; stdin is a separate command pipe. stdout/stderr are engine
logs, never protocol. Model and speaker paths are trusted startup configuration,
not request-controlled. One worker owns one loaded engine and speaker.

Each frame is a 13-byte header: one ASCII kind byte, little-endian uint64 request
ID, little-endian uint32 payload length, then payload. Commands:

- `S`: start UTF-8 text, 1–6000 bytes, no NUL. IDs must increase, one outstanding
  request; there is no hidden request queue.
- `C`: cancel the exact active ID, empty payload.

Events: `R` ready (ID zero), `A` audio (24kHz mono float32, <=262144 bytes),
`D` synthesis complete, `C` cancellation settled. Terminal events have no payload.
Completion means generated, not played or acknowledged by a listener. The caller
must fence/discard queued audio immediately when cancelling, then await terminal
before reusing the worker. Already-piped chunks can precede cancellation's terminal.
No old-request events follow that terminal. Seed42 is fixed for qualification;
waveform equality is not an acceptance condition.

Malformed/stale/busy commands, failed or empty synthesis, native exceptions,
nonfinite PCM, broken output or >2-second output backpressure terminate with70.
A supervisor must fence the request, replace the process on failure, and never
replay emitted speech. It also needs startup/request/cancellation deadlines and
must kill a hung native process: callback cancellation alone cannot interrupt a
hung CUDA kernel. EOF asks for cancellation and shutdown. Authentication, client
identity, bounded queues, delivery acknowledgements and cross-process routing
belong in the separate service supervisor, not this worker.

Tests `tests/worker_protocol_test.py` execute this source with the explicit
`tests/worker_fake` engine double. They establish protocol behavior only. Real
Pascal execution, chunk gaps, coexistence, listening quality, memory stability,
cross-user isolation and actual CUDA crash/recovery remain qualification tasks.
No network endpoint or live voice route is changed by building this target.

A cancel for the exact most-recent completed ID is a no-op: its terminal event
may have been in flight when the controller cancelled. It emits no second
terminal. Cancel for an older/different ID is still fatal, including when a
newer request is active. This avoids turning the normal completion/cancellation
race into a process failure while retaining request ownership checks.
