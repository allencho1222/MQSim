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
    const std::vector<std::string> workloadDefsFilePaths,
    std::vector<std::unique_ptr<IO_Flow_Parameter_Set>> &sceneDefs) {
  for (const auto &workloadDefsFilePath : workloadDefsFilePaths) {
    const auto workloadDef = YAML::LoadFile(workloadDefsFilePath);
    const auto &ioFlowType = workloadDef["type"].as<std::string>();
    std::unique_ptr<IO_Flow_Parameter_Set> ioFlow;
    if (ioFlowType == "SYNTHETIC") {
      ioFlow = std::make_unique<IO_Flow_Parameter_Set_Synthetic>();
    } else if (ioFlowType == "TRACE") {
      ioFlow = std::make_unique<IO_Flow_Parameter_Set_Trace_Based>();
    } else {
      assert(false);
    }
    ioFlow->parseYAML(workloadDef);
    sceneDefs.push_back(std::move(ioFlow));
  }
}

int main(int argc, char *argv[]) {
  spdlog::set_level(spdlog::level::trace);
  spdlog::set_pattern("%v");

  po::options_description desc("Allowed options");
  desc.add_options()(
    "config,c", po::value<std::string>()->required(),
    "path to ssd config file")(
    "workload,w",
    po::value<std::vector<std::string>>()->multitoken()->required(),
    "path to workload config file")(
    "model,m", po::value<std::string>()->required(),
    "path to model config file");
  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  auto ssdConfigFilePath = vm["config"].as<std::string>();
  auto workloadDefFilePaths = vm["workload"].as<std::vector<std::string>>();
  auto blockModelFilePath = vm["model"].as<std::string>();

  auto execParams = std::make_unique<Execution_Parameter_Set>();
  read_configuration_parameters(ssdConfigFilePath, *execParams);
  std::vector<std::unique_ptr<IO_Flow_Parameter_Set>> sceneDefs;
  read_workload_definitions(workloadDefFilePaths, sceneDefs);

  // The simulator should always be reset, before starting the actual
  // simulation
  Simulator->Reset();
  execParams->Host_Configuration.IO_Flow_Definitions.clear();
  execParams->Host_Configuration.IO_Flow_Definitions = std::move(sceneDefs);

  // Create SSD_Device based on the specified parameters
  SSD_Device ssd(blockModelFilePath, 
                 execParams->SSD_Device_Configuration,
                 *execParams,
                 execParams->Host_Configuration.IO_Flow_Definitions);
  // Create Host_System based on the specified parameters
  execParams->Host_Configuration.Input_file_path = "workload";
  Host_System host(&execParams->Host_Configuration,
                   execParams->SSD_Device_Configuration.Enabled_Preconditioning,
                   ssd.Host_interface);
  host.Attach_ssd_device(&ssd);
  Simulator->initSim();

  // WARNING (sungjun): Currently, preconditinoing is only supported for
  // the TRACE-based simulation.
  auto flow_type = 
    (execParams->Host_Configuration).IO_Flow_Definitions[0]->Type;
  if (flow_type == Flow_Type::TRACE) {
    IO_Flow_Parameter_Set_Trace_Based *flow_param =
        static_cast<IO_Flow_Parameter_Set_Trace_Based *>(
            (execParams->Host_Configuration).IO_Flow_Definitions[0].get());
    // Run preconditioning only if the preconditionin trace file is provided.
    for (int i = 0; i < flow_param->Preconditioning_File_Paths.size(); ++i) {
      fmt::print("preconditioning start (num workloads: {})\n",
                 workloadDefFilePaths.size());
      auto startTime = std::chrono::high_resolution_clock::now();
      Simulator->Start_simulation(true);
      auto endTime = std::chrono::high_resolution_clock::now();
      fmt::print("preconditioning complete ({:%T})\n",
           std::chrono::floor<std::chrono::milliseconds>(endTime - startTime));
      Simulator->finishPreconditioning();
    }
  }
  fmt::print("simulation start (num workloads: {})\n",
             workloadDefFilePaths.size());
  auto startTime = std::chrono::high_resolution_clock::now();
  Simulator->Start_simulation(false);
  auto endTime = std::chrono::high_resolution_clock::now();
  fmt::print("simulation complete ({:%T})\n",
       std::chrono::floor<std::chrono::milliseconds>(endTime - startTime));

  host.reportResults(*execParams);
  ssd.reportResults(*execParams);

  return 0;
}
