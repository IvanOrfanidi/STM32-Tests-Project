#include "math.h"
#include "includes.h"
#include "eeprom.h"
#include "ram.h"

extern TEepConfig g_stEepConfig;

#define STABLE_G_MG 20               //���������� ��������� ��� ��������� ��������
#define G_NOISE_MG 50                //���������� ��� �������������
#define G_MOVE_NOISE_MG 100          //���������� ��� �� ���� ������������� ��� ��������
#define VIOLATION_TIMEOUT_MS 5000    //������ ��� ������ ��������� � ���������
#define G_FILTER_SET_UP_MS 6000      //����� ��������� ������� ���� �������
#define ACCIDENT_ALARM_MG 2500       //����� � mG ��� ������������ ������� ������
#define ROTATE_ALARM_DEG 70          //����� � �������� ��� ������� ����������
#define ROTATE_T_MS 500              //������� ���������� ������������ � �� ��� ����������� �����

#define ROTATE_X BIT(0)
#define ROTATE_Y BIT(1)
#define ROTATE_Z BIT(2)

#define G_MIN (800)
#define G_MAX (1200)

//��� ��������� ��������� �� �������������
#define A_MEAS_STEP_MS 10

/******************************************************************************
        ������� ���������
******************************************************************************/
//��������������� ������ ��� �������� ������� ���������
float raw_data_filt[3];
//��������������� �������� �� 3� ���� ������������� � ������ (� ������ �������� ������� ���������)
float a_filt[4];
//��������������� �������� ���� ������� �� 3� ����
float g_level[3];
//������� ��������� ������� ��� ��������� ����������
#define FAST_SET_G_FILTER (1.5 * ACC_FREQ_HZ)
uint8_t fast_set_g_filter = FAST_SET_G_FILTER;
//���������� �� ������ �� ���� g_level[i] - a_filt[i]
int16_t da[3];

/******************************************************************************
        ���������� ��� ������� ��������� ��������
******************************************************************************/
//����� ����������� ��������� �� ��� Z
uint16_t z_axis_const_ms = 0;

/******************************************************************************
      �������� ��� �������� ������� ���������
******************************************************************************/
#define SYS_CALIB_OK_BIT 0
#define SYS_CALIB_CALC_BIT 1
#define SYS_G_CALIB_OK_BIT 2
uint8_t system_calib = 0;                  //���� ���������� ����������� ����
float cos_alpha = 1.0, sin_alpha = 0.0;    //�������� �� ���� ������ ��� �
float cos_beta = 1.0, sin_beta = 0.0;      //�������� �� ���� ������ ��� y
float cos_gamma = 1.0, sin_gamma = 0.0;    //�������� �� ���� ������ ��� z

/******************************************************************************
      ������� ��������� (����� ��� �������������� ����)
      ������� �� ��������� 5 ���. � ���������� ��������� 1 ���.
******************************************************************************/
//��������� �� ���� � ������ �������� ������� ���������
int16_t a_hstry[5][3];
//����� ������
int16_t raw_data_hstry[5][3];
//������������ ��������� ����� �� 5 ���
uint8_t d_course;
//���� �������� ������ �� GPS
uint8_t gps_valid;
//�������� �� GPS � ��/�
int16_t speed_gps[5];
//���� �� GPS � ����
int16_t course_gps[5];
//������� ������� �����
int16_t max_overload_mg = 0;
//������� ����������
int16_t rotate_deg_max = 0;
int16_t rotate_t_ms = 0;

/******************************************************************************
      ����������� ������
******************************************************************************/
int16_t j_curr = 0;
int16_t j_peak = 0;
int16_t g_curr = 0;
int16_t g_peak = 0;
int16_t cnt_curr = 0;
int16_t cnt_peak = 0;

int16_t g_lcd_peak = 0;

float w_deg[3] = { 0, 0, 0 };
float w_peak[3] = { 0, 0, 0 };

TDSm_Limits* stLimits;
TDSm_Calib* stCalib;
TDS_Violation DS_Violation;
//���� ���������� ����������
uint8_t allow_calib = 0;

void DSm_CalcCalib();
void DSm_FilterA(float* p_raw);

//���������� ����������
float w_shake_dbg = 0, cnt_shake_dbg = 0;
// DS_Debug ds_dbg;

int16_t GetAccidentAlarm()
{
    return max_overload_mg;
}

int16_t GetRotateAlarm()
{
    int16_t ms;
    int16_t deg;

    ENTR_CRT_SECTION();
    ms = rotate_t_ms;
    deg = rotate_deg_max;
    EXT_CRT_SECTION();

    if(ms >= ROTATE_T_MS)
        return deg;
    else
        return 0;
}

void ResetRotateAlarm()
{
    ENTR_CRT_SECTION();
    rotate_deg_max = 0;
    rotate_t_ms = 0;
    EXT_CRT_SECTION();
}

void ResetAccidentAlarm()
{
    max_overload_mg = 0;
}

void DSm_Init()
{
    memset(raw_data_filt, 0, sizeof(raw_data_filt));
    memset(a_filt, 0, sizeof(a_filt));
    memset(g_level, 0, sizeof(g_level));
    memset(da, 0, sizeof(da));

    z_axis_const_ms = 0;
    system_calib = 0;
    gps_valid = 0;
    cos_alpha = 1.0;
    sin_alpha = 0;
    cos_beta = 1.0;
    sin_beta = 0;
    cos_gamma = 1.0;
    sin_gamma = 0;

    memset(a_hstry, 0, sizeof(a_hstry));
    memset(raw_data_hstry, 0, sizeof(raw_data_hstry));
    memset(speed_gps, 0, sizeof(speed_gps));
    memset(course_gps, 0, sizeof(course_gps));

    j_curr = 0;
    j_peak = 0;
    g_curr = 0;
    g_peak = 0;
    cnt_curr = 0;
    cnt_peak = 0;
    g_lcd_peak = 0;
    fast_set_g_filter = FAST_SET_G_FILTER;

    memset(w_deg, 0, sizeof(w_deg));
    memset(w_peak, 0, sizeof(w_peak));
    memset(&DS_Violation, 0, sizeof(TDS_Violation));

    gps_valid = 0;
    d_course = 0;
    allow_calib = 0;
    max_overload_mg = 0;
    rotate_deg_max = 0;
    rotate_t_ms = 0;

    //���������� ������ �� EEPROM
#ifdef _DRIVE_SIMPLE_
    stLimits = &g_stEepConfig.stDSm_Data.stLimits;
    stCalib = &g_stEepConfig.stDSm_Data.stCalib;
#endif

    //������������ �������� �� ������ ����������� ������
    system_calib = 0;
    DSm_CalcCalib();
    /*
   DP_GSM("drive style calib: %i\r\n", system_calib);
   DP_GSM("alpha cos: %.2f, sin: %.2f\r\n", cos_alpha, sin_alpha);
   DP_GSM("beta cos: %.2f, sin: %.2f\r\n", cos_beta, sin_beta);
   DP_GSM("gamma cos: %.2f, sin: %.2f\r\n", cos_gamma, sin_gamma);
   */
}

