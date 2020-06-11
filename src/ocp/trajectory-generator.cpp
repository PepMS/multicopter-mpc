#include "multicopter_mpc/ocp/trajectory-generator.hpp"

namespace multicopter_mpc {

TrajectoryGenerator::TrajectoryGenerator(const boost::shared_ptr<pinocchio::Model> model,
                                         const boost::shared_ptr<MultiCopterBaseParams>& mc_params, const double& dt,
                                         const boost::shared_ptr<Mission>& mission)
    : OcpAbstract(model, mc_params, dt), mission_(mission) {
  n_knots_ = mission_->getTotalKnots();

  initializeDefaultParameters() :
}

TrajectoryGenerator::~TrajectoryGenerator() {}

void TrajectoryGenerator::initializeDefaultParameters() {
  params_.w_state_position.fill(1.);
  params_.w_state_orientation.fill(1.);
  params_.w_state_velocity_lin.fill(1.);
  params_.w_state_velocity_ang.fill(1000.);

  params_.w_state_running = 1e-6;
  params_.w_control_running = 1e-4;
  params_.w_pos_running = 1e-2;
  params_.w_vel_running = 1e-2;

  params_.w_pos_terminal = 100;
  params_.w_vel_terminal = 10;
}


void TrajectoryGenerator::loadParameters(const yaml_parser::ParamsServer& server) {
  std::vector<std::string> state_weights = server.getParam<std::vector<std::string>>("ocp/state_weights");
  std::map<std::string, std::string> current_state =
      yaml_parser::converter<std::map<std::string, std::string>>::convert(state_weights[0]);

  params_.w_state_position = yaml_parser::converter<Eigen::Vector3d>::convert(current_state["position"]);
  params_.w_state_orientation = yaml_parser::converter<Eigen::VectorXd>::convert(current_state["orientation"]);
  params_.w_state_velocity_lin = yaml_parser::converter<Eigen::VectorXd>::convert(current_state["velocity_lin"]);
  params_.w_state_velocity_ang = yaml_parser::converter<Eigen::VectorXd>::convert(current_state["velocity_ang"]);

  params_.w_state_running = server.getParam<double>("ocp/cost_running_state_weight");
  params_.w_control_running = server.getParam<double>("ocp/cost_running_control_weight");
  params_.w_pos_running = server.getParam<double>("ocp/cost_running_pos_weight");
  params_.w_vel_running = server.getParam<double>("ocp/cost_running_vel_weight");
  params_.w_pos_terminal = server.getParam<double>("ocp/cost_terminal_pos_weight");
  params_.w_vel_terminal = server.getParam<double>("ocp/cost_terminal_vel_weight");
}

void TrajectoryGenerator::createProblem(const SolverTypes::Type& solver_type) {
  assert(mission_->waypoints_.size() > 0);

  boost::shared_ptr<crocoddyl::CostModelAbstract> cost_reg_state = createCostStateRegularization();
  boost::shared_ptr<crocoddyl::CostModelAbstract> cost_reg_control = createCostControlRegularization();

  for (std::vector<WayPoint>::const_iterator wp = mission_->waypoints_.begin(); wp != mission_->waypoints_.end();
       ++wp) {
    boost::shared_ptr<crocoddyl::CostModelSum> cost_model_running =
        boost::make_shared<crocoddyl::CostModelSum>(state_, actuation_->get_nu());
    boost::shared_ptr<crocoddyl::CostModelSum> cost_model_terminal =
        boost::make_shared<crocoddyl::CostModelSum>(state_, actuation_->get_nu());

    // Regularitzations
    cost_model_running->addCost("x_reg", cost_reg_state, params_.w_state_running);      // 1e-6
    cost_model_running->addCost("u_reg", cost_reg_control, params_.w_control_running);  // 1e-4
    if (std::next(wp) != mission_->waypoints_.end()) {
      cost_model_terminal->addCost("x_reg_cost", cost_reg_state, params_.w_state_running);      // 1e-6
      cost_model_terminal->addCost("u_reg_cost", cost_reg_control, params_.w_control_running);  // 1e-4
    }

    // Waypoint cost related
    crocoddyl::FramePlacement frame_ref = crocoddyl::FramePlacement(frame_base_link_id_, wp->pose);
    boost::shared_ptr<crocoddyl::CostModelAbstract> cost_goal =
        boost::make_shared<crocoddyl::CostModelFramePlacement>(state_, frame_ref, actuation_->get_nu());
    cost_model_running->addCost("track_pos_cost", cost_goal, params_.w_pos_running);   // 1e-2
    cost_model_terminal->addCost("goal_pos_cost", cost_goal, params_.w_pos_terminal);  // 100

    if (wp->vel != boost::none) {
      crocoddyl::FrameMotion vel_ref(frame_base_link_id_, *(wp->vel));
      boost::shared_ptr<crocoddyl::CostModelAbstract> cost_goal_vel =
          boost::make_shared<crocoddyl::CostModelFrameVelocity>(state_, vel_ref, actuation_->get_nu());
      cost_model_running->addCost("track_vel_cost", cost_goal_vel, params_.w_vel_running);   // 1e-2
      cost_model_terminal->addCost("goal_vel_cost", cost_goal_vel, params_.w_vel_terminal);  // 10
    }

    // Diff & Integrated models
    boost::shared_ptr<crocoddyl::DifferentialActionModelFreeFwdDynamics> diff_model_running =
        boost::make_shared<crocoddyl::DifferentialActionModelFreeFwdDynamics>(state_, actuation_, cost_model_running);
    boost::shared_ptr<crocoddyl::DifferentialActionModelFreeFwdDynamics> diff_model_terminal =
        boost::make_shared<crocoddyl::DifferentialActionModelFreeFwdDynamics>(state_, actuation_, cost_model_terminal);

    boost::shared_ptr<crocoddyl::IntegratedActionModelEuler> int_model_running =
        boost::make_shared<crocoddyl::IntegratedActionModelEuler>(diff_model_running, dt_);
    boost::shared_ptr<crocoddyl::IntegratedActionModelEuler> int_model_terminal =
        boost::make_shared<crocoddyl::IntegratedActionModelEuler>(diff_model_terminal, dt_);

    if (std::next(wp) != mission_->waypoints_.end()) {
      int_model_running->set_u_lb(tau_lb_);
      int_model_running->set_u_ub(tau_ub_);
      int_model_terminal->set_u_lb(tau_lb_);
      int_model_terminal->set_u_ub(tau_ub_);

      std::vector<boost::shared_ptr<crocoddyl::DifferentialActionModelFreeFwdDynamics>> diff_models_running(
          wp->knots - 1, diff_model_running);
      diff_models_running.push_back(diff_model_terminal);
      diff_models_running_.insert(diff_models_running_.end(), diff_models_running.begin(), diff_models_running.end());

      std::vector<boost::shared_ptr<crocoddyl::ActionModelAbstract>> int_models_running(wp->knots - 1,
                                                                                        int_model_running);
      int_models_running.push_back(int_model_terminal);
      int_models_running_.insert(int_models_running_.end(), int_models_running.begin(), int_models_running.end());
    } else {
      int_model_running->set_u_lb(tau_lb_);
      int_model_running->set_u_ub(tau_ub_);

      std::vector<boost::shared_ptr<crocoddyl::DifferentialActionModelFreeFwdDynamics>> diff_models_running(
          wp->knots, diff_model_running);
      diff_models_running_.insert(diff_models_running_.end(), diff_models_running.begin(), diff_models_running.end());
      diff_model_terminal_ = diff_model_terminal;

      std::vector<boost::shared_ptr<crocoddyl::ActionModelAbstract>> int_models_running(wp->knots, int_model_running);
      int_models_running_.insert(int_models_running_.end(), int_models_running.begin(), int_models_running.end());
      int_model_terminal_ = int_model_terminal;
    }
  }
  problem_ = boost::make_shared<crocoddyl::ShootingProblem>(mission_->x0_, int_models_running_, int_model_terminal_);
  setSolver(solver_type);

  state_trajectory_ = std::vector<Eigen::VectorXd>(n_knots_, state_->zero());
}

boost::shared_ptr<crocoddyl::CostModelAbstract> TrajectoryGenerator::createCostStateRegularization() {
  Eigen::VectorXd state_weights(state_->get_ndx());

  state_weights.head(3) = params_.w_state_position;                         // Position 1.
  state_weights.segment(3, 3) = params_.w_state_orientation;                // Orientation 1.
  state_weights.segment(model_->nv, 3) = params_.w_state_velocity_lin;      // Linear velocity 1.
  state_weights.segment(model_->nv + 3, 3) = params_.w_state_velocity_ang;  // Angular velocity 1000.

  boost::shared_ptr<crocoddyl::ActivationModelWeightedQuad> activation_state =
      boost::make_shared<crocoddyl::ActivationModelWeightedQuad>(state_weights);

  boost::shared_ptr<crocoddyl::CostModelAbstract> cost_reg_state =
      boost::make_shared<crocoddyl::CostModelState>(state_, activation_state, state_->zero(), actuation_->get_nu());

  return cost_reg_state;
}

boost::shared_ptr<crocoddyl::CostModelAbstract> TrajectoryGenerator::createCostControlRegularization() {
  boost::shared_ptr<crocoddyl::CostModelAbstract> cost_reg_control =
      boost::make_shared<crocoddyl::CostModelControl>(state_, actuation_->get_nu());

  return cost_reg_control;
}

void TrajectoryGenerator::solve() {
  problem_->set_x0(mission_->x0_);
  solver_->solve();
  // in the unit test check that the solve trajecotry and the stae_trajectory have the same size
  std::copy(solver_->get_xs().begin(), solver_->get_xs().end(), state_trajectory_.begin());
  state_hover_ = state_->zero();
  state_hover_.head(7) = solver_->get_xs().back().head(7);
}

const boost::shared_ptr<const Mission> TrajectoryGenerator::getMission() const { return mission_; }

std::vector<Eigen::VectorXd> TrajectoryGenerator::getTrajectoryPortion(const std::size_t& idx_init,
                                                                       const std::size_t& idx_end) const {
  assert(idx_init < idx_end);
  std::vector<Eigen::VectorXd>::const_iterator first = state_trajectory_.begin() + idx_init;
  std::vector<Eigen::VectorXd>::const_iterator last = state_trajectory_.begin() + idx_end + 1;
  return std::vector<Eigen::VectorXd>(first, last);
}

const Eigen::VectorXd& TrajectoryGenerator::getTrajectoryState(const std::size_t& cursor) const {
  if (cursor < state_trajectory_.size()) {
    return state_trajectory_[cursor];
  } else {
    return state_hover_;
  }
}

}  // namespace multicopter_mpc