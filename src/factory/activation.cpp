#include "multicopter_mpc/factory/activation.hpp"

#include "multicopter_mpc/utils/log.hpp"

namespace multicopter_mpc {

ActivationModelFactory::ActivationModelFactory() {}
ActivationModelFactory::~ActivationModelFactory() {}

boost::shared_ptr<crocoddyl::ActivationModelAbstract> ActivationModelFactory::create(const std::string& path_to_cost,
                                                                                     const ParamsServer& server,
                                                                                     const std::size_t& nr) const {
  boost::shared_ptr<crocoddyl::ActivationModelAbstract> activation;

  std::string name;
  try {
    name = server.getParam<std::string>(path_to_cost + "activation");
  } catch (const std::exception& e) {
    MMPC_WARN << e.what() << " Set to quadratic activation.";
    name = "ActivationModelQuad";
  }

  Eigen::VectorXd weights;

  switch (ActivationModelTypes_map.at(name)) {
    case ActivationModelTypes::ActivationModelQuad: {
      activation = boost::make_shared<crocoddyl::ActivationModelQuad>(nr);
    } break;
    case ActivationModelTypes::ActivationModelWeightedQuad: {
      try {
        weights = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "weights"));
      } catch (const std::exception& e) {
        MMPC_WARN << e.what() << " Set to a unitary vector";
        weights = Eigen::VectorXd::Ones(nr);
      }

      if (weights.size() != nr) {
        throw std::runtime_error("Weights vector @" + path_to_cost + "weights has dimension " +
                                 std::to_string(weights.size()) + ". Should be " + std::to_string(nr));
      }

      activation = boost::make_shared<crocoddyl::ActivationModelWeightedQuad>(weights);
    } break;
    case ActivationModelTypes::ActivationModelQuadraticBarrier: {
      Eigen::VectorXd lb = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "l_bound"));
      Eigen::VectorXd ub = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "u_bound"));

      if (lb.size() != nr) {
        throw std::runtime_error("l_bound vector @" + path_to_cost + "l_bound has dimension " +
                                 std::to_string(lb.size()) + ". Should be " + std::to_string(nr));
      }
      if (ub.size() != nr) {
        throw std::runtime_error("u_bound vector @" + path_to_cost + "u_bound has dimension " +
                                 std::to_string(ub.size()) + ". Should be " + std::to_string(nr));
      }
      crocoddyl::ActivationBounds bounds(lb, ub);
      activation = boost::make_shared<crocoddyl::ActivationModelQuadraticBarrier>(bounds);
    } break;
    case ActivationModelTypes::ActivationModelWeightedQuadraticBarrier: {
      Eigen::VectorXd lb = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "l_bound"));
      Eigen::VectorXd ub = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "u_bound"));
      try {
        weights = converter<Eigen::VectorXd>::convert(server.getParam<std::string>(path_to_cost + "weights"));
      } catch (const std::exception& e) {
        MMPC_WARN << e.what() << " Set to a unitary vector";
        weights = Eigen::VectorXd::Ones(nr);
      }

      if (weights.size() != nr) {
        throw std::runtime_error("Weights vector @" + path_to_cost + "weights has dimension " +
                                 std::to_string(weights.size()) + ". Should be " + std::to_string(nr));
      }
      if (lb.size() != nr) {
        throw std::runtime_error("l_bound vector @" + path_to_cost + "l_bound has dimension " +
                                 std::to_string(lb.size()) + ". Should be " + std::to_string(nr));
      }
      if (ub.size() != nr) {
        throw std::runtime_error("u_bound vector @" + path_to_cost + "u_bound has dimension " +
                                 std::to_string(ub.size()) + ". Should be " + std::to_string(nr));
      }
      crocoddyl::ActivationBounds bounds(lb, ub);
      activation = boost::make_shared<crocoddyl::ActivationModelWeightedQuadraticBarrier>(bounds, weights);
    } break;
    default: {
      throw std::runtime_error("Activation '" + name + "' @" + path_to_cost + "activation not found");
    } break;
  }

  return activation;
}

}  // namespace multicopter_mpc