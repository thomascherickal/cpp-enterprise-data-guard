# Enterprise Data Guard — Operational Manual

## 1. How It Works

**Enterprise Data Guard** prevents API keys, credentials, and personal data (PII) from leaking into AI prompts by running a **C++17 scanning engine** directly inside the user's browser via **WebAssembly** (`data_guard.wasm`), governed by policies authored in a **native C++ GTK+ 3 Admin Control Centre** (`dataguard-admin`).

```
+-----------------------------------------------------------------------+
| 1. Admin Control Centre (C++ GTK+ 3: build/native/dataguard-admin)    |
|    - Configure rules, severity, entropy threshold, allowlists         |
|    - Test against sample prompts using shared C++ core                |
|    - Export policy.json                                               |
+-----------------------------------+-----------------------------------+
                                    | policy.json
                                    v
+-----------------------------------------------------------------------+
| 2. User Browser (Zero-Trust Client-Side Execution)                    |
|    - Web Dashboard (web/index.html) or AI Tool Interceptor            |
|    - Loads data_guard.wasm (compiled from the exact same C++ core)    |
|    - Scans & redacts prompts in WebAssembly linear memory             |
|    - Zero prompt text or findings ever leave the browser              |
+-----------------------------------------------------------------------+
```

Because both the Admin GUI and the browser WebAssembly module compile from `src/core/`, policy behavior tested by an administrator matches what every user experiences in the browser.

---

## 2. The C++ Scanning Pipeline

Every call to `Scanner::scan(input)` (or `wasm.scanText(text, policyJson)` in the browser) executes the following deterministic pipeline:

1. **Line/Column Indexing (`src/core/text_index.cpp`)**:
   - `TextIndex` records byte offsets of every newline (`\n`) in `input` so any match offset is mapped in `O(log N)` time to 1-based `(line, column)` coordinates.

2. **Policy & Severity Filtering (`src/core/policy.cpp`)**:
   - Parses `policy.json` using the zero-dependency JSON parser (`src/core/json.cpp`).
   - Enforces `minSeverity` (`LOW=1`, `MEDIUM=2`, `HIGH=3`, `CRITICAL=4`). Rules below `minSeverity` are skipped.

3. **Secret Detection (`src/core/rule_registry.cpp` & `src/core/scanner.cpp`)**:
   When `enableSecrets` is `true`, the scanner evaluates:
   - `SEC-AWS-KEY`: AWS Access Key IDs (`AKIA...`) — `CRITICAL`
   - `SEC-GITHUB-TOKEN`: GitHub PATs (`ghp_`, `gho_`, `ghu_`, `ghs_`, `ghr_`, `github_pat_`) — `CRITICAL`
   - `SEC-ANTHROPIC-KEY` / `SEC-OPENAI-KEY` / `SEC-GOOGLE-API-KEY` / `SEC-STRIPE-KEY` / `SEC-SLACK-TOKEN`
   - `SEC-PRIVATE-KEY`: PEM private key headers (`-----BEGIN ... PRIVATE KEY-----`) — `CRITICAL`
   - `SEC-JWT`: Three-part `eyJ...` JSON Web Tokens — `MEDIUM`
   - `SEC-DB-URI`: Database URIs with embedded credentials (`postgres://user:pass@...`, `mysql://`, `mongodb://`, `redis://`) — `CRITICAL`
   - `SEC-GENERIC-ASSIGN`: Hardcoded `password`, `secret`, `api_key`, `access_token`, or `auth_token` assignments — `HIGH`
   - `SEC-HIGH-ENTROPY-BEARER` (when `enableEntropy` is `true`): `Authorization: Bearer <token>` headers whose Shannon entropy meets or exceeds `entropyThreshold` (default `3.8` bits/char) — `HIGH`

4. **Personal Data (PII) Detection & Checksum Validation (`src/core/validators.cpp`)**:
   When `enablePii` is `true`, regex matches are verified by domain validators to eliminate false positives:
   - `PII-CREDIT-CARD`: 13–19 digit sequences validated with **Luhn mod-10** (`isValidLuhn`, rejecting all-identical digits) — `CRITICAL`
   - `PII-SSN`: `AAA-GG-SSSS` validated against invalid area/group/serial ranges (`000`, `666`, `900–999`, `00`, `0000`) — `CRITICAL`
   - `PII-IBAN`: 15–34 char IBANs validated with **ISO 13616 mod-97 == 1** (`isValidIban`) — `HIGH`
   - `PII-EMAIL` (`MEDIUM`), `PII-PHONE` (`LOW`), and optional `PII-IPV4` (`LOW`, controlled by `enableIpAddresses`).

5. **Custom Organization Rules & Allowlist Filtering**:
   - Enabled entries in `policy.customRules` are compiled and matched. Invalid regex patterns are caught and skipped safely at scan time.
   - Any match containing a substring from `policy.allowlist` (e.g., `AKIAIOSFODNN7EXAMPLE`, `test@example.com`) is ignored.

