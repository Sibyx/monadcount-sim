#include "monadcount_sim/wifi/RssiBasedAssocManager.hpp"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/mac48-address.h"

namespace monadcount_sim {
    namespace wifi {

        NS_LOG_COMPONENT_DEFINE ("RssiBasedAssocManager");
        NS_OBJECT_ENSURE_REGISTERED (RssiBasedAssocManager);

        ns3::TypeId
        RssiBasedAssocManager::GetTypeId (void)
        {
            static ns3::TypeId tid = ns3::TypeId ("monadcount_sim::wifi::RssiBasedAssocManager")
                    .SetParent<ns3::WifiAssocManager> ()
                    .SetGroupName ("MonadCountSim");
            return tid;
        }

        RssiBasedAssocManager::RssiBasedAssocManager ()
        {
            NS_LOG_FUNCTION (this);
        }

        RssiBasedAssocManager::~RssiBasedAssocManager ()
        {
            NS_LOG_FUNCTION (this);
        }

        void
        RssiBasedAssocManager::DoDispose (void)
        {
            NS_LOG_FUNCTION (this);
            ns3::WifiAssocManager::DoDispose ();
        }

        void
        RssiBasedAssocManager::TriggerHandover (ns3::Ptr<ns3::StaWifiMac> staMac, uint32_t targetApId)
        {
            return;
        }

    }
}
