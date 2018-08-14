#include "ros/ros.h"
#include "solver.h"

#include "visualization_msgs/Marker.h"
#include "visualization_msgs/MarkerArray.h"
#include <sensor_msgs/point_cloud_conversion.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <tf2_ros/transform_listener.h>
#include <tf2_ros/buffer.h>

#include <atomic>

#include <Eigen/Dense>

#include <acl_msgs/State.h>
#include <acl_msgs/QuadGoal.h>
#include <acl_msgs/QuadFlightMode.h>
#include <acl_msgs/TermGoal.h>
#include <mutex>

// JPS3D includes
#include "timer.hpp"
#include "read_map.hpp"
#include <jps_basis/data_utils.h>
#include <jps_planner/jps_planner/jps_planner.h>

struct kdTreeStamped
{
  pcl::KdTreeFLANN<pcl::PointXYZ> kdTree;
  ros::Time time;
};

class CVX
{
public:
  CVX(ros::NodeHandle nh, ros::NodeHandle nh_replan_CB, ros::NodeHandle nh_pub_CB);

private:
  // class methods
  void pubTraj(double** x);
  void pubTraj(Eigen::MatrixXd X);
  void goalCB(const acl_msgs::TermGoal& msg);
  void stateCB(const acl_msgs::State& msg);
  void modeCB(const acl_msgs::QuadFlightMode& msg);
  void pubCB(const ros::TimerEvent& e);
  void replanCB(const ros::TimerEvent& e);

  double callOptimizer(double u_max, double x0[], double xf[]);
  int checkConvergence(double xf[], double xf_opt[]);
  void genNewTraj(double u_max, double xf[]);
  void interpInput(double dt, double xf[], double u0[], double x0[], double** u, double** x, Eigen::MatrixXd& U,
                   Eigen::MatrixXd& X);
  visualization_msgs::Marker createMarkerLineStrip(Eigen::MatrixXd X);
  void createMarkerSetOfArrows(Eigen::MatrixXd X, bool isFree);
  void clearMarkerSetOfArrows();
  void clearMarkerActualTraj();
  void mapCB(const sensor_msgs::PointCloud2ConstPtr& pcl2ptr_msg);
  void pclCB(const sensor_msgs::PointCloud2ConstPtr& pcl2ptr_msg);
  bool trajIsFree(Eigen::MatrixXd X);
  Eigen::Vector3d computeForce(Eigen::Vector3d x, Eigen::Vector3d g);
  std_msgs::ColorRGBA color(int id);
  Eigen::Vector3d createForceArrow(Eigen::Vector3d x, Eigen::Vector3d f_att, Eigen::Vector3d f_rep,
                                   visualization_msgs::MarkerArray* forces);

  float solvePolyOrder2(Eigen::Vector3f coeff);
  geometry_msgs::Point pointOrigin();
  geometry_msgs::Point eigen2point(Eigen::Vector3d vector);
  void pubActualTraj();
  void solveJPS3D(pcl::PointCloud<pcl::PointXYZ>::Ptr pclptr);
  void vectorOfVectors2MarkerArray(vec_Vecf<3> traj, visualization_msgs::MarkerArray* m_array);
  visualization_msgs::MarkerArray clearArrows();
  geometry_msgs::Vector3 vectorNull();
  geometry_msgs::Vector3 getPos(int i);
  geometry_msgs::Vector3 getVel(int i);
  geometry_msgs::Vector3 getAccel(int i);
  geometry_msgs::Vector3 getJerk(int i);

  visualization_msgs::Marker setpoint_;
  acl_msgs::QuadGoal quadGoal_;
  acl_msgs::QuadGoal nextQuadGoal_;
  acl_msgs::QuadFlightMode flight_mode_;
  acl_msgs::State state_;
  acl_msgs::TermGoal term_goal_;

  ros::NodeHandle nh_;
  ros::NodeHandle nh_replan_CB_;
  ros::NodeHandle nh_pub_CB_;

  ros::Publisher pub_goal_;
  ros::Publisher pub_traj_;
  ros::Publisher pub_setpoint_;
  ros::Publisher pub_trajs_sphere_;
  ros::Publisher pub_forces_;
  ros::Publisher pub_actual_traj_;
  ros::Publisher pub_path_jps_;
  ros::Subscriber sub_goal_;
  ros::Subscriber sub_state_;
  ros::Subscriber sub_mode_;
  ros::Subscriber sub_map_;
  ros::Subscriber sub_pcl_;
  ros::Timer pubCBTimer_;
  ros::Timer replanCBTimer_;

  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener* tfListener;
  std::string name_drone_;

  visualization_msgs::MarkerArray trajs_sphere_;  // all the trajectories generated in the sphere
  visualization_msgs::MarkerArray path_jps_;
  int markerID_ = 0;
  int markerID_last_ = 0;
  int actual_trajID_ = 0;
  Eigen::MatrixXd U_, X_;  // Contains the intepolated input/states that will be sent to the drone
  Eigen::MatrixXd U_temp_,
      X_temp_;  // Contains the intepolated input/states of a traj. If the traj. is free, it will be copied to U_, X_
  bool replan_, optimized_, use_ff_;
  double u_min_, u_max_, z_start_, spinup_time_, z_land_;
  int N_ = 20;
  pcl::KdTreeFLANN<pcl::PointXYZ> kdtree_map_;  // kdtree of the point cloud of the map
  bool kdtree_map_initialized_ = 0;
  // vector that has all the kdtrees of the pclouds not included in the map:
  std::vector<kdTreeStamped> v_kdtree_new_pcls_;
  bool replanning_needed_ = true;
  bool goal_click_initialized_ = false;

  int cells_x_;  // Number of cells of the map in X
  int cells_y_;  // Number of cells of the map in Y
  int cells_z_;  // Number of cells of the map in Z

  Eigen::Vector3d directionJPS_;

  std::mutex mtx;
};
