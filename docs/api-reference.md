# API Reference

## WebAssembly JavaScript API (`data_guard.js`)

Load the Emscripten module with `const wasm = await createDataGuardModule();`.

### `wasm.scanText(text, policyJson) -> string`
Scans `text` against `policyJson` (or `""` for Enterprise defaults) and returns a JSON string:
```json
{
  "blocked": true,
  "riskScore": 40,
  "findingCount": 1,
  "redactedText": "key=[REDACTED:SEC-AWS-KEY]",
  "findings": [
    {
      "ruleId": "SEC-AWS-KEY",
      "category": "SECRET",
      "severity": "CRITICAL",
      "description": "AWS Access Key ID",
      "maskedMatch": "AKIA****CDEF",
      "start": 4,
      "end": 24,
      "line": 1,
      "column": 5,
      "entropy": 4.08
    }
  ]
}
```

### `wasm.redactText(text, policyJson) -> string`
Returns only the redacted text according to `policy.redactionMode` (`PLACEHOLDER`, `MASK`, or `REMOVE`).

### `wasm.getDefaultPolicy() -> string`
Returns the formatted default Enterprise `policy.json`.

### `wasm.validatePolicy(policyJson) -> string`
Returns `{"valid": true, "errors": []}` or `{"valid": false, "errors": ["..."]}`.

### `wasm.getVersion() -> string`
Returns the scanner engine version (`"1.0.0"`).

## C++ Core API (`dataguard`)

- `dataguard::Scanner::scan(const std::string& input) -> ScanResult`
- `dataguard::Scanner::redact(const std::string& input) -> std::string`
- `dataguard::Policy::fromJson(const std::string& json, std::string& err) -> Policy`
- `dataguard::Policy::validate(std::vector<std::string>& errors) const -> bool`

## Visual Architecture

```mermaid
flowchart LR
    JS["JavaScript Caller"] --> Bridge["embind wasm_bridge.cpp"]
    Bridge --> Core["dataguard::Scanner"]
    Core --> JSON["JSON ScanResult"]
    style JS fill:#1e2430,stroke:#94a3b8,color:#ffffff
    style Bridge fill:#0063b1,stroke:#60a5fa,color:#ffffff
    style Core fill:#004e8c,stroke:#38bdf8,color:#ffffff
    style JSON fill:#065f46,stroke:#34d399,color:#ffffff
```
