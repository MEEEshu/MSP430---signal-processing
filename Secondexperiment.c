#include <msp430.h> 
//MCLK de 8 MHz si SMCLk de 2MHz
//Masterclock at 8MHz and SMclk at 2MHz
//br =38400 baudrate
//for terminal output will send 2 different messages from pushing the pull-up buttons from the board
//se poate si trimite un caracter cu pragma usci
//it may send a character from the keyboard
//adc cu input la a9
//a9 input for ADC
//vref- = 0//VREF = 3.3V
//Fs = 800 Hz datarate
//pagina 77 - 77th page at familly user guide
//DAC - AM ADAUGAT PT VARIABILA VALORI 100 DE VALORI- ASADAR FRECVENTA ESTE DE Fs/NrVAL.
//DAC - 100 values added for DAC, so the frequency is Fs/valuesnumber
//rutina de tratare a intrerupilor pentru dac pentru a afisa 2 frecvente diferite
//interrupt routine can show us 2 different frequencies
//Fs/nrval

//for semnal 0 Fs = 800/100 = 8 hz
//for semnal 1 Fs = 800/120 = 6.66 hz
//Daca dorim schimbarea tipului de semnal, folosim intreruperea cu semnal si schimbam tipul prin punerea la
//pull down a pinului 3.5
//the type of the signal can be switched using a pull down to P3.5

/**
 * main.c
 */
unsigned int semnal = 0;
unsigned int DAC_data=0;
volatile unsigned int adcResult;
volatile unsigned char cuv[20]={'s','t','a','n','g','a',' ',10,13};
volatile unsigned char cuv2[20]={'d','r','e','a','p','t','a',10,13};
volatile unsigned int i = 0;
volatile unsigned int j  = 0;
volatile int VALORI[100] ={0x800,0x880,0x900,0x97f,0x9fd,0xa78,0xaf1,0xb67,0xbda,0xc49,
                           0xcb3,0xd19,0xd79,0xdd4,0xe29,0xe78,0xec0,0xf02,0xf3c,0xf6f,
                           0xf9b,0xfbf,0xfdb,0xfef,0xffb,0xfff,0xffb,0xfef,0xfdb,0xfbf,
                           0xf9b,0xf6f,0xf3c,0xf02,0xec0,0xe78,0xe29,0xdd4,0xd79,0xd19,
                           0xcb3,0xc49,0xbda,0xb67,0xaf1,0xa78,0x9fd,0x97f,0x900,0x880,
                           0x800,0x77f,0x6ff,0x680,0x602,0x587,0x50e,0x498,0x425,0x3b6,
                           0x34c,0x2e6,0x286,0x22b,0x1d6,0x187,0x13f,0xfd,0xc3,0x90,
                           0x64,0x40,0x24,0x10,0x4,0x0,0x4,0x10,0x24,0x40,
                           0x64,0x90,0xc3,0xfd,0x13f,0x187,0x1d6,0x22b,0x286,0x2e6,
                           0x34c,0x3b6,0x425,0x498,0x50e,0x587,0x602,0x680,0x6ff,0x77f};
volatile int VALORI2[120] = {0x800,0x86b,0x8d6,0x940,0x9a9,0xa11,0xa78,0xadd,
                             0xb40,0xba1,0xbff,0xc5b,0xcb3,0xd08,0xd5a,0xda7,
                             0xdf1,0xe37,0xe78,0xeb5,0xeed,0xf20,0xf4e,0xf77,
                             0xf9b,0xfb9,0xfd2,0xfe6,0xff4,0xffc,0xfff,0xffc,
                             0xff4,0xfe6,0xfd2,0xfb9,0xf9b,0xf77,0xf4e,0xf20,
                             0xeed,0xeb5,0xe78,0xe37,0xdf1,0xda7,0xd5a,0xd08,
                             0xcb3,0xc5b,0xbff,0xba1,0xb40,0xadd,0xa78,0xa11,
                             0x9a9,0x940,0x8d6,0x86b,0x800,0x794,0x729,0x6bf,
                             0x656,0x5ee,0x587,0x522,0x4bf,0x45e,0x400,0x3a4,
                             0x34c,0x2f7,0x2a5,0x258,0x20e,0x1c8,0x187,0x14a,
                             0x112,0xdf,0xb1,0x88,0x64,0x46,0x2d,0x19,
                             0xb,0x3,0x0,0x3,0xb,0x19,0x2d,0x46,
                             0x64,0x88,0xb1,0xdf,0x112,0x14a,0x187,0x1c8,
                             0x20e,0x258,0x2a5,0x2f7,0x34c,0x3a4,0x400,0x45e,
                             0x4bf,0x522,0x587,0x5ee,0x656,0x6bf,0x729,0x794};
