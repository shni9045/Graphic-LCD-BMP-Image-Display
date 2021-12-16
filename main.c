#include "msp.h"


#define CE    0x01      /* P6.0 chip select */
#define RESET 0x40      /* P6.6 reset */
#define DC    0x80      /* P6.7 register select */

/* define the pixel size of display */
#define GLCD_WIDTH  84
#define GLCD_HEIGHT 48

void GLCD_setCursor(unsigned char x, unsigned char y);
void GLCD_clear(void);
void GLCD_init(void);
void GLCD_data_write(unsigned char data);
void GLCD_command_write(unsigned char data);
void GLCD_putchar(int c);
void SPI_init(void);
void SPI_write(unsigned char data);

/* sample font table */
const char font_table[][6] = {
    {0x7e, 0x11, 0x11, 0x11, 0x7e, 0},  /* A */
    {0x7f, 0x49, 0x49, 0x49, 0x36, 0},  /* B */
    {0x3e, 0x41, 0x41, 0x41, 0x22, 0},
    {0x7f, 0x41, 0x41, 0x22, 0x1c},{0x46, 0x49, 0x49, 0x49, 0x31},{0x7f, 0x02, 0x0c, 0x02, 0x7f}}; /* C */

int main(void) {
    GLCD_init();        /* initialize the GLCD controller */
    GLCD_clear();       /* clear display and home the cursor */

   // GLCD_putchar(0);    /* display letter A */
    GLCD_putchar(1);    /* display letter B */
    //GLCD_putchar(2);    /* display letter C */
    GLCD_putchar(3);
    GLCD_putchar(4);
    GLCD_putchar(5);



    while(1) { }
}

void GLCD_putchar(int c) {
    int i;
    for (i = 0; i < 6; i++)
        GLCD_data_write(font_table[c][i]);
}

void GLCD_setCursor(unsigned char x, unsigned char y) {
    GLCD_command_write(0x80 | x);  /* column */
    GLCD_command_write(0x40 | y);  /* bank (8 rows per bank) */
}

/* clears the GLCD by writing zeros to the entire screen */
void GLCD_clear(void) {
    int32_t index;
    for (index = 0 ; index < (GLCD_WIDTH * GLCD_HEIGHT / 8) ; index++)
        GLCD_data_write(0x00);

    GLCD_setCursor(0, 0); /* return to the home position */
}

/* send the initialization commands to PCD8544 GLCD controller */
void GLCD_init(void) {
    SPI_init();

    /* hardware reset of GLCD controller */
    P6->OUT |= RESET;               /* deassert reset */

    GLCD_command_write(0x21);       /* set extended command mode */
    GLCD_command_write(0xB8);       /* set LCD Vop for contrast */
    GLCD_command_write(0x04);       /* set temp coefficient */
    GLCD_command_write(0x14);       /* set LCD bias mode 1:48 */
    GLCD_command_write(0x20);       /* set normal command mode */
    GLCD_command_write(0x0C);       /* set display normal mode */
}

/* write to GLCD controller data register */
void GLCD_data_write(unsigned char data) {
    P6->OUT |= DC;                  /* select data register */
    SPI_write(data);                /* send data via SPI */
}

/* write to GLCD controller command register */
void GLCD_command_write(unsigned char data) {
    P6->OUT &= ~DC;                 /* select command register */
    SPI_write(data);                /* send data via SPI */
}

void SPI_init(void) {
    EUSCI_B2->CTLW0 = 0x0001;       /* put UCB0 in reset mode */
    EUSCI_B2->CTLW0 = 0x69C1;       /* PH=0, PL=1, MSB first, Master, SPI, SMCLK */
    EUSCI_B2->BRW = 3;              /* 3 MHz / 3 = 1 MHz */
    EUSCI_B2->CTLW0 &= ~0x0001;     /* enable UCB0 after config */


    P3->SEL0 |= 0x60;
    P3->SEL1 &= ~0x60;

    P6->DIR |= CE | RESET | DC;     /* P6.7, P6.6, P6.0 set as output */
    P6->OUT |= CE;                  /* CE idle high */
    P6->OUT &= ~RESET;              /* assert reset */
}

void SPI_write(unsigned char data) {
    P6->OUT &= ~CE;                 /* assert /CE */
    EUSCI_B2->TXBUF = data;         /* write data */
    while(EUSCI_B2->STATW & 0x01) ; /* wait for transmit done */
    P6->OUT |= CE;                  /* deasssert /CE */
}
