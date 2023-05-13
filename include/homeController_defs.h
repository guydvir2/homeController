#define TOT_Relays 8
#define TOT_Inputs 12
#define MAX_TOPIC_SIZE 40 // <----- Verfy max Topic size

struct Cotroller_Ent_telemetry
{
    uint8_t id;           /* of entity instance*/
    uint8_t type;         /* Entiry type 0- win. 1 sw */
    uint8_t state;        /* Up/Down/ Off */
    uint8_t trig;         /* What triggered the button */
    float position = 200; /* For windows only */
    bool newMSG = false;  /* NewMSG for loop function */
    unsigned long timeout = 0;
};
enum ENT_TYPE : const uint8_t
{
    WIN_ENT,
    SW_ENT
};