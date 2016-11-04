/*****************************************************************************/
/*       FileName  :   INT.C                                                 */
/*       Content   :   DSA-208 INT Module                                    */
/*       Date      :   Fri  02-22-2002                                       */
/*                     DSASoftWare(c)                                        */
/*                     CopyRight 2002             DSA-GROUP                  */
/*****************************************************************************/
//-�������intel 80C196 
//-���������﷨Ҫ��
//-#pragma interrupt( )��ʾ�жϷ����ӳ���
//-�Ҿ�����Ϊһ��ϵͳ����,��û��ʲô�ȶ�����,�����ܶ���ʼ��ִ����Ҫ�Ĵ�������,Ȼ��
//-����ʵ����������ж�,��"����"�߼��жϳ������.

//-�Դ����������ϵͳ����Ͳ�Ҫ�����������ʲôͻ����,����Ч�ʵ��µ���Ϊ,����Ҫ����
//-����Ӧ��������ϵͳ,��������������Ż�,���Ǳ����ܹ��������Լ���Ҫ������,�ŵ��Լ�
//-��ϵͳ��,�����ܱ����˿�ס




#pragma  regconserve

#include _SFR_H_
#include _FUNCS_H_

#include "common.h"
#include "device.h"
#include "reg.h"
#include "ram.h"
#include "rom.h"
#include "comuse.h"


//-���ڿ�������,�ж���������,����ͨ���Ĵ��������ж�,Ȼ���ض��źžͻᴥ����Ӧ�ж�
//-����ͻ�����жϷ����ӳ�����д���,Ȼ������жϷ���
//-���ڵ�������������������жϵ�ַ�����Ӧ��������?
//-����û������,������Ϊ�������ʵ��������ܵĳ����
#pragma  interrupt(NMI_int        =0x1f)    /*  NMI        for None     int  */
#pragma  interrupt(port_IL_intr   =0x1e)    /*  EXINT3     for 554_2    int  */	//-���ж�����δ�����
#pragma  interrupt(port_EH_intr   =0x1d)    /*  EXINT2     for 554_1    int  */
#pragma  interrupt(EPAOvr2_3_int  =0x1c)    /*  EPAOvr2_3  for None     int  */
#pragma  interrupt(EPAOvr0_1_int  =0x1b)    /*  EPAOvr0_1  for None     int  */
#pragma  interrupt(EPA3_int       =0x1a)    /*  EPA3       for None     int  */
#pragma  interrupt(EPA2_int       =0x19)    /*  EPA2       for None     int  */
#pragma  interrupt(soft1ms_int    =0x18)    /*  EPA1       for soft_1ms int  */
#pragma  interrupt(port_AD_intr   =0x07)    /*  EPA0       for 554_0    int  */
#pragma  interrupt(SIO_int        =0x06)    /*  SIORec     for SIORec   int  */
#pragma  interrupt(SIO_int        =0x05)    /*  SIOTran    for SIOTran  int  */
//#pragma  interrupt(exint1_int     =0x04)    /*  EXINT1     for CAN_1    int  */
//#pragma  interrupt(exint0_int     =0x03)    /*  EXINT0     for CAN_0    int  */
#pragma  interrupt(reserved_int   =0x02)    /*  reserve    for None     int  */
#pragma  interrupt(T2OverFlow_int =0x01)    /*  T2Ovr      for None     int  */
#pragma  interrupt(T1OverFlow_int =0x00)    /*  T1Ovr      for None     int  */

//-��ϵͳ�ж��ܽ�:����554������һ���ж�(���ⲿ),1ms���жϲ�֪������ô������(��ʱ��1��ʱ����),����һ��SIO���շ����ж�
//-�ܹ��ǿ�����6���ж�,����Ĵ��ڽ��պͷ����ж��Ǻ�Һ�����ϵ�Ƭ��ͨѶ�õ�.





/*---------------------------------------------------------------------------*/
/*                    Definition  of  global  variables                      */
/*---------------------------------------------------------------------------*/

//    none


/*---------------------------------------------------------------------------*/
/*                    Definition  of  local  variables                       */
/*---------------------------------------------------------------------------*/

//    none




/*---------------------------------------------------------------------------*/
/*                        IMPORT            functions                        */
/*---------------------------------------------------------------------------*/

extern void set_receive();
extern void set_send();






//-�����ж��Ѿ����Խ�����,������û���յ��ַ����������һֱ���ᷢ���жϵ�,���յ���Ч
//-�ַ�������Ч�ַ�(��Ҫ����)ʱ�Żᷢ���ж�
/*---------------------------------------------------------------------------*/
/*                        LOCAL             functions                        */
/*---------------------------------------------------------------------------*/

