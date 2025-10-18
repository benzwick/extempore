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

int32_t mfem_mesh_get_bdr_attributes_max(mfem_Mesh mesh) {
    if (mesh) {
        mfem::Mesh* m = (mfem::Mesh*)mesh;
        if (m->bdr_attributes.Size() > 0) {
            return m->bdr_attributes.Max();
        }
    }
    return 0;
}

int32_t mfem_mesh_get_attributes_max(mfem_Mesh mesh) {
    if (mesh) {
        mfem::Mesh* m = (mfem::Mesh*)mesh;
        if (m->attributes.Size() > 0) {
            return m->attributes.Max();
        }
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
        std::ifstream imesh(filename);
        if (!imesh) {
            g_last_error = std::string("Cannot open parallel mesh file: ") + filename;
            return nullptr;
        }
        mfem::ParMesh* mesh = new mfem::ParMesh(comm, imesh);
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
            case MFEM_FE_H1_TYPE:
                fec = new mfem::H1_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_L2_TYPE:
                fec = new mfem::L2_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_RT_TYPE:
                fec = new mfem::RT_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_ND_TYPE:
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

int32_t mfem_fespace_get_truevsize(mfem_FiniteElementSpace fespace) {
    if (fespace) {
        return ((mfem::FiniteElementSpace*)fespace)->GetTrueVSize();
    }
    return 0;
}

mfem_FiniteElementSpace mfem_fespace_create_vdim(mfem_Mesh mesh, int32_t fe_type, int32_t order, int32_t vdim) {
    MFEM_C_TRY
        mfem::Mesh* m = (mfem::Mesh*)mesh;
        mfem::FiniteElementCollection* fec = nullptr;

        switch (fe_type) {
            case MFEM_FE_H1_TYPE:
                fec = new mfem::H1_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_L2_TYPE:
                fec = new mfem::L2_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_RT_TYPE:
                fec = new mfem::RT_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_ND_TYPE:
                fec = new mfem::ND_FECollection(order, m->Dimension());
                break;
            default:
                g_last_error = "Invalid finite element type";
                return nullptr;
        }

        mfem::FiniteElementSpace* fespace = new mfem::FiniteElementSpace(m, fec, vdim);
        return (mfem_FiniteElementSpace)fespace;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_fespace_get_boundary_truedofs(mfem_FiniteElementSpace fespace, mfem_IntArray bdr_dofs) {
    if (fespace && bdr_dofs) {
        mfem::FiniteElementSpace* fes = (mfem::FiniteElementSpace*)fespace;
        mfem::Array<int>* dofs = (mfem::Array<int>*)bdr_dofs;
        fes->GetBoundaryTrueDofs(*dofs);
    }
}

void mfem_fespace_get_essential_truedofs(mfem_FiniteElementSpace fespace,
                                          mfem_IntArray bdr_attr_is_ess,
                                          mfem_IntArray ess_tdof_list) {
    if (fespace && bdr_attr_is_ess && ess_tdof_list) {
        mfem::FiniteElementSpace* fes = (mfem::FiniteElementSpace*)fespace;
        mfem::Array<int>* bdr_attr = (mfem::Array<int>*)bdr_attr_is_ess;
        mfem::Array<int>* ess_dofs = (mfem::Array<int>*)ess_tdof_list;
        fes->GetEssentialTrueDofs(*bdr_attr, *ess_dofs);
    }
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
            case MFEM_FE_H1_TYPE:
                fec = new mfem::H1_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_L2_TYPE:
                fec = new mfem::L2_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_RT_TYPE:
                fec = new mfem::RT_FECollection(order, m->Dimension());
                break;
            case MFEM_FE_ND_TYPE:
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

void mfem_bilinearform_form_linear_system(mfem_BilinearForm bf, mfem_IntArray ess_tdof_list,
                                           mfem_GridFunction x, mfem_LinearForm b,
                                           mfem_SparseMatrix* A_out, mfem_Vector* X_out, mfem_Vector* B_out) {
    if (bf && x && b && A_out && X_out && B_out) {
        mfem::BilinearForm* a = (mfem::BilinearForm*)bf;
        mfem::Array<int>* ess_dofs = (mfem::Array<int>*)ess_tdof_list;
        mfem::GridFunction* gf_x = (mfem::GridFunction*)x;
        mfem::LinearForm* lf_b = (mfem::LinearForm*)b;

        mfem::SparseMatrix* A = new mfem::SparseMatrix();
        mfem::Vector* X = new mfem::Vector();
        mfem::Vector* B = new mfem::Vector();

        a->FormLinearSystem(*ess_dofs, *gf_x, *lf_b, *A, *X, *B);

        *A_out = (mfem_SparseMatrix)A;
        *X_out = (mfem_Vector)X;
        *B_out = (mfem_Vector)B;
    }
}

void mfem_bilinearform_recover_solution(mfem_BilinearForm bf, mfem_Vector X,
                                         mfem_LinearForm b, mfem_GridFunction x) {
    if (bf && X && b && x) {
        mfem::BilinearForm* a = (mfem::BilinearForm*)bf;
        mfem::Vector* vec_X = (mfem::Vector*)X;
        mfem::LinearForm* lf_b = (mfem::LinearForm*)b;
        mfem::GridFunction* gf_x = (mfem::GridFunction*)x;

        a->RecoverFEMSolution(*vec_X, *lf_b, *gf_x);
    }
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

/* Wrapper class for function coefficients that bridges C function pointers to C++ */
class CFunctionCoefficient : public mfem::Coefficient {
private:
    mfem_coeff_function func;
public:
    CFunctionCoefficient(mfem_coeff_function f) : func(f) {}

    virtual double Eval(mfem::ElementTransformation &T, const mfem::IntegrationPoint &ip) {
        double x[3] = {0.0, 0.0, 0.0};
        mfem::Vector transip;
        T.Transform(ip, transip);
        for (int i = 0; i < transip.Size() && i < 3; i++) {
            x[i] = transip[i];
        }
        /* Time parameter is always 0.0 for time-independent problems */
        double t = 0.0;
        return func(x, t);
    }
};

mfem_Coefficient mfem_function_coefficient_create(mfem_coeff_function func) {
    MFEM_C_TRY
        if (!func) {
            g_last_error = "Function pointer cannot be NULL";
            return nullptr;
        }
        CFunctionCoefficient* coeff = new CFunctionCoefficient(func);
        return (mfem_Coefficient)coeff;
    MFEM_C_CATCH
    return nullptr;
}

mfem_Coefficient mfem_pwconst_coefficient_create(mfem_Vector constants) {
    MFEM_C_TRY
        if (!constants) return nullptr;
        mfem::Vector* vec = (mfem::Vector*)constants;
        mfem::PWConstCoefficient* coeff = new mfem::PWConstCoefficient(*vec);
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
 * Vector Coefficient Functions
 *===========================================================================*/

mfem_VectorArrayCoefficient mfem_vector_array_coefficient_create(int32_t dim) {
    MFEM_C_TRY
        mfem::VectorArrayCoefficient* vac = new mfem::VectorArrayCoefficient(dim);
        return (mfem_VectorArrayCoefficient)vac;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_vector_array_coefficient_set(mfem_VectorArrayCoefficient vac, int32_t index, mfem_Coefficient coeff) {
    if (vac && coeff) {
        mfem::VectorArrayCoefficient* v = (mfem::VectorArrayCoefficient*)vac;
        mfem::Coefficient* c = (mfem::Coefficient*)coeff;
        v->Set(index, c);
    }
}

void mfem_vector_array_coefficient_destroy(mfem_VectorArrayCoefficient vac) {
    if (vac) {
        delete (mfem::VectorArrayCoefficient*)vac;
    }
}

/*============================================================================
 * Integrator Functions
 *===========================================================================*/

mfem_LinearFormIntegrator mfem_domain_lf_integrator_create(mfem_Coefficient coeff) {
    MFEM_C_TRY
        if (!coeff) return nullptr;
        mfem::DomainLFIntegrator* integ = new mfem::DomainLFIntegrator(*((mfem::Coefficient*)coeff));
        return (mfem_LinearFormIntegrator)integ;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_linearform_add_domain_integrator(mfem_LinearForm lf, mfem_LinearFormIntegrator integ) {
    MFEM_C_TRY
        if (lf && integ) {
            ((mfem::LinearForm*)lf)->AddDomainIntegrator((mfem::LinearFormIntegrator*)integ);
            /* Note: LinearForm takes ownership of the integrator */
        }
    MFEM_C_CATCH
}

mfem_LinearFormIntegrator mfem_vector_boundary_lf_integrator_create(mfem_VectorCoefficient vcoeff) {
    MFEM_C_TRY
        if (!vcoeff) return nullptr;
        mfem::VectorBoundaryLFIntegrator* integ = new mfem::VectorBoundaryLFIntegrator(*((mfem::VectorCoefficient*)vcoeff));
        return (mfem_LinearFormIntegrator)integ;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_linearform_add_boundary_integrator(mfem_LinearForm lf, mfem_LinearFormIntegrator integ) {
    MFEM_C_TRY
        if (lf && integ) {
            ((mfem::LinearForm*)lf)->AddBoundaryIntegrator((mfem::LinearFormIntegrator*)integ);
        }
    MFEM_C_CATCH
}

mfem_BilinearFormIntegrator mfem_diffusion_integrator_create(mfem_Coefficient coeff) {
    MFEM_C_TRY
        if (!coeff) {
            /* If no coefficient provided, create with default (coefficient = 1.0) */
            mfem::DiffusionIntegrator* integ = new mfem::DiffusionIntegrator();
            return (mfem_BilinearFormIntegrator)integ;
        } else {
            mfem::DiffusionIntegrator* integ = new mfem::DiffusionIntegrator(*((mfem::Coefficient*)coeff));
            return (mfem_BilinearFormIntegrator)integ;
        }
    MFEM_C_CATCH
    return nullptr;
}

mfem_BilinearFormIntegrator mfem_mass_integrator_create(mfem_Coefficient coeff) {
    MFEM_C_TRY
        if (!coeff) {
            mfem::MassIntegrator* integ = new mfem::MassIntegrator();
            return (mfem_BilinearFormIntegrator)integ;
        } else {
            mfem::MassIntegrator* integ = new mfem::MassIntegrator(*((mfem::Coefficient*)coeff));
            return (mfem_BilinearFormIntegrator)integ;
        }
    MFEM_C_CATCH
    return nullptr;
}

mfem_BilinearFormIntegrator mfem_elasticity_integrator_create(mfem_Coefficient lambda, mfem_Coefficient mu) {
    MFEM_C_TRY
        if (!lambda || !mu) return nullptr;
        mfem::ElasticityIntegrator* integ = new mfem::ElasticityIntegrator(
            *((mfem::Coefficient*)lambda),
            *((mfem::Coefficient*)mu)
        );
        return (mfem_BilinearFormIntegrator)integ;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_bilinearform_add_domain_integrator(mfem_BilinearForm bf, mfem_BilinearFormIntegrator integ) {
    MFEM_C_TRY
        if (bf && integ) {
            ((mfem::BilinearForm*)bf)->AddDomainIntegrator((mfem::BilinearFormIntegrator*)integ);
            /* Note: BilinearForm takes ownership of the integrator */
        }
    MFEM_C_CATCH
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
 * Additional Iterative Solvers
 *===========================================================================*/

mfem_Solver mfem_gmres_solver_create() {
    MFEM_C_TRY
        mfem::GMRESSolver* solver = new mfem::GMRESSolver();
        solver->SetPrintLevel(0);  /* Suppress output by default */
        return (mfem_Solver)solver;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_gmres_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat) {
    MFEM_C_TRY
        if (solver && mat) {
            ((mfem::GMRESSolver*)solver)->SetOperator(*((mfem::SparseMatrix*)mat));
        }
    MFEM_C_CATCH
}

void mfem_gmres_solver_set_tolerance(mfem_Solver solver, double tol) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::GMRESSolver*)solver)->SetRelTol(tol);
        }
    MFEM_C_CATCH
}

void mfem_gmres_solver_set_max_iter(mfem_Solver solver, int32_t max_it) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::GMRESSolver*)solver)->SetMaxIter(max_it);
        }
    MFEM_C_CATCH
}

void mfem_gmres_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x) {
    MFEM_C_TRY
        if (solver && b && x) {
            ((mfem::GMRESSolver*)solver)->Mult(*((mfem::Vector*)b), *((mfem::Vector*)x));
        }
    MFEM_C_CATCH
}

mfem_Solver mfem_bicgstab_solver_create() {
    MFEM_C_TRY
        mfem::BiCGSTABSolver* solver = new mfem::BiCGSTABSolver();
        solver->SetPrintLevel(0);  /* Suppress output by default */
        return (mfem_Solver)solver;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_bicgstab_solver_set_operator(mfem_Solver solver, mfem_SparseMatrix mat) {
    MFEM_C_TRY
        if (solver && mat) {
            ((mfem::BiCGSTABSolver*)solver)->SetOperator(*((mfem::SparseMatrix*)mat));
        }
    MFEM_C_CATCH
}

void mfem_bicgstab_solver_set_tolerance(mfem_Solver solver, double tol) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::BiCGSTABSolver*)solver)->SetRelTol(tol);
        }
    MFEM_C_CATCH
}

