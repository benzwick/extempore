/*
 * MFEM C API Wrapper
 *
 * This provides a C interface to MFEM's C++ library for use with Extempore's XTLang.
 * Design inspired by PyMFEM's SWIG interface files.
 */

#ifndef MFEM_C_API_H
#define MFEM_C_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Opaque handles to MFEM C++ objects */
typedef void* mfem_Mesh;
typedef void* mfem_ParMesh;
typedef void* mfem_FiniteElementSpace;
typedef void* mfem_ParFiniteElementSpace;
typedef void* mfem_GridFunction;
typedef void* mfem_ParGridFunction;
typedef void* mfem_LinearForm;
typedef void* mfem_ParLinearForm;
typedef void* mfem_BilinearForm;
typedef void* mfem_ParBilinearForm;
typedef void* mfem_SparseMatrix;
typedef void* mfem_HypreParMatrix;
typedef void* mfem_Vector;
typedef void* mfem_Coefficient;
typedef void* mfem_Solver;

/* Element types - match mfem::Element::Type */
#define MFEM_ELEMENT_POINT 0
#define MFEM_ELEMENT_SEGMENT 1
#define MFEM_ELEMENT_TRIANGLE 2
#define MFEM_ELEMENT_QUAD 3
#define MFEM_ELEMENT_TETRAHEDRON 4
#define MFEM_ELEMENT_HEXAHEDRON 5
#define MFEM_ELEMENT_WEDGE 6
#define MFEM_ELEMENT_PYRAMID 7

/* Finite Element types - common collections */
#define MFEM_FE_H1 0
#define MFEM_FE_L2 1
#define MFEM_FE_RT 2  /* Raviart-Thomas */
#define MFEM_FE_ND 3  /* Nedelec */

/*============================================================================
 * Mesh Functions
 *===========================================================================*/

/* Create mesh from file */
mfem_Mesh mfem_mesh_load_file(const char* filename);

/* Create simple Cartesian meshes */
mfem_Mesh mfem_mesh_make_cartesian_2d(int32_t nx, int32_t ny, int32_t element_type,
                                       double sx, double sy);
mfem_Mesh mfem_mesh_make_cartesian_3d(int32_t nx, int32_t ny, int32_t nz,
                                       int32_t element_type,
                                       double sx, double sy, double sz);

/* Mesh operations */
void mfem_mesh_uniform_refine(mfem_Mesh mesh, int32_t levels);
void mfem_mesh_save(mfem_Mesh mesh, const char* filename);

/* Mesh queries */
int32_t mfem_mesh_get_ne(mfem_Mesh mesh);  /* number of elements */
int32_t mfem_mesh_get_nv(mfem_Mesh mesh);  /* number of vertices */
int32_t mfem_mesh_get_dimension(mfem_Mesh mesh);

/* Cleanup */
void mfem_mesh_destroy(mfem_Mesh mesh);

/*============================================================================
 * Parallel Mesh Functions (MPI)
 *===========================================================================*/

#ifdef MFEM_USE_MPI
mfem_ParMesh mfem_parmesh_load_file(MPI_Comm comm, const char* filename);
void mfem_parmesh_uniform_refine(mfem_ParMesh mesh, int32_t levels);
void mfem_parmesh_save(mfem_ParMesh mesh, const char* filename);
void mfem_parmesh_destroy(mfem_ParMesh mesh);
#endif

/*============================================================================
 * Finite Element Space Functions
 *===========================================================================*/

/* Create FE space */
mfem_FiniteElementSpace mfem_fespace_create(mfem_Mesh mesh, int32_t fe_type, int32_t order);

/* FE space queries */
int32_t mfem_fespace_get_ndof(mfem_FiniteElementSpace fespace);
int32_t mfem_fespace_get_order(mfem_FiniteElementSpace fespace);

/* Cleanup */
void mfem_fespace_destroy(mfem_FiniteElementSpace fespace);

#ifdef MFEM_USE_MPI
mfem_ParFiniteElementSpace mfem_parfespace_create(mfem_ParMesh mesh, int32_t fe_type, int32_t order);
void mfem_parfespace_destroy(mfem_ParFiniteElementSpace fespace);
#endif

/*============================================================================
 * Grid Function Functions
 *===========================================================================*/

/* Create grid function */
mfem_GridFunction mfem_gridfunc_create(mfem_FiniteElementSpace fespace);

/* Set values */
void mfem_gridfunc_set_constant(mfem_GridFunction gf, double value);
void mfem_gridfunc_project_coefficient(mfem_GridFunction gf, mfem_Coefficient coeff);

/* Get data */
double* mfem_gridfunc_get_data(mfem_GridFunction gf);
int32_t mfem_gridfunc_size(mfem_GridFunction gf);

/* I/O */
void mfem_gridfunc_save(mfem_GridFunction gf, const char* filename);

/* Cleanup */
void mfem_gridfunc_destroy(mfem_GridFunction gf);

#ifdef MFEM_USE_MPI
mfem_ParGridFunction mfem_pargridfunc_create(mfem_ParFiniteElementSpace fespace);
void mfem_pargridfunc_destroy(mfem_ParGridFunction gf);
#endif

/*============================================================================
 * Linear Form Functions
 *===========================================================================*/

