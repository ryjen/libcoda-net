# Historical libcoda component: libcoda-net

**Disposition: ARCHIVE.** The active project is [Seamwork](https://github.com/ryjen/seamwork), a C++23 interface/architecture refinement laboratory—not an aggregate utility distribution.

## Why

Transport, TLS and protocol-boundary work remains useful as threat-model and test evidence, but is not a maintained networking stack.

New networking experiments must start from a bounded consumer with explicit TLS identity, ownership, and resource guarantees.

## Reproducible provenance

- Repository: https://github.com/ryjen/libcoda-net
- Exact revision referenced by the former Seamwork aggregate: [`5fd99d81f9c22bb9fa7f7de6a7ec8e2fb9ad5a43`](https://github.com/ryjen/libcoda-net/commit/5fd99d81f9c22bb9fa7f7de6a7ec8e2fb9ad5a43)
- Former aggregate submodule retirement: [Seamwork #34](https://github.com/ryjen/seamwork/issues/34)
- Aggregate migration record and further source pins: https://github.com/ryjen/seamwork/blob/development/docs/migration/seamwork-rebrand.md
- Historical code, original tests, and commit lineage are preserved in this repository. No code is transferred wholesale.

Use the pinned commit to reproduce the pre-cutover component. This is historical source, **not a maintained or security-supported release**. Check the repository's own LICENSE before reusing code; cross-repository movement must preserve licensing/attribution.

## Maintenance and CI

The original project-specific source and tests remain here for archaeology and study comparison. Automatic release-style CI is not appropriate for an archived component; the GitHub Actions workflow is manually dispatchable for deliberate historical validation.

New code belongs in a concrete Seamwork C++23 study with a real consumer and an explicit API/ownership/failure contract. Avoid mechanical `coda::` namespace or `libcoda` package renaming.
