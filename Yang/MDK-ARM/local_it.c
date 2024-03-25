#include "main.h"
//测频法重装载回调函数
void HAL_TIM_PeriodElapsedCallback (TIM_HandleTypeDef *htim)
{
	if(htim == &htim3) // 判断触发溢出中断的定时器
	{
		//一次计时时间越长越准确
		pwm_flag++;
		__HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
		pwm_val+=htim2.Instance->CNT;
		htim2.Instance->CNT=0;
		if(pwm_flag==10){
			printf("The frequency is %d Hz\n",pwm_val);
			pwm_val=0;
			pwm_flag=0;
		}
	}
}

/////////////////////////////////////////////
//测周法测量较为低频信号
void mode_1(){
	switch(capture_state)
	{
		case 0:
			//设置上升捕获
			capture_state++;
			__HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
			HAL_TIM_IC_Start_IT(&htim4, TIM_CHANNEL_1);	
			break;
		case 4:
			capture_state=0;
			float high = capture_buf[1]- capture_buf[0];
			float low = capture_buf[2]- capture_buf[1]; 		
			float period = capture_buf[2]- capture_buf[0];
			
			//判断是否测量了_cnt次
			int _cnt=3;
			if(flag_cnt<_cnt)
			{
				high_time += high;
				low_time 	+= low;
				period_time += period;
				flag_cnt++;
			}
			else
			{
				flag_cnt = 0;
				high_time /= _cnt;
				low_time 	/= _cnt;
				period_time /= _cnt;
				//判断本轮测量是否与上丿轮测量差距过大，如果过大将其舍去（主要是防止定时器溢出影响）
				if(period_time_old!=0 && (period_time>65500 || period_time>10*period_time || period<period_time/10))
				{
					high_time = low_time = period_time = 0;
					break;
				}
				
				//判断是否有串口数据输入若有，舍弃本次测量
				if(new_flag==1)
				{
					new_flag=0;
					break;
				}
				//正常状濁输凿
				if(period_time!=period_time_old)
				{
					//printf("the period time is %.2f us\n",period_time);
					printf("The frequency is %.2f Hz\n",1/period_time*1000*1000);
					//printf("the Duty Ratio is %.2f\n",high_time/period_time);
				}
				high_time_old = high_time;
				low_time_old = low_time;
				period_time_old = period_time;
				high_time = low_time = period_time = 0;
				Delay_Ms(10);
			}
			break;
	}
}


//下面两个是中断处理函数
//串口输入数据就重置一切测量状态，重新测量
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	HAL_UART_Receive_IT(&huart1,mes_uart,20);
	PAUSE=0;
	PERIOD=0;
	for(int i=0;i<10;i++){	
		PAUSE+=((mes_uart[10+i]-'0')*pow((double)10,(double)(10-i)));
		PERIOD+=((mes_uart[i]-'0')*pow((double)10,(double)(10-i)));
	}
	__HAL_TIM_SET_AUTORELOAD(&htim5,PERIOD-1);
	__HAL_TIM_SET_COMPARE(&htim5,TIM_CHANNEL_1,PAUSE*100);
	new_flag=1;
	high_time = 0;
	low_time = 0;
	period_time = 0;
	high_time_old = 0;
	low_time_old = 0;
	period_time_old = 0;
	Delay_Ms(20);
}
//输入捕获模式测量频率
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
	//捕获模式状濁机设置
	if(htim->Instance == htim4.Instance)
	{
		switch(capture_state){
			case 1:
				//捕获上升时间点，修改为下降捕莿
				capture_state++;
				capture_buf[0] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//record
				__HAL_TIM_SET_CAPTUREPOLARITY(&htim4,TIM_CHANNEL_1,TIM_ICPOLARITY_FALLING);  
				break;
			case 2:
				//捕获下降时间，修改为上升捕获
				capture_state++; 
				capture_buf[1] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//record
				__HAL_TIM_SET_CAPTUREPOLARITY(&htim4, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
				break;
			case 3:
				//捕获上升时间，停止捕获，本周期捕获结板
				capture_state++;
				capture_buf[2] = HAL_TIM_ReadCapturedValue(&htim4,TIM_CHANNEL_1);//record
				HAL_TIM_IC_Stop_IT(&htim4,TIM_CHANNEL_1); //stop		
				break;			
		}
	}
}
