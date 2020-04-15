#include "Client.h"


//enum of current connection states
typedef enum {
  scanning,
  opening,
  discoverServices,
  discoverCharacteristics,
  readCharacteristics,
  enableIndication,
  running
} ConnState;
//enum of current connection properties
typedef struct {
  uint8_t  connectionHandle;
  int8_t   rssi;
  uint16_t serverAddress;
  uint32_t thermometerServiceHandle;
  uint16_t thermometerCharacteristicHandle;
  uint32_t temperature;
} ConnProperties;


// Flag for indicating DFU Reset must be performed
uint8_t bootToDfu = 0;
// Array for holding properties of multiple (parallel) connections
ConnProperties connProperties[MAX_CONNECTIONS];
// Counter of active connections
uint8_t activeConnectionsNum;
// State of the connection under establishment
ConnState connState;

// Health Thermometer service UUID defined by Bluetooth SIG
const uint8_t thermoService[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
const uint8_t thermoChar[2] = { 0x1c, 0x2a };


// Init connection properties
void initProperties(void)
{
  uint8_t i;
  activeConnectionsNum = 0;

  for (i = 0; i < MAX_CONNECTIONS; i++) {
    connProperties[i].connectionHandle = CONNECTION_HANDLE_INVALID;
    connProperties[i].thermometerServiceHandle = SERVICE_HANDLE_INVALID;
    connProperties[i].thermometerCharacteristicHandle = CHARACTERISTIC_HANDLE_INVALID;
    connProperties[i].temperature = TEMP_INVALID;
    connProperties[i].rssi = RSSI_INVALID;
  }
}

// Parse advertisements looking for advertised Health Thermometer service
uint8_t findServiceInAdvertisement(uint8_t *data, uint8_t len)
{
  uint8_t adFieldLength;
  uint8_t adFieldType;
  uint8_t i = 0;
  // Parse advertisement packet
  while (i < len) {
    adFieldLength = data[i];
    adFieldType = data[i + 1];
    // Partial ($02) or complete ($03) list of 16-bit UUIDs
    if (adFieldType == 0x02 || adFieldType == 0x03) {
      // compare UUID to Health Thermometer service UUID
      if (memcmp(&data[i + 2], thermoService, 2) == 0) {
        return 1;
      }
    }
    // advance to the next AD struct
    i = i + adFieldLength + 1;
  }
  return 0;
}

// Find the index of a given connection in the connection_properties array
uint8_t findIndexByConnectionHandle(uint8_t connection)
{
  for (uint8_t i = 0; i < activeConnectionsNum; i++) {
    if (connProperties[i].connectionHandle == connection) {
      return i;
    }
  }
  return TABLE_INDEX_INVALID;
}

// Add a new connection to the connection_properties array
void addConnection(uint8_t connection, uint16_t address)
{
  connProperties[activeConnectionsNum].connectionHandle = connection;
  connProperties[activeConnectionsNum].serverAddress    = address;
  activeConnectionsNum++;
}

// Remove a connection from the connection_properties array
void removeConnection(uint8_t connection)
{
  uint8_t i;
  uint8_t table_index = findIndexByConnectionHandle(connection);

  if (activeConnectionsNum > 0) {
    activeConnectionsNum--;
  }
  // Shift entries after the removed connection toward 0 index
  for (i = table_index; i < activeConnectionsNum; i++) {
    connProperties[i] = connProperties[i + 1];
  }
  // Clear the slots we've just removed so no junk values appear
  for (i = activeConnectionsNum; i < MAX_CONNECTIONS; i++) {
    connProperties[i].connectionHandle = CONNECTION_HANDLE_INVALID;
    connProperties[i].thermometerServiceHandle = SERVICE_HANDLE_INVALID;
    connProperties[i].thermometerCharacteristicHandle = CHARACTERISTIC_HANDLE_INVALID;
    connProperties[i].temperature = TEMP_INVALID;
    connProperties[i].rssi = RSSI_INVALID;
  }
}


/**
 * bluetooth stack function to handle the specific commands and events
 *
 * @param evt which will be passed from gecko external signal
 *
 */
void gecko_update_client(struct gecko_cmd_packet* evt)
{

	 displayPrintf(DISPLAY_ROW_NAME,"CLIENT");
	  uint8_t* charValue;
	  uint16_t addrValue;
	  uint8_t tableIndex;

  // Handle stack events
  switch (BGLIB_MSG_ID(evt->header)) {
    // This boot event is generated when the system boots up after reset
    case gecko_evt_system_boot_id:
      printf("\r\nBLE Central started\r\n");
      // Set passive scanning on 1Mb PHY
      displayPrintf(DISPLAY_ROW_CONNECTION,"DISCOVERING");
      gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, SCAN_PASSIVE);
      // Set scan interval and scan window
      gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, SCAN_INTERVAL, SCAN_WINDOW);
      // Set the default connection parameters for subsequent connections
      gecko_cmd_le_gap_set_conn_parameters(connection_int_min, connection_int_max, slave_latency, timeout);
      // Start scanning - looking for thermometer devices
      gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
      connState = scanning;
      break;


    // This event is generated when an advertisement packet or a scan response
    // is received from a slave
    case gecko_evt_le_gap_scan_response_id:
      // Parse advertisement packets
      if (evt->data.evt_le_gap_scan_response.packet_type == 0) {
        // If a thermometer advertisement is found...
        if (findServiceInAdvertisement(&(evt->data.evt_le_gap_scan_response.data.data[0]),
                                       evt->data.evt_le_gap_scan_response.data.len) != 0) {
          // then stop scanning for a while
          gecko_cmd_le_gap_end_procedure();

          // and connect to that device
          if (activeConnectionsNum < MAX_CONNECTIONS) {
            gecko_cmd_le_gap_connect(evt->data.evt_le_gap_scan_response.address,
                                     evt->data.evt_le_gap_scan_response.address_type,
                                     le_gap_phy_1m);
            connState = opening;
          }
        }
      }
      break;




    // This event is generated when a new connection is established
    case gecko_evt_le_connection_opened_id:
      // Get last two bytes of sender address
      addrValue = (uint16_t)(evt->data.evt_le_connection_opened.address.addr[1] << 8) \
                  + evt->data.evt_le_connection_opened.address.addr[0];
      // Add connection to the connection_properties array

      addConnection(evt->data.evt_le_connection_opened.connection, addrValue);
      displayPrintf(DISPLAY_ROW_CONNECTION,"CONNECTED");
      gecko_cmd_le_connection_set_parameters(val, connection_int_min, connection_int_max, slave_latency, timeout);

	    // Server address stored
  	uint8_t address_buff[6];
  	address_buff[0] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[0]);
	address_buff[1] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[1]);
	address_buff[2] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[2]);
	address_buff[3] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[3]);
	address_buff[4] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[4]);
	address_buff[5] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[5]);

	//Prints address on lcd
	displayPrintf(DISPLAY_ROW_BTADDR2,"%01x:%01x:%01x:%01x:%01x:%01x ",  address_buff[5],address_buff[4],address_buff[3],address_buff[2],address_buff[1],address_buff[0]);


      // Discover Health Thermometer service on the slave device
      gecko_cmd_gatt_discover_primary_services_by_uuid(evt->data.evt_le_connection_opened.connection,
                                                       2,
                                                       (const uint8_t*)thermoService);
      //get client address
	  struct gecko_msg_system_get_bt_address_rsp_t* address=gecko_cmd_system_get_bt_address();
	  displayPrintf(DISPLAY_ROW_NAME,"CLIENT");

	  char buffer[18];
	  sprintf(buffer,"%s",(char *)address);
	  //displays the client address
	  displayPrintf(DISPLAY_ROW_BTADDR,"%x:%x:%x:%x:%x:%x", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);

      connState = discoverServices;				//connection state changed to discover
      break;

    // This event is generated when a new service is discovered
    case gecko_evt_gatt_service_id:
      tableIndex = findIndexByConnectionHandle(evt->data.evt_gatt_service.connection);
      if (tableIndex != TABLE_INDEX_INVALID) {
        // Save service handle for future reference
        connProperties[tableIndex].thermometerServiceHandle = evt->data.evt_gatt_service.service;
      }
      break;

    // This event is generated when a new characteristic is discovered
    case gecko_evt_gatt_characteristic_id:
      tableIndex = findIndexByConnectionHandle(evt->data.evt_gatt_characteristic.connection);
      if (tableIndex != TABLE_INDEX_INVALID) {
        // Save characteristic handle for future reference
        connProperties[tableIndex].thermometerCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
      }
      break;


    // This event is generated for various procedure completions, e.g. when a
    // write procedure is completed, or service discovery is completed
    case gecko_evt_gatt_procedure_completed_id:
      tableIndex = findIndexByConnectionHandle(evt->data.evt_gatt_procedure_completed.connection);
      if (tableIndex == TABLE_INDEX_INVALID) {
        break;
      }
      // If service discovery finished
      if (connState == discoverServices \
          && connProperties[tableIndex].thermometerServiceHandle != SERVICE_HANDLE_INVALID) {
    // Discover thermometer characteristic on the slave device
        gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                        connProperties[tableIndex].thermometerServiceHandle,
                                                        2,
                                                        (const uint8_t*)thermoChar);
        connState = discoverCharacteristics;
        break;
      }

      // If characteristic discovery finished
      if (connState == discoverCharacteristics \
          && connProperties[tableIndex].thermometerCharacteristicHandle != CHARACTERISTIC_HANDLE_INVALID) {
        // stop discovering
        gecko_cmd_le_gap_end_procedure();
        // enable indications

        gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                       connProperties[tableIndex].thermometerCharacteristicHandle,
                                                       gatt_indication);
        connState = enableIndication;
        break;
      }
      // If indication enable process finished
      if (connState == enableIndication) {
        // and we can connect to more devices
        if (activeConnectionsNum < MAX_CONNECTIONS) {
        	 displayPrintf(DISPLAY_ROW_CONNECTION,"HANDLING INDICATIONS");
          // start scanning again to find new devices
          gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
          connState = scanning;
        } else {
          connState = running;
        }
        break;
      }
      break;

    // This event is generated when a connection is dropped
    case gecko_evt_le_connection_closed_id:
    {
        // remove connection from active connections
        // start scanning again to find new devices
			displayPrintf(DISPLAY_ROW_CONNECTION,"Discovering");
			if(bootToDfu==1)
				{
					gecko_cmd_system_reset(2);
				}
			else if(bootToDfu==0)
				{
				gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);
				}

        displayPrintf(DISPLAY_ROW_CONNECTION,"DISCOVERING");
        connState = scanning;

      break;
    }

    // This event is generated when a characteristic value was received e.g. an indication
    case gecko_evt_gatt_characteristic_value_id:
      charValue = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
      tableIndex = findIndexByConnectionHandle(evt->data.evt_gatt_characteristic_value.connection);
      if (tableIndex != TABLE_INDEX_INVALID) {
        connProperties[tableIndex].temperature = (charValue[1] << 0) + (charValue[2] << 8) + (charValue[3] << 16);
      }
      //display the temperature obtained from server on client board
      displayPrintf(DISPLAY_ROW_TEMPVALUE,"%f",(float)(connProperties[tableIndex].temperature)/1000);


      // Send confirmation for the indication
      gecko_cmd_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
      // Trigger RSSI measurement on the connection

      break;



    default:
      break;
  }
}
