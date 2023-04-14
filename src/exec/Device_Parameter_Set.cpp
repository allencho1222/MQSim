#include "Device_Parameter_Set.h"
#include <algorithm>

std::unordered_map<std::string, NVM::NVM_Type>
    Device_Parameter_Set::memTypeMap = {{"FLASH", NVM::NVM_Type::FLASH}};
std::unordered_map<std::string, HostInterface_Types>
    Device_Parameter_Set::hostInterfaceTypeMap = {
        {"NVME", HostInterface_Types::NVME},
        {"SATA", HostInterface_Types::SATA}};
std::unordered_map<std::string, SSD_Components::Caching_Mechanism>
    Device_Parameter_Set::cachingMechanismMap = {
        {"SIMPLE", SSD_Components::Caching_Mechanism::SIMPLE},
        {"ADVANCED", SSD_Components::Caching_Mechanism::ADVANCED}};
std::unordered_map<std::string, SSD_Components::Cache_Sharing_Mode>
    Device_Parameter_Set::cacheSharingModeMap = {
        {"SHARED", SSD_Components::Cache_Sharing_Mode::SHARED},
        {"EQUAL_PARTITIONING",
         SSD_Components::Cache_Sharing_Mode::EQUAL_PARTITIONING}};

std::unordered_map<std::string, SSD_Components::Flash_Address_Mapping_Type>
    Device_Parameter_Set::pageMappingMap = {
        {"PAGE_LEVEL", SSD_Components::Flash_Address_Mapping_Type::PAGE_LEVEL},
        {"HYBRID", SSD_Components::Flash_Address_Mapping_Type::HYBRID}};
std::unordered_map<std::string, SSD_Components::CMT_Sharing_Mode>
    Device_Parameter_Set::cmtSharingModeMap = {
        {"SHARED", SSD_Components::CMT_Sharing_Mode::SHARED},
        {"EQUAL_SIZE_PARTITIONING",
         SSD_Components::CMT_Sharing_Mode::EQUAL_SIZE_PARTITIONING}};
