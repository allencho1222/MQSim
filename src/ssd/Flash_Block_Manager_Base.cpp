#include "Flash_Block_Manager.h"
#include <cassert>
#include <yaml-cpp/yaml.h>

namespace SSD_Components {
unsigned int Block_Pool_Slot_Type::Page_vector_size = 0;
Flash_Block_Manager_Base::Flash_Block_Manager_Base(
    GC_and_WL_Unit_Base *gc_and_wl_unit,
    unsigned int max_allowed_block_erase_count,
    unsigned int total_concurrent_streams_no, unsigned int channel_count,
    unsigned int chip_no_per_channel, unsigned int die_no_per_chip,
    unsigned int plane_no_per_die, unsigned int block_no_per_plane,
    unsigned int page_no_per_block, const std::string blockModelFile)
    : gc_and_wl_unit(gc_and_wl_unit),
      max_allowed_block_erase_count(max_allowed_block_erase_count),
      total_concurrent_streams_no(total_concurrent_streams_no),
      channel_count(channel_count), chip_no_per_channel(chip_no_per_channel),
      die_no_per_chip(die_no_per_chip), plane_no_per_die(plane_no_per_die),
      block_no_per_plane(block_no_per_plane),
      pages_no_per_block(page_no_per_block) {
  const auto yaml = YAML::LoadFile(blockModelFile);
  for (auto it = std::cbegin(yaml); it != std::cend(yaml); ++it) {
    auto blockModelID = it->first.as<std::string>();
    const auto ratio = it->second["ratio"].as<int>();
    const auto stat = it->second["status"];
    auto blockModel = BlockModel();
    for (auto s = std::cbegin(stat); s != std::cend(stat); ++s) {
      const auto pec = s->first.as<int>();
      auto blockStats = BlockStats();
      for (const auto &loop : s->second) {
        const auto eraseLatency = loop["latency"].as<sim_time_type>();
        assert(loop["num_fail_bits"].IsSequence());
        const auto [min, max] = loop["num_fail_bits"].as<std::pair<int, int>>();
        blockStats.push_back({eraseLatency, {min, max}});
      }
      blockModel.insert(std::make_pair(pec, std::move(blockStats)));
    }
    blockModels.push_back(std::move(blockModel));
  }
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

          // Initialize block pool for plane
          for (unsigned int blockID = 0; blockID < block_no_per_plane;
               blockID++) {
            Block_Pool_Slot_Type::Page_vector_size =
                pages_no_per_block / (sizeof(uint64_t) * 8) +
                (pages_no_per_block % (sizeof(uint64_t) * 8) == 0 ? 0 : 1);
            auto &block = plane_manager[channelID][chipID][dieID][planeID]
                              .Blocks[blockID];
            // TODO: init values in the constructor
            block.BlockID = blockID;
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

void Flash_Block_Manager_Base::Set_GC_and_WL_Unit(GC_and_WL_Unit_Base *gcwl) {
  this->gc_and_wl_unit = gcwl;
}

Block_Pool_Slot_Type::Block_Pool_Slot_Type() {
}

void Block_Pool_Slot_Type::Erase() {
  Current_page_write_index = 0;
  Invalid_page_count = 0;
  Erase_count++;
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
  block.Has_ongoing_gc_wl = true;
  block.willBeAdaptivelyErased = false;
  block.isBlockErased = false;
  block.nextEraseLoopCount = 0;
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
  assert(block.nextEraseLoopCount > 0);
  assert(!block.isBlockErased);
  assert(block.willBeAdaptivelyErased);
  block.isBlockErased = true;
}

bool Flash_Block_Manager_Base::isBlockErased(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  return block.isBlockErased;
}

std::optional<sim_time_type> Flash_Block_Manager_Base::getNextEraseLatency(
    const NVM::FlashMemory::Physical_Page_Address &addr) const {
  const PlaneBookKeepingType *planeRecord =
      &plane_manager[addr.ChannelID][addr.ChipID][addr.DieID][addr.PlaneID];
  const auto &block = planeRecord->Blocks[addr.BlockID];
  const auto &blockModel = blockModels[block.blockModelID];
  int PEC = block.Erase_count;
  const auto blockStatsIter = blockModel.lower_bound(PEC);
  assert(blockStatsIter != blockModel.end());
  const auto &blockStats = blockStatsIter->second;
  if (block.nextEraseLoopCount < blockStats.size()) {
    return blockStats[block.nextEraseLoopCount].eraseLatency;
  } else {
    return std::nullopt;
  }
}

} // namespace SSD_Components
