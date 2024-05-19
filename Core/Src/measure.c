#include "measure.h"
#include "tim.h"
#include "gpio.h"

int count[2];
int rising_time_stamp[2];
int capture_cnt = 0;
int old_ccr2, old_ccr3;
uint8_t measurecplt = 0;
uint8_t capture_state = 0;
uint8_t is_channel2_first;
uint16_t update_cnt;

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim2)
    {
        count[1] = __HAL_TIM_GET_COUNTER(&htim5);
        measurecplt = 1;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (m == FREQUENCY)
    {
        switch (capture_state)
        {
        case 1:
            count[0] = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
            __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_ICPOLARITY_FALLING);
            capture_state++;
            break;
        case 2:
            __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
            capture_state++;
            break;
        case 3:
            capture_state++;
            count[1] = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
            HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
            break;
        }
    }
    else if (m == GAP)
    {
        capture_cnt++;
        if (htim2.Instance->CCR2 != old_ccr2 && htim2.Instance->CCR3 != old_ccr3)
        {
            __NVIC_DisableIRQ(TIM2_IRQn);
					capture_cnt=2;
        }
    }
}

double MeasureFreq_main()
{
    double Hertz;
    switch (method)
    {
    case MEASURE_PERIOD_LOW_METHOD:
        Hertz = MeasureFreq_Period();
        break;
    case MEASURE_PERIOD_HIGH_METHOD:
        Hertz = MeasureFreq_Period();
        break;
    case MEASURE_FREQ_METHOD:
        Hertz = MeasureFreq_Freq();
        break;
    }
    // SWITCH METHOD
    if (Hertz < 10000 && Hertz > 100)
        method = MEASURE_PERIOD_HIGH_METHOD;
    else if (Hertz <= 100)
        method = MEASURE_PERIOD_LOW_METHOD;
    else
        method = MEASURE_FREQ_METHOD;

    return Hertz;
}

double MeasureFreq_Period()
{
    if (method == MEASURE_PERIOD_HIGH_METHOD)
    {
        __HAL_TIM_SET_PRESCALER(&htim2, 47); // 5MHz
    }
    else
    {
        __HAL_TIM_SET_PRESCALER(&htim2, 3999); // 60kHz
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_GenerateEvent(&htim2, TIM_EventSource_Update);

    while (capture_state != 4)
    {
        if (capture_state == 0)
        {
            __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
            HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
            capture_state++;
        }
    }

    capture_state = 0;
    double Hertz;
    double c = count[1] - count[0];
    if (method == MEASURE_PERIOD_HIGH_METHOD)
        Hertz = 5000000 / c;
    else
        Hertz = 60000 / c;
    return Hertz;
}

double MeasureFreq_Freq()
{
    __HAL_TIM_SET_PRESCALER(&htim2, 23999); // 10k Hz
    __HAL_TIM_SetCompare(&htim2, TIM_CHANNEL_1, 10000);
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim5, 4294967295);
    HAL_TIM_GenerateEvent(&htim2, TIM_EventSource_Update);
    HAL_TIM_GenerateEvent(&htim5, TIM_EVENTSOURCE_UPDATE);

    int k = get_timer_frequency();

    HAL_TIM_OC_Start_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_Base_Start(&htim5);
    count[0] = __HAL_TIM_GET_COUNTER(&htim5);
    // wait for measurement
    while (!measurecplt)
        ;
    measurecplt = 0;
    int Hertz = count[1] - count[0];
    HAL_TIM_OC_Stop_IT(&htim2, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim5);
    return Hertz;
}

// 获取定时器2的频率
uint32_t get_timer_frequency()
{
    // 获取时钟源频率
    uint32_t clk_freq = HAL_RCC_GetPCLK1Freq(); // 这里假设PCLK1是定时器2的时钟源

    // 获取预分频器值
    uint32_t prescaler = htim2.Init.Prescaler; // 假设你已经初始化了定时器2的配置

    // 计算定时器频率
    uint32_t timer_freq = clk_freq / (prescaler + 1);

    return timer_freq;
}

double MeasureGap()
{
    double gap;
    __HAL_TIM_SET_PRESCALER(&htim2, 0); // 240MHz
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_TIM_GenerateEvent(&htim2, TIM_EventSource_Update);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
    __HAL_TIM_SET_CAPTUREPOLARITY(&htim2, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);

    capture_cnt = 0;
    old_ccr2 = htim2.Instance->CCR2;
    old_ccr3 = htim2.Instance->CCR3;
    __NVIC_DisableIRQ(TIM2_IRQn);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim2, TIM_CHANNEL_3);
    __NVIC_EnableIRQ(TIM2_IRQn);

    while (capture_cnt < 2)
        ;

    __NVIC_DisableIRQ(TIM2_IRQn);
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_2);
    HAL_TIM_IC_Stop_IT(&htim2, TIM_CHANNEL_3);
    __NVIC_EnableIRQ(TIM2_IRQn);

    rising_time_stamp[0] = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
    rising_time_stamp[1] = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_3);

    gap = abs(rising_time_stamp[1] - rising_time_stamp[0]);
    gap *= 1 / 240000000.0;
    return gap;
}