#include "measure.h"
#include "tim.h"
#include "gpio.h"

int count[3];
uint8_t measurecplt = 0;
uint8_t capture_state = 0;

void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim == &htim4)
    {
        count[1] = __HAL_TIM_GET_COUNTER(&htim5);
        measurecplt = 1;
    }
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    switch (capture_state)
    {
    case 1:
        count[0] = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_2);
        __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_2, TIM_ICPOLARITY_FALLING);

        capture_state++;
        break;
    case 2:
        __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
        capture_state++;
        break;
    case 3:
        capture_state++;
        count[1] = HAL_TIM_ReadCapturedValue(&htim4, TIM_CHANNEL_2);
        count[2] = __HAL_TIM_GetCounter(&htim3);
        HAL_TIM_IC_Stop_IT(&htim4, TIM_CHANNEL_2);
        HAL_TIM_Base_Stop(&htim3);
        break;
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
        __HAL_TIM_SET_PRESCALER(&htim4, 47); // 5MHz
    }
    else
    {
        __HAL_TIM_SET_PRESCALER(&htim4, 3999); // 60kHz
    }

    __HAL_TIM_SetCompare(&htim5, TIM_CHANNEL_1, 1);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SetCounter(&htim3, 0);
    HAL_TIM_GenerateEvent(&htim4, TIM_EventSource_Update);
    HAL_TIM_GenerateEvent(&htim5, TIM_EVENTSOURCE_UPDATE);
    HAL_TIM_GenerateEvent(&htim3, TIM_EVENTSOURCE_UPDATE);

    while (capture_state != 4)
    {
        if (capture_state == 0)
        {
            __HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
            HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_2);
            HAL_TIM_Base_Start(&htim3);
            capture_state++;
        }
    }

    capture_state = 0;
    double Hertz;
    double c = 65536 * count[2] + count[1] - count[0];
    if (method == MEASURE_PERIOD_HIGH_METHOD)
        Hertz = 5000000 / c;
    else
        Hertz = 60000 / c;
    return Hertz;
}

double MeasureFreq_Freq()
{
    __HAL_TIM_SET_PRESCALER(&htim4, 23999); // 10k Hz
    __HAL_TIM_SetCompare(&htim4, TIM_CHANNEL_1, 10000);
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    __HAL_TIM_SET_AUTORELOAD(&htim5, 4294967295);
    HAL_TIM_GenerateEvent(&htim4, TIM_EventSource_Update);
    HAL_TIM_GenerateEvent(&htim5, TIM_EVENTSOURCE_UPDATE);

    int k = get_timer_frequency();

    HAL_TIM_OC_Start_IT(&htim4, TIM_CHANNEL_1);
    HAL_TIM_Base_Start(&htim5);
    count[0] = __HAL_TIM_GET_COUNTER(&htim5);
    // wait for measurement
    while (!measurecplt)
        ;
    measurecplt = 0;
    int Hertz = count[1] - count[0];
    HAL_TIM_OC_Stop_IT(&htim4, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim5);
    return Hertz;
}

// 获取定时器2的频率
uint32_t get_timer_frequency()
{
    // 获取时钟源频率
    uint32_t clk_freq = HAL_RCC_GetPCLK1Freq(); // 这里假设PCLK1是定时器2的时钟源

    // 获取预分频器值
    uint32_t prescaler = htim4.Init.Prescaler; // 假设你已经初始化了定时器2的配置

    // 计算定时器频率
    uint32_t timer_freq = clk_freq / (prescaler + 1);

    return timer_freq;
}