std::unordered_map<std::string,
                   SSD_Components::Flash_Plane_Allocation_Scheme_Type>
    Device_Parameter_Set::planeAllocationSchemeMap = {
        {"CDPW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDPW},
        {"CDWP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDWP},
        {"CPDW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPDW},
        {"CPWD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPWD},
        {"CWDP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWDP},
        {"CWPD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWPD},
        {"DCPW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCPW},
        {"DCWP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCWP},
        {"DPCW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPCW},
        {"DPWC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPWC},
        {"DWCP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWCP},
        {"DWPC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWPC},
        {"PCDW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCDW},
        {"PCWD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCWD},
        {"PDCW", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDCW},
        {"PDWC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDWC},
        {"PWCD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWCD},
        {"PWDC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWDC},
        {"WCDP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCDP},
        {"WCPD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCPD},
        {"WDCP", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDCP},
        {"WDPC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDPC},
        {"WPCD", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPCD},
        {"WPDC", SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPDC}};
std::unordered_map<std::string, SSD_Components::Flash_Scheduling_Type>
    Device_Parameter_Set::schedulingTypeMap = {
        {"FLIN", SSD_Components::Flash_Scheduling_Type::FLIN},
        {"OUT_OF_ORDER", SSD_Components::Flash_Scheduling_Type::OUT_OF_ORDER},
        {"PRIORITY_OUT_OF_ORDER",
         SSD_Components::Flash_Scheduling_Type::PRIORITY_OUT_OF_ORDER}};
std::unordered_map<std::string, SSD_Components::GC_Block_Selection_Policy_Type>
    Device_Parameter_Set::gcBlockSelectionPolicyMap = {
        {"FLIN", SSD_Components::GC_Block_Selection_Policy_Type::FIFO},
        {"OUT_OF_ORDER",
         SSD_Components::GC_Block_Selection_Policy_Type::GREEDY},
        {"RANDOM", SSD_Components::GC_Block_Selection_Policy_Type::RANDOM},
        {"RANDOM_P", SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_P},
        {"RANDOM_PP",
         SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_PP},
        {"RGA", SSD_Components::GC_Block_Selection_Policy_Type::RGA}};
std::unordered_map<std::string, SSD_Components::ONFI_Protocol>
    Device_Parameter_Set::protocolMap = {
        {"NVDDR2", SSD_Components::ONFI_Protocol::NVDDR2}};

unsigned int Device_Parameter_Set::Initial_Erase_Count = 0;
int Device_Parameter_Set::Seed = 123; // Seed for random number generation (used
                                      // in device's random number generators)
bool Device_Parameter_Set::Enabled_Preconditioning = true;
NVM::NVM_Type Device_Parameter_Set::Memory_Type = NVM::NVM_Type::FLASH;
HostInterface_Types Device_Parameter_Set::HostInterface_Type =
    HostInterface_Types::NVME;
uint16_t Device_Parameter_Set::IO_Queue_Depth =
    1024; // For NVMe, it determines the size of the submission/completion
          // queues; for SATA, it determines the size of NCQ_Control_Structure
uint16_t Device_Parameter_Set::Queue_Fetch_Size =
    512; // Used in NVMe host interface
SSD_Components::Caching_Mechanism Device_Parameter_Set::Caching_Mechanism =
    SSD_Components::Caching_Mechanism::ADVANCED;
SSD_Components::Cache_Sharing_Mode
    Device_Parameter_Set::Data_Cache_Sharing_Mode =
        SSD_Components::Cache_Sharing_Mode::
            SHARED; // Data cache sharing among concurrently running I/O flows,
                    // if NVMe host interface is used
unsigned int Device_Parameter_Set::Data_Cache_Capacity =
    1024 * 1024 * 512; // Data cache capacity in bytes
unsigned int Device_Parameter_Set::Data_Cache_DRAM_Row_Size =
    8192; // The row size of DRAM in the data cache, the unit is bytes
unsigned int Device_Parameter_Set::Data_Cache_DRAM_Data_Rate =
    800; // Data access rate to access DRAM in the data cache, the unit is MT/s
unsigned int Device_Parameter_Set::Data_Cache_DRAM_Data_Busrt_Size =
    4; // The number of bytes that are transferred in one burst (it depends on
       // the number of DRAM chips)
sim_time_type Device_Parameter_Set::Data_Cache_DRAM_tRCD =
    13; // tRCD parameter to access DRAM in the data cache, the unit is
        // nano-seconds
sim_time_type Device_Parameter_Set::Data_Cache_DRAM_tCL =
    13; // tCL parameter to access DRAM in the data cache, the unit is
        // nano-seconds
sim_time_type Device_Parameter_Set::Data_Cache_DRAM_tRP =
    13; // tRP parameter to access DRAM in the data cache, the unit is
        // nano-seconds
SSD_Components::Flash_Address_Mapping_Type
    Device_Parameter_Set::Address_Mapping =
        SSD_Components::Flash_Address_Mapping_Type::PAGE_LEVEL;
bool Device_Parameter_Set::Ideal_Mapping_Table =
    false; // If mapping is ideal, then all the mapping entries are found in the
           // DRAM and there is no need to read mapping entries from flash
unsigned int Device_Parameter_Set::CMT_Capacity =
    2 * 1024 * 1024; // Size of SRAM/DRAM space that is used to cache address
                     // mapping table in bytes
SSD_Components::CMT_Sharing_Mode Device_Parameter_Set::CMT_Sharing_Mode =
    SSD_Components::CMT_Sharing_Mode::SHARED; // How the entire CMT space is
                                              // shared among concurrently
                                              // running flows
SSD_Components::Flash_Plane_Allocation_Scheme_Type
    Device_Parameter_Set::Plane_Allocation_Scheme =
        SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWDP;
SSD_Components::Flash_Scheduling_Type
    Device_Parameter_Set::Transaction_Scheduling_Policy =
        SSD_Components::Flash_Scheduling_Type::OUT_OF_ORDER;
double Device_Parameter_Set::Overprovisioning_Ratio =
    0.07; // The ratio of spare space with respect to the whole available
          // storage space of SSD
double Device_Parameter_Set::GC_Exec_Threshold =
    0.05; // The threshold for the ratio of free pages that used to trigger GC
SSD_Components::GC_Block_Selection_Policy_Type
    Device_Parameter_Set::GC_Block_Selection_Policy =
        SSD_Components::GC_Block_Selection_Policy_Type::RGA;
bool Device_Parameter_Set::Use_Copyback_for_GC = false;
bool Device_Parameter_Set::Preemptible_GC_Enabled = true;
bool Device_Parameter_Set::True_Lazy_Erase = true;
double Device_Parameter_Set::GC_Hard_Threshold =
    0.005; // The hard gc execution threshold, used to stop preemptible gc
           // execution
bool Device_Parameter_Set::Dynamic_Wearleveling_Enabled = true;
bool Device_Parameter_Set::Static_Wearleveling_Enabled = true;
unsigned int Device_Parameter_Set::Static_Wearleveling_Threshold = 100;
sim_time_type Device_Parameter_Set::Preferred_suspend_erase_time_for_read =
    700000; // in nano-seconds
sim_time_type Device_Parameter_Set::Preferred_suspend_erase_time_for_write =
    700000; // in nano-seconds
sim_time_type Device_Parameter_Set::Preferred_suspend_write_time_for_read =
    100000; // in nano-seconds
unsigned int Device_Parameter_Set::Flash_Channel_Count = 8;
unsigned int Device_Parameter_Set::Flash_Channel_Width =
    1; // Channel width in byte
unsigned int Device_Parameter_Set::Channel_Transfer_Rate = 300; // MT/s
unsigned int Device_Parameter_Set::Chip_No_Per_Channel = 4;
SSD_Components::ONFI_Protocol Device_Parameter_Set::Flash_Comm_Protocol =
    SSD_Components::ONFI_Protocol::NVDDR2;
Flash_Parameter_Set Device_Parameter_Set::Flash_Parameters;

void Device_Parameter_Set::parseYAML(const YAML::Node &deviceParams) {
  try {
    Initial_Erase_Count = 
      deviceParams["initial_erase_count"].as<unsigned int>();
    Seed = deviceParams["seed"].as<int>();
    Enabled_Preconditioning = deviceParams["enable_preconditioning"].as<bool>();
    const auto memType = deviceParams["memory_type"].as<std::string>();
    if (!memTypeMap.contains(memType)) {
      PRINT_ERROR("Unknown NVM type specified in the SSD configuration file")
    } else {
      Memory_Type = memTypeMap[memType];
    }
    const auto hostInterfaceType =
        deviceParams["host_interface_type"].as<std::string>();
    if (!hostInterfaceTypeMap.contains(hostInterfaceType)) {
      PRINT_ERROR("Unknown host interface type specified in the SSD "
                  "configuration file")
    } else {
      HostInterface_Type = hostInterfaceTypeMap[hostInterfaceType];
    }
    IO_Queue_Depth = deviceParams["io_queue_depth"].as<uint16_t>();
    Queue_Fetch_Size = deviceParams["queue_fetch_size"].as<uint16_t>();
    const auto cachingMechanism =
        deviceParams["caching_mechanism"].as<std::string>();
    if (!cachingMechanismMap.contains(cachingMechanism)) {
      PRINT_ERROR("Unknown data caching mechanism specified in the SSD "
                  "configuration file")
    } else {
      Caching_Mechanism = cachingMechanismMap[cachingMechanism];
    }

    const auto &dataCacheYAMLNode = deviceParams["data_cache"];
    Data_Cache_Capacity = dataCacheYAMLNode["capacity"].as<unsigned int>();
    Data_Cache_DRAM_Row_Size =
        dataCacheYAMLNode["dram_row_size"].as<unsigned int>();
    Data_Cache_DRAM_Data_Rate =
        dataCacheYAMLNode["dram_data_rate"].as<unsigned int>();
    Data_Cache_DRAM_Data_Busrt_Size =
        dataCacheYAMLNode["burst_size"].as<unsigned int>();
    Data_Cache_DRAM_tRCD = dataCacheYAMLNode["tRCD"].as<unsigned int>();
    Data_Cache_DRAM_tCL = dataCacheYAMLNode["tCL"].as<unsigned int>();
    Data_Cache_DRAM_tRP = dataCacheYAMLNode["tRP"].as<unsigned int>();
    const auto &cacheSharingMode = dataCacheYAMLNode["mode"].as<std::string>();
    if (!cacheSharingModeMap.contains(cacheSharingMode)) {
      PRINT_ERROR("Unknown data cache sharing mode specified in the SSD "
                  "configuration file")
    } else {
      Data_Cache_Sharing_Mode = cacheSharingModeMap[cacheSharingMode];
    }

    const auto &pageMappingYAMLNode = deviceParams["page_mapping"];
    Ideal_Mapping_Table = pageMappingYAMLNode["ideal"].as<bool>();
    const auto &pageMappingMode = pageMappingYAMLNode["mode"].as<std::string>();
    if (!pageMappingMap.contains(pageMappingMode)) {
      PRINT_ERROR("Unknown address mapping type specified in the SSD "
                  "configuration file")
    } else {
      Address_Mapping = pageMappingMap[cacheSharingMode];
    }

    const auto &cmtYAMLNode = deviceParams["cmt"];
    CMT_Capacity = cmtYAMLNode["capacity"].as<unsigned int>();
    const auto &cmtSharingMode = cmtYAMLNode["mode"].as<std::string>();
    if (!pageMappingMap.contains(pageMappingMode)) {
      PRINT_ERROR("Unknown CMT sharing mode specified in the SSD "
                  "configuration file")
    } else {
      CMT_Sharing_Mode = cmtSharingModeMap[cmtSharingMode];
    }

    const auto &planeAllocationScheme =
        deviceParams["plane_allocation_scheme"].as<std::string>();
    if (!planeAllocationSchemeMap.contains(planeAllocationScheme)) {
      PRINT_ERROR("Unknown plane allocation scheme type specified in the "
                  "SSD configuration file")
    } else {
      Plane_Allocation_Scheme = planeAllocationSchemeMap[planeAllocationScheme];
    }

    const auto &schedulingType =
        deviceParams["transaction_scheduling_policy"].as<std::string>();
    if (!schedulingTypeMap.contains(schedulingType)) {
      PRINT_ERROR("Unknown transaction scheduling type specified in the "
                  "SSD configuration file")
    } else {
      Transaction_Scheduling_Policy = schedulingTypeMap[schedulingType];
    }
    True_Lazy_Erase = deviceParams["true_lazy_erase"].as<bool>();

    const auto &gcYAMLNode = deviceParams["garbage_collection"];
    Overprovisioning_Ratio = gcYAMLNode["overprovisioning_ratio"].as<double>();
    GC_Exec_Threshold = gcYAMLNode["exec_threshold"].as<double>();
    GC_Hard_Threshold = gcYAMLNode["hard_threshold"].as<double>();
    Use_Copyback_for_GC = gcYAMLNode["use_copyback"].as<bool>();
    Preemptible_GC_Enabled = gcYAMLNode["enable_preemption"].as<bool>();
    const auto &gcBlockSelectionPolicy =
        gcYAMLNode["block_selection_policy"].as<std::string>();
    if (!gcBlockSelectionPolicyMap.contains(gcBlockSelectionPolicy)) {
      PRINT_ERROR("Unknown GC block selection policy specified in the SSD "
                  "configuration file")
    } else {
      GC_Block_Selection_Policy =
          gcBlockSelectionPolicyMap[gcBlockSelectionPolicy];
    }

    const auto &wlYAMLNode = deviceParams["wear_leveling"];
    Dynamic_Wearleveling_Enabled = wlYAMLNode["enable_dynamic"].as<bool>();
    Static_Wearleveling_Enabled = wlYAMLNode["enable_static"].as<bool>();
    Static_Wearleveling_Threshold =
        wlYAMLNode["static_threshold"].as<unsigned int>();

    const auto &suspendYAMLNode = deviceParams["suspend"];
    Preferred_suspend_erase_time_for_read =
        suspendYAMLNode["erase_time_for_read"].as<unsigned long long>();
    Preferred_suspend_erase_time_for_write =
        suspendYAMLNode["erase_time_for_write"].as<unsigned long long>();
    Preferred_suspend_write_time_for_read =
        suspendYAMLNode["write_time_for_read"].as<unsigned long long>();

    const auto &channelYAMLNode = deviceParams["channel"];
    Flash_Channel_Count = channelYAMLNode["channel_count"].as<unsigned int>();
    Flash_Channel_Width = channelYAMLNode["channel_width"].as<unsigned int>();
    Channel_Transfer_Rate = channelYAMLNode["transfer_rate"].as<unsigned int>();
    Chip_No_Per_Channel = channelYAMLNode["num_chips"].as<unsigned int>();
    const auto &protocol = channelYAMLNode["protocol"].as<std::string>();
    if (!protocolMap.contains(protocol)) {
      PRINT_ERROR("Unknown flash communication protocol type specified in "
                  "the SSD configuration file")
    } else {
      Flash_Comm_Protocol = protocolMap[protocol];
    }

    const auto &flashParams = deviceParams["flash_parameters"];
    Flash_Parameters.parseYAML(flashParams);
  } catch (...) {
    PRINT_ERROR("Error in Device_Parameter_Set!")
  }
}

// void Device_Parameter_Set::XML_serialize(Utils::XmlWriter &xmlwriter) {
//   std::string tmp;
//   tmp = "Device_Parameter_Set";
//   xmlwriter.Write_open_tag(tmp);
//
//   std::string attr = "Seed";
//   std::string val = std::to_string(Seed);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Enabled_Preconditioning";
//   val = (Enabled_Preconditioning ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Memory_Type";
//   val;
//   switch (Memory_Type) {
//   case NVM::NVM_Type::FLASH:
//     val = "FLASH";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "HostInterface_Type";
//   val;
//   switch (HostInterface_Type) {
//   case HostInterface_Types::NVME:
//     val = "NVME";
//     break;
//   case HostInterface_Types::SATA:
//     val = "SATA";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "IO_Queue_Depth";
//   val = std::to_string(IO_Queue_Depth);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Queue_Fetch_Size";
//   val = std::to_string(Queue_Fetch_Size);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Caching_Mechanism";
//   switch (Caching_Mechanism) {
//   case SSD_Components::Caching_Mechanism::SIMPLE:
//     val = "SIMPLE";
//     break;
//   case SSD_Components::Caching_Mechanism::ADVANCED:
//     val = "ADVANCED";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_Sharing_Mode";
//   switch (Data_Cache_Sharing_Mode) {
//   case SSD_Components::Cache_Sharing_Mode::SHARED:
//     val = "SHARED";
//     break;
//   case SSD_Components::Cache_Sharing_Mode::EQUAL_PARTITIONING:
//     val = "EQUAL_PARTITIONING";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_Capacity";
//   val = std::to_string(Data_Cache_Capacity);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_Row_Size";
//   val = std::to_string(Data_Cache_DRAM_Row_Size);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_Data_Rate";
//   val = std::to_string(Data_Cache_DRAM_Data_Rate);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_Data_Busrt_Size";
//   val = std::to_string(Data_Cache_DRAM_Data_Busrt_Size);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_tRCD";
//   val = std::to_string(Data_Cache_DRAM_tRCD);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_tCL";
//   val = std::to_string(Data_Cache_DRAM_tCL);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Data_Cache_DRAM_tRP";
//   val = std::to_string(Data_Cache_DRAM_tRP);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Address_Mapping";
//   switch (Address_Mapping) {
//   case SSD_Components::Flash_Address_Mapping_Type::PAGE_LEVEL:
//     val = "PAGE_LEVEL";
//     break;
//   case SSD_Components::Flash_Address_Mapping_Type::HYBRID:
//     val = "HYBRID";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Ideal_Mapping_Table";
//   val = (Use_Copyback_for_GC ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "CMT_Capacity";
//   val = std::to_string(CMT_Capacity);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "CMT_Sharing_Mode";
//   switch (CMT_Sharing_Mode) {
//   case SSD_Components::CMT_Sharing_Mode::SHARED:
//     val = "SHARED";
//     break;
//   case SSD_Components::CMT_Sharing_Mode::EQUAL_SIZE_PARTITIONING:
//     val = "EQUAL_SIZE_PARTITIONING";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Plane_Allocation_Scheme";
//   switch (Plane_Allocation_Scheme) {
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDPW:
//     val = "CDPW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDWP:
//     val = "CDWP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPDW:
//     val = "CPDW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPWD:
//     val = "CPWD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWDP:
//     val = "CWDP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWPD:
//     val = "CWPD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCPW:
//     val = "DCPW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCWP:
//     val = "DCWP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPCW:
//     val = "DPCW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPWC:
//     val = "DPWC";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWCP:
//     val = "DWCP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWPC:
//     val = "DWPC";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCDW:
//     val = "PCDW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCWD:
//     val = "PCWD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDCW:
//     val = "PDCW";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDWC:
//     val = "PDWC";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWCD:
//     val = "PWCD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWDC:
//     val = "PWDC";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCDP:
//     val = "WCDP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCPD:
//     val = "WCPD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDCP:
//     val = "WDCP";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDPC:
//     val = "WDPC";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPCD:
//     val = "WPCD";
//     break;
//   case SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPDC:
//     val = "WPDC";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Transaction_Scheduling_Policy";
//   switch (Transaction_Scheduling_Policy) {
//   case SSD_Components::Flash_Scheduling_Type::OUT_OF_ORDER:
//     val = "OUT_OF_ORDER";
//     break;
//   case SSD_Components::Flash_Scheduling_Type::PRIORITY_OUT_OF_ORDER:
//     val = "PRIORITY_OUT_OF_ORDER";
//     break;
//   case SSD_Components::Flash_Scheduling_Type::FLIN:
//     val = "FLIN";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Overprovisioning_Ratio";
//   val = std::to_string(Overprovisioning_Ratio);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "GC_Exec_Threshold";
//   val = std::to_string(GC_Exec_Threshold);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "GC_Block_Selection_Policy";
//   switch (GC_Block_Selection_Policy) {
//   case SSD_Components::GC_Block_Selection_Policy_Type::GREEDY:
//     val = "GREEDY";
//     break;
//   case SSD_Components::GC_Block_Selection_Policy_Type::RGA:
//     val = "RGA";
//     break;
//   case SSD_Components::GC_Block_Selection_Policy_Type::RANDOM:
//     val = "RANDOM";
//     break;
//   case SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_P:
//     val = "RANDOM_P";
//     break;
//   case SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_PP:
//     val = "RANDOM_PP";
//     break;
//   case SSD_Components::GC_Block_Selection_Policy_Type::FIFO:
//     val = "FIFO";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Use_Copyback_for_GC";
//   val = (Use_Copyback_for_GC ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Preemptible_GC_Enabled";
//   val = (Preemptible_GC_Enabled ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "GC_Hard_Threshold";
//   val = std::to_string(GC_Hard_Threshold);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Dynamic_Wearleveling_Enabled";
//   val = (Dynamic_Wearleveling_Enabled ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Static_Wearleveling_Enabled";
//   val = (Static_Wearleveling_Enabled ? "true" : "false");
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Static_Wearleveling_Threshold";
//   val = std::to_string(Static_Wearleveling_Threshold);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Preferred_suspend_erase_time_for_read";
//   val = std::to_string(Preferred_suspend_erase_time_for_read);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Preferred_suspend_erase_time_for_write";
//   val = std::to_string(Preferred_suspend_erase_time_for_write);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Preferred_suspend_write_time_for_read";
//   val = std::to_string(Preferred_suspend_write_time_for_read);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Flash_Channel_Count";
//   val = std::to_string(Flash_Channel_Count);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Flash_Channel_Width";
//   val = std::to_string(Flash_Channel_Width);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Channel_Transfer_Rate";
//   val = std::to_string(Channel_Transfer_Rate);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Chip_No_Per_Channel";
//   val = std::to_string(Chip_No_Per_Channel);
//   xmlwriter.Write_attribute_string(attr, val);
//
//   attr = "Flash_Comm_Protocol";
//   switch (Flash_Comm_Protocol) {
//   case SSD_Components::ONFI_Protocol::NVDDR2:
//     val = "NVDDR2";
//     break;
//   default:
//     break;
//   }
//   xmlwriter.Write_attribute_string(attr, val);
//
//   Flash_Parameters.XML_serialize(xmlwriter);
//
//   xmlwriter.Write_close_tag();
// }
//
// void Device_Parameter_Set::XML_deserialize(rapidxml::xml_node<> *node) {
//   try {
//     for (auto param = node->first_node(); param;
//          param = param->next_sibling()) {
//       if (strcmp(param->name(), "Seed") == 0) {
//         std::string val = param->value();
//         Seed = std::stoi(val);
//       } else if (strcmp(param->name(), "Enabled_Preconditioning") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Enabled_Preconditioning = (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "Memory_Type") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "FLASH") == 0)
//           Memory_Type = NVM::NVM_Type::FLASH;
//         else
//           PRINT_ERROR(
//               "Unknown NVM type specified in the SSD configuration file")
//       } else if (strcmp(param->name(), "HostInterface_Type") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "NVME") == 0) {
//           HostInterface_Type = HostInterface_Types::NVME;
//         } else if (strcmp(val.c_str(), "SATA") == 0) {
//           HostInterface_Type = HostInterface_Types::SATA;
//         } else {
//           PRINT_ERROR("Unknown host interface type specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "IO_Queue_Depth") == 0) {
//         std::string val = param->value();
//         IO_Queue_Depth = (uint16_t)std::stoull(val);
//       } else if (strcmp(param->name(), "Queue_Fetch_Size") == 0) {
//         std::string val = param->value();
//         Queue_Fetch_Size = (uint16_t)std::stoull(val);
//       } else if (strcmp(param->name(), "Caching_Mechanism") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "SIMPLE") == 0) {
//           Caching_Mechanism = SSD_Components::Caching_Mechanism::SIMPLE;
//         } else if (strcmp(val.c_str(), "ADVANCED") == 0) {
//           Caching_Mechanism = SSD_Components::Caching_Mechanism::ADVANCED;
//         } else {
//           PRINT_ERROR("Unknown data caching mechanism specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "Data_Cache_Sharing_Mode") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "SHARED") == 0) {
//           Data_Cache_Sharing_Mode = SSD_Components::Cache_Sharing_Mode::SHARED;
//         } else if (strcmp(val.c_str(), "EQUAL_PARTITIONING") == 0) {
//           Data_Cache_Sharing_Mode =
//               SSD_Components::Cache_Sharing_Mode::EQUAL_PARTITIONING;
//         } else {
//           PRINT_ERROR("Unknown data cache sharing mode specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "Data_Cache_Capacity") == 0) {
//         std::string val = param->value();
//         Data_Cache_Capacity = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_Row_Size") == 0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_Row_Size = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_Data_Rate") == 0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_Data_Rate = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_Data_Busrt_Size") ==
//                  0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_Data_Busrt_Size = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_tRCD") == 0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_tRCD = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_tCL") == 0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_tCL = std::stoul(val);
//       } else if (strcmp(param->name(), "Data_Cache_DRAM_tRP") == 0) {
//         std::string val = param->value();
//         Data_Cache_DRAM_tRP = std::stoul(val);
//       } else if (strcmp(param->name(), "Address_Mapping") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "PAGE_LEVEL") == 0) {
//           Address_Mapping =
//               SSD_Components::Flash_Address_Mapping_Type::PAGE_LEVEL;
//         } else if (strcmp(val.c_str(), "HYBRID") == 0) {
//           Address_Mapping = SSD_Components::Flash_Address_Mapping_Type::HYBRID;
//         } else {
//           PRINT_ERROR("Unknown address mapping type specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "Ideal_Mapping_Table") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Ideal_Mapping_Table = (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "CMT_Capacity") == 0) {
//         std::string val = param->value();
//         CMT_Capacity = std::stoul(val);
//       } else if (strcmp(param->name(), "CMT_Sharing_Mode") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "SHARED") == 0) {
//           CMT_Sharing_Mode = SSD_Components::CMT_Sharing_Mode::SHARED;
//         } else if (strcmp(val.c_str(), "EQUAL_PARTITIONING") == 0) {
//           CMT_Sharing_Mode =
//               SSD_Components::CMT_Sharing_Mode::EQUAL_SIZE_PARTITIONING;
//         } else {
//           PRINT_ERROR("Unknown CMT sharing mode specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "Plane_Allocation_Scheme") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "CDPW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDPW;
//         } else if (strcmp(val.c_str(), "CDWP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CDWP;
//         } else if (strcmp(val.c_str(), "CPDW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPDW;
//         } else if (strcmp(val.c_str(), "CPWD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CPWD;
//         } else if (strcmp(val.c_str(), "CWDP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWDP;
//         } else if (strcmp(val.c_str(), "CWPD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::CWPD;
//         } else if (strcmp(val.c_str(), "DCPW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCPW;
//         } else if (strcmp(val.c_str(), "DCWP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DCWP;
//         } else if (strcmp(val.c_str(), "DPCW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPCW;
//         } else if (strcmp(val.c_str(), "DPWC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DPWC;
//         } else if (strcmp(val.c_str(), "DWCP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWCP;
//         } else if (strcmp(val.c_str(), "DWPC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::DWPC;
//         } else if (strcmp(val.c_str(), "PCDW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCDW;
//         } else if (strcmp(val.c_str(), "PCWD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PCWD;
//         } else if (strcmp(val.c_str(), "PDCW") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDCW;
//         } else if (strcmp(val.c_str(), "PDWC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PDWC;
//         } else if (strcmp(val.c_str(), "PWCD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWCD;
//         } else if (strcmp(val.c_str(), "PWDC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::PWDC;
//         } else if (strcmp(val.c_str(), "WCDP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCDP;
//         } else if (strcmp(val.c_str(), "WCPD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WCPD;
//         } else if (strcmp(val.c_str(), "WDCP") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDCP;
//         } else if (strcmp(val.c_str(), "WDPC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WDPC;
//         } else if (strcmp(val.c_str(), "WPCD") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPCD;
//         } else if (strcmp(val.c_str(), "WPDC") == 0) {
//           Plane_Allocation_Scheme =
//               SSD_Components::Flash_Plane_Allocation_Scheme_Type::WPDC;
//         } else {
//           PRINT_ERROR("Unknown plane allocation scheme type specified in the "
//                       "SSD configuration file")
//         }
//       } else if (strcmp(param->name(), "Transaction_Scheduling_Policy") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "OUT_OF_ORDER") == 0) {
//           Transaction_Scheduling_Policy =
//               SSD_Components::Flash_Scheduling_Type::OUT_OF_ORDER;
//         } else if (strcmp(val.c_str(), "PRIORITY_OUT_OF_ORDER") == 0) {
//           Transaction_Scheduling_Policy =
//               SSD_Components::Flash_Scheduling_Type::PRIORITY_OUT_OF_ORDER;
//         } else if (strcmp(val.c_str(), "FLIN") == 0) {
//           Transaction_Scheduling_Policy =
//               SSD_Components::Flash_Scheduling_Type::FLIN;
//         } else {
//           PRINT_ERROR("Unknown transaction scheduling type specified in the "
//                       "SSD configuration file")
//         }
//       } else if (strcmp(param->name(), "Overprovisioning_Ratio") == 0) {
//         std::string val = param->value();
//         Overprovisioning_Ratio = std::stod(val);
//         if (Overprovisioning_Ratio < 0.05) {
//           PRINT_MESSAGE("The specified overprovisioning ratio is too small. "
//                         "The simluation may not run correctly.")
//         }
//       } else if (strcmp(param->name(), "GC_Exec_Threshold") == 0) {
//         std::string val = param->value();
//         GC_Exec_Threshold = std::stod(val);
//       } else if (strcmp(param->name(), "GC_Block_Selection_Policy") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "GREEDY") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::GREEDY;
//         } else if (strcmp(val.c_str(), "RGA") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::RGA;
//         } else if (strcmp(val.c_str(), "RANDOM") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::RANDOM;
//         } else if (strcmp(val.c_str(), "RANDOM_P") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_P;
//         } else if (strcmp(val.c_str(), "RANDOM_PP") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::RANDOM_PP;
//         } else if (strcmp(val.c_str(), "FIFO") == 0) {
//           GC_Block_Selection_Policy =
//               SSD_Components::GC_Block_Selection_Policy_Type::FIFO;
//         } else {
//           PRINT_ERROR("Unknown GC block selection policy specified in the SSD "
//                       "configuration file")
//         }
//       } else if (strcmp(param->name(), "Use_Copyback_for_GC") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Use_Copyback_for_GC = (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "Preemptible_GC_Enabled") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Preemptible_GC_Enabled = (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "GC_Hard_Threshold") == 0) {
//         std::string val = param->value();
//         GC_Hard_Threshold = std::stod(val);
//       } else if (strcmp(param->name(), "Dynamic_Wearleveling_Enabled") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Dynamic_Wearleveling_Enabled =
//             (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "Static_Wearleveling_Enabled") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         Static_Wearleveling_Enabled =
//             (val.compare("FALSE") == 0 ? false : true);
//       } else if (strcmp(param->name(), "Static_Wearleveling_Threshold") == 0) {
//         std::string val = param->value();
//         Static_Wearleveling_Threshold = std::stoul(val);
//       } else if (strcmp(param->name(),
//                         "Prefered_suspend_erase_time_for_read") == 0) {
//         std::string val = param->value();
//         Preferred_suspend_erase_time_for_read = std::stoull(val);
//       } else if (strcmp(param->name(),
//                         "Preferred_suspend_erase_time_for_write") == 0) {
//         std::string val = param->value();
//         Preferred_suspend_erase_time_for_write = std::stoull(val);
//       } else if (strcmp(param->name(),
//                         "Preferred_suspend_write_time_for_read") == 0) {
//         std::string val = param->value();
//         Preferred_suspend_write_time_for_read = std::stoull(val);
//       } else if (strcmp(param->name(), "Flash_Channel_Count") == 0) {
//         std::string val = param->value();
//         Flash_Channel_Count = std::stoul(val);
//       } else if (strcmp(param->name(), "Flash_Channel_Width") == 0) {
//         std::string val = param->value();
//         Flash_Channel_Width = std::stoul(val);
//       } else if (strcmp(param->name(), "Channel_Transfer_Rate") == 0) {
//         std::string val = param->value();
//         Channel_Transfer_Rate = std::stoul(val);
//       } else if (strcmp(param->name(), "Chip_No_Per_Channel") == 0) {
//         std::string val = param->value();
//         Chip_No_Per_Channel = std::stoul(val);
//       } else if (strcmp(param->name(), "Flash_Comm_Protocol") == 0) {
//         std::string val = param->value();
//         std::transform(val.begin(), val.end(), val.begin(), ::toupper);
//         if (strcmp(val.c_str(), "NVDDR2") == 0) {
//           Flash_Comm_Protocol = SSD_Components::ONFI_Protocol::NVDDR2;
//         } else {
//           PRINT_ERROR("Unknown flash communication protocol type specified in "
//                       "the SSD configuration file")
//         }
//       } else if (strcmp(param->name(), "Flash_Parameter_Set") == 0) {
//         Flash_Parameters.XML_deserialize(param);
//       }
//     }
//   } catch (...) {
//     PRINT_ERROR("Error in Device_Parameter_Set!")
//   }
// }
