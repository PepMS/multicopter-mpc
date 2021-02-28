#ifndef BINDINGS_PYTHON_MULTICOPTER_MPC_BASE_HPP_
#define BINDINGS_PYTHON_MULTICOPTER_MPC_BASE_HPP_

#include "multicopter_mpc/mpc-base.hpp"
#include "python/multicopter_mpc/utils/vector-converter.hpp"

namespace multicopter_mpc {
namespace python {

namespace bp = boost::python;

class MpcAbstract_wrap : public MpcAbstract, public bp::wrapper<MpcAbstract> {
 public:
  MpcAbstract_wrap(const std::string& yaml_path) : MpcAbstract(yaml_path), bp::wrapper<MpcAbstract>() {}

  // using MpcAbstract::actuation_;
  // using MpcAbstract::actuation_squash_;
  // using MpcAbstract::platform_params_;
  // using MpcAbstract::robot_model_;
  // using MpcAbstract::robot_state_;
  // using MpcAbstract::squash_;

  // using MpcAbstract::costs_;

  // using MpcAbstract::createProblem;
  void createProblem() { this->get_override("createProblem")();}
};

void exposeMpcAbstract() {
  bp::class_<MpcAbstract_wrap, boost::noncopyable>(
      "MpcAbstract", "Abstract class to generate an MPC Controller to run on multicopter or aerial manipulators",
      bp::init<const std::string&>(bp::args("self", "yaml_path"), "Initialize the MPC Controller abstract class"))
      .def("createProblem", bp::pure_virtual(&MpcAbstract::createProblem), bp::args("self"))
      .add_property("robot_model", bp::make_function(&MpcAbstract_wrap::get_robot_model,
                                                     bp::return_value_policy<bp::return_by_value>()))
      .add_property("robot_model_path", bp::make_function(&MpcAbstract_wrap::get_robot_model_path,
                                                          bp::return_value_policy<bp::return_by_value>()))
      .add_property("platform_params", bp::make_function(&MpcAbstract_wrap::get_platform_params,
                                                         bp::return_value_policy<bp::return_by_value>()))
      .add_property("state", bp::make_function(&MpcAbstract_wrap::get_robot_state,
                                               bp::return_value_policy<bp::return_by_value>()))
      .add_property("actuation", bp::make_function(&MpcAbstract_wrap::get_actuation,
                                                   bp::return_value_policy<bp::return_by_value>()))
      .add_property("actuation_squash", bp::make_function(&MpcAbstract_wrap::get_actuation_squash,
                                                          bp::return_value_policy<bp::return_by_value>()))
      .add_property("squash",
                    bp::make_function(&MpcAbstract_wrap::get_squash, bp::return_value_policy<bp::return_by_value>()))
      .add_property("costs",
                    bp::make_function(&MpcAbstract_wrap::get_costs, bp::return_value_policy<bp::return_by_value>()),
                    "cost model sum for the given stage");
}

}  // namespace python
}  // namespace multicopter_mpc
#endif