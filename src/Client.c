#include "Client.h"

int value;
//enum of current connection states
typedef enum {
  scanning,
  opening,
  discoverServices,
  discoverCharacteristics,
  readCharacteristics,
  enableIndication,
  Indication,
  running
} ConnState;
//enum of current connection properties
typedef struct {
  uint8_t  connectionHandle;
  uint8_t uuidHandle;
  int8_t   rssi;
  uint16_t serverAddress;
  uint32_t thermometerServiceHandle;
  uint16_t thermometerCharacteristicHandle;
  uint32_t temperature;


  uint32_t buttonServiceHandle;
  uint16_t buttonCharacteristicHandle;
  uint32_t buttonstate;
} ConnProperties;

// Flag for indicating DFU Reset must be performed
uint8_t bootToDfu = 0;
// Array for holding properties of multiple (parallel) connections
ConnProperties connProperties[MAX_CONNECTIONS];

////////////////////Conn_properties conn_properties[MAX_CONNECTIONS];

uint32_t temperature_flag=0;
uint32_t button_flag=0;

// Counter of active connections
uint8_t activeConnectionsNum;
// State of the connection under establishment
ConnState connState;



// Health Thermometer service UUID defined by Bluetooth SIG
const uint8_t thermoService[2] = { 0x09, 0x18 };
// Temperature Measurement characteristic UUID defined by Bluetooth SIG
const uint8_t thermoChar[2] = { 0x1c, 0x2a };

const uint8_t buttonService[16]={0x89,0x62,0x13,0x2d,0x2a,0x65,0xec,0x87,0x3e,0x43,0xc8,0x38,0x01,0x00,0x00,0x00};

const uint8_t buttonChar[16]={0x89,0x62,0x13,0x2d,0x2a,0x65,0xec,0x87,0x3e,0x43,0xc8,0x38,0x02,0x00,0x00,0x00};


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