/*float test_brake[3];

void SetBrake()
{
  test_brake[0] = raw_data_filt[0];
  test_brake[1] = raw_data_filt[1];
  test_brake[2] = raw_data_filt[2];
}

void TestCalib()
{
  memset(stCalib, 0, sizeof(TDSm_Calib));
  
  stCalib.g_vector[0][0] = (int16_t)raw_data_filt[0];
  stCalib.g_vector[0][1] = (int16_t)raw_data_filt[1];
  stCalib.g_vector[0][2] = (int16_t)raw_data_filt[2];
  
  stCalib.brake_vector[0][0] = (int16_t)test_brake[0];
  stCalib.brake_vector[0][1] = (int16_t)test_brake[1];
  stCalib.brake_vector[0][2] = (int16_t)test_brake[2];

  system_calib = 0;
  DSm_CalcCalib();
  
  if (system_calib)
  {
    DP_ACC("alpha cos: %.2f, sin: %.2f\r\n", cos_alpha, sin_alpha);
    DP_ACC("beta cos: %.2f, sin: %.2f\r\n", cos_beta, sin_beta);
    DP_ACC("gamma cos: %.2f, sin: %.2f\r\n", cos_gamma, sin_gamma);
    
    float temp[3];
    
    for (uint16_t i = 0; i < 500; i++)
    {
      temp[0] = stCalib.g_vector[0][0];
      temp[1] = stCalib.g_vector[0][1];
      temp[2] = stCalib.g_vector[0][2];
      DSm_FilterA(temp);
    }
    DP_ACC("%i %i %i\r\n", (int)a_filt[0], (int)a_filt[1], (int)a_filt[2]);
    
    for (uint16_t i = 0; i < 500; i++)
    {
      temp[0] = test_brake[0];
      temp[1] = test_brake[1];
      temp[2] = test_brake[2];
      DSm_FilterA(temp);
    }
    DP_ACC("%i %i %i\r\n", (int)a_filt[0], (int)a_filt[1], (int)a_filt[2]);
  }
}*/

void DSm_ConfigInit(TDSm_DATA* p)
{
    //������������� ������� �� ���������
    memset(p, 0, sizeof(TDSm_DATA));

    p->stLimits.g_turn = DEFAULT_G_TURN;
    p->stLimits.g_brake = DEFAULT_G_BRAKE;
    p->stLimits.g_accel = DEFAULT_G_ACCEL;
    p->stLimits.g_shake = DEFAULT_G_SHAKE;
    p->stLimits.w_shake = DEFAULT_W_SHAKE;

    p->stLimits.violation_dur_ms[0] = DEFAULT_DUR_BRAKE;
    p->stLimits.violation_dur_ms[1] = DEFAULT_DUR_ACCEL;
    p->stLimits.violation_dur_ms[2] = DEFAULT_DUR_TURN;
    p->stLimits.violation_dur_ms[3] = DEFAULT_DUR_W_SHAKE;
    p->stLimits.k_j_shake = DEFAULT_K_J_SHAKE;

    p->stCalib.on = BIT(0) | BIT(1);

    p->bDriverSimple = 1;
}

//������� ������� �� ���� �� ���� x,y,z
// axis ������� ����
void RotateVector(float* v, uint8_t axis)
{
    float temp[3];
    if(axis & BIT(0)) {
        //������� ������ ��� x
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[1] = cos_alpha * temp[1] - sin_alpha * temp[2];
        v[2] = sin_alpha * temp[1] + cos_alpha * temp[2];
    }
    if(axis & BIT(1)) {
        //������� ������ ��� y
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[0] = cos_beta * temp[0] - sin_beta * temp[2];
        v[2] = sin_beta * temp[0] + cos_beta * temp[2];
    }
    if(axis & BIT(2)) {
        //������� ������ ��� z
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[0] = cos_gamma * temp[0] + sin_gamma * temp[1];
        v[1] = -sin_gamma * temp[0] + cos_gamma * temp[1];
    }
}

//�������� ������ � �������� ������� ���������
//����� ������ � ������������
void GetRawVector(float* v, uint8_t axis)
{
    float temp[3];
    if(axis & BIT(2)) {
        //������� ������ ��� z
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[0] = cos_gamma * temp[0] - sin_gamma * temp[1];
        v[1] = sin_gamma * temp[0] + cos_gamma * temp[1];
    }
    if(axis & BIT(1)) {
        //������� ������ ��� y
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[0] = cos_beta * temp[0] + sin_beta * temp[2];
        v[2] = -sin_beta * temp[0] + cos_beta * temp[2];
    }
    if(axis & BIT(0)) {
        //������� ������ ��� x
        temp[0] = v[0];
        temp[1] = v[1];
        temp[2] = v[2];
        v[1] = cos_alpha * temp[1] + sin_alpha * temp[2];
        v[2] = -sin_alpha * temp[1] + cos_alpha * temp[2];
    }
}

