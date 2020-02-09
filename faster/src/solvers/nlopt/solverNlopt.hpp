
#ifndef SOLVER_NLOPT_HPP
#define SOLVER_NLOPT_HPP
#include <Eigen/Dense>

#include <iomanip>  //set precision
#include <nlopt.hpp>
#include "./../../faster_types.hpp"
//#include <sstream>
#include "./../../utils.hpp"
#include "./../../timer.hpp"
//#include <decomp_util/ellipsoid_decomp.h>  //For Polyhedron definition
#include <decomp_geometry/polyhedron.h>  //For Polyhedron definition

typedef JPS::Timer MyTimer;

class SolverNlopt
{
public:
  SolverNlopt(int num_pol, int deg_pol, int num_obst, double weight, double epsilon_tol_constraints,
              bool force_final_state, std::string &solver);

  ~SolverNlopt();

  bool optimize();

  void setInitAndFinalStates(state &initial_state, state &final_state);

  void setHulls(ConvexHullsOfCurves_Std &hulls);
  void setTminAndTmax(double t_min, double t_max);

  void setMaxValues(double v_max, double a_max);

  void setDC(double dc);

  std::vector<state> X_temp_;

  void setMaxRuntime(double deltaT);

  // Guesses
  void useJPSGuess(vec_Vecf<3> &jps_path);
  void useRRTGuess(vec_E<Polyhedron<3>> &polyhedra);
  void useRandomInitialGuess();

  void getGuessForPlanes(std::vector<Hyperplane3D> &planes);

protected:
private:
  void generateGuessNFromQ(const std::vector<Eigen::Vector3d> &q, std::vector<Eigen::Vector3d> &n);

  void generateRandomN(std::vector<Eigen::Vector3d> &n);
  void fillXTempFromCPs(std::vector<Eigen::Vector3d> &q);

  nlopt::algorithm getSolver(std::string &solver);

  bool isADecisionCP(int i);

  void assignEigenToVector(double *grad, int index, const Eigen::Vector3d &tmp);

  template <class T>
  void toEigen(T &x, std::vector<Eigen::Vector3d> &q, std::vector<Eigen::Vector3d> &n);

  int gIndexQ(int i);  // Element jth of control point ith
  int gIndexN(int i);  // Element jth of normal ith
  // int gIndexD(int i);

  void printQN(std::vector<Eigen::Vector3d> &q, std::vector<Eigen::Vector3d> &n);

  // r is the constraint index
  // nn is the number of variables
  // var_gindex is the index of the variable of the first element of the vector
  void toGradDiffConstraintsDiffVariables(int var_gindex, const Eigen::Vector3d &tmp, double *grad, int r, int nn);

  void toGradSameConstraintDiffVariables(int var_gindex, const Eigen::Vector3d &tmp, double *grad, int r, int nn);

  void assignValueToGradConstraints(int var_gindex, const double &tmp, double *grad, int r, int nn);

  // This function has to be static, see example
  // https://github.com/HKUST-Aerial-Robotics/Fast-Planner/blob/master/fast_planner/bspline_opt/src/bspline_optimizer.cpp
  // and here: https://github.com/stevengj/nlopt/issues/246
  static double myObjFunc(unsigned nn, const double *x, double *grad, void *my_func_data);

  // See example https://github.com/stevengj/nlopt/issues/168
  static void myIneqConstraints(unsigned m, double *result, unsigned nn, const double *x, double *grad, void *f_data);

  double computeObjFuction(unsigned nn, double *grad, std::vector<Eigen::Vector3d> &q, std::vector<Eigen::Vector3d> &n);

  void computeConstraints(unsigned m, double *constraints, unsigned nn, double *grad, std::vector<Eigen::Vector3d> &q,
                          std::vector<Eigen::Vector3d> &n);

  void initializeNumOfConstraints();

  void qntoX(const std::vector<Eigen::Vector3d> &q, const std::vector<Eigen::Vector3d> &n, std::vector<double> &x);

  void printInfeasibleConstraints(std::vector<Eigen::Vector3d> &q, std::vector<Eigen::Vector3d> &n);

  // template <class T>
  // void printInfeasibleConstraints(const T x);

  template <class T>
  int getNumberOfInfeasibleConstraints(const T &constraints);

  template <class T>
  bool areTheseConstraintsFeasible(const T &constraints);

  template <class T>
  void printInfeasibleConstraints(const T &constraints);

  std::string getResultCode(int &result);

  void findCentroidHull(const std::vector<Eigen::Vector3d> &hull, Eigen::Vector3d &centroid);

  int lastDecCP();

  int deg_pol_ = 3;
  int num_pol_ = 5;
  int p_ = 5;
  int i_min_;
  int i_max_;
  int j_min_;
  int j_max_;
  // int k_min_;
  // int k_max_;
  int M_;
  int N_;

  int num_of_variables_;
  int num_of_normals_;
  int num_of_constraints_;
  int num_obst_;
  int num_of_segments_;

  nlopt::algorithm solver_;

  bool got_a_feasible_solution_ = false;
  double time_first_feasible_solution_ = 0.0;

  double best_cost_so_far_ = std::numeric_limits<double>::max();

  std::vector<double> best_feasible_sol_so_far_;

  std::vector<int> signs_;
  std::vector<Hyperplane3D> planes_;

  double epsilon_tol_constraints_;

  std::vector<double> x_;  // Here the initial guess, and the solution, are stored

  double dc_;
  Eigen::RowVectorXd knots_;
  double t_min_;
  double t_max_;
  double deltaT_;
  Eigen::Vector3d v_max_;
  Eigen::Vector3d a_max_;

  double weight_ = 10000;

  bool force_final_state_ = true;

  state initial_state_;
  state final_state_;

  double constraints_[10000];  // this number should be very big!! (hack)

  Eigen::Vector3d q0_, q1_, q2_;  //, qNm2_, qNm1_, qN_;

  ConvexHullsOfCurves_Std hulls_;

  MyTimer opt_timer_;

  double max_runtime_ = 2;  //[seconds]

  // Eigen::Vector3d initial_point_;
  // Eigen::Vector3d final_point_;
  nlopt::opt *opt_ = NULL;
  nlopt::opt *local_opt_ = NULL;

  // Guesses
  std::vector<Eigen::Vector3d> n_guess_;  // Guesses for the normals
  std::vector<Eigen::Vector3d> q_guess_;  // Guesses for the normals

  Eigen::MatrixXd R_;  // This matrix is [r0, r1, r2, r3, r0, r1, r2, r3] (for two segments)
};
#endif