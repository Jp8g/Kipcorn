# LMFW v0.1.1

(**L**ow-level **M**inimal **F**ramework for **W**indowing)

LMFW is a low level, and cross-platform windowing library written in C, with minimal dependencies.

Currently, LMFW only supports Linux with Wayland, Xcb, and Xlib. It does not have a broad featureset, but it supports simple usages (like creating and destroying platform specific windows, and supporting software rendering). It does not (as of yet, and intentionally) have a cross-platform interface for managing windows, right now it only exposes helpers for managing platform-dependent windows.

At LMFW v1.0, it is planned to support all major desktop platforms (Linux, Windows, MacOS) with extensive modular features and a cross-platform interface (optional, not required).
