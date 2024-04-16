// ISACHI MIHAI
// Basic Software - Workaround with registers
// Based from MSP430FR2355 datasheet, familly user guide and resources
// Configuring ClockSystem and setting OUTPUT pin for measurements P1.0
// Configuring Timer_B to generate PWM signals with a Duty cycle = 50%
// Processing signals from ADC and generate with DAC (SAC in our case) a ramp signal
// Hope you will enjoy :)


#include <msp430.h>
volatile unsigned int adcResult = 0;
unsigned int DAC_data=0;

int main(void)
{
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watch dog timer
  WDTCTL = WDTPW + WDTHOLD;                 // Stop watch dog timer
  // configurare CS SMCLK=24MHz

  // Configure two FRAM waitstate as required by the device datasheet for MCLK
  // operation at 24MHz(beyond 8MHz) _before_ configuring the clock system.
  FRCTL0 = FRCTLPW | NWAITS_2;

  __bis_SR_register(SCG0);                           // disable FLL
  CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source
  CSCTL0 = 0;                                        // clear DCO and MOD registers
  CSCTL1 |= DCORSEL_7;                               // Set DCO = 24MHz
  CSCTL2 = FLLD_0 + 731;                             // DCOCLKDIV = 24MHz
  __delay_cycles(3);
  __bic_SR_register(SCG0);                           // enable FLL
  while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked

  CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;        // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                                     // default DCOCLKDIV as MCLK and SMCLK source
  P1DIR |= BIT0;       // set P1.0 as SMCLK  output
  P1SEL1 |= BIT0;      // set ACLK and  SMCLK pin as second function


  //    ADCCTL0 |= ADCENC | ADCSC;
  ADCCTL0 |= ADCSHT_10 |ADCMSC_1 | ADCON; //| ADCENC | ADCSC;//ADCSHT_8= div 256                                  // ADC ON,temperature sample period>30us
  ADCCTL1 |= ADCSHP_1 | ADCSHS_0 | ADCDIV_7 | ADCSSEL_3 | ADCCONSEQ_0; // SMCLK, Div =1                                           // s/w trig, single ch/conv, MODOSC
  ADCCTL2 &= ~ADCRES;                                           // clear ADCRES in ADCCTL
  ADCCTL2 |= ADCRES_2;// | ADCPDIV_2; //CLK/64                                         // 12-bit conversion results
  ADCMCTL0 |= ADCSREF_1 | ADCINCH_10; // A6 -> canal activ                           // ADC input ch A12 => temp sense
  //    SYSCFG2 |= ADCPCTL_8;
  ADCIE |=ADCIE0;
  ADCCTL0 |= ADCENC | ADCSC;// enable conversion
  // Enable the Interrupt request for a completed ADC_B conversion
  P5SEL1 |= BIT2;
  P5SEL0 &= ~BIT2;


  P1SEL0 |= BIT1;                           // Select P1.1 as OA0O function
  P1SEL1 |= BIT1;                           // OA is used as buffer for DAC

  PM5CTL0 &= ~LOCKLPM5;                     // Disable the GPIO power-on default high-impedance mode
  P3DIR |= BIT5;
  P3SEL0 |= BIT5;                           // to activate previously configured port settings
  P6SEL0 |= BIT1;
  // Configure reference module
  PMMCTL0_H = PMMPW_H;                      // Unlock the PMM registers
  PMMCTL2 = INTREFEN | REFVSEL_2;           // Enable internal 2.5V reference
  while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles

  SAC3DAC = DACSREF_0 + DACLSEL_2 + DACIE;  // Select int Vref as DAC reference
  SAC3DAT = DAC_data;                       // Initial DAC data
  SAC3DAC |= DACEN;                         // Enable DAC

  SAC3OA = NMUXEN + PMUXEN + PSEL_1 + NSEL_1;//Select positive and negative pin input
  SAC3OA |= OAPM;                            // Select low speed and low power mode
  SAC3PGA = MSEL_1;                          // Set OA as buffer mode
  SAC3OA |= SACEN + OAEN;                    // Enable SAC and OA

  // Use TB2.1 as DAC hardware trigger
  TB2CCR0 = 4000-1;                           // PWM Period/2
  TB2CCTL1 = OUTMOD_6;                       // TBCCR1 toggle/set
  TB2CCR1 = 3600    ;                          // TBCCR1 PWM duty cycle
  TB2CTL = TBSSEL__SMCLK | MC_1 | TBCLR;     // SMCLK, up mode, clear TBR

  __bis_SR_register(LPM3_bits + GIE);        // Enter LPM3, Enable Interrupt
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector = SAC0_SAC2_VECTOR
__interrupt void SAC0_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(SAC0_SAC2_VECTOR))) SAC0_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(SAC0IV,SACIV_4))
  {
    case SACIV_0: break;
    case SACIV_2: break;
    case SACIV_4:
        DAC_data= DAC_data + 1;
        if (DAC_data > 2)
            DAC_data = 0;
        DAC_data &= 0xFFF;
        SAC0DAT = DAC_data;                 // DAC12 output positive ramp
        break;
    default: break;
  }
}

// ADC interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC_VECTOR
__interrupt void ADC_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(ADC_VECTOR))) ADC_ISR (void)
#else
#error Compiler not supported!
#endif
{

    switch(__even_in_range(ADCIV,ADCIV_ADCIFG))
    {
        case ADCIV_NONE:
            break;
        case ADCIV_ADCOVIFG:
            break;
        case ADCIV_ADCTOVIFG:
            break;
        case ADCIV_ADCHIIFG:
            break;
        case ADCIV_ADCLOIFG:
            break;
        case ADCIV_ADCINIFG:
            break;
        case ADCIV_ADCIFG:
            SAC0DAT = ADCMEM0;
            adcResult = ADCMEM0;
/*
 *   semnal[i]=ADCMEM0;
            i++;
            if(i==100)
                i=0;

*/
              TB2CCR0 = 20000-1;                           // PWM Period/2
              TB2CCTL1 = OUTMOD_7;                       // TBCCR1 toggle/set
              TB2CCR1 = (int)(350 + (1.95*adcResult));                          // TBCCR1 PWM duty cycle
              TB2CTL = TBSSEL__SMCLK | MC_1 | TBCLR;     // SMCLK, up mode, clear TBR
              __delay_cycles(10000);
            P6OUT ^= BIT6;

            break;
        default:
            break;
    }
}
