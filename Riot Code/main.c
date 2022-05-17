#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#include "periph/gpio.h"
#include "xtimer.h"
#include "thread.h"
#include "periph/adc.h"
#include <inttypes.h>

#include "cpu.h"
#include "board.h"
#include "periph/pwm.h"
#include "servo.h"
#include "main.h"

//mq135
#define RES ADC_RES_10BIT
#define MIN         (100U)
//servo
#define DEV         PWM_DEV(0)
#define CHANNEL     0
#define SERVO_MIN        (1000U)
#define SERVO_MAX        (2000U)

//MQTTS
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)
#define _IPV6_DEFAULT_PREFIX_LEN        (64U)
#define BROKER_PORT         1885
#define BROKER_ADDRESS      "fec0:affe::1"
#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)
#define TOPIC_IN            "topic_in"
#define TOPIC_OUT1          "alcool_level"

//to add addresses to board interface
extern int _gnrc_netif_config(int argc, char **argv);

//if box_keys=0 -> closed, otherwise -> open
static int box_keys=0;

//servo
static servo_t servo;

//MQTTS
static char stack[THREAD_STACKSIZE_DEFAULT];
static msg_t queue[8];

static emcute_sub_t subscriptions[NUMOFSUBS];
static char topics[NUMOFSUBS][TOPIC_MAXLEN];

//box button
gpio_t box_pin = GPIO_PIN(PORT_C, 8);

//buzzer
gpio_t buzzer_pin = GPIO_PIN(PORT_C, 6);

//traffic light
gpio_t red_pin = GPIO_PIN(PORT_A, 10); //D2
gpio_t yellow_pin = GPIO_PIN(PORT_B, 5); //D4 
gpio_t green_pin = GPIO_PIN(PORT_A, 6); //D12 

//HC-SR04 ultrasonic
gpio_t trigger_pin = GPIO_PIN(PORT_A, 9); //D8
gpio_t echo_pin = GPIO_PIN(PORT_A, 8); //D7

uint32_t echo_time_start;
uint32_t echo_time;

void call_back(void* arg){
	int val = gpio_read(echo_pin);
	uint32_t echo_time_stop;
    (void) arg;

	if(val){
		echo_time_start = xtimer_now_usec();
	}
    else{
		echo_time_stop = xtimer_now_usec();
		echo_time = echo_time_stop - echo_time_start;
	}
}

int distance_ultrasonic(void){ 

	uint32_t dist;
    dist=0;
    echo_time = 0;

    gpio_clear(trigger_pin);
    xtimer_usleep(20); 
    gpio_set(trigger_pin);

    xtimer_msleep(100); 

    if(echo_time > 0){
        dist = echo_time/58; //from datasheet
    }
	
    return dist;
}

//mq3 sensor
int read_mq3(void){
    int sample = 0;
    int min = 100;
    sample = adc_sample(ADC_LINE(0), RES);
    sample = (sample > min) ? sample - min : 0;
    return sample;
}

//MQTTS
static void *emcute_thread(void *arg){
    (void)arg;
    emcute_run(BROKER_PORT, "board");
    return NULL;
}

static int pub(char* topic,char* msg){
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_0;

    printf("pub with topic: %s and name %s and flags 0x%02x\n", topic, msg, (int)flags);

    /* step 1: get topic id */
    t.name = topic;
    if (emcute_reg(&t) != EMCUTE_OK) {
        puts("error: unable to obtain topic ID");
        return 1;
    }

    /* step 2: publish data */
    if (emcute_pub(&t, msg, strlen(msg), flags) != EMCUTE_OK) {
        printf("error: unable to publish data to topic '%s [%i]'\n",t.name, (int)t.id);
        return 1;
    }

    printf("Published %i bytes to topic '%s [%i]'\n", (int)strlen(msg), t.name, t.id);

    return 0;
}

static void on_pub(const emcute_topic_t *topic, void *data, size_t len){
    (void)topic;

    char *in = (char *)data;
    printf("### got publication for topic '%s' [%i] ###\n", topic->name, (int)topic->id);
    for (size_t i = 0; i < len; i++) {
        printf("%c", in[i]);
    }
    puts("");

    char msg[len+1];
    strncpy(msg, in, len);
    msg[len] = '\0';
    if (strcmp(msg, "open") == 0){
        if(box_keys==0){
            //open box keys
            servo_set(&servo, SERVO_MIN);
            box_keys=1;
            }
    }
    else if (strcmp(msg, "close") == 0){
        if(box_keys==1){
            //close box keys
            servo_set(&servo, SERVO_MAX);
            box_keys=0;
        }
    }
    puts("");

}

