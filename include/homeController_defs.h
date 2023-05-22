#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

struct Cotroller_Ent_telemetry
{
    // Common fields //
    uint8_t id;                  /* of entity instance*/
    uint8_t type;                /* Entiry type 0- win. 1 sw */
    uint8_t state;               /* Up/Down/ Off */
    uint8_t trig;                /* What triggered the button */
    bool newMSG = false;         /* NewMSG for loop function */
    bool lockdown_state = false; /* For windows only */

    // Switch only fields //
    uint8_t pwm = 255;         /* PWM % */
    uint8_t pressCount = 0;    // multi_press switch
    unsigned long timeout = 0; // Timeout end time

    // Window only fields //
    float position = 200; /* Open position 0-100 */
};
enum ENT_TYPE : const uint8_t
{
    WIN_ENT,
    SW_ENT
};
