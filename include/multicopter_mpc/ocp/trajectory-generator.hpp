#ifndef MULTICOPTER_MPC_OCP_TRAJECTORY_GENERATOR_HPP_
#define MULTICOPTER_MPC_OCP_TRAJECTORY_GENERATOR_HPP_

#include <assert.h>

#include <boost/shared_ptr.hpp>

#include "multicopter_mpc/ocp-base.hpp"
#include "multicopter_mpc/mission.hpp"
#include "multicopter_mpc/multicopter-base-params.hpp"

namespace multicopter_mpc {

class TrajectoryGenerator : public OcpAbstract {
 public:
  TrajectoryGenerator(const boost::shared_ptr<pinocchio::Model> model,
                      const boost::shared_ptr<MultiCopterBaseParams>& mc_params,
                      const boost::shared_ptr<Mission>& mission, const size_t& frame_base_link_id);
  ~TrajectoryGenerator();

  virtual void createProblem();

 private:
  boost::shared_ptr<Mission> mission_;
};
}  // namespace multicopter_mpc

#endif