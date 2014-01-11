/*
 *    This file is part of CasADi.
 *
 *    CasADi -- A symbolic framework for dynamic optimization.
 *    Copyright (C) 2010 by Joel Andersson, Moritz Diehl, K.U.Leuven. All rights reserved.
 *
 *    CasADi is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License as published by the Free Software Foundation; either
 *    version 3 of the License, or (at your option) any later version.
 *
 *    CasADi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with CasADi; if not, write to the Free Software
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#include "rk_base_internal.hpp"
#include "symbolic/stl_vector_tools.hpp"
#include "symbolic/matrix/sparsity_tools.hpp"
#include "symbolic/matrix/matrix_tools.hpp"
#include "symbolic/sx/sx_tools.hpp"
#include "symbolic/fx/sx_function.hpp"
#include "symbolic/mx/mx_tools.hpp"

using namespace std;
namespace CasADi{

  RKBaseInternal::RKBaseInternal(const FX& f, const FX& g) : IntegratorInternal(f,g){
    addOption("number_of_finite_elements",     OT_INTEGER,  20, "Number of finite elements");
  }

  void RKBaseInternal::deepCopyMembers(std::map<SharedObjectNode*,SharedObject>& already_copied){    
    IntegratorInternal::deepCopyMembers(already_copied);
  }

  RKBaseInternal::~RKBaseInternal(){
  }

  void RKBaseInternal::init(){
    // Call the base class init
    IntegratorInternal::init();
  
    // Number of finite elements and time steps
    nk_ = getOption("number_of_finite_elements");
    h_ = (tf_ - t0_)/nk_;
  }

  void RKBaseInternal::integrate(double t_out){
    // Get discrete time sought
    int k_out = std::ceil((t_out-t0_)/h_);
    k_out = std::min(k_out,nk_); //  make sure that rounding errors does not result in k_out>nk_

    // Take time steps until end time has been reached
    while(k_<k_out){
      F_.input(DAE_T).set(t_);
      F_.input(DAE_X).set(output(INTEGRATOR_XF));
      F_.input(DAE_Z).set(z_);
      F_.input(DAE_P).set(input(INTEGRATOR_P));
      F_.evaluate();
      F_.output(DAE_ODE).get(output(INTEGRATOR_XF));
      F_.output(DAE_ALG).get(z_);
      transform(F_.output(DAE_QUAD).begin(),F_.output(DAE_QUAD).end(),output(INTEGRATOR_QF).begin(),output(INTEGRATOR_QF).begin(),std::plus<double>());
      k_++;
      t_ = t0_ + k_*h_;
    }
  }

  void RKBaseInternal::integrateB(double t_out){
    casadi_error("Not implemented");
  }

  void RKBaseInternal::reset(){
    // Reset the base classes
    IntegratorInternal::reset();

    // Bring discrete time to the beginning
    k_ = 0;
  }

  void RKBaseInternal::resetB(){
    // Reset the base classes
    IntegratorInternal::resetB();

    // Bring discrete time to the end
    k_ = nk_;
  }


} // namespace CasADi