/***********************************************/
/* P554_int_proc      procedure                */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void P554_int_proc(BYTE the_port_no)
{
         BYTE  ram_axl;
         BYTE  ram_axh;
         BYTE  the_port_no_mirr;
         BYTE  the_port_rece_temp;
    far  BYTE *p554_base_addr;
    far  BYTE *the_ram_base_addr;
    
    //-���е���Щ����,�϶��Ƿ��ϳ�ʶ��,���������ŵ�,��Ȼ���ܲ�һ���������������
    //-���ǿ϶����������п�,���������޷����͵�,�������������Ŀ�ľ���Ϊ��ʹ��
    //-�����Ҷ��ڷ��ͱ��ּĴ���Ϊ�յĽ�����������(������ôʵ�ֵ���˵�����,����Ӧ�õ�
    //-Ŀ��֮һ�϶������ֿ����Դ���)���ǵ������˱��ּĴ���Ϊ���ж�֮��,������жϱ�
    //-�����,Ҫ���ж��ٴα����ҵ�ǰ���������ǶԱ������ͱ��ּĴ���д������һ������
    //-�����̸�жϱ������,
    p554_base_addr = (far  BYTE *)Register_554[the_port_no];
    ram_axh = p554_base_addr[P554_IIR];	//-�ж�ʶ��Ĵ���,,��ζ���������˷��ͱ��ּĴ����ж�,Ҫ���ٴγ���,����������THRд1���ַ�

	//-IIR0 ������ʱ˵���жϴ���δ��״̬
	//-IIR1	ȷ��������ȼ���δ���ж�
	//-IIR2
    if((ram_axh & 0x07)==0x04)     // receive int
    {
        ram_axh=0;
        ram_axl=p554_base_addr[P554_LSR];	//-***�м�ֻҪ��һ������ԾͲ�Ҫ��ȥ�����������ֲ���,����������ԭ��
        if((port_mirror[the_port_no] & 0x10)!=0)
        {
            the_port_no_mirr=port_mirror[the_port_no] & 0x0f;	//-Ŀ�ĺ���
        }
        else
        {
            the_port_no_mirr=0xff;
        }

        while((ram_axl & 0x01)!=0)	//-ֵΪ1˵�����ݽ��յ���,CPU�����Զ�����
        {
            the_port_rece_temp=p554_base_addr[P554_RBR];	//-�������յ�����ֵ����������,����ǽ��յ������ݴ�
            
            port_recv[the_port_no][port_recv_pt[the_port_no]] = the_port_rece_temp;	//-ʵ�ʽ��յ����ݱ������������
            port_recv_pt[the_port_no]++;	//-�ɹ�����һ���ֽڵ�����,���ֵ�ͼ�һ��
            port_recv_pt[the_port_no] &= 0x1ff;
            //-���ڵļ򵥳����������û�д�����,ֱ��������	
            if(the_port_no_mirr<0x0c)
            {
                port_send[the_port_no_mirr][ram_axh]=the_port_rece_temp;
                ram_axh++;
            }
            else
            {
            	if(the_port_no_mirr==0x0e)
            	{
            		set_port_rece_send_buf[set_port_rs_buf_sav_pt]=the_port_rece_temp;
            		set_port_rs_buf_sav_pt++;
            		if(set_port_rs_buf_sav_pt==set_port_rs_buf_tak_pt) set_port_rs_buf_tak_pt++;
            	}
            }
            
            if((ram_axl & 0x0c)!=0)  // not no parity error, frame error;
            {
                //save to rcd_info_myself ???
                SIO_CAN_Need_Reset=SIO_CAN_Need_Reset | (0x0001<<the_port_no);   // bit x
            }
    
            ram_axl=p554_base_addr[P554_LSR];	//-������״̬���Ƿ���������Ҫ����,����һ���������ù���
        }
            
        if(ram_axh!=0)
        {
            port_send_len[the_port_no_mirr]=ram_axh;
            port_send_begin_no_monitor(the_port_no_mirr);
        }
    }
    else
    {
        if((ram_axh & 0x07)==0x02) // transmit int
        {
            if(port_send_pt[the_port_no]<port_send_len[the_port_no])	//-С��˵����������Ҫ����
            {
                the_ram_base_addr=&port_send[the_port_no][port_send_pt[the_port_no]];
                if( (port_send_pt[the_port_no]+8)<port_send_len[the_port_no] )
                {	//-��ʾ���ٿ���һ���Է���8���ֽ�
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[0];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[1];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[2];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[3];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[4];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[5];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[6];
                    the_ram_base_addr++;
                    p554_base_addr[P554_THR]=*the_ram_base_addr;//[7];
           
                    port_send_pt[the_port_no]=port_send_pt[the_port_no]+8;
                }
                else
                {
                    do
                    {
                        p554_base_addr[P554_THR]=the_ram_base_addr[0];	//-���������ּĴ���,,��Щ����,�����Ѿ�����˵����,Ҳ����˵������,�����Ͷ���
                        the_ram_base_addr++;
                        port_send_pt[the_port_no]++;
                    }
                    while(port_send_pt[the_port_no]<port_send_len[the_port_no]);
                }
            }
            else
            {
/*                if(the_port_no==0x0b)
                {
                    p554_base_addr[P554_MCR]=0x08;//0x0a;
                }
                else
                {*/
                    P554_Port_Tran_Close_RTS_Cn[the_port_no]=Time_1ms_Counter
                                                            +P554_XTAL16M_CLOSE_RTS_DELAY[port_info[the_port_no].bauds];
                    P554_Port_Transing_Lastbyte[the_port_no]=YES;
//                }    
                port_mon[the_port_no]=0;
            }
        }
    }
    
}


