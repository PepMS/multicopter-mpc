#ifndef MULTICOPTER_MPC_FACTORY_INT_ACTION_HPP_
#define MULTICOPTER_MPC_FACTORY_INT_ACTION_HPP_

#include <map>

#include "crocoddyl/core/action-base.hpp"
#include "crocoddyl/core/diff-action-base.hpp"
#include "crocoddyl/core/integrator/euler.hpp"
#include "crocoddyl/core/integrator/rk4.hpp"

#include "multicopter_mpc/stage.hpp"

namespace multicopter_mpc {

// class Stage;

enum class IntegratedActionModelTypes {
  IntegratedActionModelEuler,
  IntegratedActionModelRK4,
  NbIntegratedActionModelTypes

};

static std::map<std::string, IntegratedActionModelTypes> IntegratedActionModelTypes_init_map() {
  std::map<std::string, IntegratedActionModelTypes> m;
  m.clear();
  m.insert({"IntegratedActionModelEuler", IntegratedActionModelTypes::IntegratedActionModelEuler});
  m.insert({"IntegratedActionModelRK4", IntegratedActionModelTypes::IntegratedActionModelRK4});
  return m;
}

static const std::map<std::string, IntegratedActionModelTypes> IntegratedActionModelTypes_map =
    IntegratedActionModelTypes_init_map();

class IntegratedActionModelFactory {
 public:
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

  explicit IntegratedActionModelFactory();
  ~IntegratedActionModelFactory();

  boost::shared_ptr<crocoddyl::ActionModelAbstract> create(
      const std::string& integration_method, const std::size_t& dt,
      const boost::shared_ptr<crocoddyl::DifferentialActionModelAbstract>& diff_model) const;
};

}  // namespace multicopter_mpc
#endif