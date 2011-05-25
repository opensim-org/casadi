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

#include <iostream>

#include <casadi/stl_vector_tools.hpp>
#include <casadi/sx/sx_tools.hpp>
#include <casadi/fx/sx_function.hpp>
//#include <casadi/fx/jacobian.hpp>


#include "Ode.hpp"
#include "Ocp.hpp"
#include "OcpMultipleShooting.hpp"

#include <string>
#include <map>

using namespace CasADi;
using namespace std;

void
dxdt(map<string,SX> &xDot, map<string,SX> state, map<string,SX> action, map<string,SX> param, SX t __attribute__((unused)))
{
	SX x = state["x"];
	SX v = state["v"];
	SX thrust = action["thrust"];
	SX tEnd = param["tEnd"];

	double mass = 1.0;

	xDot["x"] = v;
	xDot["v"] = thrust/mass;
}

int
main()
{
	Ode ode("rocket");
	ode.addState("x");
	ode.addState("v");
	ode.addAction("thrust");
	ode.addParam("tEnd");

	ode.dxdt = &dxdt;

	OcpMultipleShooting ocp(&ode);

	ocp.discretize(300);

	SX tEnd = ocp.getParam("tEnd");
	ocp.setTimeInterval(0.0, tEnd);
	ocp.f = tEnd;

	// Bounds/initial condition
	ocp.boundParam("tEnd", 1, 30);
	for (int k=0; k<ocp.N; k++){
		ocp.boundStateAction("x", -15, 15, k);
		ocp.boundStateAction("v", -100, 100, k);
		ocp.boundStateAction("thrust", -1, 1, k);
	}

	ocp.boundStateAction("x", 0, 0, 0);
	ocp.boundStateAction("v", 0, 0, 0);

	ocp.boundStateAction("x", 10, 10, ocp.N-1);
	ocp.boundStateAction("v", 0, 0, ocp.N-1);



	// Create the NLP solver
	SXFunction ffcn(ocp.designVariables, ocp.f); // objective function
	SXFunction gfcn(ocp.designVariables, ocp.g); // constraint
	gfcn.setOption("ad_mode","reverse");
	gfcn.setOption("symbolic_jacobian",false);

	IpoptSolver solver(ffcn,gfcn);
	//IpoptSolver solver(ffcn,gfcn,FX(),Jacobian(gfcn));

	// Set options
	solver.setOption("tol",1e-10);
	solver.setOption("hessian_approximation","limited-memory");

	// initialize the solver
	solver.init();

	solver.setInput(    ocp.lb, NLP_LBX);
	solver.setInput(    ocp.ub, NLP_UBX);
	solver.setInput( ocp.guess, NLP_X_INIT);

	// Bounds on g
	solver.setInput( ocp.gMin, NLP_LBG);
	solver.setInput( ocp.gMax, NLP_UBG);

	// Solve the problem
	solver.solve();

	

	// Print the optimal cost
	double cost;
	solver.getOutput(cost,NLP_COST);
	cout << "optimal time: " << cost << endl;

	// Print the optimal solution
	vector<double>xopt(ocp.getBigN());
	solver.getOutput(xopt,NLP_X_OPT);
	//cout << "optimal solution: " << xopt << endl;

	return 0;
}