/*---------------------------------------------------------------------------*/
/*                       PUBLIC             functions                        */
/*---------------------------------------------------------------------------*/

/***********************************************/
/* interrupt NMI     for none   svr procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void NMI_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt EXINT3  for 554_2  svr procedure  */
/***********************************************/
//-ͬ��������оƬ,�����������ź����ǲ�һ����,��������ʵ�ֵĹ���Ҳ�ǲ�ͬ��
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void port_IL_intr(void)
{
    BYTE  port_noiii;

    RAM_CPU_Int_Moni.EXINT3_INT_DEAD_COUNTER=0;
    
re_check_int_com2:
    for(port_noiii=8;port_noiii<12;port_noiii++)
    {
        P554_int_proc(port_noiii);	//-ÿ��554оƬ��4��ͨ��,һ����������
    }

    port_noiii=p3_pin;
    if((port_noiii & 0x80)==0x00) //-��Գ����жϵ����,�����жϷ�������������ж���ʧ��
    	goto re_check_int_com2;
    port_noiii=p3_pin;
    if((port_noiii & 0x80)==0x00) 
    	goto re_check_int_com2;

}



/***********************************************/
/* interrupt EXINT2  for 554_1  svr procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void port_EH_intr(void)
{
    BYTE  port_noii;

    RAM_CPU_Int_Moni.EXINT2_INT_DEAD_COUNTER=0;
    
re_check_int_com1:
    for(port_noii=4;port_noii<8;port_noii++)
    {
        P554_int_proc(port_noii);
    }

    port_noii=p3_pin;
    if((port_noii & 0x40)==0x00) 
    	goto re_check_int_com1;
    port_noii=p3_pin;
    if((port_noii & 0x40)==0x00) 
    	goto re_check_int_com1;

}



/***********************************************/
/* interrupt EPAOvr2_3 for none svr procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void EPAOvr2_3_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt EPAOvr0_1 for none svr procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void EPAOvr0_1_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt EPA3  for none    svr  procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void EPA3_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt EPA2  for none    svr  procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void EPA2_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt soft1ms_int svr procedure         */
/***********************************************/
//-����ж�����ô������,���ⲿ�ź�,�����ڲ�������׽?
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void soft1ms_int(void)   // EPA1
{
    register WORD  reg_ibx;
    far BYTE *ram_base_addr;


//  process 1ms run
    reg_ibx=REG_TimeTemp+EPA1_PERIOD;

    REG_TimeTemp=timer1;

    epa1_time=timer1+EPA1_PERIOD;	//-�����´��жϵ�ʱ��

    REG_Surplus_Time=REG_Surplus_Time+REG_TimeTemp-reg_ibx;

    while(REG_Surplus_Time>=EPA1_PERIOD)
    {
        REG_Surplus_Time=REG_Surplus_Time-EPA1_PERIOD;
        REG_1Msecond++;
    }
  
    REG_1Msecond++;

    Clock_Process();
    
    Time_1ms_Counter++;
    
    if((Time_1ms_Counter & 0x7ff)==0) Time_2048ms_Counter++;
    
//  process CAN transmit

//  process 554 transmit
    for(reg_ibx=0;reg_ibx<12;reg_ibx++)
    {
        //if(Port_Info[reg_ibx].PORT_SEMIDUPLEX==YES)
        {
            if(P554_Port_Transing_Lastbyte[reg_ibx]==YES)
            {
                if(Time_1ms_Counter==P554_Port_Tran_Close_RTS_Cn[reg_ibx])
                {
                    port_send_pt[reg_ibx]++;
                    ram_base_addr=(far BYTE *)Register_554[reg_ibx];
                    
//                    if(reg_ibx==0x0b)
//                    {
//                        ram_base_addr[P554_MCR]=0x08;//0x0a;
//                    }
//                    else
//                    {
//                        ram_base_addr[P554_MCR]=0x0a;//0x08;          // disable txd, enable rxd.
//                    }   
                    if(reg_ibx!=10)
                    	ram_base_addr[P554_MCR]=0x08;	//-�ⲿ����ͨ���жϱ�ʹ��,ʹ485���ڽ���״̬,��鵽û�з��͵�ʱ��
                    if(reg_ibx==7)
                    	ram_base_addr[P554_MCR]=0x0a;
                    P554_Port_Transing_Lastbyte[reg_ibx]=NO;
                }
            }
        }
    }
    
    
//  process wachdog 
    RAM_CPU_Int_Moni.EPA1_INT_DEAD_COUNTER=0;
    
    RAM_CPU_Int_Moni.MAINLOOP_DEAD_COUNTER++;
    if(RAM_CPU_Int_Moni.MAINLOOP_DEAD_COUNTER>50000)  // 50s
    {
        Rcd_Info_Myself_Tmp[0]=0;
        Rcd_Info_Myself_Tmp[1]=RCD_INFO_MYSELF_AREA0_CPU;
        Rcd_Info_Myself_Tmp[2]=RCD_INFO_MYSELF_AREA1_CPU_MAINLOOP;
        Rcd_Info_Myself_Tmp[3]=RCD_INFO_MYSELF_AREA2_CPU_RUN_DEAD;
        Rcd_Info_Myself_Tmp[4]=0;
        Rcd_Info_Myself_Tmp[5]=0;
        Rcd_Info_Myself_Tmp[6]=0;
        Rcd_Info_Myself_Tmp[7]=0;
        Store_Rcd_Info_Myself();
        
        RAM_CPU_INT_Rst_Cn[6]++;
        while(1) {};
    }
}


