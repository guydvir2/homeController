#include "homeController.h"

homeCtl::homeCtl(bool use_debug)
{
  useDebug = use_debug;
}

bool homeCtl::loop()
{
  _SW_loop();
  _Win_loop();

  if (RFdefs.useRF)
  {
    _RF_loop();
  }
  return _MSG.newMSG;
}
void homeCtl::Win_switchCB(uint8_t i, uint8_t state)
{
  winSW_V[i]->set_WINstate(state, MQTT);
#if RETAINED_MSG
  MQTT_clear_retained(winSW_V[i]->name);
#endif
}
void homeCtl::Win_switchCB(uint8_t i, float position)
{
  winSW_V[i]->set_Win_position(position);
  /* need to update state */
}
void homeCtl::SW_switchCB(uint8_t i, uint8_t state, unsigned int TO)
{
  if (state == 1) /* ON */
  {
    SW_v[i]->turnON_cb(EXT_0, TO);
  }
  else if (state == 0) /* OFF */
  {
    SW_v[i]->turnOFF_cb(EXT_0);
  }
#if RETAINED_MSG
  MQTT_clear_retained(SW_v[i]->name);
#endif
}
void homeCtl::create_Win(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, bool is_virtual, bool use_ext_sw,
                         float to_to_up, float time_to_down, float stick_time, float end_move_time)
{
  winSW_V[entCounter.win] = new WinSW(useDebug);
  winSW_V[entCounter.win]->set_input(_input_pins[ioCounter.inputs], _input_pins[ioCounter.inputs + 1]);
  winSW_V[entCounter.win]->set_name(topic);

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (is_virtual) /* a virtCMD on output */
  {
    winSW_V[entCounter.win]->set_output(); /* empty definition --> virtCMD */
  }
  else /* Physical Switching input & output */
  {
    winSW_V[entCounter.win]->set_output(_output_pins[ioCounter.outputs], _output_pins[ioCounter.outputs + 1]);
    winSW_V[entCounter.win]->set_motor_properties(to_to_up, time_to_down, stick_time, end_move_time);
    ioCounter.outputs += 2;
  }

  // <<<<<<<<<<< Define Ext_input pins , if needed >>>>>>>>>>>>>>
  if (use_ext_sw)
  {
    winSW_V[entCounter.win]->set_ext_input(_input_pins[ioCounter.inputs + 2], _input_pins[ioCounter.inputs + 3]);
    ioCounter.inputs += 2;
  }

  // <<<<<<<<<<< Init instance  >>>>>>>>>>>>>>
  winSW_V[entCounter.win]->set_extras(); /* Timeout & lockdown */
  winSW_V[entCounter.win]->print_preferences();

  // <<<<<<<<< Incrementing Counters >>>>>>>>>>
  entCounter.win++;
  ioCounter.inputs += 2;
}
void homeCtl::create_SW(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, uint8_t sw_type,
                        bool is_virtual, int timeout_m, uint8_t RF_ch)
{
  SW_v[entCounter.sw] = new smartSwitch(useDebug);
  SW_v[entCounter.sw]->set_name(topic);
  SW_v[entCounter.sw]->set_input(_input_pins[ioCounter.inputs], sw_type); /* input is an option */
  SW_v[entCounter.sw]->set_id(entCounter.sw);
  SW_v[entCounter.sw]->set_timeout(timeout_m);

  /* Phsycal or Virtual output ?*/
  if (!is_virtual)
  {
    SW_v[entCounter.sw]->set_output(_output_pins[ioCounter.outputs]);
    ioCounter.outputs++;
  }
  else
  {
    SW_v[entCounter.sw]->set_output();
  }

  /* Assign RF to SW */
  if (RFdefs.freq[RF_ch] != 255) /* Make sure freq is valid */
  {
    RFdefs.ch2SW[entCounter.sw] = RF_ch; /* Which _RF_Chanel# goes to SW */
    _init_RF();
  }
  SW_v[entCounter.sw]->print_preferences();
  ioCounter.inputs++;
  entCounter.sw++;
}
bool homeCtl::get_useRF()
{
  return RFdefs.useRF;
}
const char *homeCtl::get_ent_ver(uint8_t type)
{
  if (type == WIN_ENT)
  {
    return winSW_V[0]->ver;
  }
  else if (type == SW_ENT)
  {
    return SW_v[0]->ver;
  }
  else
  {
    return "err";
  }
}
uint8_t homeCtl::get_ent_counter(uint8_t type)
{
  if (type == WIN_ENT)
  {
    return entCounter.win;
  }
  else if (type == SW_ENT)
  {
    return entCounter.sw;
  }
  else
  {
    return 0;
  }
}
uint8_t homeCtl::get_ent_state(uint8_t type, uint8_t i)
{
  if (type == WIN_ENT)
  {
    return winSW_V[i]->get_winState();
  }
  else if (type == SW_ENT)
  {
    return SW_v[i]->get_SWstate();
  }
  else
  {
    return 255;
  }
}
void homeCtl::get_telemetry(Cotroller_Ent_telemetry &M)
{
  M = _MSG;
}
void homeCtl::get_entity_prop(uint8_t ent_type, uint8_t i, SW_props &sw_prop)
{
  if (ent_type == SW_ENT)
  {
    SW_v[i]->get_SW_props(sw_prop);
  }
}
void homeCtl::get_entity_prop(uint8_t ent_type, uint8_t i, Win_props &win_prop)
{
  if (ent_type == WIN_ENT)
  {
    winSW_V[i]->get_Win_props(win_prop);
  }
}
const char *homeCtl::get_ent_name(uint8_t i, uint8_t ent_type)
{
  if (ent_type == WIN_ENT)
  {
    return winSW_V[i]->name;
  }
  else if (ent_type == SW_ENT)
  {
    return SW_v[i]->name;
  }
  else
    return "Err";
}
void homeCtl::set_ent_name(uint8_t i, uint8_t ent_type, const char *name)
{
  if (ent_type == WIN_ENT)
  {
    winSW_V[i]->set_name(name);
  }
  else if (ent_type == SW_ENT)
  {
    SW_v[i]->set_name(name);
  }
  else
  {
    return;
  }
}
void homeCtl::set_RFch(long arr[], uint8_t arr_size)
{
  for (uint8_t i = 0; i < arr_size; i++)
  {
    RFdefs.freq[i] = arr[i];
  }
}
void homeCtl::set_RF(uint8_t pin)
{
  RFdefs.pin = pin;
}

