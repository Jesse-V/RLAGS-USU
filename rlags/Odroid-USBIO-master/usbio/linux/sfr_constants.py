# this defines the SFR registers as constants.  Use the class Bunch() to instantiate the class
# gpio = Bunch(pic_registers)
# 
# add your own after the ending comma or create a new dictionary like pic_registers =  
# version: 0.3
# date: 2/28/2013
#   (added DAC registers)
pic_registers = { 
'INTCON':0xf2,   'INTCON2':0xf1,  'INTCON3':0xf0, 'SSP1CON3':0xcb, 'SSP1MSK':0xca,  'SSP1BUF':0xc9,  'SSP1ADD':0xc8, 'SSP1STAT':0xc7,
'SSP1CON1':0xc6, 'SSP1CON2':0xc5, 'TMR2':0xbc,    'T2CON':0xba,    'PSTR1CON':0xb9, 'BAUDCON1':0xb8, 'PWM1CON':0xb7, 'SPBRGH1':0xb0,
'TRISE':0x96,    'TRISD':0x95,    'TRISC':0x94,   'TRISB':0x93,    'TRISA':0x92,    'LATE':0x8d,     'LATD':0x8c,    'LATC':0x8b, 
'LATB':0x8a,     'LATA':0x89,     'PORTE':0x84,   'PORTD':0x83,    'PORTC':0x82,    'PORTB':0x81,    'PORTA':0x80,   'ANSELE':0x5f,
'ANSELD':0x5e,   'ANSELC':0x5d,   'ANSELB':0x5c,  'ANSELA':0x5b,   'GP_RAM1':0x52,  'GP_RAM2':0x51,  'GP_RAM3':0x50, 'GP_RAM4':0x4f,
'SPBRG1':0xaf,   'CCPTMRS':0x59,  'CCP1CON':0xbd, 'CCPR1H':0xbf,   'CCPR1L':0xbe,   'T2CON':0xba,    'PR2':0xbb, 'VREFCON1':0x7c,
'VREFCON2':0x7b, 'VREFCON0':0x7d, 'ADCON0': 0xc2,
}

pic_bits = {
'SSP1STAT_SMP':7, 'SSP1STAT_CKE':6, 'SSP1STAT_BF':0, 'SSP1CON1_WCOL':7, 'SSP1CON1_SSPOV':6, 'SSP1CON1_SSPEN':5, 'SSP1CON1_CKP':4,
'VREFCON1_DACEN':7, 'VREFCON1_DACLPS':6, 'VREFCON1_DACOE':5, 'VREFCON1_DACNSS':0, 'VREFCON1_DACPSS1':3, 'VREFCON1_DACPSS0':2,
}
