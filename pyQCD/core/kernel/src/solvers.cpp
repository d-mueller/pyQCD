#include <solvers.hpp>

VectorXcd cg(LinearOperator* linop, const VectorXcd& lhs,
	     const double tolerance, const int maxIterations,
	     double& finalResidual, int& totalIterations)
{
  // Perform the conjugate gradient algorithm to solve
  // linop * solution = lhs for solution
  VectorXcd solution = VectorXcd::Zero(lhs.size());

  // Use the Hermitian form of the linear operator as that's what CG requires
  VectorXcd r = lhs - linop->applyHermitian(solution);
  VectorXcd p = r;

  double oldRes = r.squaredNorm();

  for (int i = 0; i < maxIterations; ++i) {
    VectorXcd linopP = linop->applyHermitian(p);
    double alpha = oldRes / p.dot(linopP).real();
    solution = solution + alpha * p;
    r = r - alpha * linopP;

    double newRes = r.squaredNorm();

    //cout << sqrt(newRes) << endl;

    if (sqrt(newRes) < tolerance) {
      totalIterations = i + 1;
      finalResidual = newRes;
      break; 
    }
    
    double beta = newRes / oldRes;
    p = r + beta * p;
    oldRes = newRes;
  }

  return linop->undoHermiticity(solution);
}



VectorXcd bicgstab(LinearOperator* linop, const VectorXcd& lhs,
		   const double tolerance, const int maxIterations,
		   double& finalResidual, int& totalIterations)
{
  // Perform the biconjugate gradient stabilized algorithm to
  // solve linop * solution = lhs for solution
  VectorXcd solution = VectorXcd::Zero(lhs.size());
  
  // Use the normal for of the linear operator as there's no requirement
  // for the linop to be hermitian
  VectorXcd r = lhs - linop->apply(solution);
  VectorXcd rhat = r;
  double rhatNorm = r.squaredNorm();
  
  double rho = 1.0;
  double alpha = 1.0;
  double omega = 1.0;

  VectorXcd p = VectorXcd::Zero(lhs.size());
  VectorXcd v = VectorXcd::Zero(lhs.size());

  for (int i = 0; i < maxIterations; ++i) {
    double newRho = rhat.dot(r).real();
    if (newRho == 0.0) {
      totalIterations = -1;
      finalResidual = -1;
      break;
    }
    double beta = newRho / rho * alpha / omega;
    
    p = r + beta * (p - omega * v);
    v = linop->apply(p);

    alpha = newRho / rhat.dot(v).real();
    VectorXcd s = r - alpha * v;
    VectorXcd t = linop->apply(s);

    omega = t.dot(s).real() / t.squaredNorm();
    solution = solution + alpha * p + omega * s;

    r = s - omega * t;

    double residual = r.squaredNorm();

    //cout << "Res = " << sqrt(residual) << endl;
    
    if (sqrt(residual / rhatNorm) < tolerance) {
      totalIterations = i + 1;
      finalResidual = sqrt(residual / rhatNorm);
      break;
    }

    rho = newRho;
  }

  return solution;
}