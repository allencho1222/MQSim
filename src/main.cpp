#include "IO_Flow_Parameter_Set.h"
#include "exec/Execution_Parameter_Set.h"
#include "exec/Host_System.h"
#include "exec/SSD_Device.h"
#include "ssd/SSD_Defs.h"
#include "utils/DistributionTypes.h"
#include <boost/program_options.hpp>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>
#include <string>
#include <yaml-cpp/yaml.h>

namespace po = boost::program_options;

void read_configuration_parameters(const std::string &ssdConfigFilePath,
                                   Execution_Parameter_Set &execParams) {
  const auto ssdConfig = YAML::LoadFile(ssdConfigFilePath);
  execParams.parseYAML(ssdConfig);
}

void read_workload_definitions(
    const std::string workloadDefsFilePath,
    std::vector<std::unique_ptr<IO_Flow_Parameter_Set>> &sceneDefs) {
  const auto workloadDefs = YAML::LoadFile(workloadDefsFilePath);
  for (const auto &sceneDef : workloadDefs["io_scenario"]) {
    const auto &ioFlowType = sceneDef["type"].as<std::string>();
    std::unique_ptr<IO_Flow_Parameter_Set> ioFlow;
    if (ioFlowType == "SYNTHETIC") {
      ioFlow = std::make_unique<IO_Flow_Parameter_Set_Synthetic>();
    } else if (ioFlowType == "TRACE") {
      ioFlow = std::make_unique<IO_Flow_Parameter_Set_Trace_Based>();
    } else {
      assert(false);
    }
    ioFlow->parseYAML(sceneDef["setup"]);
    sceneDefs.push_back(std::move(ioFlow));
  }
}

void collectResults(SSD_Device &ssd, Host_System &host,
                     const char *output_file_path) {
  Utils::XmlWriter xmlwriter;
  xmlwriter.Open(output_file_path);

  std::string tmp("MQSim_Results");
  xmlwriter.Write_open_tag(tmp);

  host.Report_results_in_XML("", xmlwriter);
  ssd.Report_results_in_XML("", xmlwriter);

  xmlwriter.Write_close_tag();

  std::vector<Host_Components::IO_Flow_Base *> IO_flows = host.Get_io_flows();
  for (unsigned int stream_id = 0; stream_id < IO_flows.size(); stream_id++) {
    std::cout << "Flow " << IO_flows[stream_id]->ID()
              << " - total requests generated: "
              << IO_flows[stream_id]->Get_generated_request_count()
              << " total requests serviced:"
              << IO_flows[stream_id]->Get_serviced_request_count() << std::endl;
    std::cout << "                   - device response time: "
              << IO_flows[stream_id]->Get_device_response_time() << " (us)"
              << " end-to-end request delay:"
              << IO_flows[stream_id]->Get_end_to_end_request_delay() << " (us)"
              << std::endl;
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::trace);

  po::options_description desc("Allowed options");
  desc.add_options()("config,c", po::value<std::string>()->required(),
                     "path to ssd config file")(
      "workload,w", po::value<std::string>()->required(),
      "path to workload config file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  auto ssdConfigFilePath = vm["config"].as<std::string>();
  auto workloadDefFilePath = vm["workload"].as<std::string>();

  auto execParams = std::make_unique<Execution_Parameter_Set>();
  read_configuration_parameters(ssdConfigFilePath, *execParams);
  std::vector<std::unique_ptr<IO_Flow_Parameter_Set>> sceneDefs;
  read_workload_definitions(workloadDefFilePath, sceneDefs);

  // The simulator should always be reset, before starting the actual
  // simulation
  Simulator->Reset();
  execParams->Host_Configuration.IO_Flow_Definitions.clear();
  execParams->Host_Configuration.IO_Flow_Definitions = std::move(sceneDefs);

  // Create SSD_Device based on the specified parameters
  SSD_Device ssd(execParams->SSD_Device_Configuration,
                 execParams->Host_Configuration.IO_Flow_Definitions);
  // Create Host_System based on the specified parameters
  execParams->Host_Configuration.Input_file_path =
      workloadDefFilePath.substr(0, workloadDefFilePath.find_last_of("."));
  Host_System host(&execParams->Host_Configuration,
                   execParams->SSD_Device_Configuration.Enabled_Preconditioning,
                   ssd.Host_interface);
  host.Attach_ssd_device(&ssd);

  fmt::print("simulation start\n");
  auto startTime = std::chrono::high_resolution_clock::now();
  Simulator->Start_simulation();
  auto endTime = std::chrono::high_resolution_clock::now();
  fmt::print("simulation complete\n");

  auto simTime = endTime - startTime;
  fmt::print("Total simulation time: {:%T}\n",
             std::chrono::floor<std::chrono::milliseconds>(simTime));

  collectResults(
      ssd, host,
      (workloadDefFilePath.substr(0, workloadDefFilePath.find_last_of(".")) +
       "_scenario_" + std::to_string(0) + ".xml")
          .c_str());

  return 0;
}
