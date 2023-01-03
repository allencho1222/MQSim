#ifndef FLASH_TRANSACTION_QUEUE_H
#define FLASH_TRANSACTION_QUEUE_H

#include "../sim/Sim_Reporter.h"
#include "NVM_Transaction_Flash.h"
#include "Queue_Probe.h"
#include <fmt/format.h>
#include <list>
#include <string>

namespace SSD_Components {
class Flash_Transaction_Queue : public std::list<NVM_Transaction_Flash *> {
public:
  Flash_Transaction_Queue();
  Flash_Transaction_Queue(std::string name);
  void Set_id(std::string name, int channelID, int chipID,
              std::string priority = "NONE");
  void push_back(NVM_Transaction_Flash *const &);
  void push_front(NVM_Transaction_Flash *const &);
  std::list<NVM_Transaction_Flash *>::iterator
  insert(list<NVM_Transaction_Flash *>::iterator position,
         NVM_Transaction_Flash *const &transaction);
  void remove(NVM_Transaction_Flash *const &transaction);
  void remove(std::list<NVM_Transaction_Flash *>::iterator const &itr_pos);
  void pop_front();
  // void Report_results_in_XML(std::string name_prefix,
  //                            Utils::XmlWriter &xmlwriter);
  int getNumEnqueued() const;
  int getNumDequeued() const;
  int getAvgQueueLength() const;
  int getMaxQueueLength() const;
  int getAvgWaitTime() const;
  int getMaxWaitTime() const;

  std::string getName() const;
  int getChannelID() const;
  int getChipID() const;
  std::string getPriority() const;

private:
  std::string name;
  int channelID;
  int chipID;
  std::string priority;
  Queue_Probe RequestQueueProbe;
};
} // namespace SSD_Components

template <> struct fmt::formatter<SSD_Components::Flash_Transaction_Queue> {
  // Parses format specifications of the form ['f' | 'e'].
  constexpr auto parse(format_parse_context &ctx) -> decltype(ctx.begin()) {
    return ctx.begin();
  }
  template <typename FormatContext>
  auto format(const SSD_Components::Flash_Transaction_Queue &q,
              FormatContext &ctx) const -> decltype(ctx.out()) {
    return fmt::format_to(
        ctx.out(), "{} {} {} {} {} {} {} {} {} {}", q.getName(),
        q.getChannelID(), q.getChipID(), q.getPriority(), q.getNumEnqueued(),
        q.getNumDequeued(), q.getAvgQueueLength(), q.getMaxQueueLength(),
        q.getAvgWaitTime(), q.getMaxWaitTime());
  }
};

#endif // !FLASH_TRANSACTION_QUEUE_H