//���������� ���� ����� ����� �������� � ��������
float GetAngle(const int16_t* v1, const int16_t* v2)
{
    float ch = v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
    float zn = sqrt(v1[0] * v1[0] + v1[1] * v1[1] + v1[2] * v1[2]) * sqrt(v2[0] * v2[0] + v2[1] * v2[1] + v2[2] * v2[2]);

    //��� ��������
    ch /= zn;
    if(ch > 0.9999)
        ch = 0.9999;
    if(ch < -0.9999)
        ch = -0.9999;

    float ang = acos(ch) * 57.2957795;

    //�������� float NaN
    if(ang != ang)
        ang = 0;

    return ang;
}

//��� ������� ���������� ��������� � ������������ �� 5 ���.
//������� ��������� g_level - ���� �������
void FastSetGFilter()
{
    int16_t aver[3] = { 0, 0, 0 };
    uint8_t fast_set = 1;

    //������� ��������
    for(uint8_t i = 0; i < 5; i++) {
        aver[0] += a_hstry[i][0];
        aver[1] += a_hstry[i][1];
        aver[2] += a_hstry[i][2];
    }
    aver[0] /= 5;
    aver[1] /= 5;
    aver[2] /= 5;

    //��������� ��� � ������� ������ �������� ������ � ��������
    for(uint8_t i = 0; i < 5; i++) {
        //��������� ������� �� ����
        for(uint8_t j = 0; j < 3; j++)
            if(aver[j] + STABLE_G_MG < a_hstry[i][j] || a_hstry[i][j] < aver[j] - STABLE_G_MG) {
                fast_set = 0;
                break;
            }
        //��������� ���������� ������
        float mod = sqrt(a_hstry[i][0] * a_hstry[i][0] + a_hstry[i][1] * a_hstry[i][1] + a_hstry[i][2] * a_hstry[i][2]);
        if(mod > G_MAX || mod < G_MIN)
            fast_set = 0;
        if(fast_set == 0)
            break;
    }

    if(fast_set) {
        //������� ��������� ������
        g_level[0] = a_hstry[3][0];
        g_level[1] = a_hstry[3][1];
        g_level[2] = a_hstry[3][2];
    }
}

void DSm_SaveHstry()
{
    //��������� � �������
    for(uint8_t i = 0; i < 4; i++)
        for(uint8_t j = 0; j < 3; j++) {
            a_hstry[i][j] = a_hstry[i + 1][j];
            raw_data_hstry[i][j] = raw_data_hstry[i + 1][j];
        }

    for(uint8_t j = 0; j < 3; j++) {
        a_hstry[4][j] = (int16_t)a_filt[j];
        raw_data_hstry[4][j] = (int16_t)raw_data_filt[j];
    }
}

void DSm_SaveSpeed(const float speed /*�������� � ��/���*/, const float course /*���� � ��������*/, const uint8_t valid)
{
    u16 aver_speed = 0;
#define SPEED_CONST_KMH 7

    if(valid) {
        int16_t cur_c = (int16_t)course;
        int16_t max_c;
        int16_t d_c;
        uint8_t d_max = 0;

        //���� ������������ ��������� ����� �� 5 ���
        for(uint8_t i = 0; i < 5; i++) {
            if(cur_c <= course_gps[i])
                max_c = course_gps[i];
            else
                max_c = cur_c;

            d_c = max_c - course_gps[i];
            if(d_c > 180)
                d_c = 360 - max_c + course_gps[i];
            //������������ ��������� ����� � ����
            if(d_c > d_max)
                d_max = d_c;
        }
        d_course = d_max;

        //��������� ������� ����
        for(uint8_t i = 0; i < 4; i++)
            course_gps[i] = course_gps[i + 1];
        course_gps[4] = cur_c;

        //��������� ��������
        for(uint8_t i = 0; i < 4; i++)
            speed_gps[i] = speed_gps[i + 1];
        speed_gps[4] = (u16)speed;

        //��������� ���������� �������� �� ��������� 3 �������
        for(uint8_t i = 2; i < 5; i++)
            aver_speed += speed_gps[i];
        aver_speed /= 3;

        //�������� �������� ���������� ����� ������ ��������
        if(aver_speed > 20)
            allow_calib = 1;

        uint8_t speed_const = 1;
        for(uint8_t i = 2; i < 5; i++)
            if(speed_gps[i] < aver_speed - SPEED_CONST_KMH || speed_gps[i] > aver_speed + SPEED_CONST_KMH)
                speed_const = 0;

        //��������� ������� ����������
        DSm_SaveHstry();
        //������� ��������� ������� G
        if(speed_const)
            FastSetGFilter();
    }
    else {
        d_course = 0;
        memset(speed_gps, 0, sizeof(speed_gps));
    }

    gps_valid = valid;
}

//��������� ��������� ��������� (����� �� X,Y) �� ���� ������� �� ����
//��� ���������� ������� ����������
void SlowSetGFilter(float* p_raw)
{
    static int16_t prev_a[3] = { 0, 0, 0 };     //���������� �������� �������
    static uint16_t stable_cnt = 0;             //������� ���������� ��������� �� ����
    static int16_t d_accum[3] = { 0, 0, 0 };    //��������� ������ �� ������ �� ����

    //������ �������� � ����������� ���������
    for(uint8_t j = 0; j < 3; j++) {
        d_accum[j] += (int16_t)a_filt[j] - prev_a[j];
        prev_a[j] = (int16_t)a_filt[j];

        if(d_accum[j] > STABLE_G_MG * 1.5 || d_accum[j] < -STABLE_G_MG * 1.5)
            stable_cnt = 0;
    }
    //��������� ���������� ������
    if(a_filt[3] > G_MAX || a_filt[3] < G_MIN)
        stable_cnt = 0;

    if(stable_cnt == 0) {
        d_accum[0] = d_accum[1] = d_accum[2] = 0;
    }

#define K_EXP_G_MIN 0.003
#define K_EXP_G_MAX 0.1
    float k_exp = K_EXP_G_MIN;

    //������ 1000 �� ����� ������ �� ���������
    if(stable_cnt < 100) {
        k_exp = 0.0;
    }
    //����� 2 �. ���������� ��������� �������� ��������� ����������
    if(stable_cnt > 200) {
        //��������������� ����������� ����. ����. �������
        float x = ((float)stable_cnt - 100) / 100.0;

        k_exp = 0.00175 * x * x + K_EXP_G_MIN;
    }

    //������� ��������� ��� ������� ����������
    if(fast_set_g_filter) {
        fast_set_g_filter--;
        k_exp = K_EXP_G_MAX;
    }
    if(k_exp > K_EXP_G_MAX)
        k_exp = K_EXP_G_MAX;

    g_level[0] = p_raw[0] * k_exp + g_level[0] * (1.0 - k_exp);
    g_level[1] = p_raw[1] * k_exp + g_level[1] * (1.0 - k_exp);
    g_level[2] = p_raw[2] * k_exp + g_level[2] * (1.0 - k_exp);

    if(++stable_cnt > 540) {
        stable_cnt = 500;
        for(uint8_t j = 0; j < 3; j++) {
            if(d_accum[j] < 0)
                d_accum[j]++;
            if(d_accum[j] > 0)
                d_accum[j]--;
        }
    }
}

