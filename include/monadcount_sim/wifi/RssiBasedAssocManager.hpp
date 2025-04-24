#ifndef MONADCOUNT_SIM_WIFI_RSSI_BASED_ASSOC_MANAGER_HPP
#define MONADCOUNT_SIM_WIFI_RSSI_BASED_ASSOC_MANAGER_HPP

#include "ns3/object.h"
#include "ns3/wifi-assoc-manager.h"
#include "ns3/sta-wifi-mac.h"
#include "ns3/mac48-address.h"

namespace monadcount_sim {
    namespace wifi {

/**
 * \brief Custom association manager that triggers realistic handover based on RSSI.
 *
 * This class extends ns3::WifiAssocManager by providing a method to trigger a handover
 * procedure. The handover is performed by first deauthenticating the STA and then, after a short delay,
 * associating with the target Access Point (AP) specified by its identifier.
 */
        class RssiBasedAssocManager : public ns3::WifiAssocManager
        {
        public:
            static ns3::TypeId GetTypeId (void);
            RssiBasedAssocManager ();
            virtual ~RssiBasedAssocManager ();

            /**
             * \brief Trigger a handover for the given STA WiFi MAC.
             * \param staMac Pointer to the STA’s WiFi MAC instance.
             * \param targetApId Identifier of the target AP.
             *
             * This method simulates a realistic handover by performing a deauthentication,
             * then scheduling reassociation with the new AP after a short delay.
             */
            void TriggerHandover (ns3::Ptr<ns3::StaWifiMac> staMac, uint32_t targetApId);

        protected:
            virtual void DoDispose (void);
        };

    } // namespace wifi
} // namespace monadcount_sim

#endif // MONADCOUNT_SIM_WIFI_RSSI_BASED_ASSOC_MANAGER_HPP
