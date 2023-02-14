#include "NVM_Transaction_Flash_ER.h"
#include <spdlog/spdlog.h>

namespace SSD_Components {
NVM_Transaction_Flash_ER::NVM_Transaction_Flash_ER(
    Transaction_Source_Type source, stream_id_type streamID,
    const NVM::FlashMemory::Physical_Page_Address &address,
    bool isAdaptiveErase)
    : NVM_Transaction_Flash(source,
                            isAdaptiveErase ? Transaction_Type::ADAPTIVE_ERASE
                                            : Transaction_Type::PROXY_ERASE,
                            streamID, 0, NO_LPA, NO_PPA, address, NULL,
                            IO_Flow_Priority_Class::UNDEFINED) {}
} // namespace SSD_Components