void DSm_FilterA(float* p_raw)
{
#define K_EXP_RAW 0.15

    //��������� ����� ������ � �������������
    raw_data_filt[0] = p_raw[0] * K_EXP_RAW + raw_data_filt[0] * (1.0 - K_EXP_RAW);
    raw_data_filt[1] = p_raw[1] * K_EXP_RAW + raw_data_filt[1] * (1.0 - K_EXP_RAW);
    raw_data_filt[2] = p_raw[2] * K_EXP_RAW + raw_data_filt[2] * (1.0 - K_EXP_RAW);

    //������������ ������
    RotateVector(p_raw, ROTATE_X | ROTATE_Y | ROTATE_Z);

    //��� ��������� ������
    static int16_t prev_z = -30000;
    if(prev_z != -30000)
        j_curr = ((int16_t)p_raw[2] - prev_z) / 10;
    else
        j_curr = 0;

    prev_z = (int16_t)p_raw[2];

    //���� ������� ���������� � �����
    int16_t mod1, mod2;

    mod1 = j_peak;
    mod2 = j_curr;

    if(mod1 < 0)
        mod1 *= -1;
    if(mod2 < 0)
        mod2 *= -1;
    if(mod1 < mod2)
        j_peak = j_curr;

    g_curr = (int16_t)p_raw[2] - 1000;

    mod1 = g_peak;
    mod2 = g_curr;

    if(mod1 < 0)
        mod1 *= -1;
    if(mod2 < 0)
        mod2 *= -1;
    if(mod1 < mod2)
        g_peak = g_curr;

    //��� ������� �� ���
    mod1 = g_lcd_peak;
    mod2 = g_curr;

    if(mod1 < 0)
        mod1 *= -1;
    if(mod2 < 0)
        mod2 *= -1;
    if(mod1 < mod2)
        g_lcd_peak = g_curr;

    //��������� ���������� ��� ����������� ������
    mod2 = (int16_t)sqrt(p_raw[0] * p_raw[0] + p_raw[1] * p_raw[1] + p_raw[2] * p_raw[2]);
    if(mod2 > ACCIDENT_ALARM_MG && mod2 > max_overload_mg)
        max_overload_mg = mod2;

#define K_EXP_1 0.035

    //���������������� ������
    a_filt[0] = p_raw[0] * K_EXP_1 + a_filt[0] * (1.0 - K_EXP_1);
    a_filt[1] = p_raw[1] * K_EXP_1 + a_filt[1] * (1.0 - K_EXP_1);
    a_filt[2] = p_raw[2] * K_EXP_1 + a_filt[2] * (1.0 - K_EXP_1);
    //������� ������
    a_filt[3] = sqrt(a_filt[0] * a_filt[0] + a_filt[1] * a_filt[1] + a_filt[2] * a_filt[2]);

    //��������� �������� ������ �� ����
    //���������� ���� ������
    SlowSetGFilter(p_raw);

    //�������� ���������� �� ����
    for(uint8_t i = 0; i < 3; i++)
        da[i] = (int16_t)(a_filt[i] - g_level[i]);

//��������� ������ �� ���� x,y
#define K_EXP_W_1 0.02
#define K_EXP_W_2 0.01
#define K_EXP_W_3 0.08
    static int16_t prev[3] = { 1, 1, 1 };
    int16_t curr[3];

    curr[0] = (int16_t)p_raw[0];
    curr[1] = (int16_t)p_raw[1];
    curr[2] = (int16_t)p_raw[2];

    float ang = GetAngle(curr, prev) * 100.0;
    w_deg[0] = ang * K_EXP_W_1 + w_deg[0] * (1.0 - K_EXP_W_1);
    w_deg[1] = ang * K_EXP_W_2 + w_deg[1] * (1.0 - K_EXP_W_2);
    w_deg[2] = ang * K_EXP_W_3 + w_deg[2] * (1.0 - K_EXP_W_3);
    prev[0] = curr[0];
    prev[1] = curr[1];
    prev[2] = curr[2];

    if(w_peak[0] < w_deg[0])
        w_peak[0] = w_deg[0];
    if(w_peak[1] < w_deg[1])
        w_peak[1] = w_deg[1];
    if(w_peak[2] < w_deg[2])
        w_peak[2] = w_deg[2];
}

//��������� ���� ��� �������� ������� ���������
void DSm_SetAxis(const int16_t* g_const, const int16_t* brake_const)
{
    float g[3], brake[3];
    float len;

    for(uint8_t j = 0; j < 3; j++) {
        g[j] = (float)g_const[j];
        brake[j] = (float)brake_const[j];
    }

    //��������� ���� ��� �������� �������
    cos_alpha = 1;
    sin_alpha = 0;
    cos_beta = 1;
    sin_beta = 0;
    cos_gamma = 1;
    sin_gamma = 0;

    //������ ��� x
    len = sqrt(g[2] * g[2] + g[1] * g[1]);
    if(len > 100) {
        cos_alpha = g[2] / len;
        sin_alpha = g[1] / len;
    }
    RotateVector(g, ROTATE_X);
    //������ ��� y
    len = sqrt(g[2] * g[2] + g[0] * g[0]);
    if(len > 100) {
        cos_beta = g[2] / len;
        sin_beta = g[0] / len;
    }
    RotateVector(g, ROTATE_Y);

    //������ ��� z
    len = sqrt(brake[1] * brake[1] + brake[0] * brake[0]);
    if(len > 100) {
        cos_gamma = brake[0] / len;
        sin_gamma = brake[1] / len;
    }
}

