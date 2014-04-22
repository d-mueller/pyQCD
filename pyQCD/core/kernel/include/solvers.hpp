#ifndef SOLVERS_HPP
#define SOLVERS_HPP

#include <Eigen/Dense>
#include <boost/timer/timer.hpp>

#include <omp.h>
#include <iostream>

#include <linear_operators/linear_operator.hpp>

using namespace Eigen;
using namespace std;

class LinearOperator;

void arnoldi(MatrixXcd& V, MatrixXcd& H, LinearOperator* linop,
	     const VectorXcd& rhs, const int numIterations);

VectorXcd cg(LinearOperator* linop, const VectorXcd& rhs,
	     double& tolerance, int& maxIterations, double& time);

VectorXcd bicgstab(LinearOperator* linop, const VectorXcd& rhs,
		   double& tolerance, int& maxIterations, double& time);

VectorXcd gmres(LinearOperator* linop, const VectorXcd& rhs,
		   double& tolerance, int& maxIterations, double& time);

#endif
