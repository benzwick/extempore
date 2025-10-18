# Reference Materials

This directory contains reference materials for Extempore development, including cloned repositories, papers, and documentation that inform the implementation.

## Contents

### MFEM (Modular Finite Element Methods)

**Repository**: https://github.com/mfem/mfem.git

MFEM is a C++ library for scalable finite element discretization. We use this as reference for creating C API bindings in `libs/external/mfem.xtm`.

To clone or update:
```bash
cd reference
git clone https://github.com/mfem/mfem.git
# or to update:
cd mfem && git pull
```

Key files to examine:
- C++ API: `mfem/fem/mesh.hpp`, `mfem/fem/fespace.hpp`, `mfem/fem/gridfunc.hpp`
- Examples: `mfem/examples/`

### PyMFEM (Python MFEM Bindings)

**Repository**: https://github.com/mfem/PyMFEM.git

PyMFEM provides Python bindings for MFEM. This is a valuable reference for understanding how to create language bindings for MFEM's C++ API.

To clone or update:
```bash
cd reference
git clone https://github.com/mfem/PyMFEM.git
# or to update:
cd PyMFEM && git pull
```

Key files to examine:
- Binding implementation: `PyMFEM/mfem/`
- C wrapper code (if any): Look for `.cpp` or `.c` files that wrap C++ classes

## Purpose

These materials are **not** built as part of Extempore. The actual MFEM library used by Extempore is downloaded and built by CMake's `ExternalProject_Add` in `CMakeLists.txt`.

This reference directory exists to:
1. Understand MFEM's C++ API structure
2. Design appropriate C API bindings for Extempore's XTLang
3. See how other language bindings (Python) handle the C++ interface
4. Reference examples and documentation during development

## Adding More References

Feel free to add other reference materials here:
- Academic papers (PDFs)
- Other related repositories
- Documentation snapshots
- Example code from other projects

Keep the directory organized and update this README when adding new materials.
