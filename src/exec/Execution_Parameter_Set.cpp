#include "Execution_Parameter_Set.h"

Host_Parameter_Set Execution_Parameter_Set::Host_Configuration;
Device_Parameter_Set Execution_Parameter_Set::SSD_Device_Configuration;

std::string Execution_Parameter_Set::hostResultFilePath = "host.result";
std::string Execution_Parameter_Set::hostInterfaceResultFilePath =
    "host_interface.result";
std::string Execution_Parameter_Set::ftlResultFilePath = "ftl.result";
std::string Execution_Parameter_Set::tsuResultFilePath = "tsu.result";
std::string Execution_Parameter_Set::chipResultFilePath = "chip.result";
std::string Execution_Parameter_Set::transactionHistoryFilePath = "transaction_history.result";

void Execution_Parameter_Set::parseYAML(const YAML::Node &ssdConfig) {
  const auto &resultFileYAMLNode = ssdConfig["result_file"];
  hostResultFilePath = resultFileYAMLNode["host"].as<std::string>();
  hostInterfaceResultFilePath =
      resultFileYAMLNode["host_interface"].as<std::string>();
  ftlResultFilePath = resultFileYAMLNode["ftl"].as<std::string>();
  tsuResultFilePath = resultFileYAMLNode["tsu"].as<std::string>();
  chipResultFilePath = resultFileYAMLNode["chip"].as<std::string>();
  transactionHistoryFilePath = resultFileYAMLNode["transaction"].as<std::string>();
  Host_Configuration.parseYAML(ssdConfig["host_parameters"]);
  SSD_Device_Configuration.parseYAML(ssdConfig["device_parameters"]);
}

// void Execution_Parameter_Set::XML_serialize(Utils::XmlWriter &xmlwriter) {
//   std::string tmp;
//   tmp = "Execution_Parameter_Set";
//   xmlwriter.Write_open_tag(tmp);
//
//   Host_Configuration.XML_serialize(xmlwriter);
//   SSD_Device_Configuration.XML_serialize(xmlwriter);
//
//   xmlwriter.Write_close_tag();
// }
//
// void Execution_Parameter_Set::XML_deserialize(rapidxml::xml_node<> *node) {
//   try {
//     for (auto param = node->first_node(); param;
//          param = param->next_sibling()) {
//       if (strcmp(param->name(), "Host_Parameter_Set") == 0) {
//         Host_Configuration.XML_deserialize(param);
//       } else if (strcmp(param->name(), "Device_Parameter_Set") == 0) {
//         SSD_Device_Configuration.XML_deserialize(param);
//       }
//     }
//   } catch (...) {
//     PRINT_ERROR("Error in the Execution_Parameter_Set!")
//   }
// }
