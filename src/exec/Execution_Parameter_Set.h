#ifndef EXECUTION_PARAMETER_SET_H
#define EXECUTION_PARAMETER_SET_H

#include "Device_Parameter_Set.h"
#include "Host_Parameter_Set.h"
#include "IO_Flow_Parameter_Set.h"
#include "Parameter_Set_Base.h"
#include <vector>
#include <yaml-cpp/yaml.h>

class Execution_Parameter_Set {
public:
  static Host_Parameter_Set Host_Configuration;
  static Device_Parameter_Set SSD_Device_Configuration;

  // void XML_serialize(Utils::XmlWriter &xmlwriter);
  // void XML_deserialize(rapidxml::xml_node<> *node);
  void parseYAML(const YAML::Node &ssdConfig);
  static std::string hostResultFilePath;
  static std::string hostInterfaceResultFilePath;
  static std::string ftlResultFilePath;
  static std::string tsuResultFilePath;
  static std::string chipResultFilePath;
};

#endif // !EXECUTION_PARAMETER_SET_H
