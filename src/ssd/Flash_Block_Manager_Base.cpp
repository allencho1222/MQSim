#include "Flash_Block_Manager.h"
#include "Flash_Block_Manager_Base.h"
#include <cassert>
#include <yaml-cpp/yaml.h>
#include <map>
#include <numeric>
#include <random>
#include <algorithm>
#include <spdlog/spdlog.h>

namespace SSD_Components {
unsigned int Block_Pool_Slot_Type::Page_vector_size = 0;
Flash_Block_Manager_Base::Flash_Block_Manager_Base(
    GC_and_WL_Unit_Base *gc_and_wl_unit,
    unsigned int max_allowed_block_erase_count,
    unsigned int total_concurrent_streams_no, unsigned int channel_count,
    unsigned int chip_no_per_channel, unsigned int die_no_per_chip,
    unsigned int plane_no_per_die, unsigned int block_no_per_plane,
    unsigned int page_no_per_block, const std::string blockModelFile,
    unsigned int initialEraseCount,
    unsigned int maxReadToken, unsigned int maxWriteToken,
    int missPredictionRatio)
    : gc_and_wl_unit(gc_and_wl_unit),
      max_allowed_block_erase_count(max_allowed_block_erase_count),
      total_concurrent_streams_no(total_concurrent_streams_no),
      channel_count(channel_count), chip_no_per_channel(chip_no_per_channel),
      die_no_per_chip(die_no_per_chip), plane_no_per_die(plane_no_per_die),
      block_no_per_plane(block_no_per_plane),
      pages_no_per_block(page_no_per_block),
      initialEraseCount(initialEraseCount),
      generator(10000),
      distribution(1, 100),
      missPredictionRatio (missPredictionRatio) {
  uint32_t lastCategoryID = 0;
  uint32_t accBlockNum = 0;
  //std::unordered_map<unsigned int, unsigned int> blockCategory;
  std::vector<unsigned int> tempBlockCategories;
  // parse model file
  const auto yaml = YAML::LoadFile(blockModelFile);
  maxEraseLatency = yaml["max_erase_latency"].as<std::vector<sim_time_type>>();
  //auto targetPEC = yaml["target_pec"].as<uint32_t>();
  auto blockIndexFilePath = yaml["block_index_file"].as<std::string>();
  const auto categories = yaml["category"];
  double totalRatio = 0;
  eraseLatency.reserve(categories.size());
  std::set<uint32_t> dupIDs;
  for (auto it = std::cbegin(categories); it != std::cend(categories); ++it) {
    lastCategoryID = it->first.as<uint32_t>();
    assert(!dupIDs.contains(lastCategoryID));
    dupIDs.insert(lastCategoryID);
    const auto ratio = it->second["ratio"].as<double>();
    unsigned int numBlocksInCategory = 
      block_no_per_plane * (ratio / static_cast<double>(100.0));
    totalRatio += ratio;
    accBlockNum += numBlocksInCategory;
    for (int i = 0; i < numBlocksInCategory; ++i) {
      tempBlockCategories.push_back(lastCategoryID);
    }
    //blockCategory.insert_or_assign(lastCategoryID, numBlocksInCategory);
    const auto latency = it->second["latency"].as<sim_time_type>();
    eraseLatency[lastCategoryID] = latency;
  }
  assert(static_cast<int>(totalRatio) == 100);
  for (int i = accBlockNum; i < block_no_per_plane; ++i) {
    tempBlockCategories.push_back(lastCategoryID);
  }
  std::ifstream blockIndexFile(blockIndexFilePath);
  if (!blockIndexFile.is_open()) {
    std::cout << "Unable to open block index file\n";
    exit(-1);
  }
  std::vector<int> blockIndices;
  int num;
  while (blockIndexFile >> num) {
      blockIndices.push_back(num);
  }
  blockIndexFile.close();
  std::vector<uint32_t> blockCategories(tempBlockCategories.size());
  int i = 0;
  std::set<uint32_t> dupCheck;
  for (const auto& index : blockIndices) {
    assert(!dupCheck.contains(index));
    dupCheck.insert(index);
    blockCategories[i++] = tempBlockCategories[index];
  }
  assert(i == tempBlockCategories.size());
  fmt::print("correctly set block categories\n");

  plane_manager = new PlaneBookKeepingType ***[channel_count];
  for (unsigned int channelID = 0; channelID < channel_count; channelID++) {
    plane_manager[channelID] = new PlaneBookKeepingType **[chip_no_per_channel];
    for (unsigned int chipID = 0; chipID < chip_no_per_channel; chipID++) {
      plane_manager[channelID][chipID] =
          new PlaneBookKeepingType *[die_no_per_chip];
      for (unsigned int dieID = 0; dieID < die_no_per_chip; dieID++) {
        plane_manager[channelID][chipID][dieID] =
            new PlaneBookKeepingType[plane_no_per_die];
        // Initialize plane book keeping data structure
        for (unsigned int planeID = 0; planeID < plane_no_per_die; planeID++) {
          plane_manager[channelID][chipID][dieID][planeID].rwToken = 
              {0, (double)maxReadToken, (double)maxReadToken, (double)maxReadToken};
          plane_manager[channelID][chipID][dieID][planeID].numTotalCopyingPages = 0;
          plane_manager[channelID][chipID][dieID][planeID].Total_pages_count =
              block_no_per_plane * pages_no_per_block;
          plane_manager[channelID][chipID][dieID][planeID].Free_pages_count =
              block_no_per_plane * pages_no_per_block;
          plane_manager[channelID][chipID][dieID][planeID].Valid_pages_count =
              0;
          plane_manager[channelID][chipID][dieID][planeID].Invalid_pages_count =
              0;
          plane_manager[channelID][chipID][dieID][planeID]
              .Ongoing_erase_operations.clear();
          plane_manager[channelID][chipID][dieID][planeID].Blocks =
              new Block_Pool_Slot_Type[block_no_per_plane];

          // For each block in the plane, assign block category
          std::vector<unsigned int> categories = blockCategories;
          // Initialize block pool for plane
          for (unsigned int blockID = 0; blockID < block_no_per_plane;
               blockID++) {
            Block_Pool_Slot_Type::Page_vector_size =
                pages_no_per_block / (sizeof(uint64_t) * 8) +
                (pages_no_per_block % (sizeof(uint64_t) * 8) == 0 ? 0 : 1);
            auto& block = 
              plane_manager[channelID][chipID][dieID][planeID].Blocks[blockID];
            block.categoryID = categories.back();
            categories.pop_back();
            // TODO: init values in the constructor
            block.LatencyInitiated = false;
            block.BlockID = blockID;
            block.nextEraseLoopCount = 0;
            block.Current_page_write_index = 0;
            block.Current_status = Block_Service_Status::IDLE;
            block.Invalid_page_count = 0;
            block.Erase_count = 0;
            block.Holds_mapping_data = false;
            block.Has_ongoing_gc_wl = false;
            block.Erase_transaction = NULL;
            block.Ongoing_user_program_count = 0;
            block.Ongoing_user_read_count = 0;
            block.isBlockErased = true;
            block.remainingEraseLatency = 0;
            block.willBeAdaptivelyErased = true;
            block.Invalid_page_bitmap =
                new uint64_t[Block_Pool_Slot_Type::Page_vector_size];
            for (unsigned int i = 0; i < Block_Pool_Slot_Type::Page_vector_size;
                 i++) {
              block.Invalid_page_bitmap[i] = All_VALID_PAGE;
            }
            plane_manager[channelID][chipID][dieID][planeID]
                .Add_to_free_block_pool(&block, false);
          }
          assert(categories.empty());
          plane_manager[channelID][chipID][dieID][planeID].Data_wf =
              new Block_Pool_Slot_Type *[total_concurrent_streams_no];
          plane_manager[channelID][chipID][dieID][planeID].Translation_wf =
              new Block_Pool_Slot_Type *[total_concurrent_streams_no];
          plane_manager[channelID][chipID][dieID][planeID].GC_wf =
              new Block_Pool_Slot_Type *[total_concurrent_streams_no];
          for (unsigned int stream_cntr = 0;
               stream_cntr < total_concurrent_streams_no; stream_cntr++) {
            plane_manager[channelID][chipID][dieID][planeID]
                .Data_wf[stream_cntr] =
                plane_manager[channelID][chipID][dieID][planeID]
                    .Get_a_free_block(stream_cntr, false);
            plane_manager[channelID][chipID][dieID][planeID]
                .Translation_wf[stream_cntr] =
                plane_manager[channelID][chipID][dieID][planeID]
                    .Get_a_free_block(stream_cntr, true);
            plane_manager[channelID][chipID][dieID][planeID]
                .GC_wf[stream_cntr] =
                plane_manager[channelID][chipID][dieID][planeID]
                    .Get_a_free_block(stream_cntr, false);
          }
        }
      }
    }
  }

}

Flash_Block_Manager_Base::~Flash_Block_Manager_Base() {
  for (unsigned int channel_id = 0; channel_id < channel_count; channel_id++) {
    for (unsigned int chip_id = 0; chip_id < chip_no_per_channel; chip_id++) {
      for (unsigned int die_id = 0; die_id < die_no_per_chip; die_id++) {
        for (unsigned int plane_id = 0; plane_id < plane_no_per_die;
             plane_id++) {
          for (unsigned int blockID = 0; blockID < block_no_per_plane;
               blockID++) {
            delete[] plane_manager[channel_id][chip_id][die_id][plane_id]
                .Blocks[blockID]
                .Invalid_page_bitmap;
          }
          delete[] plane_manager[channel_id][chip_id][die_id][plane_id].Blocks;
          delete[] plane_manager[channel_id][chip_id][die_id][plane_id].GC_wf;
          delete[] plane_manager[channel_id][chip_id][die_id][plane_id].Data_wf;
          delete[] plane_manager[channel_id][chip_id][die_id][plane_id]
              .Translation_wf;
        }
        delete[] plane_manager[channel_id][chip_id][die_id];
      }
      delete[] plane_manager[channel_id][chip_id];
    }
    delete[] plane_manager[channel_id];
  }
  delete[] plane_manager;
}

void Flash_Block_Manager_Base::resetEraseCount() {
  // for (unsigned int channel_id = 0; channel_id < channel_count; channel_id++) {
  //   for (unsigned int chip_id = 0; chip_id < chip_no_per_channel; chip_id++) {
  //     for (unsigned int die_id = 0; die_id < die_no_per_chip; die_id++) {
  //       for (unsigned int plane_id = 0; plane_id < plane_no_per_die;
  //            plane_id++) {
  //         for (unsigned int blockID = 0; blockID < block_no_per_plane;
  //              blockID++) {
  //           auto &block = plane_manager[channel_id][chip_id][die_id][plane_id]
  //                             .Blocks[blockID];
  //           block.Erase_count = initialEraseCount;
  //         }
  //       }
  //     }
  //   }
  // }
}

void Flash_Block_Manager_Base::Set_GC_and_WL_Unit(GC_and_WL_Unit_Base *gcwl) {
  this->gc_and_wl_unit = gcwl;
}

Block_Pool_Slot_Type::Block_Pool_Slot_Type() {
}

void Block_Pool_Slot_Type::Erase() {
  Current_page_write_index = 0;
  Invalid_page_count = 0;
  //Erase_count++;
  for (unsigned int i = 0; i < Block_Pool_Slot_Type::Page_vector_size; i++) {
    Invalid_page_bitmap[i] = All_VALID_PAGE;
  }
  Stream_id = NO_STREAM;
  Holds_mapping_data = false;
  Erase_transaction = NULL;
}

// TODO: how to get num fail bits
// unsigned int
// Block_Pool_Slot_Type::getNumFailBits(unsigned int eraseLatency) const {
//   // TODO:
//   return numFailBits.at(std::make_pair(Erase_count, eraseLatency));
// }

Block_Pool_Slot_Type *
PlaneBookKeepingType::Get_a_free_block(stream_id_type stream_id,
                                       bool for_mapping_data) {
  Block_Pool_Slot_Type *new_block = NULL;
  new_block =
      (*Free_block_pool.begin()).second; // Assign a new write frontier block
  if (Free_block_pool.size() == 0) {
    PRINT_ERROR("Requesting a free block from an empty pool!")
  }
  Free_block_pool.erase(Free_block_pool.begin());
  new_block->Stream_id = stream_id;
  new_block->Holds_mapping_data = for_mapping_data;
  Block_usage_history.push_back(new_block->BlockID);

  return new_block;
}

void PlaneBookKeepingType::Check_bookkeeping_correctness(
    const NVM::FlashMemory::Physical_Page_Address &plane_address) {
  if (Total_pages_count !=
      Free_pages_count + Valid_pages_count + Invalid_pages_count) {
    PRINT_ERROR("Inconsistent status in the plane bookkeeping record!")
  }
  if (Free_pages_count == 0) {
    PRINT_ERROR("Plane "
                << "@" << plane_address.ChannelID << "@" << plane_address.ChipID
                << "@" << plane_address.DieID << "@" << plane_address.PlaneID
                << " pool size: " << Get_free_block_pool_size()
                << " ran out of free pages! Bad resource management! It is not "
                   "safe to continue simulation!");
  }
}

unsigned int PlaneBookKeepingType::Get_free_block_pool_size() {
  return (unsigned int)Free_block_pool.size();
}

void PlaneBookKeepingType::Add_to_free_block_pool(Block_Pool_Slot_Type *block,
                                                  bool consider_dynamic_wl) {
  if (consider_dynamic_wl) {
    std::pair<unsigned int, Block_Pool_Slot_Type *> entry(block->Erase_count,
                                                          block);
    Free_block_pool.insert(entry);
  } else {
    std::pair<unsigned int, Block_Pool_Slot_Type *> entry(0, block);
    Free_block_pool.insert(entry);
  }
}

unsigned int Flash_Block_Manager_Base::Get_min_max_erase_difference(
    const NVM::FlashMemory::Physical_Page_Address &plane_address) {
  unsigned int min_erased_block = 0;
  unsigned int max_erased_block = 0;
  PlaneBookKeepingType *plane_record =
      &plane_manager[plane_address.ChannelID][plane_address.ChipID]
                    [plane_address.DieID][plane_address.PlaneID];

  for (unsigned int i = 1; i < block_no_per_plane; i++) {
    if (plane_record->Blocks[i].Erase_count >
        plane_record->Blocks[max_erased_block].Erase_count) {
      max_erased_block = i;
    }
    if (plane_record->Blocks[i].Erase_count <
        plane_record->Blocks[min_erased_block].Erase_count) {
      min_erased_block = i;
    }
  }

  return max_erased_block - min_erased_block;
}

flash_block_ID_type Flash_Block_Manager_Base::Get_coldest_block_id(
    const NVM::FlashMemory::Physical_Page_Address &plane_address) {
  unsigned int min_erased_block = 0;
  PlaneBookKeepingType *plane_record =
      &plane_manager[plane_address.ChannelID][plane_address.ChipID]
                    [plane_address.DieID][plane_address.PlaneID];

  for (unsigned int i = 1; i < block_no_per_plane; i++) {
    if (plane_record->Blocks[i].Erase_count <
        plane_record->Blocks[min_erased_block].Erase_count) {
      min_erased_block = i;
    }
  }

  return min_erased_block;
}

PlaneBookKeepingType *Flash_Block_Manager_Base::Get_plane_bookkeeping_entry(
    const NVM::FlashMemory::Physical_Page_Address &plane_address) {
  return &(plane_manager[plane_address.ChannelID][plane_address.ChipID]
                        [plane_address.DieID][plane_address.PlaneID]);
}

bool Flash_Block_Manager_Base::Block_has_ongoing_gc_wl(
    const NVM::FlashMemory::Physical_Page_Address &block_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[block_address.ChannelID][block_address.ChipID]
                    [block_address.DieID][block_address.PlaneID];
  return plane_record->Blocks[block_address.BlockID].Has_ongoing_gc_wl;
}

bool Flash_Block_Manager_Base::Can_execute_gc_wl(
    const NVM::FlashMemory::Physical_Page_Address &block_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[block_address.ChannelID][block_address.ChipID]
                    [block_address.DieID][block_address.PlaneID];
  return (
      plane_record->Blocks[block_address.BlockID].Ongoing_user_program_count +
          plane_record->Blocks[block_address.BlockID].Ongoing_user_read_count ==
      0);
}

void Flash_Block_Manager_Base::GC_WL_started(
    const NVM::FlashMemory::Physical_Page_Address &block_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[block_address.ChannelID][block_address.ChipID]
                    [block_address.DieID][block_address.PlaneID];
  auto &block = plane_record->Blocks[block_address.BlockID];
  // assert(block.isFullyErased);
  // assert(block.isShallowlyErased);
  assert(block.nextEraseLoopCount == 0);
  assert(block.willBeAdaptivelyErased);
  assert(block.isBlockErased);
  block.Has_ongoing_gc_wl = true;
  block.willBeAdaptivelyErased = false;
  block.isBlockErased = false;
}

void Flash_Block_Manager_Base::program_transaction_issued(
    const NVM::FlashMemory::Physical_Page_Address &page_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[page_address.ChannelID][page_address.ChipID]
                    [page_address.DieID][page_address.PlaneID];
  plane_record->Blocks[page_address.BlockID].Ongoing_user_program_count++;
}

void Flash_Block_Manager_Base::Read_transaction_issued(
    const NVM::FlashMemory::Physical_Page_Address &page_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[page_address.ChannelID][page_address.ChipID]
                    [page_address.DieID][page_address.PlaneID];
  plane_record->Blocks[page_address.BlockID].Ongoing_user_read_count++;
}

void Flash_Block_Manager_Base::Program_transaction_serviced(
    const NVM::FlashMemory::Physical_Page_Address &page_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[page_address.ChannelID][page_address.ChipID]
                    [page_address.DieID][page_address.PlaneID];
  assert(plane_record->Blocks[page_address.BlockID].Ongoing_user_program_count >
         0);
  plane_record->Blocks[page_address.BlockID].Ongoing_user_program_count--;
}

void Flash_Block_Manager_Base::Read_transaction_serviced(
    const NVM::FlashMemory::Physical_Page_Address &page_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[page_address.ChannelID][page_address.ChipID]
                    [page_address.DieID][page_address.PlaneID];
  assert(plane_record->Blocks[page_address.BlockID].Ongoing_user_read_count >
         0);
  plane_record->Blocks[page_address.BlockID].Ongoing_user_read_count--;
}

bool Flash_Block_Manager_Base::Is_having_ongoing_program(
    const NVM::FlashMemory::Physical_Page_Address &block_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[block_address.ChannelID][block_address.ChipID]
                    [block_address.DieID][block_address.PlaneID];
  return plane_record->Blocks[block_address.BlockID]
             .Ongoing_user_program_count > 0;
}

void Flash_Block_Manager_Base::GC_WL_finished(
    const NVM::FlashMemory::Physical_Page_Address &block_address) {
  PlaneBookKeepingType *plane_record =
      &plane_manager[block_address.ChannelID][block_address.ChipID]
                    [block_address.DieID][block_address.PlaneID];
  plane_record->Blocks[block_address.BlockID].Has_ongoing_gc_wl = false;
}

bool Flash_Block_Manager_Base::Is_page_valid(Block_Pool_Slot_Type *block,
                                             flash_page_ID_type page_id) {
  if ((block->Invalid_page_bitmap[page_id / 64] & (((uint64_t)1) << page_id)) ==
      0) {
    return true;
  }
  return false;
}

void Flash_Block_Manager_Base::finishEraseLoop(
    const NVM::FlashMemory::Physical_Page_Address &addr) {
  PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  auto &block = planeRecord->Blocks[addr.BlockID];
  assert(block.nextEraseLoopCount < maxEraseLatency.size());
  sim_time_type _maxEraseLatency = maxEraseLatency[block.nextEraseLoopCount];
  if (block.remainingEraseLatency < _maxEraseLatency) {
    block.remainingEraseLatency = 0;
  } else {
    block.remainingEraseLatency -= _maxEraseLatency;
  }
  block.nextEraseLoopCount++;
}

bool Flash_Block_Manager_Base::isAdaptiveEraseInitiated(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  return block.willBeAdaptivelyErased;
}

void Flash_Block_Manager_Base::initiateAdaptiveErase(
    const NVM::FlashMemory::Physical_Page_Address &addr) {
  PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  auto &block = planeRecord->Blocks[addr.BlockID];
  block.willBeAdaptivelyErased = true;
}

void Flash_Block_Manager_Base::eraseBlock(
    const NVM::FlashMemory::Physical_Page_Address &addr) {
  PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  auto &block = planeRecord->Blocks[addr.BlockID];
  // const auto &blockModel = blockModels[block.categoryID];
  // int PEC = block.Erase_count;
  // const auto blockStatsIter = blockModel.upper_bound(PEC);
  // assert(blockStatsIter != blockModel.end());
  // const auto &blockStats = blockStatsIter->second;
  // assert(block.nextEraseLoopCount == blockStats.size());
  assert(block.remainingEraseLatency == 0);
  assert(!block.isBlockErased);
  assert(block.willBeAdaptivelyErased);
  block.isBlockErased = true;
  block.LatencyInitiated = false;
  // TODO (sungjun): need to separate logical and physical erase count.
  // physical erase count: # of erase operation.
  // logical erase count: # of ISPEs
  block.Erase_count++;
  block.nextEraseLoopCount = 0;
}

bool Flash_Block_Manager_Base::isBlockErased(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  return block.isBlockErased;
}

void Flash_Block_Manager_Base::initEraseLatency(
    const NVM::FlashMemory::Physical_Page_Address &addr) {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  auto &block = planeRecord->Blocks[addr.BlockID];
  assert(block.remainingEraseLatency == 0);
  assert(block.categoryID != -1);
  block.remainingEraseLatency = eraseLatency[block.categoryID];
  if (distribution(generator) <= missPredictionRatio) {
    block.remainingEraseLatency += 500000;
  }
  
  block.LatencyInitiated = true;
}

std::optional<sim_time_type> Flash_Block_Manager_Base::getNextEraseLatency(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  // const auto &blockModel = blockModels[block.categoryID];
  if (block.remainingEraseLatency == 0) {
    return std::nullopt;
  } else {
    assert(block.nextEraseLoopCount < maxEraseLatency.size());
    sim_time_type _maxEraseLatency = maxEraseLatency[block.nextEraseLoopCount];
    if (block.remainingEraseLatency > _maxEraseLatency) {
      return _maxEraseLatency;
    } else {
      return block.remainingEraseLatency;
    }
  }
}

bool Flash_Block_Manager_Base::isLatencyInitiated(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  auto &block = planeRecord->Blocks[addr.BlockID];
  assert(block.categoryID != -1);
  return block.LatencyInitiated;
}

int Flash_Block_Manager_Base::numRemainingEraseLoops(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  int tempLoopCount = block.nextEraseLoopCount;
  sim_time_type tempLatency = block.remainingEraseLatency;
  while (tempLatency != 0) {
    assert(tempLoopCount < maxEraseLatency.size());
    sim_time_type maxLatency = maxEraseLatency[tempLoopCount++];
    if (tempLatency > maxLatency) {
      tempLatency -= maxLatency;
    } else {
      tempLatency = 0;
    }
  }
  return tempLoopCount - block.nextEraseLoopCount;
}

void Flash_Block_Manager_Base::addGCToken(
    const NVM::FlashMemory::Physical_Page_Address& addr,
    uint32_t numValids) {
  if (numValids > 0) {
    auto plane =
        &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
    plane->numTotalCopyingPages += numValids;
    uint32_t numInvalids = pages_no_per_block - numValids;
    double tokenIncrement = static_cast<double>(numInvalids) / numValids;
    double maxToken = std::max(tokenIncrement, 1.0);
    Token gcToken = {numValids, tokenIncrement, maxToken, tokenIncrement};
    plane->gcTokens.push(gcToken);
  }
}

bool Flash_Block_Manager_Base::isTokenAvailable(
    const NVM::FlashMemory::Physical_Page_Address& addr,
    bool isGC) const {
  const auto plane =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  if (isGC) {
    assert(!plane->gcTokens.empty());
    const auto& gcToken = plane->gcTokens.front();
    return gcToken.curToken > 1.0;
  } else {
    return plane->rwToken.curToken > 1.0;
  }
}

void Flash_Block_Manager_Base::useToken(
    const NVM::FlashMemory::Physical_Page_Address& addr,
    bool isGC) {
  auto plane =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  if (isGC) {
    assert(!plane->gcTokens.empty());
    auto& gcToken = plane->gcTokens.front();
    assert(gcToken.curToken > 0);
    assert(gcToken.numCopies > 0);
    gcToken.curToken -= 1.0;
  } else {
    plane->rwToken.curToken -= 1.0;
  }
}

void Flash_Block_Manager_Base::resetToken(
    const NVM::FlashMemory::Physical_Page_Address& addr,
    bool isGC) {
  auto plane =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  if (isGC) {
    assert(!plane->gcTokens.empty());
    auto& gcToken = plane->gcTokens.front();
    //assert(gcToken.curToken == 0);
    gcToken.curToken = std::min(
      gcToken.curToken + gcToken.tokenIncrement, gcToken.maxToken);

    gcToken.numCopies -= 1;
    if (gcToken.numCopies == 0) {
      plane->gcTokens.pop();
    }
    assert(plane->numTotalCopyingPages > 0);
    plane->numTotalCopyingPages -= 1;
    if (plane->numTotalCopyingPages == 0) {
      assert(plane->gcTokens.empty());
    }
  } else {
    auto& rwToken = plane->rwToken;
    rwToken.curToken = std::min(
      rwToken.curToken + rwToken.tokenIncrement, rwToken.maxToken);
  }
}

bool Flash_Block_Manager_Base::isPlaneDoingGC(
    const NVM::FlashMemory::Physical_Page_Address& addr) const {
  const auto plane =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  return plane->numTotalCopyingPages > 0;
}

void Flash_Block_Manager_Base::printToken(
    const NVM::FlashMemory::Physical_Page_Address& addr,
    bool rw, bool gc) const {
  if (!gc_and_wl_unit->isPre) {
    const auto plane =
        &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
    if (rw) {
      double token = plane->rwToken.curToken;
      SPDLOG_TRACE("TOKEN,{},{},{},{}", addr.ChannelID, addr.ChipID, addr.PlaneID, token);
    }
    if (gc) {
      double token = plane->gcTokens.empty() ? -1 : plane->gcTokens.front().curToken;
      SPDLOG_TRACE("TOKEN,{},{},{},{}", addr.ChannelID, addr.ChipID, addr.PlaneID, token);
    }
  }
}
} // namespace SSD_Components