mfem_LinearForm mfem_linearform_create(mfem_FiniteElementSpace fespace);
void mfem_linearform_assemble(mfem_LinearForm lf);
void mfem_linearform_destroy(mfem_LinearForm lf);

/*============================================================================
 * Bilinear Form Functions
 *===========================================================================*/

mfem_BilinearForm mfem_bilinearform_create(mfem_FiniteElementSpace fespace);
void mfem_bilinearform_assemble(mfem_BilinearForm bf);
mfem_SparseMatrix mfem_bilinearform_get_matrix(mfem_BilinearForm bf);
void mfem_bilinearform_destroy(mfem_BilinearForm bf);

/*============================================================================
 * Coefficient Functions
 *===========================================================================*/

/* Function pointer type for custom coefficient functions */
typedef double (*mfem_coeff_function)(const double* x, double t);

/* Create coefficients */
mfem_Coefficient mfem_constant_coefficient_create(double value);
mfem_Coefficient mfem_function_coefficient_create(mfem_coeff_function func);
void mfem_coefficient_destroy(mfem_Coefficient coeff);

/*============================================================================
 * Integrator Functions (for Linear/Bilinear Forms)
 *===========================================================================*/

typedef void* mfem_LinearFormIntegrator;
typedef void* mfem_BilinearFormIntegrator;

/* Domain integrators for linear forms */
mfem_LinearFormIntegrator mfem_domain_lf_integrator_create(mfem_Coefficient coeff);
void mfem_linearform_add_domain_integrator(mfem_LinearForm lf, mfem_LinearFormIntegrator integ);

/* Domain integrators for bilinear forms */
mfem_BilinearFormIntegrator mfem_diffusion_integrator_create(mfem_Coefficient coeff);
mfem_BilinearFormIntegrator mfem_mass_integrator_create(mfem_Coefficient coeff);
void mfem_bilinearform_add_domain_integrator(mfem_BilinearForm bf, mfem_BilinearFormIntegrator integ);

/*============================================================================
 * Vector Functions
 *===========================================================================*/

mfem_Vector mfem_vector_create(int32_t size);
void mfem_vector_set(mfem_Vector vec, int32_t index, double value);
double mfem_vector_get(mfem_Vector vec, int32_t index);
int32_t mfem_vector_size(mfem_Vector vec);
double* mfem_vector_get_data(mfem_Vector vec);
void mfem_vector_destroy(mfem_Vector vec);

/*============================================================================
 * Solver Functions
 *===========================================================================*/

/* CG Solver */
mfem_Solver mfem_cg_solver_create();
void mfem_cg_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat);
void mfem_cg_solver_set_tolerance(mfem_Solver solver, double tol);
void mfem_cg_solver_set_max_iter(mfem_Solver solver, int32_t max_it);
void mfem_cg_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x);
void mfem_solver_destroy(mfem_Solver solver);

/* GMRES Solver */
mfem_Solver mfem_gmres_solver_create();
void mfem_gmres_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat);
void mfem_gmres_solver_set_tolerance(mfem_Solver solver, double tol);
void mfem_gmres_solver_set_max_iter(mfem_Solver solver, int32_t max_it);
void mfem_gmres_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x);

/* BiCGSTAB Solver */
mfem_Solver mfem_bicgstab_solver_create();
void mfem_bicgstab_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat);
void mfem_bicgstab_solver_set_tolerance(mfem_Solver solver, double tol);
void mfem_bicgstab_solver_set_max_iter(mfem_Solver solver, int32_t max_it);
void mfem_bicgstab_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x);

#ifdef MFEM_USE_MPI
/* HYPRE BoomerAMG Solver (parallel only) */
mfem_Solver mfem_hypre_boomeramg_create();
void mfem_hypre_boomeramg_set_operator(mfem_Solver solver, mfem_HypreParMatrix mat);
void mfem_hypre_boomeramg_set_tolerance(mfem_Solver solver, double tol);
void mfem_hypre_boomeramg_set_max_iter(mfem_Solver solver, int32_t max_it);
void mfem_hypre_boomeramg_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x);
#endif

/*============================================================================
 * ODE Solver Functions
 *===========================================================================*/

typedef void* mfem_ODESolver;
typedef void* mfem_TimeDependentOperator;

/* Explicit ODE solvers */
mfem_ODESolver mfem_forward_euler_solver_create();
mfem_ODESolver mfem_rk2_solver_create();
mfem_ODESolver mfem_rk4_solver_create();

/* Implicit ODE solvers */
mfem_ODESolver mfem_backward_euler_solver_create();
mfem_ODESolver mfem_sdirk23_solver_create();
mfem_ODESolver mfem_sdirk34_solver_create();

/* ODE solver operations */
void mfem_odesolver_init(mfem_ODESolver solver, mfem_TimeDependentOperator op);
void mfem_odesolver_step(mfem_ODESolver solver, mfem_Vector x, double* t, double* dt);
void mfem_odesolver_destroy(mfem_ODESolver solver);

/*============================================================================
 * Utility Functions
 *===========================================================================*/

/* Print MFEM version info */
const char* mfem_get_version();

/* Error handling - returns last error message or NULL */
const char* mfem_get_last_error();

#ifdef __cplusplus
}
#endif

#endif /* MFEM_C_API_H */
