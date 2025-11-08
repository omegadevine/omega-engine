# Changelog

All notable changes to the Omega Engine project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added
- Advanced rendering system with post-processing effects (bloom, color grading, vignette, chromatic aberration)
- Save/Load system with encryption, compression, and auto-save
- Comprehensive documentation (Advanced Rendering, Save System, Performance Optimization)
- Post-processing stack for composable effects
- Saveable interface for automatic object persistence

### Changed
- Updated GitHub Actions: actions/checkout v4→v5, codeql-action v3→v4
- Organized Python examples into dedicated examples/ folder
- Enhanced .gitignore with Python-specific patterns

### Fixed
- Repository organization and workflow improvements

## [0.2.0] - 2025-11-08

### Added
- Debug visualization tools
- Performance profiler with CPU, GPU, and memory tracking
- Asset pipeline with texture, model, and audio loading
- Batch 2 completion with debug tools, profiler, and asset pipeline

## [0.1.0] - 2025-11-07

### Added
- Initial repository setup
- Particle system with emitters and effects
- Tilemap system with layers and collision
- Text rendering system
- UI system with buttons, labels, and panels
- Basic project structure with CMake build system
- CI/CD workflows with GitHub Actions
- Security policies and contribution guidelines
- MIT License

[Unreleased]: https://github.com/omegadevine/omega-engine/compare/v0.2.0...HEAD
[0.2.0]: https://github.com/omegadevine/omega-engine/compare/v0.1.0...v0.2.0
[0.1.0]: https://github.com/omegadevine/omega-engine/releases/tag/v0.1.0
