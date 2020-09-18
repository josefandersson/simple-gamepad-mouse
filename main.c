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

#include "def.c"

#define LENGTH(X) (sizeof X / sizeof X[0])

typedef union {
    unsigned short us;
} Arg;

typedef struct {
    unsigned char number;
    unsigned char mask;
    void (*func)(const Arg *);
    const Arg arg;
} ButtonRule;

typedef struct {
    unsigned char number;
    unsigned char modifier;
    unsigned char switches;
    void (*func)(const Arg *);
    const Arg arg;
} AxisRule;

int emit(unsigned short type, unsigned short code, int value);
double joystick_to_mouse(int joystick_value);
void loop();
void *loop_mouse(void *arg);
static void morse_reset_or_backspace(const Arg *arg);
static void morse_input(const Arg *arg);
static void morse_write_or_space(const Arg *arg);
static void mouse_movement(const Arg *arg);
static void mouse_multiplier(const Arg *arg);
static void mouse_scroll(const Arg *arg);
static void morse_write();
static void send_key(const Arg *arg);
static int setup();

#include "config.h"

struct js_event js_ev;
struct uinput_user_dev uinp;
struct input_event inp_ev;

static int js_fd;
static int uinp_fd;

static int button_states;

static char modifiers;
static double mouse_x, mouse_y, mouse_multi;
static int scroll_x, scroll_y;
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
    return !(-deadzone < joystick_value && joystick_value < deadzone) * (joystick_value / 32768.0);
}

static void mouse_movement(const Arg *arg) {
    if (arg->us == X) mouse_x = joystick_to_mouse(js_ev.value);
    else              mouse_y = joystick_to_mouse(js_ev.value);
}

static void mouse_multiplier(const Arg *arg) {
    mouse_multi = (1 - (js_ev.value + 32768) / 65536.0) * (mouse_fast - mouse_slow) + mouse_slow;
}

static void mouse_scroll(const Arg *arg) {
    if (arg->us == X) scroll_x = joystick_to_mouse(js_ev.value) * 2;
    else              scroll_y = -joystick_to_mouse(js_ev.value) * 2;
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
            for (int i = 0; i < LENGTH(btnRules); i++) {
                if(btnRules[i].number == js_ev.number && (btnRules[i].mask & button_states) == btnRules[i].mask) {
                    btnRules[i].func(&btnRules[i].arg);
                    // printf("Button %d matches, mask:%d\n", js_ev.number, btnRules[i].mask);
                    break;
                }
            }
            button_states = js_ev.value ?
                button_states | GPB_TO_MOD(js_ev.number) :
                button_states & ~GPB_TO_MOD(js_ev.number);
        } else if (js_ev.type == JS_EVENT_AXIS) {
            for (int i = 0; i < LENGTH(axisRules); i++) {
                AxisRule rule = axisRules[i];
                if (rule.number == js_ev.number)
                    rule.func(&rule.arg);
            }
            // button_states = js_ev.value + 32768 > JT_THRES ? button_states | GPB_TO_MOD(js_ev.number) : button_states & ~GPB_TO_MOD(js_ev.number);
        }

        usleep(10);
    }
}

void *loop_mouse(void *arg) {
    int scroll_x_wait = 0, scroll_y_wait = 0;
    while (1) {
        if (mouse_x != 0) emit(EV_REL, REL_X, mouse_x * mouse_multi);
        if (mouse_y != 0) emit(EV_REL, REL_Y, mouse_y * mouse_multi);

        if (scroll_x != 0) {
            if (--scroll_x_wait < 0) {
                emit(EV_REL, REL_HWHEEL, scroll_x);
                scroll_x_wait = 4 * (1 - mouse_multi / mouse_fast) + 1;
            }
        } else scroll_x_wait = 0;

        if (scroll_y != 0) {
            if (--scroll_y_wait < 0) {
                emit(EV_REL, REL_WHEEL, scroll_y);
                scroll_y_wait = 4 * (1 - mouse_multi / mouse_fast) + 1;
            }
        } else scroll_y_wait = 0;

        if (mouse_x != 0 || mouse_y != 0 || scroll_y != 0 || scroll_x != 0)
            emit(EV_SYN, SYN_REPORT, 0);
        usleep(loop_delay);
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

    mouse_multi = mouse_fast;
    loop();
    
    ioctl(uinp_fd, UI_DEV_DESTROY);

    close(js_fd);
    close(uinp_fd);

    return 0;
}

static void morse_input(const Arg *arg) {
    if (!js_ev.value)
        return;
    morse_index++;
    morse_sequence = (morse_sequence << 1) | (arg->us == MORSE_LONG);
    if (morse_index > 4)
        morse_write();
}

static void morse_reset() {
    morse_index = 0;
    morse_sequence = 0;
}

static void morse_reset_or_backspace(const Arg *arg) {
    if (!js_ev.value)
        return;
    if (morse_index == 0) {
        Arg narg = { .us=KEY_BACKSPACE };
        send_key(&narg);
    } else {
        morse_reset();
    }
}

static void morse_write_or_space(const Arg *arg) {
    if (!js_ev.value)
        return;
    if (arg->us != 0)
        send_key(arg);
    if (morse_index == 0) {
        Arg narg = { .us = KEY_SPACE };
        send_key(&narg);
    } else {
        morse_write();
    }
    if (arg->us != 0) {
        js_ev.value = 0;
        send_key(arg);
    }
}

static void morse_write() {
    int row_offset = 0;
    for (int row = morse_index - 1;row != 0;row--) row_offset += 1 << row;
    unsigned short key = morse_tree[row_offset + morse_sequence];
    if (key != 0) {
        signed short prev = js_ev.value;
        Arg narg = { .us=key };
        js_ev.value = 1;
        send_key(&narg);
        Arg marg = { .us=key };
        js_ev.value = 0;
        send_key(&marg);
        js_ev.value = prev;
    }
    morse_reset();
}

static void send_key(const Arg *arg) {
    emit(EV_KEY, arg->us, js_ev.value);
    emit(EV_SYN, SYN_REPORT, 0);
}

static int setup() {
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

    for (int i = 0; i < 256; i++)
        ioctl(uinp_fd, UI_SET_KEYBIT, i);

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