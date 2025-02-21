#pragma once

#include <SofaCaribou/config.h>
#include <SofaCaribou/Solver/EigenSolver.h>

DISABLE_ALL_WARNINGS_BEGIN
#include <sofa/helper/OptionsGroup.h>
DISABLE_ALL_WARNINGS_END

namespace SofaCaribou::solver {

/**
 * Implementation of a direct LLT sparse linear solver.
 *
 * This class provides a LL^T Cholesky factorizations of sparse matrices that are selfadjoint and positive definite.
 * In order to reduce the fill-in, a symmetric permutation P is applied prior to the factorization such that the
 * factorized matrix is P A P^-1.
 *
 * The component uses the Eigen SimplicialLLT class as the solver backend.
 *
 * @tparam EigenSolver_t Eigen direct solver type
 */
template <class EigenSolver_t>
class LLTSolver : public EigenSolver<typename EigenSolver_t::MatrixType> {
public:
    SOFA_CLASS(SOFA_TEMPLATE(LLTSolver, EigenSolver_t), SOFA_TEMPLATE(EigenSolver, typename EigenSolver_t::MatrixType));

    template <typename T>
    using Data = sofa::Data<T>;

    using Base = EigenSolver<typename EigenSolver_t::MatrixType>;
    using Matrix = typename Base::Matrix;
    using Vector = typename Base::Vector;

    
    LLTSolver();

    /** @see LinearSolver::analyze_pattern */
    
    bool analyze_pattern() override;

    /** @see LinearSolver::factorize */
    
    bool factorize() override;

    /**
     * @see SofaCaribou::solver::LinearSolver::solve
     */
    
    bool solve(const sofa::linearalgebra::BaseVector * F, sofa::linearalgebra::BaseVector * X) override;

    /// Get the backend name of the class derived from the EigenSolver_t template parameter
    
    static std::string BackendName();
private:
    /// Solver backend used (Eigen or Pardiso)
    Data<sofa::helper::OptionsGroup> d_backend;

    /// The actual Eigen solver used (its type is passed as a template parameter and must be derived from Eigen::SparseSolverBase)
    EigenSolver_t p_solver;
};

} // namespace SofaCaribou::solver
