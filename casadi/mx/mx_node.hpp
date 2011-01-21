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

#ifndef MX_NODE_HPP
#define MX_NODE_HPP

#include "mx.hpp"
#include "../fx/mx_function.hpp"
#include "../matrix/matrix.hpp"
#include <vector>

namespace CasADi{

  /// Input and output structure for the evaluation
  struct MXNodeIO{
    
    /// Input
    std::vector<const double*> input;
    
    /// Result of the evaluation
    double* output;
    
    /// Forward seeds
    std::vector< std::vector<const double*> > fwdSeed;

    /// Forward sensitivities
    std::vector<double*> fwdSens;
    
    /// Adjoint seeds
    std::vector<const double*> adjSeed;

    /// Adjoint sensitivities
    std::vector< std::vector<double*> > adjSens;
    
    /// Number of forward sensitivities to be calculated
    int nfwd;
    
    /// Number of adjoint sensitivities to be calculated
    int nadj;
  };
  
/** \brief Node class for MX objects
    \author Joel Andersson 
    \date 2010
    Internal class.
*/
class MXNode : public SharedObjectNode{
  friend class MX;
  friend class MXFunctionInternal;
  
  public:
  
    /** \brief  Constructor */
    explicit MXNode();

    /** \brief  Destructor */
    virtual ~MXNode();

    /** \brief  Clone function */
    virtual MXNode* clone() const = 0;

    /** \brief  Print description */
    virtual void print(std::ostream &stream) const;

    /** \brief  Evaluate the function and store the result in the node (old!) */
    virtual void evaluate(int fsens_order, int asens_order)=0;

    /** \brief  Evaluate the function (new, option 1, easier to debug) */
    virtual void evaluate(MXNodeIO& arg);

    /** \brief  Evaluate the function (new, option 2, compatibility with C) */
    virtual void evaluate(const double** input, double* output, 
                          const double*** fwdSeed, double** fwdSens, 
                          const double** adjSeed, double*** adjSens, 
                          int nfwd, int nadj);
    
    /** \brief  Initialize */
    virtual void init();
    
    /** \brief  Get the name */
    virtual const std::string& getName() const;
    
    /** \brief  Check if symbolic */
    virtual bool isSymbolic() const;

    /** \brief  Check if constant */
    virtual bool isConstant() const;

    /** \brief  dependencies - functions that have to be evaluated before this one */
    MX& dep(int ind=0);
    const MX& dep(int ind=0) const;
    
    /** \brief  Number of dependencies */
    int ndep() const;

    /** \brief  Numerical value */
    const Matrix<double>& input(int ind) const;
    const Matrix<double>& fwdSeed(int ind, int dir=0) const;
    const Matrix<double>& adjSeed(int dir=0) const;
    Matrix<double>& output();
    Matrix<double>& fwdSens(int dir=0);
    Matrix<double>& adjSens(int ind, int dir=0);
    Matrix<double>& adjSeed(int dir=0);

  protected:
    
    /// Set size
    void setSize(int nrow, int ncol);
    
    /// Set the sparsity
    void setSparsity(const CRSSparsity& sparsity);
    
    /// Set unary dependency
    void setDependencies(const MX& dep);
    
    /// Set binary dependencies
    void setDependencies(const MX& dep1, const MX& dep2);
    
    /// Set ternary dependencies
    void setDependencies(const MX& dep1, const MX& dep2, const MX& dep3);
    
    /// Set multiple dependencies
    void setDependencies(const std::vector<MX>& dep);
    
    //! Number of derivatives
    int maxord_;
    
    //! Number of derivative directions - move to MXFunction
    int nfdir_, nadir_;
    
    /// Get size
    int size1() const;
    
    /// Get size
    int size2() const;
    
    /** \brief  dependencies - functions that have to be evaluated before this one */
    std::vector<MX> dep_;
    
  private:
    
    /** \brief  The sparsity pattern */
    CRSSparsity sparsity_;
    
    /** \brief  Numerical value of output  - move this into the MXFunction class */
    Matrix<double> output_;

    /** \brief  Numerical value of forward sensitivities - move this into the MXFunction class */
    std::vector<Matrix<double> > forward_sensitivities_;

    /** \brief  Numerical value of adjoint seeds - move this into the MXFunction class */
    std::vector<Matrix<double> > adjoint_seeds_;
    
};

} // namespace CasADi


#endif // MX_NODE_HPP
