# MFEM/GLVis Integration with Extempore

## Overview

This integration brings the power of MFEM (Modular Finite Element Methods) library to Extempore, enabling real-time, interactive finite element simulations on HPC systems with live coding capabilities. The integration supports heat/diffusion equations, fluid dynamics, solid mechanics, and electromagnetics simulations with audio-reactive features and real-time visualization through GLVis.

## Features

- **Full MFEM Core Functionality**: Mesh handling, finite element spaces, grid functions, linear/bilinear forms
- **HPC Support**: MPI parallel computing, GPU acceleration (CUDA/HIP), distributed memory
- **Real-time Control**: Live parameter modification, solver switching, mesh refinement during execution
- **Audio Integration**: Drive simulations with Extempore's audio engine - heat sources that respond to music
- **Visualization**: GLVis socket protocol for real-time visualization, ParaView Catalyst support
- **Hot Reload**: Modify simulation logic without restart

## Building

### Prerequisites

- CMake >= 3.19
- C++11 compiler
- MPI implementation (OpenMPI, MPICH)
- (Optional) CUDA or HIP for GPU support
- (Optional) GLVis for visualization

### Build Instructions

```bash
mkdir build && cd build
cmake -DEXTERNAL_SHLIBS_SIMULATION=ON \
      -DMFEM_USE_CUDA=OFF \
      -DMFEM_USE_HIP=OFF \
      ..
make
```

For GPU support:
- CUDA: `-DMFEM_USE_CUDA=ON`
- AMD ROCm: `-DMFEM_USE_HIP=ON`

## Usage

### Basic Heat Equation Example

```scheme
;; Load libraries
(sys:load "libs/external/mfem.xtm")
(sys:load "libs/external/glvis.xtm")

;; Create 2D mesh
(bind-func create_simulation
  (lambda ()
    (let ((mesh (mfem_create_2d_mesh 32 32 MFEM_ELEMENT_QUAD))
          (fespace (mfem_create_fespace mesh MFEM_FE_H1 2))
          (u (mfem_init_gridfunc fespace 0.0)))
      ;; Set up heat equation...
      u)))

;; Run simulation
(create_simulation)
```

### Running the Interactive Example

1. **Start GLVis** (in a separate terminal):
   ```bash
   glvis
   ```

2. **Run Extempore**:
   ```bash
   ./extempore
   ```

3. **Load the heat equation example**:
   ```scheme
   (sys:load "examples/external/mfem_heat_equation.xtm")
   ```

4. **Interact with the simulation**:
   ```scheme
   ;; Change parameters live
   (set_diffusion 0.05)
   (set_source_strength 20.0)
   (toggle_simulation)
   (refine_mesh)
   ```

### Audio-Reactive Simulations

The heat equation example responds to audio input:

```scheme
;; Connect audio input to simulation
(dsp:set! audio_callback)

;; Play music or make sound - the heat sources will respond!
```

## API Reference

### Core Types

- `mfem_Mesh` - Finite element mesh
- `mfem_FiniteElementSpace` - FE space definition
- `mfem_GridFunction` - Discrete field on mesh
- `mfem_BilinearForm` - Matrix operators
- `mfem_LinearForm` - Vector operators

### Key Functions

#### Mesh Creation
```scheme
(mfem_create_2d_mesh nx:i32 ny:i32 elem_type:i32)
(mfem_create_3d_mesh nx:i32 ny:i32 nz:i32 elem_type:i32)
(mfem_mesh_uniform_refine mesh:mfem_Mesh level:i32)
```

#### FE Space
```scheme
(mfem_create_fespace mesh:mfem_Mesh fe_type:i32 order:i32)
```

#### Solvers
```scheme
(mfem_cg_solve A:mfem_SparseMatrix b:mfem_Vector x:mfem_Vector tol:double max_iter:i32)
(mfem_gmres_solve ...)
(mfem_amg_solve ...)  ;; Algebraic multigrid
```

#### Visualization
```scheme
(glvis_init)  ;; Connect to GLVis on port 19916
(glvis_show_mesh mesh)
(glvis_show_solution mesh solution)
```

#### Live Control
```scheme
(mfem_set_param "diffusion" 0.1)
(mfem_get_param "diffusion")
(mfem_update_audio_params amplitude frequency)
```

## MPI Parallel Execution

For running on HPC clusters:

```bash
# Initialize MPI in Extempore
mpirun -np 4 ./extempore --run simulation.xtm
```

In XTLang:
```scheme
(mfem_mpi_init)
(println "Running on rank" (mfem_mpi_rank) "of" (mfem_mpi_size))
```

## Advanced Features

### Custom Coefficient Functions

Create time-dependent, spatially-varying coefficients:

```scheme
(bind-func my_coefficient:mfem_coeff_func
  (lambda (x t)
    ;; x is spatial position, t is time
    (* (sin (* 2.0 PI (pref x 0)))  ;; spatial variation
       (cos (* 0.5 PI t)))))         ;; temporal variation

(bind-func create_coeff
  (lambda ()
    (mfem_create_time_coeff my_coefficient)))
```

### Hot-Reload Workflow

Modify simulation parameters and logic without restart:

```scheme
;; Initial setup
(bind-func heat_source_v1:mfem_coeff_func
  (lambda (x t) 1.0))

;; Later, redefine while running:
(bind-func heat_source_v1:mfem_coeff_func
  (lambda (x t)
    (* 10.0 (sin (* t 2.0 PI)))))
```

### GPU Acceleration

When built with CUDA/HIP support:

```scheme
;; Simulations automatically use GPU when available
;; Monitor GPU usage:
(mfem_gpu_memory_usage)
(mfem_gpu_utilization)
```

## Performance Considerations

- **AOT Compilation**: Libraries are pre-compiled for faster loading
- **Zone Memory**: MFEM objects use Extempore's zone allocation
- **Real-time Safety**: Audio callbacks don't block simulation
- **Load Balancing**: MPI automatically distributes work

## Troubleshooting

### GLVis Connection Issues
```scheme
;; Check connection
(if (glvis_init)
    (println "Connected")
    (println "Start GLVis first: run 'glvis' in terminal"))
```

### Memory Management
```scheme
;; Clean up MFEM objects
(mfem_mesh_destroy mesh)
(mfem_gridfunc_destroy solution)
```

### MPI Issues
```bash
# Check MPI installation
which mpirun
mpirun --version
```

## Examples

The `examples/external/` directory contains:
- `mfem_heat_equation.xtm` - Interactive heat equation with audio
- More examples coming: fluid flow, elasticity, electromagnetics

## Contributing

To add new MFEM functionality:

1. Add C++ bindings in `libs/external/mfem.xtm`
2. Update CMakeLists.txt if adding dependencies
3. Create example in `examples/external/`
4. Document in this file

## References

- [MFEM Documentation](https://mfem.org)
- [GLVis Documentation](https://glvis.org)
- [Extempore Documentation](https://extemporelang.github.io/docs/)

## License

This integration follows Extempore's BSD-3 license and MFEM's BSD license.