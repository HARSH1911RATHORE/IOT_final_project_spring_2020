#include "Server.h"
#include "Client.h"

int button=0;						//button state

/**
 * bluetooth stack function to handle the specific commands and events
 *
 * @param evt which will be passed from gecko external signal
 * @return void
 */
void gecko_ecen5823_update(struct gecko_cmd_packet* evt)
{
	displayPrintf(DISPLAY_ROW_NAME,"SERVER");
    switch (BGLIB_MSG_ID(evt->header)) {
      /* This boot event is generated when the system boots up after reset.
       * Do not call any stack commands before receiving the boot event.
       * Here the system is set to start advertising immediately after boot procedure. */
      case gecko_evt_system_boot_id:
        /* Set advertising parameters. 100ms advertisement interval.
         * The first two parameters are minimum and maximum advertising interval, both in
         * units of (milliseconds * 1.6). */
      	gecko_cmd_sm_delete_bondings();												//delete previous bonding
  	 	gecko_cmd_sm_configure(7,1);												//configure security and i/o requirements
  	 	gecko_cmd_sm_set_bondable_mode(1);											//set bonding new connections to true
    	displayPrintf(DISPLAY_ROW_CONNECTION,"ADVERTISING");
        gecko_cmd_le_gap_set_advertise_timing(handle, advertsing_interval_min, advertsing_interval_max, duration, maxevents);	//advertising

        /* Start general advertising and enable connections. */
        gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);
        break;

      case gecko_evt_le_connection_opened_id:															//bluetooth connection open
      {
    	  LOG_DEBUG("OPEN\n");
    	  conn_open_id=1;																				//connection id 1 then only i2c transfer event is started
    	  letimerInit();																				//all letimer i2c are started once connection  is open
    	  Gpio_sensor_enable();
    	  init_i2c();

    	  gecko_cmd_hardware_set_soft_timer(32768, 0, 0);												//set one second soft timer
    	  val=evt->data.evt_le_connection_opened.connection;											//current connection handle
    	  gecko_cmd_le_connection_set_parameters(val, connection_int_min, connection_int_max, slave_latency, timeout); //connection parameters
    	  displayPrintf(DISPLAY_ROW_CONNECTION,"CONNECTED");
    	  struct gecko_msg_system_get_bt_address_rsp_t* address=gecko_cmd_system_get_bt_address();		//get bt address of server
    	  displayPrintf(DISPLAY_ROW_NAME,"SERVER");
    	  char buffer[18];
    	  sprintf(buffer,"%s",(char *)address);

    	  displayPrintf(DISPLAY_ROW_BTADDR,"%x:%x:%x:%x:%x:%x", buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5]);    //print address on lcd
    	  break;


      }

	  case gecko_evt_sm_confirm_bonding_id:								//confirm bonding when pairing is being done and confirmation with button
	 		LOG_INFO("confirm_bonding\n");
	 		gecko_cmd_sm_bonding_confirm(val,1);						//command bonding confirm
	    	break;

	  case gecko_evt_sm_confirm_passkey_id:								//confirm the passkey when read of characteristic is pressed on mobile app
	 		LOG_INFO("confirm_passkey\n");
	 		uint32 pass=evt->data.evt_sm_passkey_display.passkey;		//get passkey displayed on mobile app
	 		displayPrintf(DISPLAY_ROW_PASSKEY,"PASSKEY %d",pass);		//print the passkey displayed on mobile app
	 		displayPrintf(DISPLAY_ROW_ACTION,"Confirm with PB0");
          	break;

	  case gecko_evt_sm_bonded_id:										//triggered when bonding is complete
	 		 LOG_INFO("bonding_successful_bonding\n");
	 		 displayPrintf(DISPLAY_ROW_PASSKEY,"");						//clear the lcd passkey row and row action
	 		 displayPrintf(DISPLAY_ROW_ACTION,"");
	 		 displayPrintf(DISPLAY_ROW_CONNECTION,"Bonded");			//print bonded on lcd
	 		 break;
		      /* This event is generated when a connected client has either
		       * 1) changed a Characteristic Client Configuration, meaning that they have enabled
		       * or disabled Notifications or Indications, or
		       * 2) sent a confirmation upon a successful reception of the indication. */
	  case gecko_evt_gatt_server_characteristic_status_id:
		  	  LOG_INFO("gatt_server_characteristic_status\n");
		        /* Check that the characteristic in question is temperature - its ID is defined
		         * in gatt.xml as "temperature_measurement". Also check that status_flags = 1, meaning that
		         * the characteristic client configuration was changed (notifications or indications
		         * enabled or disabled). */
		       if ((evt->data.evt_gatt_server_characteristic_status.characteristic == gattdb_button_state)
		            && (evt->data.evt_gatt_server_characteristic_status.status_flags == 0x01))
		       {
		          if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x02)
		          {
		            /* Indications have been turned ON - start the repeating timer. The 1st parameter '32768'
		             * tells the timer to run for 1 second (32.768 kHz oscillator), the 2nd parameter is
		             * the timer handle and the 3rd parameter '0' tells the timer to repeat continuously until
		             * stopped manually.*/
		            gecko_cmd_hardware_set_soft_timer(32768, 0, 0);
		          } else if (evt->data.evt_gatt_server_characteristic_status.client_config_flags == 0x00) {
		            /* Indications have been turned OFF - stop the timer. */
		            gecko_cmd_hardware_set_soft_timer(0, 0, 0);
		          }
		        }
		        break;

		      /* This event is generated when the software timer has ticked. In this example the temperature
		       * is read after every 1 second and then the indication of that is sent to the listening client. */
	  case gecko_evt_hardware_soft_timer_id:
		  	  LOG_INFO("hardware_software_timer\n");
		     //send_characteristic_notification to the mobile app with the current button state
		  	  gecko_cmd_gatt_server_send_characteristic_notification(connection_all, gattdb_button_state, 1, (const uint8*)&button_value);


		 	  break;

      case gecko_evt_system_external_signal_id:																	//checking for external signal event
      {
	 	  if ((evt->data.evt_system_external_signal.extsignals & button) == PB0_PRESSED)						//check if external interrupt triggers because of pb0
	 	  {

	 		    button&=~PB0_PRESSED;																			//clear the button state off pb0
	 		    button_value=1;																					//button value will be 1
	 		    gecko_cmd_sm_passkey_confirm(val,1);															//confirm passkey
	 		   //send_characteristic_notification to the mobile app with the current button state
	 		    gecko_cmd_gatt_server_send_characteristic_notification(connection_all, gattdb_button_state, 1, (const uint8*)&button_value);
	 		  	LOG_INFO("External signal button pressed 111111\n");
		 		displayPrintf(DISPLAY_ROW_PASSKEY,"");															//clear the lcd
		 		displayPrintf(DISPLAY_ROW_ACTION,"");
		 		button_value=0;																					//make button value back to zero

	 	  }

    	  if ((evt->data.evt_system_external_signal.extsignals & event_bluetooth) == event_bluetooth)    		//checking if 3 second time letimer interrupt occured
    	  {
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event=true;																						//set event of underflow interrupt
				CORE_CRITICAL_IRQ_ENABLE();
    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals &event_timer_80_bluetooth)==event_timer_80_bluetooth)
    	  {
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_timer_80=true;																			//set event of 80 mili second period
				CORE_CRITICAL_IRQ_ENABLE();
    	  }

    	  if ((evt->data.evt_system_external_signal.extsignals &event_timer_10_bluetooth)==event_timer_10_bluetooth)
    	  {
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_timer_10=true;																			//set event of 10 mili second period after write
				CORE_CRITICAL_IRQ_ENABLE();
    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals &event_timer_5_bluetooth)==event_timer_5_bluetooth)
    	  {
				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();																	//set event of 5 mili second period after read
				event_timer_5=true;
				CORE_CRITICAL_IRQ_ENABLE();

    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals&event_read_bluetooth)==event_read_bluetooth)
    	  {


				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_read=true;																				//set read event
				CORE_CRITICAL_IRQ_ENABLE();

    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals &event_i2c_progress_write)==event_i2c_progress_write)
    	  {

				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_i2c_progressing_write=true;																//transfer in progress event for write
				CORE_CRITICAL_IRQ_ENABLE();




    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals &event_i2c_progress_read)==event_i2c_progress_read)
    	  {

				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();
				event_i2c_progressing_read=true;																//transfer in progress event for read
				CORE_CRITICAL_IRQ_ENABLE();

    	  }
    	  if ((evt->data.evt_system_external_signal.extsignals&event_write_bluetooth)==event_write_bluetooth)
    	  {

				CORE_DECLARE_IRQ_STATE();
				CORE_CRITICAL_IRQ_DISABLE();

				event_write=true;																				//event write i2c made true

				CORE_CRITICAL_IRQ_ENABLE();
    	  }

	   	  break;


      }
      case gecko_evt_le_connection_rssi_id:
      {
    	  int rssi;
    	  int txpower=0;
    	  rssi=evt->data.evt_le_connection_rssi.rssi;
    	  if(rssi>-85 && rssi<=-75)															//depending on the range phone is nearby, transmit power is changed
    	  {
    		  txpower=50;																	//50 transmit power

    	  }
    	  else if(rssi>-75 && rssi<=-65)
    	  {
    		  txpower=0;																	//zero  transmit power

    	  }
    	  else if(rssi>-65 && rssi<=-55)
    	  {
    		  txpower=-50;																	//fifty  transmit power

    	  }
    	  else if(rssi>-55 && rssi<=-45)
    	  {
    		  txpower=-150;																	//-150  transmit power

    	  }
    	  else if(rssi>-45 && rssi<=-35)
    	  {
    		  txpower=-200;																	//-200  transmit power

    	  }
    	  else if(rssi>-35)
    	  {
    		  txpower=-260;																	//-260  transmit power
    	  }
    	  else
    	  {
    		  txpower=80;																	//otherwise 80 transmit power
    	  }
    	  gecko_cmd_system_halt(1);
    	  gecko_cmd_system_set_tx_power(txpower);											//send the command to transmit power
    	  gecko_cmd_system_halt(0);
    	  break;

      }

      case gecko_evt_le_connection_closed_id:
      {
          /* Stop timer in case client disconnected before indications were turned off */
          gecko_cmd_hardware_set_soft_timer(32768, 0, 0);
          /* Restart advertising after client has disconnected */
          gecko_cmd_le_gap_start_advertising(0, le_gap_general_discoverable, le_gap_connectable_scannable);  //advertise even when closed connection
          displayPrintf(DISPLAY_ROW_CONNECTION,"ADVERTISING");
          conn_open_id=0;
    	  gecko_cmd_system_halt(1);
    	  gecko_cmd_system_set_tx_power(0);												//send transmit power zero
    	  gecko_cmd_system_halt(0);
    	  shut_down_i2c_letimer();														//shutdown letimer and i2c
      }
        break;
    }
}
