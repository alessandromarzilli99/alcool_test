#ifndef MAIN_FUNCTIONS
#define MAIN_FUNCTIONS

void call_back(void* arg);
int distance_ultrasonic(void);
int read_mq135(void);
static void *emcute_thread(void *arg);
static int pub(char* topic,char* msg);
static void on_pub(const emcute_topic_t *topic, void *data, size_t len);
static int sub(char* topic);
static int con(void);
static int add_address(char* addr);
void mqtts_init(void);
void sensor_init(void);
void check_alcool(void);
void set_led(char *str);

#endif // MAIN_FUNCTIONS