/***********************************************/
/* interrupt EPA0  for 554_0   svr  procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void port_AD_intr(void)
{
    BYTE  port_noi;
    WORD  ram_ax;


    ram_ax=epa_time0;
    ram_ax=epa_time0;

    RAM_CPU_Int_Moni.EPA0_INT_DEAD_COUNTER=0;
    
re_check_int_com0:
    for(port_noi=0;port_noi<4;port_noi++)
    {
        P554_int_proc(port_noi);
    }

    port_noi=p1_pin;
    if((port_noi & 0x01)==0x00) 
    	goto re_check_int_com0;	//-Ϊʲô��Ҫ���ڸߵ�ƽ���ܽ����жϴ�����?
    port_noi=p1_pin;
    if((port_noi & 0x01)==0x00) //-Ҳ����554û��һ���ж��ź���Ч,ȫΪ0
    	goto re_check_int_com0;

}



/***********************************************/
/* interrupt SIOReceive   svr       procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//void SIOReceive_int(void)
//{
    
//}



/***********************************************/
/* interrupt SIOTransmit  svr       procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
//void SIOTransmit_int(void)
//{

//}



/***********************************************/
/* interrupt SIO          svr       procedure  */
/***********************************************/
//-����Ҳ�Ǽ��䲻һ���ĵط�,һ�����ʹ��һ���ж�,����ʹ�������жϴ������е�����
//-��������ʹ�õ��ж�,ȴ���廨����,����������������,���ⲿ�ж�,�в�׽�ж�,���ڲ��ȵ�
//-����Ҳ������͸�������һ������,����������
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void SIO_int(void)	//-����������õ�Ŀ�Ŀ��ܾ���Ϊ�����س���ʹ�õ�
{
    unsigned char sp_sta_img;

    sp_sta_img = sp_status;
    if((sp_sta_img & 0x40)!=0)     //RI
    {
        set_receive();
    }
    //else
    if((sp_sta_img & 0x20)!=0)  //TI
    {
        set_send(); 
    }
}




/***********************************************/
/* interrupt reserved     svr       procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void reserved_int(void)
{
    // do nothing
}




/***********************************************/
/* interrupt T2OverFlow   svr       procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void T2OverFlow_int(void)
{
    // do nothing
}



/***********************************************/
/* interrupt T1OverFlow   svr       procedure  */
/***********************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
void T1OverFlow_int(void)
{
    // do nothing
}









/*---------------------------------------------------------------------------*/
/*                        PUBLIC            functions                        */
/*---------------------------------------------------------------------------*/

//    none
