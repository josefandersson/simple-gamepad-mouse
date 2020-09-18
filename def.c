// Gamepad Buttons
#define GPB_A 0
#define GPB_B 1
#define GPB_X 2
#define GPB_Y 3
#define GPB_LT 4
#define GPB_RT 5
#define GPB_BCK 6
#define GPB_STRT 7
#define GPB_HME 8
#define GPB_LJ 9
#define GPB_RJ 10
#define GPB_LD 11
#define GPB_RD 12
#define GPB_UD 13
#define GPB_DD 14

// Gamepad Axes
#define GPA_LJ_X 0
#define GPA_LJ_Y 1
#define GPA_LB 2
#define GPA_RJ_X 3
#define GPA_RJ_Y 4
#define GPA_RB 5
#define GPA_DP_X 6
#define GPA_DP_Y 7

// Buttons as masks
#define GPB_TO_MOD(x) 1<<x
#define MOD_A GPB_TO_MOD(GPB_A)
#define MOD_B GPB_TO_MOD(GPB_B)
#define MOD_X GPB_TO_MOD(GPB_X)
#define MOD_Y GPB_TO_MOD(GPB_Y)
#define MOD_LT GPB_TO_MOD(GPB_LT)
#define MOD_RT GPB_TO_MOD(GPB_RT)
#define MOD_BCK GPB_TO_MOD(GPB_BCK)
#define MOD_STRT GPB_TO_MOD(GPB_STRT)
#define MOD_HME GPB_TO_MOD(GPB_HME)
#define MOD_LJ GPB_TO_MOD(GPB_LJ)
#define MOD_RJ GPB_TO_MOD(GPB_RJ)
#define MOD_LD GPB_TO_MOD(GPB_LD)
#define MOD_RD GPB_TO_MOD(GPB_RD)
#define MOD_UD GPB_TO_MOD(GPB_UD)
#define MOD_DD GPB_TO_MOD(GPB_DD)
#define MOD_LB GPB_TO_MOD(GPB_DD+GPA_LB)
#define MOD_RB GPB_TO_MOD(GPB_DD+GPA_RB)

#define MORSE_SHORT 0
#define MORSE_LONG 1
#define X 0
#define Y 1

#define DEVICE_JS0 "/dev/input/js0"
#define DEVICE_UINPUT "/dev/uinput"