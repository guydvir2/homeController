#include "homeController.h"

homeCtl::homeCtl(bool use_debug)
{
  useDebug = use_debug;
}

bool homeCtl::loop()
{
  _SW_loop();
  _Win_loop();

  if (_use_RF)
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
void homeCtl::create_Win(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, bool is_virtual, bool use_ext_sw)
{
  winSW_V[_winEntityCounter] = new WinSW(useDebug);
  winSW_V[_winEntityCounter]->set_input(_input_pins[_inIOCounter], _input_pins[_inIOCounter + 1]);
  winSW_V[_winEntityCounter]->set_name(topic);

  // <<<<<<<<<<< Define input and output pins >>>>>>>>>>>>>>
  if (is_virtual) /* a virtCMD on output */
  {
    winSW_V[_winEntityCounter]->set_output(); /* empty definition --> virtCMD */
  }
  else /* Physical Switching input & output */
  {
    winSW_V[_winEntityCounter]->set_output(_output_pins[_outIOCounter], _output_pins[_outIOCounter + 1]);
    _outIOCounter += 2;
  }

  // <<<<<<<<<<< Define Ext_input pins , if needed >>>>>>>>>>>>>>
  if (use_ext_sw) /* define a Secondary input for a window */
  {
    winSW_V[_winEntityCounter]->set_ext_input(_input_pins[_inIOCounter + 2], _input_pins[_inIOCounter + 3]);
    _inIOCounter += 2;
  }

  // <<<<<<<<<<< Init instance  >>>>>>>>>>>>>>
  winSW_V[_winEntityCounter]->set_extras(); /* Timeout & lockdown */
  winSW_V[_winEntityCounter]->print_preferences();

  // <<<<<<<<< Incrementing Counters >>>>>>>>>>
  _winEntityCounter++;
  _inIOCounter += 2;
}
void homeCtl::create_SW(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, uint8_t sw_type, bool is_virtual, int timeout_m, uint8_t RF_ch)
{
  SW_v[_swEntityCounter] = new smartSwitch(useDebug);
  SW_v[_swEntityCounter]->set_name(topic);
  SW_v[_swEntityCounter]->set_input(_input_pins[_inIOCounter], sw_type); /* input is an option */
  SW_v[_swEntityCounter]->set_id(_swEntityCounter);
  SW_v[_swEntityCounter]->set_timeout(timeout_m);

  Serial.print("INPIN: ");
  Serial.println(_input_pins[_inIOCounter]);

  /* Phsycal or Virtual output ?*/
  if (!is_virtual)
  {
    SW_v[_swEntityCounter]->set_output(_output_pins[_outIOCounter]);
    _outIOCounter++;
  }
  else
  {
    SW_v[_swEntityCounter]->set_output();
  }

  /* Assign RF to SW */
  if (_RF_freq[RF_ch] != 255) /* Make sure freq is valid */
  {
    _RF_ch_2_SW[_swEntityCounter] = RF_ch; /* Which _RF_Chanel# goes to SW */
    _init_RF();
  }
  // SW_v[_swEntityCounter]->useDebug = useDebug;
  SW_v[_swEntityCounter]->print_preferences();
  _inIOCounter++;
  _swEntityCounter++;
}
bool homeCtl::get_useRF()
{
  return _use_RF;
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
    return _winEntityCounter;
  }
  else if (type == SW_ENT)
  {
    return _swEntityCounter;
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
    _RF_freq[i] = arr[i];
  }
}
void homeCtl::set_RF(uint8_t pin)
{
  _RFpin = pin;
}

void homeCtl::Win_init_lockdown()
{
  for (uint8_t i = 0; i < _winEntityCounter; i++)
  {
    winSW_V[i]->init_lockdown();
  }
}
void homeCtl::Win_release_lockdown()
{
  for (uint8_t i = 0; i < _winEntityCounter; i++)
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

void homeCtl::_SW_newMSG(uint8_t i)
{
  _MSG.id = i;
  _MSG.type = SW_ENT;
  _MSG.newMSG = true;
  _MSG.state = SW_v[i]->telemtryMSG.state;
  _MSG.trig = SW_v[i]->telemtryMSG.reason;
  _MSG.timeout = SW_v[i]->telemtryMSG.clk_end;
}
void homeCtl::_Win_newMSG(uint8_t i)
{
  _MSG.id = i;
  _MSG.type = WIN_ENT;
  _MSG.newMSG = true;
  _MSG.state = winSW_V[i]->MSG.state;
  _MSG.trig = winSW_V[i]->MSG.reason;
  _MSG.timeout = 0;
}

void homeCtl::_SW_loop()
{
  for (uint8_t i = 0; i < _swEntityCounter; i++)
  {
    if (SW_v[i]->loop())
    {
      _SW_newMSG(i);
    }
  }
}
void homeCtl::_Win_loop()
{
  for (uint8_t x = 0; x < _winEntityCounter; x++)
  {
    if (winSW_V[x]->loop())
    {
      _Win_newMSG(x);
    }
  }
}
void homeCtl::_RF_loop()
{
  if (RF_v->available()) /* New transmission */
  {
    static unsigned long lastEntry = 0;

    for (uint8_t i = 0; i < sizeof(_RF_freq) / sizeof(_RF_freq[0]); i++)
    {
      if (_RF_freq[i] == RF_v->getReceivedValue() && millis() - lastEntry > 300)
      {
        for (uint8_t x = 0; x < _swEntityCounter; x++) /* choose the right switch to the received code */
        {
          if (_RF_ch_2_SW[x] == i)
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
  if (_RF_ch_2_SW[_swEntityCounter] != 255 && RF_v == nullptr && _RFpin != 255)
  {
    _use_RF = true;
    RF_v = new RCSwitch();
    RF_v->enableReceive(_RFpin);
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
