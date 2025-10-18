/*
 * MFEM C API Wrapper Implementation
 *
 * This wraps MFEM's C++ API in C functions for use with Extempore's XTLang.
 * Implementation patterns inspired by PyMFEM.
 */

#include "mfem_c_api.h"
#include "mfem.hpp"
#include <fstream>
#include <sstream>
#include <cstring>

/* Thread-local error storage */
static thread_local std::string g_last_error;

/* Helper to catch and store C++ exceptions */
#define MFEM_C_TRY try {
#define MFEM_C_CATCH } catch (const std::exception& e) { \
    g_last_error = e.what(); \
} catch (...) { \
    g_last_error = "Unknown C++ exception"; \
}

/*============================================================================
 * Mesh Functions
 *===========================================================================*/

extern "C" {

mfem_Mesh mfem_mesh_load_file(const char* filename) {
    MFEM_C_TRY
        std::ifstream imesh(filename);
        if (!imesh) {
            g_last_error = std::string("Cannot open mesh file: ") + filename;
            return nullptr;
        }
        mfem::Mesh* mesh = new mfem::Mesh(imesh, 1, 1);  /* generate_edges=1, refine=1 */
        return (mfem_Mesh)mesh;
    MFEM_C_CATCH
    return nullptr;
}

mfem_Mesh mfem_mesh_make_cartesian_2d(int32_t nx, int32_t ny, int32_t element_type,
                                       double sx, double sy) {
    MFEM_C_TRY
        mfem::Element::Type type = (mfem::Element::Type)element_type;
        mfem::Mesh* mesh = new mfem::Mesh(nx, ny, type, 1, sx, sy);  /* generate_edges=1 */
        return (mfem_Mesh)mesh;
    MFEM_C_CATCH
    return nullptr;
}

mfem_Mesh mfem_mesh_make_cartesian_3d(int32_t nx, int32_t ny, int32_t nz,
                                       int32_t element_type,
                                       double sx, double sy, double sz) {
    MFEM_C_TRY
        mfem::Element::Type type = (mfem::Element::Type)element_type;
        mfem::Mesh* mesh = new mfem::Mesh(nx, ny, nz, type, 1, sx, sy, sz);  /* generate_edges=1 */
        return (mfem_Mesh)mesh;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_mesh_uniform_refine(mfem_Mesh mesh, int32_t levels) {
    MFEM_C_TRY
        if (mesh) {
            ((mfem::Mesh*)mesh)->UniformRefinement();
            /* Note: MFEM's UniformRefinement() refines once.
             * For multiple levels, call this function multiple times */
        }
    MFEM_C_CATCH
}

void mfem_mesh_save(mfem_Mesh mesh, const char* filename) {
    MFEM_C_TRY
        if (mesh) {
            std::ofstream ofs(filename);
            ((mfem::Mesh*)mesh)->Print(ofs);
        }
    MFEM_C_CATCH
}

int32_t mfem_mesh_get_ne(mfem_Mesh mesh) {
    if (mesh) {
        return ((mfem::Mesh*)mesh)->GetNE();
    }
    return 0;
}

int32_t mfem_mesh_get_nv(mfem_Mesh mesh) {
    if (mesh) {
        return ((mfem::Mesh*)mesh)->GetNV();
    }
    return 0;
}

int32_t mfem_mesh_get_dimension(mfem_Mesh mesh) {
    if (mesh) {
        return ((mfem::Mesh*)mesh)->Dimension();
    }
    return 0;
}

void mfem_mesh_destroy(mfem_Mesh mesh) {
    if (mesh) {
        delete (mfem::Mesh*)mesh;
    }
}

/*============================================================================
 * Parallel Mesh Functions (MPI)
 *===========================================================================*/

#ifdef MFEM_USE_MPI

mfem_ParMesh mfem_parmesh_load_file(MPI_Comm comm, const char* filename) {
    MFEM_C_TRY
        mfem::ParMesh* mesh = new mfem::ParMesh(comm, filename);
        return (mfem_ParMesh)mesh;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_parmesh_uniform_refine(mfem_ParMesh mesh, int32_t levels) {
    MFEM_C_TRY
        if (mesh) {
            ((mfem::ParMesh*)mesh)->UniformRefinement();
        }
    MFEM_C_CATCH
}

void mfem_parmesh_save(mfem_ParMesh mesh, const char* filename) {
    MFEM_C_TRY
        if (mesh) {
            std::ofstream ofs(filename);
            ((mfem::ParMesh*)mesh)->Print(ofs);
        }
    MFEM_C_CATCH
}

void mfem_parmesh_destroy(mfem_ParMesh mesh) {
    if (mesh) {
        delete (mfem::ParMesh*)mesh;
    }
}

#endif /* MFEM_USE_MPI */

/*============================================================================
 * Finite Element Space Functions
 *===========================================================================*/

mfem_FiniteElementSpace mfem_fespace_create(mfem_Mesh mesh, int32_t fe_type, int32_t order) {
    MFEM_C_TRY
        if (!mesh) return nullptr;

        mfem::Mesh* m = (mfem::Mesh*)mesh;
        mfem::FiniteElementCollection* fec = nullptr;

        /* Create appropriate FE collection based on type */
        switch (fe_type) {
            case MFEM_FE_H1:
                fec = new mfem::H1_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_L2:
                fec = new mfem::L2_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_RT:
                fec = new mfem::RT_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_ND:
                fec = new mfem::ND_FECollection(order, m->Dimension());
                break;
            default:
                g_last_error = "Unknown FE type";
                return nullptr;
        }

        mfem::FiniteElementSpace* fespace = new mfem::FiniteElementSpace(m, fec);
        /* Note: FESpace takes ownership of fec, will delete it */
        return (mfem_FiniteElementSpace)fespace;
    MFEM_C_CATCH
    return nullptr;
}

int32_t mfem_fespace_get_ndof(mfem_FiniteElementSpace fespace) {
    if (fespace) {
        return ((mfem::FiniteElementSpace*)fespace)->GetNDofs();
    }
    return 0;
}

int32_t mfem_fespace_get_order(mfem_FiniteElementSpace fespace) {
    if (fespace) {
        return ((mfem::FiniteElementSpace*)fespace)->GetOrder(0);  /* Order of first element */
    }
    return 0;
}

void mfem_fespace_destroy(mfem_FiniteElementSpace fespace) {
    if (fespace) {
        mfem::FiniteElementSpace* fes = (mfem::FiniteElementSpace*)fespace;
        delete fes->FEColl();  /* Delete the FECollection */
        delete fes;
    }
}

#ifdef MFEM_USE_MPI

mfem_ParFiniteElementSpace mfem_parfespace_create(mfem_ParMesh mesh, int32_t fe_type, int32_t order) {
    MFEM_C_TRY
        if (!mesh) return nullptr;

        mfem::ParMesh* m = (mfem::ParMesh*)mesh;
        mfem::FiniteElementCollection* fec = nullptr;

        switch (fe_type) {
            case MFEM_FE_H1:
                fec = new mfem::H1_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_L2:
                fec = new mfem::L2_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_RT:
                fec = new mfem::RT_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_ND:
                fec = new mfem::ND_FECollection(order, m->Dimension());
                break;
            default:
                g_last_error = "Unknown FE type";
                return nullptr;
        }

        mfem::ParFiniteElementSpace* fespace = new mfem::ParFiniteElementSpace(m, fec);
        return (mfem_ParFiniteElementSpace)fespace;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_parfespace_destroy(mfem_ParFiniteElementSpace fespace) {
    if (fespace) {
        mfem::ParFiniteElementSpace* fes = (mfem::ParFiniteElementSpace*)fespace;
        delete fes->FEColl();
        delete fes;
    }
}

#endif /* MFEM_USE_MPI */

/*============================================================================
 * Grid Function Functions
 *===========================================================================*/

mfem_GridFunction mfem_gridfunc_create(mfem_FiniteElementSpace fespace) {
    MFEM_C_TRY
        if (!fespace) return nullptr;
        mfem::GridFunction* gf = new mfem::GridFunction((mfem::FiniteElementSpace*)fespace);
        *gf = 0.0;  /* Initialize to zero */
        return (mfem_GridFunction)gf;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_gridfunc_set_constant(mfem_GridFunction gf, double value) {
    if (gf) {
        *((mfem::GridFunction*)gf) = value;
    }
}

void mfem_gridfunc_project_coefficient(mfem_GridFunction gf, mfem_Coefficient coeff) {
    MFEM_C_TRY
        if (gf && coeff) {
            ((mfem::GridFunction*)gf)->ProjectCoefficient(*((mfem::Coefficient*)coeff));
        }
    MFEM_C_CATCH
}

double* mfem_gridfunc_get_data(mfem_GridFunction gf) {
    if (gf) {
        return ((mfem::GridFunction*)gf)->GetData();
    }
    return nullptr;
}

int32_t mfem_gridfunc_size(mfem_GridFunction gf) {
    if (gf) {
        return ((mfem::GridFunction*)gf)->Size();
    }
    return 0;
}

void mfem_gridfunc_save(mfem_GridFunction gf, const char* filename) {
    MFEM_C_TRY
        if (gf) {
            std::ofstream ofs(filename);
            ((mfem::GridFunction*)gf)->Save(ofs);
        }
    MFEM_C_CATCH
}

void mfem_gridfunc_destroy(mfem_GridFunction gf) {
    if (gf) {
        delete (mfem::GridFunction*)gf;
    }
}

#ifdef MFEM_USE_MPI

mfem_ParGridFunction mfem_pargridfunc_create(mfem_ParFiniteElementSpace fespace) {
    MFEM_C_TRY
        if (!fespace) return nullptr;
        mfem::ParGridFunction* gf = new mfem::ParGridFunction((mfem::ParFiniteElementSpace*)fespace);
        *gf = 0.0;
        return (mfem_ParGridFunction)gf;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_pargridfunc_destroy(mfem_ParGridFunction gf) {
    if (gf) {
        delete (mfem::ParGridFunction*)gf;
    }
}

#endif /* MFEM_USE_MPI */

/*============================================================================
 * Linear Form Functions
 *===========================================================================*/

mfem_LinearForm mfem_linearform_create(mfem_FiniteElementSpace fespace) {
    MFEM_C_TRY
        if (!fespace) return nullptr;
        mfem::LinearForm* lf = new mfem::LinearForm((mfem::FiniteElementSpace*)fespace);
        return (mfem_LinearForm)lf;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_linearform_assemble(mfem_LinearForm lf) {
    MFEM_C_TRY
        if (lf) {
            ((mfem::LinearForm*)lf)->Assemble();
        }
    MFEM_C_CATCH
}

void mfem_linearform_destroy(mfem_LinearForm lf) {
    if (lf) {
        delete (mfem::LinearForm*)lf;
    }
}

/*============================================================================
 * Bilinear Form Functions
 *===========================================================================*/

mfem_BilinearForm mfem_bilinearform_create(mfem_FiniteElementSpace fespace) {
    MFEM_C_TRY
        if (!fespace) return nullptr;
        mfem::BilinearForm* bf = new mfem::BilinearForm((mfem::FiniteElementSpace*)fespace);
        return (mfem_BilinearForm)bf;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_bilinearform_assemble(mfem_BilinearForm bf) {
    MFEM_C_TRY
        if (bf) {
            ((mfem::BilinearForm*)bf)->Assemble();
        }
    MFEM_C_CATCH
}

mfem_SparseMatrix mfem_bilinearform_get_matrix(mfem_BilinearForm bf) {
    if (bf) {
        /* Note: BilinearForm retains ownership of the matrix */
        return (mfem_SparseMatrix)&((mfem::BilinearForm*)bf)->SpMat();
    }
    return nullptr;
}

void mfem_bilinearform_destroy(mfem_BilinearForm bf) {
    if (bf) {
        delete (mfem::BilinearForm*)bf;
    }
}

/*============================================================================
 * Coefficient Functions
 *===========================================================================*/

mfem_Coefficient mfem_constant_coefficient_create(double value) {
    MFEM_C_TRY
        mfem::ConstantCoefficient* coeff = new mfem::ConstantCoefficient(value);
        return (mfem_Coefficient)coeff;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_coefficient_destroy(mfem_Coefficient coeff) {
    if (coeff) {
        delete (mfem::Coefficient*)coeff;
    }
}

/*============================================================================
 * Vector Functions
 *===========================================================================*/

mfem_Vector mfem_vector_create(int32_t size) {
    MFEM_C_TRY
        mfem::Vector* vec = new mfem::Vector(size);
        *vec = 0.0;  /* Initialize to zero */
        return (mfem_Vector)vec;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_vector_set(mfem_Vector vec, int32_t index, double value) {
    if (vec) {
        (*((mfem::Vector*)vec))[index] = value;
    }
}

double mfem_vector_get(mfem_Vector vec, int32_t index) {
    if (vec) {
        return (*((mfem::Vector*)vec))[index];
    }
    return 0.0;
}

int32_t mfem_vector_size(mfem_Vector vec) {
    if (vec) {
        return ((mfem::Vector*)vec)->Size();
    }
    return 0;
}

double* mfem_vector_get_data(mfem_Vector vec) {
    if (vec) {
        return ((mfem::Vector*)vec)->GetData();
    }
    return nullptr;
}

void mfem_vector_destroy(mfem_Vector vec) {
    if (vec) {
        delete (mfem::Vector*)vec;
    }
}

/*============================================================================
 * Solver Functions
 *===========================================================================*/

mfem_Solver mfem_cg_solver_create() {
    MFEM_C_TRY
        mfem::CGSolver* solver = new mfem::CGSolver();
        return (mfem_Solver)solver;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_cg_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat) {
    MFEM_C_TRY
        if (solver && mat) {
            ((mfem::CGSolver*)solver)->SetOperator(*((mfem::SparseMatrix*)mat));
        }
    MFEM_C_CATCH
}

void mfem_cg_solver_set_tolerance(mfem_Solver solver, double tol) {
    if (solver) {
        ((mfem::CGSolver*)solver)->SetRelTol(tol);
    }
}

void mfem_cg_solver_set_max_iter(mfem_Solver solver, int32_t max_it) {
    if (solver) {
        ((mfem::CGSolver*)solver)->SetMaxIter(max_it);
    }
}

void mfem_cg_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x) {
    MFEM_C_TRY
        if (solver && b && x) {
            ((mfem::CGSolver*)solver)->Mult(*((mfem::Vector*)b), *((mfem::Vector*)x));
        }
    MFEM_C_CATCH
}

void mfem_solver_destroy(mfem_Solver solver) {
    if (solver) {
        delete (mfem::Solver*)solver;
    }
}

/*============================================================================
 * Utility Functions
 *===========================================================================*/

const char* mfem_get_version() {
    return MFEM_VERSION_STRING;
}

const char* mfem_get_last_error() {
    if (g_last_error.empty()) {
        return nullptr;
    }
    return g_last_error.c_str();
}

} /* extern "C" */