//������� ������� ������ ����������� � ������
//���������� ����� �������� ������������ ��� ����������
uint8_t AverVector(int16_t* v, int16_t __packed v_eeprom[][3])
{
    uint8_t n_vec = 0;

    v[0] = v[1] = v[2] = 0;

    for(uint8_t i = 0; i < DSM_CALIB_VEC_NUM; i++) {
        if(v_eeprom[i][0] || v_eeprom[i][1] || v_eeprom[i][2])
            n_vec++;
        for(uint8_t j = 0; j < 3; j++)
            v[j] += v_eeprom[i][j];
    }

    if(n_vec) {
        v[0] /= n_vec;
        v[1] /= n_vec;
        v[2] /= n_vec;
    }

    return n_vec;
}

//���� ���������� ������ ������������ �������� � ������ ��� �������� ������� ���������
void DSm_CalcCalib()
{
    int16_t brake_v[3];
    int16_t g_v[3];
    uint8_t nG;
    uint8_t nB;
    //�������� ������� ������ ���������� � ���������
    nG = AverVector(g_v, stCalib->g_vector);
    nB = AverVector(brake_v, stCalib->brake_vector);

    if(nG) {
        //�������� ��� ���� ������� �������������
        system_calib |= BIT(SYS_G_CALIB_OK_BIT);
        //��������� ����������
        DSm_SetAxis(g_v, brake_v);
        //������ � ���������� �������
        if(nB && (system_calib & BIT(SYS_CALIB_OK_BIT)) == 0) {
            fast_set_g_filter = FAST_SET_G_FILTER;
            system_calib |= BIT(SYS_CALIB_OK_BIT);
        }
    }

    system_calib &= ~BIT(SYS_CALIB_CALC_BIT);
}

