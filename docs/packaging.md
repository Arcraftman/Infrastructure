Packaging guidelines

This document describes how to create distribution packages for Infra.

- Use `cpack` to generate cross-platform tar/zip artifacts.
- For system packages (deb/rpm), use distro-specific tools or `fpm` to convert CPack output.
- For Windows, consider using NSIS or WiX to produce installers.