void mfem_bicgstab_solver_set_max_iter(mfem_Solver solver, int32_t max_it) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::BiCGSTABSolver*)solver)->SetMaxIter(max_it);
        }
    MFEM_C_CATCH
}

void mfem_bicgstab_solver_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x) {
    MFEM_C_TRY
        if (solver && b && x) {
            ((mfem::BiCGSTABSolver*)solver)->Mult(*((mfem::Vector*)b), *((mfem::Vector*)x));
        }
    MFEM_C_CATCH
}

#ifdef MFEM_USE_MPI
mfem_Solver mfem_hypre_boomeramg_create() {
    MFEM_C_TRY
        mfem::HypreBoomerAMG* solver = new mfem::HypreBoomerAMG();
        solver->SetPrintLevel(0);  /* Suppress output by default */
        return (mfem_Solver)solver;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_hypre_boomeramg_set_operator(mfem_Solver solver, mfem_HypreParMatrix mat) {
    MFEM_C_TRY
        if (solver && mat) {
            ((mfem::HypreBoomerAMG*)solver)->SetOperator(*((mfem::HypreParMatrix*)mat));
        }
    MFEM_C_CATCH
}

void mfem_hypre_boomeramg_set_tolerance(mfem_Solver solver, double tol) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::HypreBoomerAMG*)solver)->SetTol(tol);
        }
    MFEM_C_CATCH
}

