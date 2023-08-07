#ifndef TSU_H
#define TSU_H

#include "../nvm_chip/flash_memory/Flash_Chip.h"
#include "../sim/Sim_Defs.h"
#include "../sim/Sim_Object.h"
#include "../sim/Sim_Reporter.h"
#include "Flash_Transaction_Queue.h"
#include "NVM_PHY_ONFI_NVDDR2.h"
#include <fmt/os.h>
#include <list>
#include <optional>

namespace SSD_Components {
enum class Flash_Scheduling_Type { OUT_OF_ORDER, PRIORITY_OUT_OF_ORDER, FLIN };
class FTL;
class TSU_Base : public MQSimEngine::Sim_Object {
public:
  TSU_Base(const sim_object_id_type &id, FTL *ftl,
           NVM_PHY_ONFI_NVDDR2 *NVMController, Flash_Scheduling_Type Type,
           unsigned int Channel_no, unsigned int chip_no_per_channel,
           unsigned int DieNoPerChip, unsigned int PlaneNoPerDie,
           sim_time_type WriteReasonableSuspensionTimeForRead,
           sim_time_type EraseReasonableSuspensionTimeForRead,
           sim_time_type EraseReasonableSuspensionTimeForWrite,
           bool EraseSuspensionEnabled, bool ProgramSuspensionEnabled);
  virtual ~TSU_Base();
  void Setup_triggers();

  /*When an MQSim needs to send a set of transactions for execution, the
   * following three funcitons should be invoked in this order:
   * Prepare_for_transaction_submit()
   * Submit_transaction(transaction)
   * .....
   * Submit_transaction(transaction)
   * Schedule()
   *
   * The above mentioned mechanism helps to exploit die-level and plane-level
   * parallelism. More precisely, first the transactions are queued and then,
   * when the Schedule function is invoked, the TSU has that opportunity to
   * schedule them together to exploit multiplane and die-interleaved execution.
   */
  void Prepare_for_transaction_submit() {
    opened_scheduling_reqs++;
    if (opened_scheduling_reqs > 1) {
      return;
    }
    transaction_receive_slots.clear();
  }

  void Submit_transaction(NVM_Transaction_Flash *transaction);

  /* Shedules the transactions currently stored in inputTransactionSlots. The
   * transactions could be mixes of reads, writes, and erases.
   */
  virtual void Schedule(bool fromGC=false) = 0;
  // virtual void Report_results_in_XML(std::string name_prefix,
  //                                    Utils::XmlWriter &xmlwriter);
  virtual void reportResults(fmt::ostream& output);
  virtual void eraseTransaction(LPA_type lpa) = 0;
  virtual void eraseLock(LPA_type lpa) = 0;

  std::vector<sim_time_type> readScheduledAt;
protected:
  FTL *ftl;
  NVM_PHY_ONFI_NVDDR2 *_NVMController;
  Flash_Scheduling_Type type;
  unsigned int channel_count;
  unsigned int chip_no_per_channel;
  unsigned int die_no_per_chip;
  unsigned int plane_no_per_die;
  bool eraseSuspensionEnabled, programSuspensionEnabled;
  sim_time_type writeReasonableSuspensionTimeForRead;
  sim_time_type eraseReasonableSuspensionTimeForRead; // the time period
  sim_time_type eraseReasonableSuspensionTimeForWrite;
  flash_chip_ID_type
      *Round_robin_turn_of_channel; // Used for round-robin service of the chips
                                    // in channels

  static TSU_Base *_my_instance;
  std::list<NVM_Transaction_Flash *>
      transaction_receive_slots; // Stores the transactions that are received
                                 // for sheduling
  std::list<NVM_Transaction_Flash *>
      transaction_dispatch_slots; // Used to submit transactions to the channel
                                  // controller
  virtual bool service_aero_read(NVM::FlashMemory::Flash_Chip *chip, bool fromGC) = 0;
  virtual bool service_aero_write(NVM::FlashMemory::Flash_Chip *chip, bool fromGC) = 0;
  virtual bool service_read_transaction(NVM::FlashMemory::Flash_Chip *chip) = 0;
  virtual bool
  service_write_transaction(NVM::FlashMemory::Flash_Chip *chip) = 0;
  virtual bool
  service_full_erase_transaction(NVM::FlashMemory::Flash_Chip *chip) = 0;
  virtual bool
  service_shallow_erase_transaction(NVM::FlashMemory::Flash_Chip *chip) = 0;
  std::optional<NVM::FlashMemory::Physical_Page_Address>
  get_first_schedulable_addr(Flash_Transaction_Queue *q) const;
  bool pop_and_front_transaction(Flash_Transaction_Queue* q, int planeID);
  bool issue_command_to_chip(Flash_Transaction_Queue *sourceQueue1,
                             Flash_Transaction_Queue *sourceQueue2,
                             Transaction_Type transactionType,
                             bool suspensionRequired);
  static void handle_transaction_serviced_signal_from_PHY(
      NVM_Transaction_Flash *transaction);
  static void handle_channel_idle_signal(flash_channel_ID_type);
  static void handle_chip_idle_signal(NVM::FlashMemory::Flash_Chip *chip);
  int opened_scheduling_reqs;
  void process_chip_requests(NVM::FlashMemory::Flash_Chip *chip) {
    // if (!_my_instance->service_read_transaction(chip)) {
      // if (!_my_instance->service_write_transaction(chip)) {
    if (!_my_instance->service_aero_read(chip)) {
      if (!_my_instance->service_aero_write(chip)) {
        if (!_my_instance->service_full_erase_transaction(chip)) {
          _my_instance->service_shallow_erase_transaction(chip);
        }
      }
    }
  }
  std::unordered_map<LPA_type, std::pair<Flash_Transaction_Queue*, std::list<NVM_Transaction_Flash*>::iterator>>
    lockedQueue;

private:
  virtual void initQueue() = 0;
  virtual void clearQueue() = 0;
  bool transaction_is_ready(NVM_Transaction_Flash *transaction) const;
};
} // namespace SSD_Components

#endif // TSU_H
