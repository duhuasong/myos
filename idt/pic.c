#include "./include/io.h"
#define MASTER_PIC_COMMAND 0x20
#define MASTER_PIC_DATA    0x21
#define SLAVE_PIC_COMMAND  0xA0
#define SLAVE_PIC_DATA     0xA1
#define PIC_EOI 0x20
//�ж���ɷ�����
void PIC_sendEOI(unsigned char irq)
{	
	if(irq >= 8)
		outb(SLAVE_PIC_COMMAND, PIC_EOI);
		
	outb(MASTER_PIC_COMMAND, PIC_EOI);
}

#define ICW1_ICW4        0x01   // ICW4(not) needed
#define ICW1_SINGLE      0x02    //single (cascade)mode
#define ICW1_INTERVAL4   0x04     //call address interval 4
#define ICW1_LEVEL       0x08    //level triggered(edge)mode
#define ICW1_INIT        0x10    //��ʼ��

#define ICW4_8086        0x01   //8086/88 mode
#define ICW4_AUTO     	 0x02   //auto (normal)EOI
#define ICW4_BUF_SLAVE   0x08   //buffered mode/slave
#define ICW4_BUF_MASTER  0x0c  //buffered mode /master
#define ICW4_SFNM        0x10  //special fully nested

void PIC_remap(int offset1, int offset2)
{
	unsigned char al, a2;
	
	a1 = inb(MASTER_PIC_DATA);   //save masks
	a2 = inb(SLAVE_PIC_DATA);   
	io_out8(PIC0_IMR,  0xff  );//������pic�ж�
	io_out8(PIC1_IMR,  0xff  ); //���ô�pic�ж�
	
	outb_p(MASTER_PIC_COMMAND, ICW1_INIT + ICW1_ICW4);// starts the initialization sequence (cascade mode)
	outb_p(SLAVE_PIC_COMMAND, ICW1_INIT + ICW1_ICW4);
	
	outb_p(MASTER_PIC_DATA, offset1);
	outb_p(SLAVE_PIC_DATA, offset2);
	
	outb_p(MASTER_PIC_DATA, 1<<2);//tell the master there is a slave PIC at irq2
	outb_p(SLAVE_PIC_DATA, 2);//tell slave pic ints cascade identify 2
	
	outb(MASTER_PIC_DATA, a1);
	outb(MASTER_PIC_DATA, a2);
}
//����ĳ���ж�
void IRQ_Set_mask(unsigned char IRQline)
{
	unsigned short port;
	unsigned char value;
	if(IRQline < 8)
		port = MASTER_PIC_DATA;
	else
	{
		port = SLAVE_PIC_DATA;
		IRQline -= 8;
	}
	
	value = inb(port) | (1 << IRQline);
	outb(port, value);
}

//����ĳ���ж�
void IRQ_Set_mask(unsigned char IRQline)
{
	unsigned short port;
	unsigned char value;
	if(IRQline < 8)
		port = MASTER_PIC_DATA;
	else
	{
		port = SLAVE_PIC_DATA;
		IRQline -= 8;
	}
	
	value = inb(port) | ~(1 << IRQline);
	outb(port, value);
}