void mfem_hypre_boomeramg_set_max_iter(mfem_Solver solver, int32_t max_it) {
    MFEM_C_TRY
        if (solver) {
            ((mfem::HypreBoomerAMG*)solver)->SetMaxIter(max_it);
        }
    MFEM_C_CATCH
}

void mfem_hypre_boomeramg_mult(mfem_Solver solver, mfem_Vector b, mfem_Vector x) {
    MFEM_C_TRY
        if (solver && b && x) {
            ((mfem::HypreBoomerAMG*)solver)->Mult(*((mfem::Vector*)b), *((mfem::Vector*)x));
        }
    MFEM_C_CATCH
}
#endif

/*============================================================================
 * ODE Solver Implementations
 *===========================================================================*/

mfem_ODESolver mfem_forward_euler_solver_create() {
    MFEM_C_TRY
        mfem::ForwardEulerSolver* solver = new mfem::ForwardEulerSolver();
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

mfem_ODESolver mfem_rk2_solver_create() {
    MFEM_C_TRY
        mfem::RK2Solver* solver = new mfem::RK2Solver(0.5);  /* midpoint method */
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

mfem_ODESolver mfem_rk4_solver_create() {
    MFEM_C_TRY
        mfem::RK4Solver* solver = new mfem::RK4Solver();
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

mfem_ODESolver mfem_backward_euler_solver_create() {
    MFEM_C_TRY
        mfem::BackwardEulerSolver* solver = new mfem::BackwardEulerSolver();
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

mfem_ODESolver mfem_sdirk23_solver_create() {
    MFEM_C_TRY
        mfem::SDIRK23Solver* solver = new mfem::SDIRK23Solver();
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

mfem_ODESolver mfem_sdirk34_solver_create() {
    MFEM_C_TRY
        mfem::SDIRK34Solver* solver = new mfem::SDIRK34Solver();
        return (mfem_ODESolver)solver;
    MFEM_C_CATCH
    return nullptr;
}

void mfem_odesolver_init(mfem_ODESolver solver, mfem_TimeDependentOperator op) {
    MFEM_C_TRY
        if (solver && op) {
            ((mfem::ODESolver*)solver)->Init(*((mfem::TimeDependentOperator*)op));
        }
    MFEM_C_CATCH
}

void mfem_odesolver_step(mfem_ODESolver solver, mfem_Vector x, double* t, double* dt) {
    MFEM_C_TRY
        if (solver && x && t && dt) {
            ((mfem::ODESolver*)solver)->Step(*((mfem::Vector*)x), *t, *dt);
        }
    MFEM_C_CATCH
}

void mfem_odesolver_destroy(mfem_ODESolver solver) {
    MFEM_C_TRY
        if (solver) {
            delete (mfem::ODESolver*)solver;
        }
    MFEM_C_CATCH
}

/*============================================================================
 * IntArray Functions
 *===========================================================================*/

mfem_IntArray mfem_intarray_create() {
    MFEM_C_TRY
        return (mfem_IntArray)(new mfem::Array<int>());
    MFEM_C_CATCH
    return nullptr;
}

mfem_IntArray mfem_intarray_create_with_size(int32_t size) {
    MFEM_C_TRY
        return (mfem_IntArray)(new mfem::Array<int>(size));
    MFEM_C_CATCH
    return nullptr;
}

void mfem_intarray_destroy(mfem_IntArray arr) {
    delete (mfem::Array<int>*)arr;
}

int32_t mfem_intarray_size(mfem_IntArray arr) {
    mfem::Array<int>* a = (mfem::Array<int>*)arr;
    return a->Size();
}

void mfem_intarray_set(mfem_IntArray arr, int32_t index, int32_t value) {
    mfem::Array<int>* a = (mfem::Array<int>*)arr;
    (*a)[index] = value;
}

int32_t mfem_intarray_get(mfem_IntArray arr, int32_t index) {
    mfem::Array<int>* a = (mfem::Array<int>*)arr;
    return (*a)[index];
}

void mfem_intarray_set_all(mfem_IntArray arr, int32_t value) {
    mfem::Array<int>* a = (mfem::Array<int>*)arr;
    *a = value;
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