volatile unsigned int tip_semnal = 0;
volatile unsigned int iS = 0;
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;
  FRCTL0 = FRCTLPW | NWAITS_2;
    __bis_SR_register(SCG0);                           // disable FLL

    CSCTL3 |= SELREF__REFOCLK;                         // Set REFO as FLL reference source

    CSCTL0 = 0;                                        // clear DCO and MOD registers

    CSCTL1 |= DCORSEL_3;                               // Set DCO = 8MHz

    CSCTL2 = FLLD_0 + 243;                             // DCOCLKDIV = 8MHz

   __delay_cycles(3);

   __bic_SR_register(SCG0);                           // enable FLL

   while (CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1));         // FLL locked

   CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK;        // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz , default DCOCLKDIV as MCLK and SMCLK source

   CSCTL5 = DIVM_0 | DIVS_2;



//MCLK = 4MHz
//SMCLK = 1MHZ
   // 3.4 SMCLK
    P3DIR |= BIT4;
    P3SEL0 |= BIT4;

    //3.0 MCLK
    P3DIR |= BIT0;
    P3SEL0 |= BIT0;

    P6OUT &=~BIT6;
    P6DIR |= BIT6;

    //A5
    P1SEL1 |= BIT5;
    P1SEL0 |= BIT5;
    Init_GPIO();
     //pini uart
    P4SEL0 |= BIT2 | BIT3;                    // set 2-UART pin as first function
     //configuram pinul 2.3 in pull-up

    P2OUT |= BIT3;                          // Configure P1.3 as pulled-up
    P2REN |= BIT3;                          // P1.3 pull-up register enable
    P2IES |= BIT3;                          // P1.3 Hi/Low edge
    P2IE |= BIT3;                           // P1.3 interrupt enabled
    //interrupt flag
    P2IFG &= ~BIT3;                         // P1.3 IFG cleared

    P3OUT |= BIT5;                          // Configure P1.3 as pulled-up
    P3REN |= BIT5;                          // P1.3 pull-up register enable
    P3IES |= BIT5;                          // P1.3 Hi/Low edge
    P3IE |= BIT5;                           // P1.3 interrupt enabled
       //interrupt flag
    P3IFG &= ~BIT5;                         // P1.3 IFG cleared


    P4OUT |= BIT1;                          // Configure P1.3 as pulled-up
    P4REN |= BIT1;                          // P1.3 pull-up register enable
    P4IES |= BIT1;                          // P1.3 Hi/Low edge
    P4IE |= BIT1;                           // P1.3 interrupt enabled
       //interrupt flag
    P4IFG &= ~BIT1;


    UCA1CTLW0 |= UCSWRST;
    //setam sursa smclk
    UCA1CTLW0 |= UCSSEL_3;  // set SMCLK 2MHz as BRCLK

