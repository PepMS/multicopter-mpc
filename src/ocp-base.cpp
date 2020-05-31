#include "multicopter_mpc/ocp-base.hpp"

namespace multicopter_mpc {

OcpAbstract::OcpAbstract(const boost::shared_ptr<pinocchio::Model> model,
                         const boost::shared_ptr<MultiCopterBaseParams>& mc_params, const double& dt)
    : model_(model), mc_params_(mc_params), dt_(dt) {
  state_ = boost::make_shared<crocoddyl::StateMultibody>(model_);
  actuation_ =
      boost::make_shared<crocoddyl::ActuationModelMultiCopterBase>(state_, mc_params_->n_rotors_, mc_params_->tau_f_);

  tau_lb_ = Eigen::VectorXd(actuation_->get_nu());
  tau_ub_ = Eigen::VectorXd(actuation_->get_nu());
  tau_lb_.head(mc_params_->n_rotors_).fill(mc_params_->min_thrust_);
  tau_ub_.head(mc_params_->n_rotors_).fill(mc_params_->max_thrust_);

  frame_base_link_id_ = model_->getFrameId(mc_params_->base_link_name_);
}

OcpAbstract::~OcpAbstract() {}

void OcpAbstract::setSolver(const SolverTypes::Type& solver_type) {
  assert(problem_ != nullptr);

  switch (solver_type) {
    case SolverTypes::BoxFDDP: {
      solver_ = boost::make_shared<crocoddyl::SolverBoxFDDP>(problem_);
      break;
    }
    case SolverTypes::SquashBoxFDDP: {
      break;
    }
    default:
      break;
  }
}

void OcpAbstract::setSolverCallbacks(const bool& activated) {
  if (activated) {
    solver_callbacks_.push_back(boost::make_shared<crocoddyl::CallbackVerbose>());
  } else {
    solver_callbacks_.clear();
  }
  solver_->setCallbacks(solver_callbacks_);
}

void OcpAbstract::solve() { solver_->solve(); }

}  // namespace multicopter_mpc