uint8_t findIndexByuuidHandle(uint8_t uuid)
{
  for (uint8_t i = 0; i < activeConnectionsNum; i++) {
    if (connProperties[i].uuidHandle == uuid) {
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

	  uint8_t table[100];


  // Handle stack events
  switch (BGLIB_MSG_ID(evt->header)) {
    // This boot event is generated when the system boots up after reset
    case gecko_evt_system_boot_id:
      printf("\r\nBLE Central started\r\n");
      // Set passive scanning on 1Mb PHY

      LOG_INFO("BOOT ID\n");
      gecko_cmd_sm_delete_bondings();
      displayPrintf(DISPLAY_ROW_CONNECTION,"DISCOVERING");
      gecko_cmd_le_gap_set_discovery_type(le_gap_phy_1m, SCAN_PASSIVE);
      // Set scan interval and scan window
      gecko_cmd_le_gap_set_discovery_timing(le_gap_phy_1m, SCAN_INTERVAL, SCAN_WINDOW);
      // Set the default connection parameters for subsequent connections
      gecko_cmd_le_gap_set_conn_parameters(connection_int_min, connection_int_max, slave_latency, timeout);
      // Start scanning - looking for thermometer devices
      gecko_cmd_le_gap_start_discovery(le_gap_phy_1m, le_gap_discover_generic);

		// Delete bonding information
		gecko_cmd_sm_delete_bondings();


      gecko_cmd_sm_configure(7,1);
      connState = scanning;
      break;


    // This event is generated when an advertisement packet or a scan response
    // is received from a slave
    case gecko_evt_le_gap_scan_response_id:

    	LOG_INFO("SCAN RESPONSE ID\n");
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

    	LOG_INFO("connection_opened_id\n");
      // Get last two bytes of sender address
      addrValue = (uint16_t)(evt->data.evt_le_connection_opened.address.addr[1] << 8) \
                  + evt->data.evt_le_connection_opened.address.addr[0];
      // Add connection to the connection_properties array

      addConnection(evt->data.evt_le_connection_opened.connection, addrValue);
      displayPrintf(DISPLAY_ROW_CONNECTION,"CONNECTED");
      gecko_cmd_le_connection_set_parameters(val, connection_int_min, connection_int_max, slave_latency, timeout);

      value=evt->data.evt_le_connection_opened.connection;
	    // Server address is stored
  	uint8_t address_buff[6];
  	address_buff[0] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[0]);
	address_buff[1] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[1]);
	address_buff[2] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[2]);
	address_buff[3] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[3]);
	address_buff[4] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[4]);
	address_buff[5] =(uint8_t)(evt->data.evt_le_connection_opened.address.addr[5]);

	//Prints address on LCD display
	displayPrintf(DISPLAY_ROW_BTADDR2,"%01x:%01x:%01x:%01x:%01x:%01x ",  address_buff[5],address_buff[4],address_buff[3],address_buff[2],address_buff[1],address_buff[0]);



    // Discover Health Thermometer service on the slave device
    gecko_cmd_gatt_discover_primary_services(evt->data.evt_le_connection_opened.connection);


      //get client address
	  struct gecko_msg_system_get_bt_address_rsp_t* address=gecko_cmd_system_get_bt_address();
	  displayPrintf(DISPLAY_ROW_NAME,"CLIENT");

	  char buffer[18];
	  sprintf(buffer,"%s",(char *)address);
	  //displays the client address
	  displayPrintf(DISPLAY_ROW_BTADDR,"%x:%x:%x:%x:%x:%x", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);

      connState = discoverServices;				//connection state changed to discover
      break;

      //case for checking external event
      case gecko_evt_system_external_signal_id:
    	  LOG_INFO("external_signal\n");
      			// Checking GPIO Interrupt flag through external signal

      				LOG_INFO("external_signal 111\n");
      				button&=~PB0_PRESSED;
      				//This command can be used for accepting or rejecting reported confirm value.
      				gecko_cmd_sm_passkey_confirm(value,1);
      				displayPrintf(DISPLAY_ROW_ACTION ,"  ");
      				displayPrintf(DISPLAY_ROW_PASSKEY ,"  ");

      			break;

	  case gecko_evt_sm_confirm_passkey_id:								//confirm the passkey when read of characteristic is pressed on mobile app
	 		LOG_INFO("confirm_passkeyYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYYY\n");
	 		gecko_cmd_sm_passkey_confirm(value,1);
	 		uint32 pass=evt->data.evt_sm_confirm_passkey.passkey;		//get passkey displayed on mobile app
	 		displayPrintf(DISPLAY_ROW_PASSKEY,"PASSKEY %d",pass);		//print the passkey displayed on mobile app
	 		displayPrintf(DISPLAY_ROW_ACTION,"Confirm with PB0");
        	break;

	  case gecko_evt_sm_bonded_id:										//triggered when bonding is complete
	 		 LOG_INFO("bonding_successfulLLLLLLLLLLLLLLLLLLLLLLLL bonding\n");
	 		 if(button_flag==1)
	 		 {

	 		 displayPrintf(DISPLAY_ROW_ACTION,"");
	 		 displayPrintf(DISPLAY_ROW_CONNECTION,"Bonded");			//print bonded on lcd
	 		gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,connProperties[0].buttonServiceHandle,
	 		                                                             16,
	 		                                                             (const uint8_t*)buttonChar);
	 		connState=discoverServices;
	 		 }
	 		break;


    // This event is generated when a new service is discovered
    case gecko_evt_gatt_service_id:
    	  //    const uint8_t table_service[32] =(evt->data.evt_gatt_service.uuid.data);
    	      LOG_INFO("\nSERVICE UUID=%d\n",table);
    	      if (!(memcmp(evt->data.evt_gatt_service.uuid.data,thermoService,2)))
    	      {
    	    	  temperature_flag=1;
    	    	  LOG_INFO("SERVICE FOR TEMPPPPPPPPPPPPPPPP\n");
    	    	  connProperties[0].thermometerServiceHandle = evt->data.evt_gatt_service.service;
    	      }

    	      if (!(memcmp(evt->data.evt_gatt_service.uuid.data,buttonService,16))) {
    	    	  button_flag=1;
    	    	  LOG_INFO("SERVICE FOR BUTTONNNNNNNNNNNNNNNN\n");
    	        // Save service handle for future reference
    	        connProperties[0].buttonServiceHandle = evt->data.evt_gatt_service.service;
    	      }
    	      connState=discoverServices;
      break;

    // This event is generated when a new characteristic is discovered
    case gecko_evt_gatt_characteristic_id:


      if(connState==discoverCharacteristics)
      {
		if(!(memcmp(evt->data.evt_gatt_characteristic.uuid.data,buttonChar,16)))
		{
			//characteristics value for the Button
			connProperties[0].thermometerCharacteristicHandle = evt->data.evt_gatt_characteristic.characteristic;
			connState = enableIndication;

		}
		else if(!(memcmp(evt->data.evt_gatt_characteristic.uuid.data,thermoChar,2)))
		{
			//characteristics value for the Temperature
			connProperties[0].buttonCharacteristicHandle= evt->data.evt_gatt_characteristic.characteristic;
			connState = enableIndication;

		}
		break;
		}
		break;


    // This event is generated for various procedure completions, e.g. when a
    // write procedure is completed, or service discovery is completed
    case gecko_evt_gatt_procedure_completed_id:

    	LOG_INFO("\nprocedure_completed\n");
      // If service discovery finished
    	if (connState == discoverServices)
    	{
    		if ( temperature_flag==1) {
    	  LOG_INFO(" temp complete\n");
    // Discover thermometer characteristic on the slave device
        gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                        connProperties[0].thermometerServiceHandle,
                                                        2,
                                                        (const uint8_t*)thermoChar);


    	}

        if (button_flag==1){
        gecko_cmd_gatt_discover_characteristics_by_uuid(evt->data.evt_gatt_procedure_completed.connection,
                                                        connProperties[0].buttonServiceHandle,
                                                        16,
                                                        (const uint8_t*)buttonChar);
        LOG_INFO(" button complete\n");



      }
        connState = discoverCharacteristics;
        break;
    	}



      // If characteristic discovery finished
      if (connState == enableIndication)  {

    	  if( temperature_flag==1 ){
        // stop discovering
    		  temperature_flag=0;
        gecko_cmd_le_gap_end_procedure();
        // enable indications
        LOG_INFO(" temp charac complete\n");
        gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                       connProperties[0].thermometerCharacteristicHandle,
                                                       gatt_indication);
    	  }
      // If characteristic discovery finished
    	  if (button_flag==1) {
        // stop discovering
    	  gecko_cmd_le_gap_end_procedure();
        // enable indications
          LOG_INFO(" button chara complete\n");
          gecko_cmd_gatt_set_characteristic_notification(evt->data.evt_gatt_procedure_completed.connection,
                                                       connProperties[0].buttonCharacteristicHandle,
                                                       gatt_indication);
          gecko_cmd_sm_increase_security(value);

      }
      connState = Indication;
      break;
      }


      break;

    // This event is generated when a connection is dropped
    case gecko_evt_le_connection_closed_id:
    {
    	LOG_INFO("\nconnection_closed\n");
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
    	LOG_INFO("\ngatt_characteristic_value\n");


    	if(connState == Indication)
    	{
      //evt->data.evt_gatt_characteristic_value
	  if(evt->data.evt_gatt_characteristic.characteristic==gattdb_temperature_measurement)
	  {
		  charValue = &(evt->data.evt_gatt_characteristic_value.value.data[0]);
		  connProperties[0].temperature = (charValue[1] << 0) + (charValue[2] << 8) + (charValue[3] << 16);
		  displayPrintf(DISPLAY_ROW_TEMPVALUE,"%f",(float)(connProperties[0].temperature)/1000);
		  LOG_INFO("%f",(float)(connProperties[0].temperature)/1000);
		  temperature_flag=0;
	  }
      if(evt->data.evt_gatt_characteristic.characteristic==gattdb_button_state)
      {
//    	  if(evt->data.evt_gatt_characteristic_value.value.data[0]==0)
//    	  	{
    	  	 displayPrintf(DISPLAY_ROW_ACTION ,"Button pressed");
//    	  	}
      }
      else
      {
    	  displayPrintf(DISPLAY_ROW_ACTION ,"Button released");
      }


      // Send confirmation for the indication
      gecko_cmd_gatt_send_characteristic_confirmation(evt->data.evt_gatt_characteristic_value.connection);
      // Trigger RSSI measurement on the connection
    	}
      break;



    default:
      break;
  }
}