//N = 52.083
//os16 = 1
//ucbrx = 3
//ucbrf = 4
//ucbrs = 0.083 = 0x04
//
//
    PMMCTL0_H = PMMPW_H;                                          // Unlock the PMM registers
    PMMCTL2 |= INTREFEN |REFVSEL_2;


    UCA1MCTLW = 0x0441;
    UCA1BRW = 0x0003;
    UCA1BR0 = 0x03;
    UCA1BR1 = 0x00;

    UCA1CTLW0 &= ~UCSWRST;                    // Initialize eUSCI
    UCA1IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt

    P2IFG &= ~BIT3;

    //configurare DAC -> out P1.1
     P1SEL0 |= BIT1;                           // Select P1.1 as OA0O function
     P1SEL1 |= BIT1;                           // OA is used as buffer for DAC


     //cofigurare input adc p5.1
     P5SEL0 |= BIT1;
     P5SEL1 |= BIT1;
     //dac - > out o2
     P3SEL0 |= BIT1;

    //    ADCCTL0 |= ADCENC | ADCSC;
   ADCCTL0 |= ADCSHT_10 |ADCMSC_1 | ADCON; //| ADCENC | ADCSC;//ADCSHT_8= div 256                                  // ADC ON,temperature sample period>30us
   ADCCTL1 |= ADCSHP_1 | ADCSHS_0 | ADCDIV_0 | ADCSSEL_3 | ADCCONSEQ_0; // SMCLK, Div =1                                           // s/w trig, single ch/conv, MODOSC
   ADCCTL2 &= ~ADCRES;                                           // clear ADCRES in ADCCTL
   ADCCTL2 |= ADCRES_2;// | ADCPDIV_2; //CLK/64                                         // 12-bit conversion results
   ADCMCTL0 |= ADCSREF_0 | ADCINCH_9; // A6 -> canal activ                           // ADC input ch A12 => temp sense
            //    SYSCFG2 |= ADCPCTL_8;
   ADCIE |=ADCIE0;
   ADCCTL0 |= ADCENC | ADCSC;// enable conversion
            // Enable the Interrupt request for a completed ADC_B conversion
   //P5SEL1 |= BIT2;
 //  P5SEL0 &= ~BIT2;
            // Configure reference
                                // Enable internal reference and temperature sensor
   __delay_cycles(400);


   while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles

   while(!(PMMCTL2 & REFGENRDY));            // Poll till internal reference settles

  // SAC0_DAC
  // SAC0DACSTS=DACIFG;// clear intreupt
  // SAC0DAT = sin[0]; // write sample
    SAC0DAC = DACSREF_0 + DACLSEL_2 + DACIE;  // Select int Vref as DAC reference
    SAC0DAT = VALORI[i];                       // Initial DAC data
    SAC0DAC |= DACEN;                         // Enable DAC

    // SAC0_AO
      SAC0OA = NMUXEN + PMUXEN + PSEL_1 + NSEL_1;//Select positive and negative pin input
      SAC0OA |= OAPM_0;                            // Select low speed and low power mode
      SAC0PGA = MSEL_1;                          // Set OA as buffer mode
      SAC0OA |= SACEN + OAEN;                    // Enable SAC and OA

    //FRECVENTA SMCLK 4 MHZ
            //CCR0 ESANTIONAM PENTRU A AJUNGE LA O FRECVENTA DE ESANTIONARE DE APROX 900 HZ

    TB2CCR0 = 2500-1;                           // PWM Period/2
    TB2CCTL1 = OUTMOD_6;                       // TBCCR1 toggle/set
    TB2CCR1 = 1250;                              // TBCCR1 PWM duty cycle
    TB2CTL = TBSSEL__SMCLK | MC_1 | TBCLR;     // SMCLK, up mode, clear TBR





     PM5CTL0 &= ~LOCKLPM5;
     __bis_SR_register(GIE);         // Enter LPM3, interrupts enabled
     __no_operation();

}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(USCI_A1_VECTOR))) USCI_A1_ISR (void)
#else
#error Compiler not supported!
#endif
{
  switch(__even_in_range(UCA1IV,USCI_UART_UCTXCPTIFG))
  {
    case USCI_NONE: break;
    case USCI_UART_UCRXIFG:
      while(!(UCA1IFG&UCTXIFG));
      UCA1TXBUF = UCA1RXBUF;
      __no_operation();
      break;
    case USCI_UART_UCTXIFG: break;
    case USCI_UART_UCSTTIFG: break;
    case USCI_UART_UCTXCPTIFG: break;
    default: break;
  }
}

// Port 2 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT2_VECTOR))) Port_2 (void)
#else
#error Compiler not supported!
#endif
{//br= 38400
    P2IFG &= ~BIT3;
    for(i=0;i<=8;i++){// Clear P1.3 IFG
   while(!(UCA1IFG&UCTXIFG));
    UCA1TXBUF =cuv[i];
    }//    __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT4_VECTOR
__interrupt void Port_4(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT4_VECTOR))) Port_4 (void)
#else
#error Compiler not supported!
#endif
{//br= 38400
    P4IFG &= ~BIT1;
    for(i=0;i<=8;i++){// Clear P1.3 IFG
   while(!(UCA1IFG&UCTXIFG));
    UCA1TXBUF =cuv2[i];
    }//    __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
}


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
           // SAC0DAT = ADCMEM0;
            adcResult = ADCMEM0;
            //se input adc
/*            semnal[i]=ADCMEM0;
            i++;
            if(i==100)
                i=0;
*/

            P6OUT ^= BIT6;

            break;
        default:
            break;
    }
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT3_VECTOR
__interrupt void Port_3(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT3_VECTOR))) Port_3 (void)
#else
#error Compiler not supported!
#endif
{
    P3IFG &= ~BIT5;
    if (semnal == 0 )
            semnal = 1 ;
        else semnal = 0;// Clear P1.3 IFG
       // __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
  __bic_SR_register_on_exit(LPM3_bits);   // Exit LPM3
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
      //  DAC_data++;
      //  DAC_data &= 0xFFF;

        i++;
        j++;
        switch (semnal){
        case 0:SAC0DAT = VALORI[i];
        semnal = 0;break;
        case 1:SAC0DAT = VALORI2[j];
        semnal = 1;
                              break;
        case 2:SAC0DAT = ADCMEM0;
        default:SAC0DAT = VALORI[i];
        semnal = 0; break;
        }
     ;                 // DAC12 output positive ramp

        if(i==99)
            i=0;
        else if (j ==119)
            j =0;
        break;
    default: break;
  }
}


void Init_GPIO()
{
    P1DIR = 0xFF; P2DIR = 0xFF;
    P1REN = 0xFF; P2REN = 0xFF;
    P1OUT = 0x00; P2OUT = 0x00;
}

