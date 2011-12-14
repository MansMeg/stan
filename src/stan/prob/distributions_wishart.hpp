#ifndef __STAN__PROB__DISTRIBUTIONS_WISHART_HPP__
#define __STAN__PROB__DISTRIBUTIONS_WISHART_HPP__

#include "stan/prob/distributions_error_handling.hpp"
#include "stan/prob/distributions_constants.hpp"

#include <stan/meta/traits.hpp>

namespace stan {
  namespace prob {
    using boost::math::tools::promote_args;
    using boost::math::policies::policy;
    
    using Eigen::Dynamic;
    using Eigen::Matrix;

    // Wishart(Sigma|n,Omega)  [Sigma, Omega symmetric, non-neg, definite; 
    //                          Sigma.dims() = Omega.dims();
    //                           n > Sigma.rows() - 1]
    /**
     * The log of the Wishart density for the given W, degrees of freedom, 
     * and scale matrix. 
     * 
     * The scale matrix, S, must be k x k, symmetric, and semi-positive definite.
     * Dimension, k, is implicit.
     * nu must be greater than k-1
     *
     * \f{eqnarray*}{
       W &\sim& \mbox{\sf{Wishart}}_{\nu} (S) \\
       \log (p (W \,|\, \nu, S) ) &=& \log \left( \left(2^{\nu k/2} \pi^{k (k-1) /4} \prod_{i=1}^k{\Gamma (\frac{\nu + 1 - i}{2})} \right)^{-1} 
                                                  \times \left| S \right|^{-\nu/2} \left| W \right|^{(\nu - k - 1) / 2}
						  \times \exp (-\frac{1}{2} \mathsf{tr} (S^{-1} W)) \right) \\
       &=& -\frac{\nu k}{2}\log(2) - \frac{k (k-1)}{4} \log(\pi) - \sum_{i=1}^{k}{\log (\Gamma (\frac{\nu+1-i}{2}))}
           -\frac{\nu}{2} \log(\det(S)) + \frac{\nu-k-1}{2}\log (\det(W)) - \frac{1}{2} \mathsf{tr} (S^{-1}W)
     \f}
     * 
     * @param W A scalar matrix
     * @param nu Degrees of freedom
     * @param S The scale matrix
     * @return The log of the Wishart density at W given nu and S.
     * @throw std::domain_error if nu is not greater than k-1
     * @throw std::domain_error if S is not square, not symmetric, or not semi-positive definite.
     * @tparam T_y Type of scalar.
     * @tparam T_dof Type of degrees of freedom.
     * @tparam T_scale Type of scale.
     */
    template <bool propto = false,
			typename T_y, typename T_dof, typename T_scale, 
			class Policy = policy<> >
    inline typename promote_args<T_y,T_dof,T_scale>::type
    wishart_log(const Matrix<T_y,Dynamic,Dynamic>& W,
		const T_dof& nu,
		const Matrix<T_scale,Dynamic,Dynamic>& S,
		const Policy& = Policy()) {
      static const char* function = "stan::prob::wishart_log<%1%>(%1%)";

      unsigned int k = W.rows();
      double result;
      if(!stan::prob::check_positive(function, nu - (k-1), "Degrees of freedom - k-1", &result, Policy()))
	return result;
      // FIXME: domain checks

			typename promote_args<T_y,T_dof,T_scale>::type lp(0.0);
			if (!propto)
				lp += nu * k * NEG_LOG_TWO_OVER_TWO - lmgamma(k, 0.5 * nu);
			if (!propto
				|| !stan::is_constant<T_dof>::value
				|| !stan::is_constant<T_scale>::value)
				lp -= (0.5 * nu) * log(S.determinant());
			if (!propto
				|| !stan::is_constant<T_scale>::value
				|| !stan::is_constant<T_y>::value)
				lp -= 0.5 * abs((S.inverse() * W).trace());
			if (!propto
				|| !stan::is_constant<T_y>::value
				|| !stan::is_constant<T_dof>::value
				|| !stan::is_constant<T_scale>::value) {
				if (nu != (k + 1))
					lp += 0.5 * (nu - k - 1.0) * log(W.determinant());
			}
			
			return lp;
    }

	}
}
#endif
