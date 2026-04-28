dget - deterministic file retrieval utility
==========================================

dget is a lightweight C-based downloader designed for deterministic, manifest-driven file retrieval. It is built around strict execution order, predictable failure handling, and minimal runtime ambiguity.

Unlike typical download utilities, dget treats downloads as a controlled execution graph rather than ad-hoc requests.

---

Architecture
------------

dget consists of three core components:

1. CLI interface
   - parses direct URL fetches
   - dispatches manifest execution mode

2. Manifest engine
   - parses structured JSON input
   - resolves file download tasks sequentially
   - enforces strict schema validation

3. Network layer
   - built on libcurl
   - handles retries and failure propagation

---

Core behavior
--------------

Single file mode:

    ./dget <url>

Downloads a single resource directly to the current working directory.

Manifest mode:

    ./dget install manifest.json

Executes a structured set of download tasks defined in JSON format.

Execution model:
- sequential processing
- stop-on-failure semantics
- deterministic output paths

---

Manifest specification
----------------------

A valid manifest must follow this structure:

{
  "files": [
    {
      "url": "https://example.com/file1.bin",
      "output": "file1.bin"
    },
    {
      "url": "https://example.com/file2.bin",
      "output": "file2.bin"
    }
  ]
}

Rules:
- `files` must be an array
- each entry must contain:
  - url (string)
  - output (string)
- execution order is preserved
- failure of any entry aborts the process

---

Failure model
-------------

dget uses strict failure semantics:

- each file has a limited retry window
- repeated failure terminates execution
- no partial success continuation in manifest mode
- system prioritizes deterministic state over completion attempts

This is intentional to avoid inconsistent filesystem state.

---

Design goals
------------

The project is built around:

- predictable execution over heuristics
- explicit control flow over abstraction
- minimal dependency surface
- reproducible behavior across environments

It is not designed as a general-purpose downloader, but as a deterministic retrieval primitive.

---

Build
-----

Requirements:
- gcc
- libcurl
- make

Build:

    make

Clean:

    make clean

---

Status
------

Early-stage systems utility. Core execution model is stable; API and manifest schema may evolve.