void homeCtl::Win_init_lockdown()
{
  for (uint8_t i = 0; i < entCounter.win; i++)
  {
    winSW_V[i]->init_lockdown();
  }
}
void homeCtl::Win_release_lockdown()
{
  for (uint8_t i = 0; i < entCounter.win; i++)
  {
    winSW_V[i]->release_lockdown();
  }
}
void homeCtl::SW_init_lockdown(uint8_t i)
{
  if (i != 255)
  {
    for (uint8_t n = 0; n < get_ent_counter(SW_ENT); n++)
    {
      SW_v[n]->set_lockSW();
    }
  }
  else
  {
    if (i < get_ent_counter(SW_ENT))
    {
      SW_v[i]->set_lockSW();
    }
  }
}
void homeCtl::SW_release_lockdown(uint8_t i)
{
  if (i != 255)
  {
    for (uint8_t n = 0; n < get_ent_counter(SW_ENT); n++)
    {
      SW_v[n]->set_unlockSW();
    }
  }
  else
  {
    if (i < get_ent_counter(SW_ENT))
    {
      SW_v[i]->set_unlockSW();
    }
  }
}
void homeCtl::clear_telemetryMSG()
{
  if (_MSG.type == WIN_ENT)
  {
    winSW_V[_MSG.id]->clear_newMSG();
  }
  else if (_MSG.type == SW_ENT)
  {
    SW_v[_MSG.id]->clear_newMSG();
  }

  _MSG.id = 0;
  _MSG.type = 255;
  _MSG.trig = 255;
  _MSG.state = 255;
  _MSG.timeout = 0;
  _MSG.newMSG = false;
  _MSG.lockdown_state = false;

  _MSG.pwm = 0;
  _MSG.position = 0;
  _MSG.pressCount = 0;
}
void homeCtl::_newMSG(uint8_t i, uint8_t ent)
{
  _MSG.id = i;
  _MSG.type = ent;
  _MSG.newMSG = true;
  if (ent == SW_ENT)
  {
    _MSG.state = SW_v[i]->telemtryMSG.state;
    _MSG.trig = SW_v[i]->telemtryMSG.reason;
    _MSG.timeout = SW_v[i]->telemtryMSG.clk_end;
  }
  else
  {
    _MSG.state = winSW_V[i]->MSG.state;
    _MSG.trig = winSW_V[i]->MSG.reason;
    _MSG.timeout = 0;
  }
}

void homeCtl::_SW_loop()
{
  for (uint8_t i = 0; i < entCounter.sw; i++)
  {
    if (SW_v[i]->loop())
    {
      _newMSG(i, SW_ENT);
    }
  }
}
void homeCtl::_Win_loop()
{
  for (uint8_t x = 0; x < entCounter.win; x++)
  {
    if (winSW_V[x]->loop())
    {
      _newMSG(x, WIN_ENT);
    }
  }
}
void homeCtl::_RF_loop()
{
  if (RF_v->available()) /* New transmission */
  {
    static unsigned long lastEntry = 0;

    for (uint8_t i = 0; i < sizeof(RFdefs.freq) / sizeof(RFdefs.freq[0]); i++)
    {
      if (RFdefs.freq[i] == RF_v->getReceivedValue() && millis() - lastEntry > 300)
      {
        for (uint8_t x = 0; x < entCounter.sw; x++) /* choose the right switch to the received code */
        {
          if (RFdefs.ch2SW[x] == i)
          {
            _toggle_SW_RF(x);
            lastEntry = millis();
          }
        }
      }
    }
    RF_v->resetAvailable();
  }
}

void homeCtl::_init_RF()
{
  if (RFdefs.ch2SW[entCounter.sw] != 255 && RF_v == nullptr && RFdefs.pin != 255)
  {
    RFdefs.useRF = true;
    RF_v = new RCSwitch();
    RF_v->enableReceive(RFdefs.pin);
    Serial.println(F(" >>> RF services started <<<"));
  }
}
void homeCtl::_toggle_SW_RF(uint8_t i)
{
  SW_props sw_prop;
  get_entity_prop(SW_ENT, i, sw_prop);

  if (sw_prop.virtCMD)
  {
    // if (sw_prop.type == 2) /* virtCMD + PushButton --> output state is unknown*/
    // {
    //   char top[50];
    //   sprintf(top, "%s/State", SW_v[i]->name);
    //   // iot.mqttClient.subscribe(top);
    // }
    // else
    // {
    // }
  }
  else
  {
    if (SW_v[i]->get_SWstate()) /* is output SW on ?*/
    {
      SW_v[i]->turnOFF_cb(EXT_1); /* # is RF remote indetifier */
    }
    else
    {
      SW_v[i]->turnON_cb(EXT_1);
    }
  }
}