void SaveBrake(int16_t* v)
{
    int16_t brake_v[3];
    uint8_t num;

    if((stCalib->on & BIT(0)) == 0)
        return;

    num = AverVector(brake_v, stCalib->brake_vector);
    float ang = GetAngle(v, brake_v);

    //��������� ������ ������� �������� �� �������� � ������
    if(ang < 10 && num == DSM_CALIB_VEC_NUM && (system_calib & BIT(SYS_CALIB_OK_BIT))) {
        // stCalib->on &= ~BIT(0);
        // SaveConfigCMD();
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
        stCalib->brake_vector[stCalib->brake_num][i] = v[i];

    if(++stCalib->brake_num >= DSM_CALIB_VEC_NUM)
        stCalib->brake_num = 0;

    SaveConfigCMD();
    system_calib |= BIT(SYS_CALIB_CALC_BIT);
}

void SaveG(int16_t* v)
{
    int16_t g_v[3];
    uint8_t num;

    if((stCalib->on & BIT(1)) == 0)
        return;

    num = AverVector(g_v, stCalib->g_vector);
    float ang = GetAngle(v, g_v);

    //��������� ������ ������� �������� �� �������� � ������
    if(ang < 10 && num == DSM_CALIB_VEC_NUM && (system_calib & BIT(SYS_CALIB_OK_BIT))) {
        // stCalib->on &= ~BIT(1);
        // SaveConfigCMD();
        return;
    }

    for(uint8_t i = 0; i < 3; i++)
        stCalib->g_vector[stCalib->g_num][i] = v[i];

    if(++stCalib->g_num >= DSM_CALIB_VEC_NUM)
        stCalib->g_num = 0;

    //���������
    SaveConfigCMD();
    system_calib |= BIT(SYS_CALIB_CALC_BIT);
}

//�������������� ����������� ���� ����������/��������� ������� ���������� � ������
void DSm_AutoCalib()
{
    //������� ���� ������ G
    static uint8_t step = 0;
    //������� �� ����� ���������� ����. ����� (������ ��� �� �� ��������� ����� ������ � ������ �����)
    static uint32_t brake_timeout = 0;
    static uint32_t g_timeout = 0;
    //������� �� ���� ����� 7 ���
#define NEXT_POINT_TIMEOUT (ACC_FREQ_HZ * 420)

    if(brake_timeout)
        brake_timeout--;
    if(g_timeout)
        g_timeout--;

    if(speed_gps[4] > 10 && (system_calib & BIT(SYS_G_CALIB_OK_BIT))) {
        //���� �������� � ������������� ���� ������� - ���� ����������
        step = 1;
    }
    else {
        //�� ���� ��������� ������� ���� ���� �������
        step = 0;
    }

    if(step == 0 && g_timeout == 0) {
        //���� ����������� ���� ������� � ��������� �����
        static uint16_t cnt = 0;

        //������� ����� ���������� ����������
        cnt += A_MEAS_STEP_MS;

        for(uint8_t i = 0; i < 3; i++) {
            if(da[i] < -50 || da[i] > 50)
                cnt = 0;
        }

        if(a_filt[3] < G_MIN || a_filt[3] > G_MAX)
            cnt = 0;
        if(speed_gps[4] > 10 || gps_valid == 0)
            cnt = 0;

        //��������� ������ ���� �������
        if(cnt > G_FILTER_SET_UP_MS * 0.75) {
            int16_t r_d[3];
            r_d[0] = (int16_t)raw_data_filt[0];
            r_d[1] = (int16_t)raw_data_filt[1];
            r_d[2] = (int16_t)raw_data_filt[2];
            //��������� ������
            SaveG(r_d);
            step = 1;
            cnt = 0;
            g_timeout = NEXT_POINT_TIMEOUT;
            // PeriphQueueMSG(BUZZ_SIGNAL_4);
        }
    }
    else if(step == 1 && brake_timeout == 0) {
        //���������� ����������� ��� ����������
        uint8_t strong_brake = 0;
        int16_t dv_total = 0;

        //������� ������������� ���������� �� ��������� 4 ���.
        for(uint8_t i = 1; i < 4; i++) {
            int16_t dv = speed_gps[i] - speed_gps[i + 1];

            //���������� �������� ������ ��� �� 5 ��/� �� ���
            if(dv > 4 && i >= 2)
                strong_brake++;
            if(dv > 10 && i >= 2)
                strong_brake += 2;
            if(dv > 15 && i >= 2)
                strong_brake += 3;

            if(dv < 0) {
                strong_brake = 0;
                break;
            }
        }
        dv_total = speed_gps[1] - speed_gps[4];

        // if (dv_total >= 0)
        //  ds_dbg.dv_total = dv_total;
        // else
        //  ds_dbg.dv_total = 0;
        // ds_dbg.d_course = d_course;
        // ds_dbg.strong_brake = strong_brake;
        // ds_dbg.check = 1;

        if(dv_total >= 10 && strong_brake >= 2 && d_course <= 10 && gps_valid) {
            int16_t ang_max = 0;
            int16_t b_vec[4][4];
            int16_t b_aver[4];
            uint8_t num = 0;
            // ds_dbg.check = 2;

            //������ ��� �������� �������� - �� ��� Z ������ ���� �������
            //����� ���������� ��������� ��� ��� �������� ������� ���������
            for(uint8_t i = 0; i < 4; i++) {
                //����� �������� �� ������ ����
                b_vec[i][0] = a_hstry[i][0] - (int16_t)g_level[0];
                b_vec[i][1] = a_hstry[i][1] - (int16_t)g_level[1];
                b_vec[i][2] = 0;
                b_vec[i][3] =
                    (int16_t)sqrt((float)b_vec[i][0] * (float)b_vec[i][0] + (float)b_vec[i][1] * (float)b_vec[i][1]);
            }
            //������� ������ ���������� "����������� ����������"
            b_aver[0] = b_aver[1] = b_aver[2] = 0;
            for(uint8_t i = 0; i < 4; i++) {
                if(b_vec[i][3] > 140) {
                    b_aver[0] += b_vec[i][0];
                    b_aver[1] += b_vec[i][1];
                    num++;
                }
            }
            b_aver[0] /= num;
            b_aver[1] /= num;
            b_aver[3] = (int16_t)sqrt((float)b_aver[0] * (float)b_aver[0] + (float)b_aver[1] * (float)b_aver[1]);

            //������� ������� �� ����
            int16_t angle;
            for(uint8_t i = 0; i < 4; i++) {
                if(b_vec[i][3] < 50)
                    continue;
                angle = (int16_t)GetAngle(b_vec[i], b_aver);
                if(angle > ang_max)
                    ang_max = angle;
            }

            // ds_dbg.ang_max = ang_max;
            // ds_dbg.mod_aver = b_aver[3];
            // ds_dbg.num = num;

            //��������� ������
            if(ang_max <= 7 && num >= 3 && b_aver[3] > 140) {
                //��������� ������
                b_aver[0] = (int16_t)((400.0 * (float)b_aver[0]) / (float)b_aver[3]);
                b_aver[1] = (int16_t)((400.0 * (float)b_aver[1]) / (float)b_aver[3]);
                b_aver[2] = 0;
                //���������� � �������� ������� ���������
                if(cos_gamma != 1.0 || sin_gamma != 0.0) {
                    float tmp[3];
                    tmp[0] = b_aver[0];
                    tmp[1] = b_aver[1];
                    tmp[2] = 0;
                    GetRawVector(tmp, ROTATE_Z);
                    b_aver[0] = (int16_t)tmp[0];
                    b_aver[1] = (int16_t)tmp[1];
                    b_aver[2] = 0;
                }
                SaveBrake(b_aver);
                brake_timeout = NEXT_POINT_TIMEOUT;
                step = 0;
                // PeriphQueueMSG(BUZZ_SIGNAL_4);
            }
        }
    }
}

void DSm_Analize()
{
    //����� ����� ���������� ���������
    static uint16_t timeout_ms[4] = { 0, 0, 0, 0 };
    //����������������� ���������� ��� �������� ���������
    static uint16_t dur_ms[4] = { 0, 0, 0, 0 };

    //������� ����� � ���. �������� ��������� �� ��� Z
    if(da[2] < G_MOVE_NOISE_MG && da[2] > -G_MOVE_NOISE_MG) {
        z_axis_const_ms += A_MEAS_STEP_MS;
        if(z_axis_const_ms >= 30000)
            z_axis_const_ms = 30000;
    }
    else
        z_axis_const_ms = 0;

    //��������� ��������� �� ����� �������� ���� ��� Z ��������� (��� ��������)
    if(z_axis_const_ms > G_FILTER_SET_UP_MS / 3) {
        static int16_t brake_max = 0, acc_max = 0, turn_max = 0;
        static int16_t v_speed[3] = { 0, 0, 0 };

        //������ ������������ ��������� ����������
        if(da[0] > stLimits->g_brake && a_filt[3] > G_MIN + 100) {
            dur_ms[0] += A_MEAS_STEP_MS;
            if(dur_ms[0] >= stLimits->violation_dur_ms[0]) {
                if(da[0] > brake_max) {
#ifdef DEBUG_DSM
                    if(brake_max == 0) {
                        DS_Violation.acc_brake = ((float)-da[0]) / 1000.0;
                        DS_Violation.speed = (float)speed_gps[4];
                    }
#endif
                    brake_max = da[0];
                    v_speed[0] = (int16_t)speed_gps[4];
                }
                timeout_ms[0] = 0;
            }
        }
        else
            dur_ms[0] = 0;
        //����� ������� ���������
        if(brake_max && da[0] < stLimits->g_brake && timeout_ms[0] > 100) {
            DS_Violation.acc_brake = -((float)brake_max) / 1000.0;
            DS_Violation.speed = v_speed[0];
            brake_max = 0;
        }

        //��� ��������� ������ 20 ��/� ����� ��������� � 2 ���� ����
        uint16_t acc_thresh;
        if(speed_gps[4] < 20)
            acc_thresh = stLimits->g_accel;
        else
            acc_thresh = stLimits->g_accel / 2;

        //������ ������������ ��������� �������
        if(-da[0] > acc_thresh && a_filt[3] > G_MIN) {
            dur_ms[1] += A_MEAS_STEP_MS;
            if(dur_ms[1] >= stLimits->violation_dur_ms[1]) {
                if(-da[0] > acc_max) {
#ifdef DEBUG_DSM
                    if(acc_max == 0) {
                        DS_Violation.acc_brake = ((float)-da[0]) / 1000.0;
                        DS_Violation.speed = (float)speed_gps[4];
                    }
#endif
                    acc_max = -da[0];
                    v_speed[1] = (int16_t)speed_gps[4];
                }
                timeout_ms[1] = 0;
            }
        }
        else
            dur_ms[1] = 0;
        //����� ������� ���������
        if(acc_max && -da[0] < acc_thresh && timeout_ms[1] > 100) {
            DS_Violation.acc_brake = ((float)acc_max) / 1000.0;
            DS_Violation.speed = v_speed[1];
            acc_max = 0;
        }

        //������� ��������
        int16_t turn_a = da[1];
        if(turn_a < 0)
            turn_a *= -1;

        if(turn_a > stLimits->g_turn && a_filt[3] > G_MIN) {
            dur_ms[2] += A_MEAS_STEP_MS;
            if(dur_ms[2] >= stLimits->violation_dur_ms[2]) {
                if(turn_a > turn_max) {
#ifdef DEBUG_DSM
                    if(turn_max == 0) {
                        DS_Violation.turn = ((float)turn_a) / 1000.0;
                        DS_Violation.speed = (float)speed_gps[4];
                    }
#endif
                    turn_max = turn_a;
                    v_speed[2] = (int16_t)speed_gps[4];
                }
                timeout_ms[2] = 0;
            }
        }
        else
            dur_ms[2] = 0;
        //����� ������� ��������
        if(turn_max && turn_a < stLimits->g_turn && timeout_ms[2] > 100) {
            DS_Violation.turn = ((float)turn_max) / 1000.0;
            DS_Violation.speed = v_speed[2];
            turn_max = 0;
        }
    }

    //������� ��������� �� ������
    static int16_t mod_g_peak = 0;
    static int16_t mod_j_peak = 0;
    int16_t mod_j = j_curr;
    int16_t mod_g = g_curr;

    if(mod_j < 0)
        mod_j *= -1;
    if(mod_g < 0)
        mod_g *= -1;

    if(mod_g > stLimits->g_shake || mod_j * 10 > stLimits->g_shake * stLimits->k_j_shake) {
        //��������� ������� ���������� ��� �����
        cnt_curr++;
        if(mod_g_peak < mod_g)
            mod_g_peak = mod_g;
        if(mod_j_peak < mod_j)
            mod_j_peak = mod_j;
    }
    else if(mod_g_peak) {
        //������� ������������ �����
        int16_t threshold = (int16_t)(((float)mod_g_peak) * 0.5);

        if(threshold < 200)
            threshold = 200;
        if(threshold > 400)
            threshold = 400;

        if(mod_g < threshold) {
            //��������� ������� ����������
            if(cnt_peak < cnt_curr)
                cnt_peak = cnt_curr;

            //��������� ������� �����
            if((cnt_curr >= 2 || mod_g_peak >= 1.5 * stLimits->g_shake) && timeout_ms[3] > 500) {
                DS_Violation.shake = mod_g_peak / 10;
                if(DS_Violation.shake < mod_j_peak)
                    DS_Violation.shake = mod_j_peak;
                DS_Violation.speed = (float)speed_gps[4];
                cnt_shake_dbg = cnt_curr;
                timeout_ms[3] = 0;
            }
            mod_g_peak = 0;
            mod_j_peak = 0;
            cnt_curr = 0;
        }
        else
            cnt_curr++;
    }
    else {
        //������ ���
    }

    //��������� �� ������
    if(w_deg[0] > stLimits->w_shake) {
        dur_ms[3] += A_MEAS_STEP_MS;
        if(dur_ms[3] >= stLimits->violation_dur_ms[3] && timeout_ms[3] > 500) {
            DS_Violation.shake = w_peak[0] / 100;
            DS_Violation.speed = (float)speed_gps[4];
            w_shake_dbg = DS_Violation.shake;
            timeout_ms[3] = 0;
        }
    }
    else
        dur_ms[3] = 0;

    for(uint8_t i = 0; i < 4; i++) {
        if(timeout_ms[i] < 2 * VIOLATION_TIMEOUT_MS)
            timeout_ms[i] += A_MEAS_STEP_MS;
    }
}

void DSm_RotateAlarm(float* v)
{
    static uint8_t cnt = 0;
#define ROTATE_CHECK_PERIOD 5

    //������ ������ 50�� ��� �� �� ������� ����
    if(++cnt >= ROTATE_CHECK_PERIOD) {
        int16_t curr[3];
        int16_t g[3];
        int16_t deg;
        curr[0] = (int16_t)v[0];
        curr[1] = (int16_t)v[1];
        curr[2] = (int16_t)v[2];
        g[0] = (int16_t)g_level[0];
        g[1] = (int16_t)g_level[1];
        g[2] = (int16_t)g_level[2];

        deg = (int16_t)GetAngle(curr, g);
        //���������� ���� ����� �������� ����������� �������������
        //� ��������������� �������� G.
        //���� ���� ����� ���� ������ ������, � ������� ��������� ��������� ������� ������
        if(fast_set_g_filter == 0 && deg > ROTATE_ALARM_DEG) {
            //������ G ����������, ���� ������ ������
            if(rotate_t_ms < 2 * ROTATE_T_MS)
                rotate_t_ms += 10 * ROTATE_CHECK_PERIOD;
            if(rotate_deg_max < deg)
                rotate_deg_max = deg;
        }
        else if(rotate_t_ms < ROTATE_T_MS) {
            //���� ������ ������, ���� ������ G �� ����������. �������� ��������� ������������� "�������" ����������
            if(rotate_t_ms > 10 * ROTATE_CHECK_PERIOD)
                rotate_t_ms -= 10 * ROTATE_CHECK_PERIOD;
            else {
                rotate_t_ms = 0;
                rotate_deg_max = 0;
            }
        }
        cnt = 0;
    }
}

int DS_Write_Violation(u8* out)
{
    int n = 0;
    if(DS_Violation.turn != 0) {
        uint8_t val = (uint8_t)(DS_Violation.turn * 100.0);
        uint8_t speed = (uint8_t)(DS_Violation.speed / 1.852);
        n += put_ds_report(DS_TURN_ALARM, val, speed, out + n);

        n += VarLog(EV_DS_TURN, DS_Violation.turn, out + n);
    }
    if(DS_Violation.acc_brake != 0) {
        if(DS_Violation.acc_brake >= 0) {
            uint8_t val = (uint8_t)(DS_Violation.acc_brake * 100.0);
            uint8_t speed = (uint8_t)(DS_Violation.speed / 1.852);
            n += put_ds_report(DS_ACC_ALARM, val, speed, out + n);
        }
        else {
            uint8_t val = (uint8_t)(-DS_Violation.acc_brake * 100.0);
            uint8_t speed = (uint8_t)(DS_Violation.speed / 1.852);
            n += put_ds_report(DS_BRAKE_ALARM, val, speed, out + n);
        }

        n += VarLog(EV_DS_ACC_BRAKE, DS_Violation.acc_brake, out + n);
    }
    if(DS_Violation.shake != 0) {
        uint8_t val = (uint8_t)DS_Violation.shake;
        uint8_t speed = (uint8_t)(DS_Violation.speed / 1.852);
        n += put_ds_report(DS_SHAKE_ALARM, val, speed, out + n);

        n += VarLog(EV_DS_SHAKE, DS_Violation.shake, out + n);
    }
    if(DS_Violation.speed)
        n += VarLog(EV_DS_SPEED, DS_Violation.speed, out + n);

    memset((void*)&DS_Violation, 0, sizeof(DS_Violation));

#ifdef USE_BUZZER
    if((n) && (g_stEepConfig.stDSm_Data.bBuzDrive) && (g_stEepConfig.stDSm_Data.bBuzWork))
        PeriphQueueMSG(BUZZ_SIGNAL_4);
#endif

    return n;
}

void DSm_MoveDetect(float* v)
{
#define MOVE_COUNT 2              //����� ��������, ��� ����������� ������� ��������
#define MOVE_DETECT_TIMEOUT 40    //����� ��������� � ������������, ������� ����� ���������� �� ����. �������� ����
#define ACC_SENSIVITY_STEP_MG 15

    static u8 cnt = 0;
    static u16 move_threshold = 0;
    static u32 cnt_reset = 0;

    if(move_threshold == 0) {
        u8 move = 0;
        //���� ���������� �� ����
        if(fast_set_g_filter == 0) {
            s16 da[3];
            for(u8 i = 0; i < 3; i++)
                da[i] = (s16)(g_level[i] - v[i]);

            for(u8 i = 0; i < 3; i++)
                if(da[i] > ACC_SENSIVITY_STEP_MG * GetAccelSensitivity())
                    move = 1;

            //���������� ������ ���������
            if(a_filt[3] < G_MIN - ACC_SENSIVITY_STEP_MG * GetAccelSensitivity() ||
                a_filt[3] > G_MAX + ACC_SENSIVITY_STEP_MG * GetAccelSensitivity())
                move = 1;
        }

        if(move) {
            cnt_reset = 0;
            move_threshold = MOVE_DETECT_TIMEOUT;
            cnt++;
        }
    }
    else {
        if(++cnt_reset >= ACC_FREQ_HZ * 180) {
            cnt_reset = 0;
            cnt = 0;
        }
        if(move_threshold)
            move_threshold--;
    }

    if(cnt >= MOVE_COUNT) {
        //������ ���� ��� ���� ��������
        AccMoveDetect();
        cnt = 0;
        cnt_reset = 0;
    }
}

void DSm_Calc(vector* a_raw)
{
    float v[3];
    v[0] = a_raw->x;
    v[1] = a_raw->y;
    v[2] = a_raw->z;

    //��������� ������
    DSm_FilterA(v);

    //���������� �������� ��������
    DSm_MoveDetect(v);

    //�������� ����������
    DSm_RotateAlarm(v);

    //���� ������ ��� �������������� ����������� ����
    if(allow_calib)
        DSm_AutoCalib();

    //������������� ����������� ���� x,y,z
    if(system_calib & BIT(SYS_CALIB_CALC_BIT))
        DSm_CalcCalib();

    //��������� �� ���������/����������/��������� � ������
    if(system_calib & BIT(SYS_CALIB_OK_BIT))
        DSm_Analize();
}

_Bool DriverFilterData(void)
{
#ifdef _DRIVE_SIMPLE_
    _Bool FilterDataTrue = 1;
    static u32 g_uiDriverTimeBack = 0;
    ACC_STATE AccelStat;
    float SpeedGps;

    u32 SecRTC;

    //�������� ������ �����������������, ���� �� LOW PWR2, �� ������� ������.
    if((g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR2_MODE) || (g_stRam.stDevice.eCurPwrStat == POWER_LOW_PWR1_MODE) ||
        (g_stEepConfig.stDSm_Data.bDriverSimple == 0)) {
        return 0;
    }

    SpeedGps = GetSpeedKmGps();
    AccelStat = AccelState();

    SecRTC = get_cur_sec();
    //��������� ����� ��������� ������
    if((SecRTC >= g_uiDriverTimeBack) && ((AccelStat != ACC_STATE_STOP) || (SpeedGps >= MIN_SPEED_GPS))) {
        g_uiDriverTimeBack = SecRTC + 1;
        FilterDataTrue = 1;
    }
    else {
        FilterDataTrue = 0;
    }

    return FilterDataTrue;
#else
    return 0;
#endif
}