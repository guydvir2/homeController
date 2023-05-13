#ifndef _homeCTRL_H
#define _homeCTRL_H

#include <Arduino.h>
#include <RCSwitch.h>    /* RF Library */
#include <myWindowSW.h>  /* WinSW Entities */
#include <smartSwitch.h> /* smartSwitch Entities */

#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

struct Cotroller_Ent_telemetry
{
    uint8_t id;    /* of entity instance*/
    uint8_t type;  /* Entiry type 0- win. 1 sw */
    uint8_t state; /* Up/Down/ Off */
    uint8_t trig;  /* What triggered the button */

    bool newMSG = false; /* NewMSG for loop function */
    unsigned long timeout = 0;
};
enum ENT_TYPE : const uint8_t
{
    WIN_ENT,
    SW_ENT
};

class homeCtl
{
#define TOT_Relays 8
#define TOT_Inputs 12

private:
    bool _use_RF = false;
    uint8_t _RFpin = 27;
    uint8_t _inIOCounter = 0;
    uint8_t _outIOCounter = 0;
    uint8_t _swEntityCounter = 0;
    uint8_t _winEntityCounter = 0;
    uint8_t _RF_ch_2_SW[TOT_Relays] = {255, 255, 255, 255, 255, 255, 255, 255};
    long _RF_freq[TOT_Relays] = {3135496, 3135492, 3135490, 3135489, 255, 255, 255, 255};

public:
    const char *ver = "smartController_v0.5";

    const char *SW_MQTT_cmds[2] = {"off", "on"};
    const char *winMQTTcmds[3] = {"off", "up", "down"};
    const char *WinStates[4] = {"Off", "Up", "Down", "Err"};
    const char *WinTrigs[5] = {"Timeout", "Button", "Button2", "Lockdown", "MQTT"};
    const char *SW_Types[4] = {"Button", "Timeout", "MQTT", "Remote"};
    const char *EntTypes[2] = {"win", "sw"}; /* Prefix to address client types when using MQTT */

private:
    RCSwitch *RF_v = nullptr;
    WinSW *winSW_V[TOT_Relays / 2]{};
    smartSwitch *SW_v[TOT_Inputs]{};

    Cotroller_Ent_telemetry _MSG;

    void _init_RF();
    void _toggle_SW_RF(uint8_t i);

    void _SW_newMSG(uint8_t i);
    void _Win_newMSG(uint8_t i);

    void _RF_loop();
    void _SW_loop();
    void _Win_loop();

public:
    /* Create entity */
    homeCtl();
    bool loop();
    void set_RF(uint8_t pin = 255);                                                   /* IO that RF recv is connected to */
    void set_RFch(long arr[], uint8_t arr_size);                                      /* Radio freq. belong to a remote control */
    void set_ent_name(uint8_t i, uint8_t ent_type, const char *name);                 /* Entity Name (SW or Win) */
    void create_Win(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, /* Create Win ent */
                    bool is_virtual = false, bool use_ext_sw = false);
    void create_SW(uint8_t _input_pins[], uint8_t _output_pins[], const char *topic, /* Create SW ent */
                   uint8_t sw_type, bool is_virtual = false, int timeout_m = 1, uint8_t RF_ch = 255);

    /* Entity Information */
    bool get_useRF();                                                       /* Using RF ? */
    uint8_t get_ent_counter(uint8_t type);                                  /* Ent. counter */
    uint8_t get_ent_state(uint8_t type, uint8_t i);                         /* Ent. State */
    const char *get_ent_name(uint8_t i, uint8_t ent_type);                  /* Ent. name */
    const char *get_ent_ver(uint8_t type);                                  /* Ent. class version */
    void get_telemetry(Cotroller_Ent_telemetry &M);                         /* System telemetry. constant updating */
    void get_entity_prop(uint8_t ent_type, uint8_t i, SW_props &sw_prop);   /* SW Property */
    void get_entity_prop(uint8_t ent_type, uint8_t i, Win_props &win_prop); /* Win Property */

    /* Win & SW callbacks*/
    void Win_switchCB(uint8_t i, uint8_t state);                     /* Win Opertional CB*/
    void SW_setPosition(uint8_t i, float position);
    void SW_switchCB(uint8_t i, uint8_t state, unsigned int TO = 0); /* SW Opertional CB*/
    void Win_init_lockdown();                                        /* Win Lockdown */
    void Win_release_lockdown();                                     /* Win release Lockdown */
    void SW_init_lockdown(uint8_t i = 255);                          /* SW Lockdown */
    void SW_release_lockdown(uint8_t i = 255);                       /* SW release Lockdown */

    void clear_telemetryMSG(); /* Reset New telemetry notification*/
};

#endif
