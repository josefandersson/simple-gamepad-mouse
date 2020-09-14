#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/joystick.h>
#include <linux/uinput.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <pthread.h>

#define DEVICE_JS0 "/dev/input/js0"
#define DEVICE_UINPUT "/dev/uinput"

#define DELAY 1000000/60
#define JT_THRES 10000
#define SPEED_MIN 7.0
#define SPEED_MAX 20.0

int emit(unsigned short type, unsigned short code, int value);
double joystick_to_mouse(int joystick_value);
void loop();
void *loop_mouse(void *arg);
void morse_reset();
void morse_input(int code); // 0=short, 1=long
void morse_write();
void send_key(unsigned short code, int value, int sync);
int setup();

struct js_event js_ev;
struct uinput_user_dev uinp;
struct input_event inp_ev;

static int js_fd;
static int uinp_fd;

static double mouse_x, mouse_y, mouse_multi = SPEED_MAX;
static int scroll_x, scroll_y;

static int morse_mode;
static unsigned char morse_index;
static unsigned char morse_sequence;

int emit(unsigned short type, unsigned short code, int value) {
    memset(&inp_ev, 0, sizeof(inp_ev));
    gettimeofday(&inp_ev.time, NULL);
    inp_ev.type = type;
    inp_ev.code = code;
    inp_ev.value = value;
    return write(uinp_fd, &inp_ev, sizeof(inp_ev));
}

double joystick_to_mouse(int joystick_value) {
    return !(-JT_THRES < joystick_value && joystick_value < JT_THRES) * (joystick_value / 32768.0);
}

void loop() {
    while (1) {
        if (read(js_fd, &js_ev, sizeof(struct js_event)) < 0) {
            printf("Failed to read joystick event: %s\n", strerror(errno));
            if (errno == EINTR)
                continue;
            break;
        }

        if (js_ev.type == JS_EVENT_BUTTON) {
            // printf("BTN   Event: time:%d, value:%d, type:%d, number:%d\n", js_ev.time, js_ev.value, js_ev.type, js_ev.number);
            switch (js_ev.number) {
                case 0: // A
                    if (morse_mode) {
                        if (js_ev.value)
                            morse_input(0);
                    } else {
                        send_key(BTN_LEFT, js_ev.value, 1);
                    }
                    break; 
                case 1: // B
                    if (morse_mode) {
                        if(js_ev.value)
                            morse_input(1);
                    } else {
                        send_key(BTN_RIGHT, js_ev.value, 1);
                    }
                    break; 
                case 2: // X
                    if (morse_mode) {
                        if (js_ev.value)
                            morse_reset();
                    } else {
                        send_key(KEY_BACKSPACE, js_ev.value, 1);
                    }
                    break;
                case 3: // Y
                    if (morse_mode) {
                        if (js_ev.value) {
                            if (morse_index == 0) {
                                send_key(KEY_SPACE, 1, 1);
                                send_key(KEY_SPACE, 0, 1);
                            } else
                                morse_write();
                        }
                    } else {
                        send_key(KEY_SPACE, js_ev.value, 1);
                    }
                    break;
                case 4: break; // left btn
                case 5: // right btn
                    if (js_ev.value == 1) {
                        morse_mode = 1;
                    } else {
                        morse_mode = 0;
                        morse_reset();
                    }
                    break;
                case 6: break; // back
                case 7: send_key(KEY_ENTER, js_ev.value, 1); break; // start
                case 8: break; // home
                case 9: send_key(KEY_ESC, js_ev.value, 1); break; // left joystick
                case 10: send_key(BTN_MIDDLE, js_ev.value, 1); break; // right joystick
                case 11: send_key(KEY_LEFT, js_ev.value, 1); break;   // left arw
                case 12: send_key(KEY_RIGHT, js_ev.value, 1); break;  // right arw
                case 13: send_key(KEY_UP, js_ev.value, 1); break;     // up arw
                case 14: send_key(KEY_DOWN, js_ev.value, 1); break;   // down arw
            }
        } else if (js_ev.type == JS_EVENT_AXIS) {
            switch (js_ev.number) {
                // LEFT JOYSTICK
                case 0: mouse_x = joystick_to_mouse(js_ev.value); break;
                case 1: mouse_y = joystick_to_mouse(js_ev.value); break;
                
                // LEFT BUMPER
                case 2: mouse_multi = (1 - (js_ev.value + 32768) / 65536.0) * (SPEED_MAX - SPEED_MIN) + SPEED_MIN; break;
                
                // RIGHT JOYSTICK
                case 3: scroll_x = joystick_to_mouse(js_ev.value) * 2; break;
                case 4: scroll_y = -joystick_to_mouse(js_ev.value) * 2; break;

                // RIGHT BUMPER
                case 5:
                    break;

                // ARROW AXISES
                case 6:
                case 7:
                    break;
            }
        }

        usleep(10);
    }
}

