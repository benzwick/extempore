# MFEM Examples for Extempore

This directory contains Extempore (XTLang) implementations of MFEM finite element examples.

## Directory Structure

```
mfem/
├── data/           # Mesh files for examples
│   ├── star.mesh
│   ├── fichera.mesh
│   ├── beam-tri.mesh
│   └── beam-quad.mesh
├── examples/       # Example implementations
│   ├── ex0.xtm     # Laplace equation (serial)
│   ├── ex1.xtm     # Poisson equation (serial)
│   ├── ex2.xtm     # Linear elasticity (serial)
│   ├── ex0p.xtm    # Laplace equation (parallel, MPI)
│   ├── ex1p.xtm    # Poisson equation (parallel, MPI)
│   └── ex2p.xtm    # Linear elasticity (parallel, MPI)
└── README.md       # This file
```

## Examples

### Example 0 - Laplace Equation

**File:** `examples/ex0.xtm`

Solves the Laplace problem: `-Δu = 1` with zero Dirichlet boundary conditions.

**Key features:**
- Basic mesh loading and refinement
- H1 finite element space
- Boundary DOF extraction
- Linear and bilinear form assembly
- PCG solver

**Run:**
```scheme
(sys:load "examples/external/mfem/examples/ex0.xtm")
(ex0_run)
```

**Output:** `mesh.mesh`, `sol.gf`

---

### Example 1 - Poisson Equation

**File:** `examples/ex1.xtm`

Solves the Poisson problem: `-Δu = 1` with homogeneous Dirichlet BC.

**Key features:**
- Adaptive mesh refinement
- Essential boundary marking by attribute
- User-specified finite element order
- Detailed solution output

**Run:**
```scheme
(sys:load "examples/external/mfem/examples/ex1.xtm")
(ex1_run)
```

**Output:** `refined.mesh`, `sol.gf`

---

### Example 2 - Linear Elasticity

**File:** `examples/ex2.xtm`

Solves linear elasticity for a multi-material beam under load.

**Key features:**
- Vector-valued FE spaces (displacement field)
- Piecewise constant material properties
- ElasticityIntegrator (stress-strain)
- Boundary force application
- Mixed essential/natural boundary conditions

**Run:**
```scheme
(sys:load "examples/external/mfem/examples/ex2.xtm")
(ex2_run)
```

**Output:** `displaced.mesh`, `displacement.gf`

---

## Parallel Examples (MPI)

The parallel versions (`ex0p.xtm`, `ex1p.xtm`, `ex2p.xtm`) are templates showing the structure for MPI-parallel implementations. They require:

- MFEM built with `MFEM_USE_MPI=ON`
- Extempore MPI bindings
- HYPRE linear solvers

**Key differences from serial:**
- `mfem_ParMesh` instead of `mfem_Mesh`
- `mfem_ParFiniteElementSpace` instead of `mfem_FiniteElementSpace`
- `mfem_ParGridFunction` instead of `mfem_GridFunction`
- HYPRE BoomerAMG solver instead of serial solvers

## Visualization

View results with GLVis:

```bash
# Example 0
glvis -m mesh.mesh -g sol.gf

# Example 1
glvis -m refined.mesh -g sol.gf

# Example 2
glvis -m displaced.mesh -g displacement.gf
```

## Implementation Notes

### MFEM C API

These examples use the MFEM C API wrapper (`libmfem_c_api.so`) which provides C bindings to MFEM's C++ library. The wrapper is located in `libs/external/mfem_c_api/`.

### XTLang Bindings

The XTLang bindings to the C API are in `libs/external/mfem.xtm`. Key binding patterns:

```scheme
;; Opaque handle types
(bind-type mfem_Mesh <i8*>)
(bind-type mfem_FiniteElementSpace <i8*>)
(bind-type mfem_GridFunction <i8*>)

;; C API functions
(bind-lib libmfem mfem_mesh_load_file [mfem_Mesh,i8*]*)
(bind-lib libmfem mfem_fespace_create [mfem_FiniteElementSpace,mfem_Mesh,i32,i32]*)
```

### Memory Management

The C API uses explicit create/destroy functions. Always pair them:

```scheme
(let ((mesh (mfem_mesh_load_file "mesh.mesh")))
  ;; ... use mesh ...
  (mfem_mesh_destroy mesh))  ;; Don't forget to cleanup!
```

### Error Handling

Check for null pointers after object creation:

```scheme
(if (null? mesh)
    (println "Error: Could not load mesh")
    ;; ... proceed with mesh ...
)
```

## References

- [MFEM Homepage](https://mfem.org)
- [MFEM Examples](https://mfem.org/examples/)
- [PyMFEM](https://github.com/mfem/PyMFEM) - Python wrapper (inspiration for C API design)
- [Extempore Documentation](https://extemporelang.github.io/docs/)

## Contributing

To add more examples:

1. Add required C API functions to `libs/external/mfem_c_api/mfem_c_api.{h,cpp}`
2. Add XTLang bindings to `libs/external/mfem.xtm`
3. Create example file in `examples/external/mfem/examples/`
4. Add any required mesh files to `examples/external/mfem/data/`
5. Update this README

## License

These examples follow the same BSD-3 license as MFEM and Extempore.
