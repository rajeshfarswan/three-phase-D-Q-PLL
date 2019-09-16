//**********************************************************//
// THREE PHASE PLL USING DQ TRANSFORMATION                  //
//**********************************************************//

//include files
#include "main.h"
#include "user.h"
#include "asmMath.h"

int main(void) //main
{

init(); // processor initialisation

starting(); // before pwm initialisations

  while(1) //main loop 
	{

//generate PLL Sin - Cos ref and generate @ PWM outputs
    if(current_Flag) //20Khz 
       {     
            asm("disi #0x3FFF");
            Avalue = asmINT_MPQ(qSin,PWM_offset); //copy sine value
            Bvalue = asmINT_MPQ(qCos,PWM_offset); //copy cos value
            Cvalue = 0;
            asm("disi #0x0000");

            asmPWM(); //generate sine at duty cycle 1 and cos at duty cycle 2          
            
                               current_Flag = 0;   
                                  } 
//PLL output//

//D-Q PLL//        
	if(pll_Flag) //1.3Khz
		{
          asm("disi #0x3FFF"); //read grid voltages
           {
          Avalue = asmADC(0x0505) - offset; //adc channel 5   
          Bvalue = asmADC(0x0606) - offset; //adc channel 6    
          Cvalue = asmADC(0x0707) - offset; //adc channel 7
            } 
          asm("disi #0x0000");
          //

          asmABCtoDQ(); //feedback to DQ frame transformation

          Vgrid_Dvalue = Dvalue; //copy grid voltage d value

          FOF_PreOut = Osc_FOFout;
          Osc_FOFout = asmFO_Filter(Qvalue,Filter_const_P); //filter grid voltage q value
          
         /*PLL PI************************************************************/
          IPreError = oscPI_out;
          oscPI_out = asmPIcontroller(0,Osc_FOFout,oscPI_Pgain,oscPI_Igain); //set grid voltage q value to zero
          oscPI_out = asmINT_MPQ(OSC_Fcentral,oscPI_out);

          if(oscPI_out >= OSC_Fmax) oscPI_out = OSC_Fmax;
          if(oscPI_out <= OSC_Fmin) oscPI_out = OSC_Fmin;
         /*PLL PI************************************************************/ 

				         pll_Flag = 0;
						  }
//PLL//	

    			ClrWdt();
    		}//while end////////////////////////////////////////////////////////

  
		ClrWdt();
	} //main end////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////


//T1 interrupt for oscillator tracking
		void _ISRFAST __attribute__((interrupt, no_auto_psv)) _T1Interrupt(void) //7.5Khz
  			{   
                // harmonic oscillator to generate sin and cos ref

                  OSC_F =  oscPI_out  + OSC_Fcentral; // update oscillator frequency
                 
                  theta = theta + OSC_F; //update theta angle 
     
         		  if(theta >= theta_2PI) //reset oscillator
            		    {
           				qSin = 0;
           				qCos = 32436; //0.99 //32440
           				theta = 0;
              				}
              		else 
                    asmHARMONIC(); //harmonic oscillator

                              
     			T1us_Flag = 0;
                
   					} //T1 interupt end

//////////////////////////////////////////////////////////////////////

			//initial startup routine
			void starting(void)
  				{
                    PWM_offset = PWM_PERIOD; //initialise PWM period value
                    PWM_offset_N = -PWM_PERIOD;

					PWM_max = PWM_offset*8; //PI saturation values
					PWM_min = -PWM_offset*8;
					SET = 0; //initialise PWM control registers
					PWMenable = 0; //reset PWM control register
					 //
					FAULT_ENABLE = 0; //0x000f; //reset fault register
					delay(30); //delay 30ms
					ADC_ON = 1;
				
					offset = asmADC(0x0e0e); //2.5V offset //read adc channel 14
					//
					
					//set pwm
					PWM1 = PWM_offset;
					PWM2 = PWM_offset;
					PWM3 = PWM_offset;
					
					//
					RL3_ON = 1;  //connect to grid
					delay(20); //delay 20ms
					RL4_ON = 1;
					delay(20); //delay 20ms
					RL5_ON = 1;
					
					delay(20); //delay 20ms
                    SET = 0x0077; //all three switces are enabled

					//PWM_InterruptEnable = 0;
					PWMenable = 1;
					T1ON = 1;
                    T2ON = 1;
                    T3ON = 1;
                    
					// 
					  	}//startup routine end

///////////////////////////////////////////////////////////////////////

			