6. **Overlap Deduplication & Risk Scoring**:
   - Raw findings are sorted by `start` offset ascending, then `severityRank` descending, then match length descending. Overlapping lower-priority matches are dropped so redaction spans never collide.
   - `riskScore` (0–100) sums severity weights (`CRITICAL=40`, `HIGH=25`, `MEDIUM=15`, `LOW=5`, capped at `100`).
   - `blocked` is `true` if `blockOnCritical && hasCritical` or if `findings.size() >= maxFindingsBeforeBlock` (when `maxFindingsBeforeBlock > 0`).

7. **Redaction (`src/core/redactor.cpp`)**:
   `Redactor::apply` produces `redactedText` according to `policy.redactionMode`:
   - `PLACEHOLDER`: replaces each finding with `[REDACTED:<ruleId>]`
   - `MASK`: replaces with `validators::maskSecret` (e.g., `AKIA****CDEF`, or `****` for short strings)
   - `REMOVE`: strips the matched span completely

---

## 3. Browser Execution & Web Dashboard

1. **WASM Initialization (`web/js/wasm-client.js`)**:
   - `index.html` loads `wasm/data_guard.js` and calls `createDataGuardModule()`, which instantiates `wasm/data_guard.wasm` via Emscripten `embind` (`src/wasm/wasm_bridge.cpp`).
2. **Policy Resolution (`web/js/policy-ui.js`)**:
   - Fetches `./policy.json` (exported by the Admin Control Centre during deployment), falling back to `./policy.default.json` or `wasm.getDefaultPolicy()`.
   - Users can also import a custom `policy.json` file or toggle local overrides if needed.
3. **Real-Time Inspection (`web/app.js` & `web/js/findings-view.js`)**:
   - Typing in the prompt box triggers a debounced `wasm.scanText(input, policy)` call.
   - Findings are rendered via `textContent` DOM nodes (no `innerHTML` injection) with severity badges, rule IDs, `L:C` locations, and masked snippets.
   - Local scan counters are persisted only in browser `localStorage` (`web/js/stats-store.js`).
4. **AI Tool Interceptor (`web/interceptor.js`)**:
   - Listens in capture phase for `Enter` (without `Shift`) on `<textarea>`, `<input type="text">`, and `contenteditable` elements.
   - If `scanText` returns `blocked: true`, it cancels submission (`preventDefault` + `stopPropagation`) and offers to replace the input with `redactedText`.

---

## 4. Admin Control Centre Operations

### Interactive GTK+ 3 GUI
```bash
./scripts/run.sh --gui-only
```
- **Policy Configuration Tab**: Toggle Secrets, PII, Entropy Bearer, IPv4, Block on Critical; adjust Minimum Severity, Redaction Mode, Entropy Threshold, and Max Findings Before Block.
- **Allowlist & Custom Rules Tab**: Add allowlist substrings or custom regex rules with severity levels.
- **Live Scanner Test Bench Tab**: Run live C++ scans and inspect the exact JSON output before exporting `policy.json`.

### Headless CLI Mode (for CI/CD & Scripts)
```bash
# Export default enterprise policy
./build/native/dataguard-admin --headless --export-policy policy.json

# Scan text against a specific policy file
./build/native/dataguard-admin --headless --policy policy.json --scan "text to check"
```
**CLI Exit Codes**:
- `0`: Success / scan allowed (`blocked == false`)
- `3`: Scan blocked (`blocked == true`)
- `1`: File I/O or policy validation error
- `2`: Invalid CLI arguments

---

## 5. Day-to-Day Operational Procedures

### Updating & Rolling Out a New Policy
1. Open the Admin Control Centre (`./scripts/run.sh --gui-only`), update rules/allowlists, verify in the **Live Scanner Test Bench**, and export `policy.json`.
2. For **Enterprise** deployments, place the exported `policy.json` in `dist/enterprise/bundle/policy.json` (Nginx serves `/policy.json` with `Cache-Control: no-store` so browsers pick up changes on next load).
3. For **Local** deployments, place it in `dist/local/web/policy.json` or import it directly in the Web Dashboard.

### Verifying System Health & 100% Test Coverage
```bash
./scripts/run.sh --test
```
Runs C++ unit tests (`ctest`), verifies **100.00% `gcov` line coverage** across all 8 C++ core/controller modules (`scripts/check-coverage.py`), builds and tests `data_guard.wasm` in Node.js (`tests/wasm/test_wasm.mjs`), and smoke-tests all 3 deployment scripts.

### Troubleshooting
- **WASM fails to load in browser**: Ensure the HTTP server serves `.wasm` with MIME type `application/wasm` (handled automatically by `deploy/enterprise/nginx.conf` and `deploy/local/local-server.py`). Opening `index.html` via `file://` is blocked by browser `fetch`/WASM origin policies; always serve over HTTP.
- **`em++ not found` during build**: Ensure Emscripten SDK is installed at `~/emsdk/upstream/emscripten/em++` or export `EMXX=/path/to/em++`.
- **False positives on example keys**: Add the safe example token or domain to `allowlist` in the Admin Control Centre and re-export `policy.json`.