void *loop_mouse(void *arg) {
    int scroll_x_wait = 0, scroll_y_wait = 0;
    while (1) {
        if (mouse_x != 0)
            emit(EV_REL, REL_X, mouse_x * mouse_multi);
        if (mouse_y != 0)
            emit(EV_REL, REL_Y, mouse_y * mouse_multi);

        if (scroll_x != 0) {
            if (--scroll_x_wait < 0) {
                emit(EV_REL, REL_HWHEEL, scroll_x);
                scroll_x_wait = 4 * (1 - mouse_multi / SPEED_MAX) + 1;
            }
        } else scroll_x_wait = 0;

        if (scroll_y != 0) {
            if (--scroll_y_wait < 0) {
                emit(EV_REL, REL_WHEEL, scroll_y);
                scroll_y_wait = 4 * (1 - mouse_multi / SPEED_MAX) + 1;
            }
        } else scroll_y_wait = 0;

        if (mouse_x != 0 || mouse_y != 0 || scroll_y != 0 || scroll_x != 0)
            emit(EV_SYN, SYN_REPORT, 0);
        usleep(DELAY);
    }
    return NULL;
}


int main() {
    if (setup() < 0)
        return -1;

    pthread_t thid;
    if (pthread_create(&thid, NULL, loop_mouse, NULL) != 0) {
        printf("Failed to create thread\n");
        return -1;
    }

    loop();
    
    ioctl(uinp_fd, UI_DEV_DESTROY);

    close(js_fd);
    close(uinp_fd);

    return 0;
}

void morse_input(int code) {
    morse_index++;
    morse_sequence <<= 1;
    morse_sequence |= code;
    if (morse_index > 4)
        morse_write();
}

void morse_reset() {
    morse_index = 0;
    morse_sequence = 0;
}

void morse_write() {
    unsigned char character = 0;
    switch (morse_index) {
        case 1:
            switch (morse_sequence) {
                case 0: character = 'e'; break;
                case 1: character = 't'; break;
            }
            break;
        case 2:
            switch (morse_sequence) {
                case 0: character = 'i'; break;
                case 1: character = 'a'; break;
                case 2: character = 'n'; break;
                case 3: character = 'm'; break;
            }
            break;
        case 3:
            switch (morse_sequence) {
                case 0: character = 's'; break;
                case 1: character = 'u'; break;
                case 2: character = 'r'; break;
                case 3: character = 'w'; break;
                case 4: character = 'd'; break;
                case 5: character = 'k'; break;
                case 6: character = 'g'; break;
                case 7: character = 'o'; break;
            }
            break;
        case 4:
            switch (morse_sequence) {
                case 0: character = 'h'; break;
                case 1: character = 'v'; break;
                case 2: character = 'f'; break;
                case 4: character = 'l'; break;
                case 6: character = 'p'; break;
                case 7: character = 'j'; break;
                case 8: character = 'b'; break;
                case 9: character = 'x'; break;
                case 10: character = 'c'; break;
                case 11: character = 'y'; break;
                case 12: character = 'z'; break;
                case 13: character = 'q'; break;
            }
            break;
    }
    printf("Morse code: %d gave character %c\n", morse_sequence, character);
    morse_reset();
}

void send_key(unsigned short code, int value, int sync) {
    emit(EV_KEY, code, value);
    if (value == 0 || sync)
        emit(EV_SYN, SYN_REPORT, 0);
}

int setup() {
    js_fd = open(DEVICE_JS0, O_RDONLY);
    if (js_fd < 0) {
        printf("Failed to open %s\n", DEVICE_JS0);
        return -1;
    }

    uinp_fd = open(DEVICE_UINPUT, O_WRONLY | O_NDELAY);
    if (uinp_fd < 0) {
        printf("Failed to open %s\n", DEVICE_UINPUT);
        return -2;
    }

    memset(&uinp, 0, sizeof(uinp));
    strncpy(uinp.name, "Simple Gamepad Mouse", UINPUT_MAX_NAME_SIZE);
    uinp.id.version = 4;
    uinp.id.bustype = BUS_VIRTUAL;

    ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
    ioctl(uinp_fd, UI_SET_EVBIT, EV_REL);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_X);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_Y);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_WHEEL);
    ioctl(uinp_fd, UI_SET_RELBIT, REL_HWHEEL);

    for (int i = 0; i < 256; i++) {
        ioctl(uinp_fd, UI_SET_KEYBIT, i);
    }

    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MIDDLE);
    ioctl(uinp_fd, UI_SET_KEYBIT, BTN_RIGHT);

    write(uinp_fd, &uinp, sizeof(uinp));
    if (ioctl(uinp_fd, UI_DEV_CREATE)) {
        printf("Unable to create UINPUT device.");
        return -1;
    }

    return 0;
}