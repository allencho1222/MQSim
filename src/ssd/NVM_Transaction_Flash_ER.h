#ifndef NVM_TRANSACTION_FLASH_ER_H
#define NVM_TRANSACTION_FLASH_ER_H

#include "../nvm_chip/flash_memory/FlashTypes.h"
#include "NVM_Transaction_Flash.h"
#include <list>

namespace SSD_Components {
  class NVM_Transaction_Flash_WR;
class NVM_Transaction_Flash_ER : public NVM_Transaction_Flash {
public:
  NVM_Transaction_Flash_ER(
      Transaction_Source_Type source, stream_id_type streamID,
      const NVM::FlashMemory::Physical_Page_Address &address, bool isFullErase = false);
  std::list<NVM_Transaction_Flash_WR *> Page_movement_activities;
};
} // namespace SSD_Components

#endif // !ERASE_TRANSACTION_H
