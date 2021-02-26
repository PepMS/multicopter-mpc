#ifndef MULTICOPTER_MPC_UTILSparams__SERVER_HPP
#define MULTICOPTER_MPC_UTILSparams__SERVER_HPP

#include "multicopter_mpc/utils/converter.hpp"

#include <vector>
#include <regex>
#include <map>
#include <exception>

namespace multicopter_mpc {

class MissingValueException : public std::runtime_error {
 public:
  MissingValueException(std::string msg) : std::runtime_error(msg) {}
};

class ParamsServer {
 private:
  std::map<std::string, std::string> params_;

 public:
  ParamsServer() { params_ = std::map<std::string, std::string>(); }
  ParamsServer(std::map<std::string, std::string> params) { params_ = params; }
  ~ParamsServer() {
    //
  }

  void print() {
    for (auto it : params_) std::cout << it.first << "~~" << it.second << std::endl;
  }

  void addParam(std::string key, std::string value) {
    params_.insert(std::pair<std::string, std::string>(key, value));
  }

  //    template<typename T>
  //    T getParam(std::string key, std::string def_value) const {
  //        if(params_.find(key) != params_.end()){
  //            return converter<T>::convert(params_.find(key)->second);
  //        }else{
  //            return converter<T>::convert(def_value);
  //        }
  //    }

  template <typename T>
  T getParam(std::string key) const {
    if (params_.find(key) != params_.end()) {
      return converter<T>::convert(params_.find(key)->second);
    } else {
      throw MissingValueException("The following key: '" + key + "' has not been found in the parameters server.");
    }
  }
};

}  // namespace multicopter_mpc

#endif