static int sub(char* topic){
    unsigned flags = EMCUTE_QOS_0;

    if (strlen(topic) > TOPIC_MAXLEN) {
        puts("error: topic name exceeds maximum possible size");
        return 1;
    }

    /* find empty subscription slot */
    unsigned i = 0;
    for (; (i < NUMOFSUBS) && (subscriptions[i].topic.id != 0); i++) {}
    if (i == NUMOFSUBS) {
        puts("error: no memory to store new subscriptions");
        return 1;
    }

    subscriptions[i].cb = on_pub;
    strcpy(topics[i], topic);
    subscriptions[i].topic.name = topics[i];
    if (emcute_sub(&subscriptions[i], flags) != EMCUTE_OK) {
        printf("error: unable to subscribe to %s\n", topic);
        return 1;
    }

    printf("Now subscribed to %s\n", topic);
    

    return 0;
}

static int con(void){
    sock_udp_ep_t gw = { .family = AF_INET6, .port = BROKER_PORT };
    char *topic = NULL;
    char *message = NULL;
    size_t len = 0;

    ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, BROKER_ADDRESS);
    
    if (emcute_con(&gw, true, topic, message, len, 0) != EMCUTE_OK) {
        printf("error: unable to connect to [%s]:%i\n", BROKER_ADDRESS, (int)gw.port);
        return 1;
    }
    printf("Successfully connected to gateway at [%s]:%i\n", BROKER_ADDRESS, (int)gw.port);

    return 0;
}


static int add_address(char* addr){
    char * arg[] = {"ifconfig", "4", "add", addr};
    return _gnrc_netif_config(4, arg);
}

//initializes the connection with the MQTT-SN broker
void mqtts_init(void){
    /* the main thread needs a msg queue to be able to run `ping`*/
    msg_init_queue(queue, ARRAY_SIZE(queue));

    /* initialize our subscription buffers */
    memset(subscriptions, 0, (NUMOFSUBS * sizeof(emcute_sub_t)));

    /* start the emcute thread */
    thread_create(stack, sizeof(stack), EMCUTE_PRIO, 0, emcute_thread, NULL, "emcute");

    char * addr1 = "fec0:affe::99";
    add_address(addr1);

    con();
    sub(TOPIC_IN);
}


void sensor_init(void){
    //ultrasonic
    gpio_init(trigger_pin, GPIO_OUT);
    gpio_init_int(echo_pin, GPIO_IN, GPIO_BOTH, &call_back, NULL);
    distance_ultrasonic(); //first read returns always 0

    //mq3
    adc_init(ADC_LINE(0));

    //traffic light
    gpio_init(red_pin, GPIO_OUT);
    gpio_init(yellow_pin, GPIO_OUT);
    gpio_init(green_pin, GPIO_OUT);

    // button box keys
    gpio_init(box_pin,GPIO_IN);

    // buzzer
    gpio_init(buzzer_pin,GPIO_OUT);

    //servo init
    servo_init(&servo, DEV, CHANNEL, SERVO_MIN, SERVO_MAX);
    servo_set(&servo, SERVO_MAX);
}


//check the alcool level through the mq3 sensor
void check_alcool(void){

    int sample = 0;
    char msg[4];

    sample=read_mq3();
    
    sprintf(msg, "%d", sample);

    if (sample > 450) {
        printf("Alert: value = %i\n", sample);
        gpio_set(buzzer_pin);
        xtimer_sleep(1);
        gpio_clear(buzzer_pin);
    } else {
        printf("Normal: value = %i\n", sample);
        //open box keys
        servo_set(&servo, SERVO_MIN);
        box_keys=1;
    }
    
    pub(TOPIC_OUT1,msg); //pub level of alcohol when measured so it can be read on the web site

}

//set led traffic light
void set_led(char *str){
    if(strcmp(str,"verde")==0){
        gpio_clear(red_pin);
        gpio_clear(yellow_pin);
        gpio_set(green_pin);
    }
    else if(strcmp(str,"rosso")==0){
        gpio_clear(yellow_pin);
        gpio_clear(green_pin);
        gpio_set(red_pin);
    }
    else if(strcmp(str,"giallo")==0){
        gpio_clear(red_pin);
        gpio_clear(green_pin);
        gpio_set(yellow_pin);
    }

}


int main(void){    
    
    uint32_t dist;
    int result;
    sensor_init();
    mqtts_init();
	
	while(true){
        if(box_keys==0){
            dist=distance_ultrasonic();
            printf("distance_ultrasonic =%" PRIu32 "\n", dist);
            if(dist<5){
                set_led("verde");
                check_alcool();    
            }
            else if(dist>=5 && dist<15){
                set_led("giallo");
                printf("more closed!\n");
            }
            else{
                printf("you must to be more closed to the sensors!\n");
                set_led("rosso");

            }
        }
        else{
            while(box_keys==1){
                result = gpio_read(box_pin);
                if(result>0){
                    box_keys=0;
                    //close box keys
                    servo_set(&servo, SERVO_MAX);
                }
                
                xtimer_sleep(0.5);
            }
        }

        xtimer_sleep(5);
	}
    return 0;
}
