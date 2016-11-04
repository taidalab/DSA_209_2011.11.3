/*****************************************************************************/
/*       FileName  :   MOD_KG.C                                              */
/*       Content   :   DSA-208 MOD_KG Module                                 */
/*       Date      :   Wed  07-11-2002                                       */
/*                     DSASoftWare(c)                                        */
/*                     CopyRight 2002             DSA-GROUP                  */
/*****************************************************************************/
//-�������ͨѶ��Ϊ�ɼ������������


#pragma  regconserve

#include _SFR_H_
#include _FUNCS_H_

#include "common.h"
#include "device.h"
#include "reg.h"
#include "ram.h"
#include "rom.h"
#include "comuse.h"




    
    
    
/*---------------------------------------------------------------------------*/
/*                    Definition  of  global  variables                      */
/*---------------------------------------------------------------------------*/

//    none


/*---------------------------------------------------------------------------*/
/*                    Definition  of  local  variables                       */
/*---------------------------------------------------------------------------*/
register  far  WORD                         *MOD_KG_PARA_BASE_ADDR;	//-���Ӧ���Ƕ����ȫ�����,����ķ��������ϵͳ�Ͽɵ�

#define   MOD_KG_PORT_NO                     port_no

#define   MOD_KG_YC_FRAME                   0x01	//-��̫������,�е����־λ
#define   MOD_KG_YX_FRAME                   0x02
#define   MOD_KG_TS_FRAME                   0x04 
#define   MOD_KG_YK_FRAME                   0x08

#define   TIME_MOD_KG_VERIFY_TIME_VALUE     150    // 2048ms*150=307s
#define   MOD_KG_WAIT_TIME_VALUE            2000    //�ȴ�����ʱ�� 1ms*500
#define   MOD_KG_YK_FX_WAIT_TIME_VALUE      10    //ң�صȴ���Уʱ�� 2048ms*15

#define   MOD_KG_begin_veriiedclk_time           (*(MOD_KG_PARA_BASE_ADDR+0x0000))
#define   MOD_KG_rxd_wait_time                   (*(MOD_KG_PARA_BASE_ADDR+0x0004))
#define   MOD_KG_transmit_disable_begin_time     (*(MOD_KG_PARA_BASE_ADDR+0x0005))
#define   MOD_KG_rxd_head_flag              byte0(*(MOD_KG_PARA_BASE_ADDR+0x0006))
#define   MOD_KG_transmit_control           byte1(*(MOD_KG_PARA_BASE_ADDR+0x0006)) 
#define   MOD_KG_comm_err_counter                 (MOD_KG_PARA_BASE_ADDR+0x0028) //0x0008-0x0027 
//receive
#define   MOD_KG_transmit_yk_host_port      byte0(*(MOD_KG_PARA_BASE_ADDR+0x0048))
#define   MOD_KG_transmit_task_busy         byte1(*(MOD_KG_PARA_BASE_ADDR+0x0048))
#define   MOD_KG_now_poll_addr              byte0(*(MOD_KG_PARA_BASE_ADDR+0x0049))
#define   MOD_KG_transmit_flag              byte1(*(MOD_KG_PARA_BASE_ADDR+0x0049))
#define   MOD_KG_rec_OK                     byte0(*(MOD_KG_PARA_BASE_ADDR+0x004a))
#define   MOD_KG_begin_addr                 byte0(*(MOD_KG_PARA_BASE_ADDR+0x004b))
#define   MOD_KG_end_addr                   byte1(*(MOD_KG_PARA_BASE_ADDR+0x004b))
#define   MOD_KG_YK_fx_wait_time                 (*(MOD_KG_PARA_BASE_ADDR+0x004c))
#define   MOD_KG_YK_fx_flag                 byte0(*(MOD_KG_PARA_BASE_ADDR+0x004d))
#define   MOD_KG_YK_poll_addr               byte1(*(MOD_KG_PARA_BASE_ADDR+0x004d))
#define   MOD_KG_transmit_wait_time              (*(MOD_KG_PARA_BASE_ADDR+0x004e))
#define   MOD_KG_begin_zf_disable_time           (*(MOD_KG_PARA_BASE_ADDR+0x004f))
#define   MOD_KG_wait_replay                byte0(*(MOD_KG_PARA_BASE_ADDR+0x0050))
#define   MOD_KG_rec_frame_type             byte1(*(MOD_KG_PARA_BASE_ADDR+0x0050))



/*---------------------------------------------------------------------------*/
/*                        IMPORT            functions                        */
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/*                        LOCAL             functions                        */
/*---------------------------------------------------------------------------*/
/************************************************/
/* MOD_KG_MOD_KG_CRC16_cal         function              */
/************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
unsigned int MOD_KG_CRC16(unsigned char *MOD_KG_CRC16_start,unsigned char MOD_KG_CRC16_bytes)    //*xΪָ��ÿ��ǰ5�����ݵ�ָ��
{	//-��������У��CRC
unsigned int bx, cx, i, j;

    bx = 0xffff;
    cx = 0xa001;
    for(i=0;i<MOD_KG_CRC16_bytes;i++)
     {
      bx=bx^MOD_KG_CRC16_start[i];
      for(j=0;j<8;j++)
       {
        if ((bx&0x0001)==1)
         {
          bx=bx>>1;
          bx=bx&0x7fff;
          bx=bx^cx;
         }
        else
         {
          bx=bx>>1;
          bx=bx&0x7fff;
         }
       }
     }
    return(bx);
}
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/************************************************/
/* MOD_KG_fcb_check         function              */
/************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static BYTE MOD_KG_not_comm_count(void)
{
    BYTE  the_ram_axl;
    BYTE  the_ram_axh;
    the_ram_axl=port_report[0];
    the_ram_axh=port_send[MOD_KG_PORT_NO][0];	//-���ǵ�0���ݱ�ʾ��ʲô��???
    if(the_ram_axl!=the_ram_axh)
      return YES;
    else 
      return NO;
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/************************************************/
/* MOD_KG_YX_cmd    function           */
/************************************************/
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void MOD_KG_YX_cmd(BYTE slave_adr)  //-������ľ��Ƿ��͸��ն�װ�õ�,���һ��Ǵ�����˿ڷ��ͳ�ȥ��
{
    WORD the_ram_ax;
    
    temp_ptS_B=(far BYTE *)&(port_send[MOD_KG_PORT_NO]);
    temp_ptS_B[0] =slave_adr;
    temp_ptS_B[1] =0x02;	//-������
    temp_ptS_B[2] =0x00; //YX��ʼ��ַ
    temp_ptS_B[3] =0x80;
    temp_ptS_B[4] =0x00;  //����YX������
    temp_ptS_B[5] =0x04;
    the_ram_ax=MOD_KG_CRC16(temp_ptS_B,6);
    temp_ptS_B[6] =byte0(the_ram_ax);
    temp_ptS_B[7] =byte1(the_ram_ax);	//-ֱ���������еı������ݶ��Ѿ�׼������
    port_send_len[MOD_KG_PORT_NO]=8;    
    port_send_begin();	//-��������,���ǽ���������������һ���ж�,ʵ�ʵ�ȫ�����������ж��н��е�
}

/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
/************************************************/
/* MOD_KG_scheduler      function               */
/************************************************/
//-�м�û��ʲô�ϸ������ϵĹ̶�,�����ִ�����߼��Ľ��,������й�
/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
static void MOD_KG_scheduler(void)	//-���ȳ���,���Ǵ����ֳ������ϰ������
{
    #define    delta_len            temp_lp_int
    #define    bv_yx_no             port_check

    far  BYTE *temp_fptS;
         WORD  the_ram_ax;
         WORD  the_ram_bx;
         BYTE  temp_axl;
    
    temp_fptS=(far BYTE *)&(port_send[MOD_KG_PORT_NO]);	//-�÷��Ϳڵĵ�ַ,�Ա�������д����
      
    if((port_recv_pt[MOD_KG_PORT_NO]!=port_recv_dl[MOD_KG_PORT_NO])&&(MOD_KG_wait_replay==YES))	//-˵���������յ����µı���
    {	//-���������,���յ�Ӧ����,�Ǵ����￪ʼ������
       if(MOD_KG_rxd_head_flag==NO)	//-���յ������ݻ�û�д�����ʱ�����NO
       {
           disable();
           if(port_recv_dl[MOD_KG_PORT_NO]>port_recv_pt[MOD_KG_PORT_NO])	//-ǰ����Ǵ���ָ���ʵ�ʵĽ���ָ����бȽ�
               delta_len=(port_recv_pt[MOD_KG_PORT_NO]+512)-port_recv_dl[MOD_KG_PORT_NO];
           else
               delta_len=port_recv_pt[MOD_KG_PORT_NO]-port_recv_dl[MOD_KG_PORT_NO];	//-һ���ĳ���
           enable();
           for(temp_loop=port_recv_dl[MOD_KG_PORT_NO];temp_loop<(delta_len+port_recv_dl[MOD_KG_PORT_NO]);temp_loop++)
           {
        	   if(port_recv[MOD_KG_PORT_NO][temp_loop]==port_send[MOD_KG_PORT_NO][0])	//-����ط��Ƚϵ��Ǵ�վ��ַ,�����Ҿ���û���κι��ɾ���ͨѶ
        	   {	//-����һ�п������õ�
        	     the_ram_ax=(port_recv_dl[temp_loop]+1)&0x1ff;
        	     if(port_recv[MOD_KG_PORT_NO][the_ram_ax]==port_send[MOD_KG_PORT_NO][1])	//-�Ƚϵ��ǹ�����
        	     {
        	         MOD_KG_rxd_head_flag=YES;	//-��ʾ�Ѿ��ɹ�ʶ����յ����±��ĵ�ͷ��
        	         break;
        	     }
        	   }  
        	   port_recv_dl[MOD_KG_PORT_NO]++;	//-����һ���ֵı���
        	   port_recv_dl[MOD_KG_PORT_NO]&=0x1ff;       	        	 
           }
       }
       if(MOD_KG_rxd_head_flag==YES)	//-�����ǲ�һ���ķ��
       {
           disable();
           if(port_recv_dl[MOD_KG_PORT_NO]>port_recv_pt[MOD_KG_PORT_NO])
               delta_len=(port_recv_pt[MOD_KG_PORT_NO]+512)-port_recv_dl[MOD_KG_PORT_NO];
           else
               delta_len=port_recv_pt[MOD_KG_PORT_NO]-port_recv_dl[MOD_KG_PORT_NO];
           enable();
           if(delta_len>=3)
           {
               the_ram_ax=(port_recv_dl[MOD_KG_PORT_NO]+1)&0x1ff;
               if(port_recv[MOD_KG_PORT_NO][the_ram_ax]==port_send[MOD_KG_PORT_NO][1])	//-������
               {
                    temp_int=(port_recv_dl[MOD_KG_PORT_NO]+2)&0x1ff;
                	if(delta_len>=(unsigned short)(port_recv[MOD_KG_PORT_NO][temp_int]+5))	//-�õ��ı��ĳ��Ⱥ������ϵı��ĳ��Ƚ��бȽ�
                    {
                  	    MOD_KG_rxd_head_flag=NO;
                  	    MOD_KG_wait_replay=NO;
                  	    MOD_KG_rec_OK=YES;             
                  	    goto rec_ok_deal;	//-�������ؿ���,���������Ϊ�ǳɹ����յ�һ�����ر�����
                    }
               }
               else if(delta_len>=8)
               {	//-�����벻ͬ��ôҲ��Ϊ�ǳɹ����յ�����,�ѵ���˵,������ܾ�����׼�����յ���һ�ַ��ر���,�����벻ͬ�Ĵ����ط�
                  	MOD_KG_rxd_head_flag=NO;
                  	MOD_KG_wait_replay=NO;
                  	MOD_KG_rec_OK=YES;             
                  	goto rec_ok_deal;
               }            
           }
       }                      
    }         
    goto rxd_out_time;
rec_ok_deal: 
    if(MOD_KG_rec_OK==YES)	//-�϶��Ƿ�ֹ�ܷɵ�.
    {	//-������Ϳ���˵��Ӧ�����Ѿ�������
        MOD_KG_rec_OK=NO;	//-�ɹ����յ������ݿ�ʼ������֮��,�ͻָ�0
        MOD_KG_transmit_flag=YES;
        MOD_KG_transmit_wait_time=Time_1ms_Counter;	//-��Ȼ����˵���Է���������,���ǻ���Ҫ��ʱһ��ʱ��,��Ϊ���յ���������Ҫ����
        //-�����Ƕ�ʵ�����ݵĴ���,�������Ǻ��Ĳ���
        the_ram_bx=(port_recv_dl[MOD_KG_PORT_NO]+1)&0x1ff;;
        if(port_recv[MOD_KG_PORT_NO][the_ram_bx]!=0x05)	//-����ǶԹ�������ж�,�����벻ͬ�жϵ�����Ҳ��һ��
        {	//-����������һ�ִ������ڿ��Բ���
          	the_ram_ax=(port_recv_dl[MOD_KG_PORT_NO]+2)&0x1ff;
          	temp_int=port_recv[MOD_KG_PORT_NO][the_ram_ax]+5+port_recv_dl[MOD_KG_PORT_NO];        
          	for(temp_loop=port_recv_dl[MOD_KG_PORT_NO];temp_loop<temp_int;temp_loop++)	//-���������ɵ����ܾ��Ǳ�֤��λ����Ҫ�����ı����ֽ�
          	{	//-�򵥵Ĳ���Ҫ�����������Ǹ��ӵĻ�����Ҫ��,��ô�������˵û������Ծͻ�ܺ�
                 if(temp_loop<=511)
           	       port_report[temp_loop-port_recv_dl[MOD_KG_PORT_NO]]=port_recv[MOD_KG_PORT_NO][temp_loop];
                 else
           	       port_report[temp_loop-port_recv_dl[MOD_KG_PORT_NO]]=port_recv[MOD_KG_PORT_NO][temp_loop-512];	//-�ѵ��Ǹ��ٸ��µ�Ե����Ҫ��ǰ���Ƴ���
          	}	//-���߻���һ�ֿ����Ծ���ͳһ����
         	port_recv_dl[MOD_KG_PORT_NO]+=delta_len;	//-����ط��������������Ĵ�������
         	port_recv_dl[MOD_KG_PORT_NO]&=0x1ff;
         	temp_int=MOD_KG_CRC16(&port_report[0],port_report[2]+3);
         	if((byte0(temp_int)!=port_report[port_report[2]+3])||(byte1(temp_int)!=port_report[port_report[2]+4]))	//-����CRC���
          		goto inrda;	//-������˵���ɹ����յ��ı���CRCУ��û��ͨ��
        }
        else
        {
         	temp_int=port_recv_dl[MOD_KG_PORT_NO]+8;
         	for(temp_loop=port_recv_dl[MOD_KG_PORT_NO];temp_loop<temp_int;temp_loop++)
            {
                 if(temp_loop<=511)
                      port_report[temp_loop-port_recv_dl[MOD_KG_PORT_NO]]=port_recv[MOD_KG_PORT_NO][temp_loop];
                 else
                      port_report[temp_loop-port_recv_dl[MOD_KG_PORT_NO]]=port_recv[MOD_KG_PORT_NO][temp_loop-512];
            }
            port_recv_dl[MOD_KG_PORT_NO]+=delta_len;	//-������Ϳ��԰ѽ��ջ������е�����������,�Ѿ�����������
            port_recv_dl[MOD_KG_PORT_NO]&=0x1ff;
            temp_int=MOD_KG_CRC16(&port_report[0],6);
            if((byte0(temp_int)!=port_report[6])||(byte1(temp_int)!=port_report[7]))
                goto inrda;	//-У��Ͳ���ȷֱ������,��ôû���յ���Ч���ľ�ֱ�ӽ����¸�װ����???
        }
        
        if(MOD_KG_comm_err_counter[port_report[0]]!=0)
        {
            MOD_KG_comm_err_counter[port_report[0]]=0;	//-����˵���˱�����������������Ч.
            temp_int=port_report[0]+MOD_KG_begin_addr-byte0(port_info[MOD_KG_PORT_NO].scan_panel);	//-�ø�PORT�����豸����Ӧ208�ڲ����ݿ��ڵĵ�Ԫ��ַ
            YX_State[IED_TX_STATE_START_WORD+temp_int/16]&=(0xffff-(0x0001<<(temp_int%16)));	//-վ�������豸(IED)�Ĺ�����ΪYX�洢,,�Ǹ���Ԥ�����ң����ʼ��
        }    //-0��ʾͨѶ����
    }
    switch(MOD_KG_rec_frame_type)	//-��ֵ���ڷ��ͱ��ĵ�ʱ�����,ָ���Ƿ�����Ҫ���յı�����ʽ
    {
          case MOD_KG_YX_FRAME:       //yx
                  Core_Src_Unit=port_report[0]+MOD_KG_begin_addr-byte0(port_info[MOD_KG_PORT_NO].scan_panel);	//-�ø�PORT�����豸����Ӧ208�ڲ����ݿ��ڵĵ�Ԫ��ַ
                  Core_Src_Pt_B=(BYTE *)&port_deal_buf[MOD_KG_PORT_NO][0];	//-�����н��ջ�����,�����б��滺����,���ڴ����˻��д���������
                  Core_Src_Len=2*unit_info[Core_Src_Unit].yx_num;	//-ÿһ����Ԫ����һ�����������ݿ�,���ݿ�˵���˾�������,,˵���˵�λ����
                  for(temp_loop=0;temp_loop<unit_info[Core_Src_Unit].yx_num;temp_loop++)
                  {	//-������������ң���ڱ�����,���滹û�п�,�Ҳ����Ȱ��ϵ��ݴ�,���жϴ���,�����ǿɿ��Ĵ���Ŀ��
                      Core_Src_Pt_B[2*temp_loop]=byte0(YX_State[unit_info[Core_Src_Unit].yx_start+temp_loop]);	//-���ݿ���һ��Ԫ�ؾ���һ������,ֻҪ���岻�ص�����
                      Core_Src_Pt_B[2*temp_loop+1]=byte1(YX_State[unit_info[Core_Src_Unit].yx_start+temp_loop]);
                  }
                  Core_Src_Pt_B=(BYTE *)&port_deal_buf[MOD_KG_PORT_NO][0];
                  //-�Դ���ܶණ��,��������Ŀ�Ĳ�ͬ,��һ��������ȷ��,����˵����
                  for(temp_loop=0;temp_loop<unit_info[Core_Src_Unit].yx_num;temp_loop++)
                  {
                      Core_Src_Pt_B[2*temp_loop]=port_report[temp_loop*2+5];	//-û���κι̶��Ķ���,ʵ�ֹ��ܾ���,,ȡ��Ч��Ϣ
                      Core_Src_Pt_B[2*temp_loop+1]=port_report[temp_loop*2+5+1];	//-��һ��ȥͷֻ����Ҫ��������Ч����,����ط�ֱ��������һ���ֵ�ң����
                  }
                  core_update_YX();    
                  break;
          default:
             break;
    }
     //-���ͱ��ĳ�ʱû���յ�������Ϣ,��������³�ʼ��Ӳ��,��������һ��ͨѶ�����¼����,Ҳ����˵,����������λ��,����û��
     //-����λ���ҽ���ȥ,������һ��״̬��¼,û�еõ��µ�����,����������ݿ����,�����Ĳ�����Ӱ��     
rxd_out_time:	//-ִ�е�����˵�����ճ�ʱ,�����ɹ�,�������ڿ��Լ���������,,�����������ִ�������,����
    if(Judge_Time_In_MainLoop(MOD_KG_rxd_wait_time,MOD_KG_WAIT_TIME_VALUE)==YES)	//-����ͨ�ŵĻ��ǲ�Ӧ�ó�ʱ��,����ʱ����Ϊ����,�������³�ʼ��
    {	//-������һ������֮��,�����ȴ��ش�ʱ��֮��,�Ϳ��Դ��·���һ��
      	MOD_KG_rec_OK=NO;
      	MOD_KG_rxd_wait_time=Time_1ms_Counter; 
      	MOD_KG_transmit_flag=YES;	//-��ʾ���ڿ�����֯��������
      	MOD_KG_wait_replay=NO;	//-��ʾ���ڻ�û�еȴ��ظ�
      	MOD_KG_transmit_wait_time=Time_1ms_Counter;
//      if(MOD_KG_not_comm_count()==YES)
//       {
       	MOD_KG_comm_err_counter[port_send[MOD_KG_PORT_NO][0]]++;	//-��ʱ��������
        if(MOD_KG_comm_err_counter[port_send[MOD_KG_PORT_NO][0]]>3)
        {
           MOD_KG_comm_err_counter[port_send[MOD_KG_PORT_NO][0]]=0;
           temp_int=port_send[MOD_KG_PORT_NO][0]+MOD_KG_begin_addr-byte0(port_info[MOD_KG_PORT_NO].scan_panel);	//-�ø�PORT�����豸����Ӧ208�ڲ����ݿ��ڵĵ�Ԫ��ַ
           YX_State[IED_TX_STATE_START_WORD+temp_int/16]|=(0x0001<<(temp_int%16));	//-�ѵ����ң�Ų�����ͨ��YX,��ô����Ҳ�������¼,�ǲ���˼ά�����˰�      	
       	}
//       }
    }
inrda:	//-CRC��ͨ��Ҳ�п����ߵ�����
    if(MOD_KG_transmit_flag==YES)
    {	//-������������������ʵĿ�ʼ�ط�
     	if(Judge_Time_In_MainLoop(MOD_KG_transmit_wait_time,80)==YES)	//-ֻ�й��˵ȴ�ʱ��Żᷢ��
           switch(MOD_KG_transmit_control)
           {
                case 1:
                     MOD_KG_rec_frame_type=MOD_KG_YX_FRAME;	//-��ʾ��Ҫ���յ�֡�����������ֵ,���յ�������ֵ������ȷ��
                     MOD_KG_wait_replay=YES;
                     MOD_KG_YX_cmd(MOD_KG_now_poll_addr);	//-����Լ�����������,�ʴ��͵�,����˿��ϵ�����װ�ö��ǽ��ղ�ѯ
                     MOD_KG_now_poll_addr++;	//-�ӻ���ַ
                     disable();
      	             port_recv_pt[MOD_KG_PORT_NO]=0;	//-���С��ʼ����Ϊ�˽��մ��������õ�
   	             port_recv_dl[MOD_KG_PORT_NO]=0;
   	             enable();
                     MOD_KG_transmit_flag=NO;	//-���ڿ�ʼ���ܷ��ͱ�����
                     MOD_KG_rxd_head_flag=NO;	//-Ϊ��ͬ��׼����,���ǲ��������еĶ�����������
                     MOD_KG_rxd_wait_time=Time_1ms_Counter;	//-Ӧ����Ҳ����ʱЧ�Ե�
                     if(MOD_KG_now_poll_addr>byte1(port_info[MOD_KG_PORT_NO].scan_panel))	//-�������豸�ĵ�ַ(�����)��Χ HIGH byte:end - LOW byte:start
                     {
                         MOD_KG_now_poll_addr=byte0(port_info[MOD_KG_PORT_NO].scan_panel);	//-�����е�ַ����ѭ������
                         MOD_KG_transmit_control=1;	//-���еĶ˿�װ�ò�ѯһ��֮��,�ٻ���һ�౨�Ĳ�ѯ
                     }              
                     break;
                default:              
                     break;
           }
    }

}


/*---------------------------------------------------------------------------*/
/*                        PUBLIC            functions                        */
/*---------------------------------------------------------------------------*/
/************************************************/
/* MOD_KG_Init       function                   */
/************************************************/
/*===========================================================================*/
void MOD_KG_Init()
{
    BYTE temp_axl;        
    MOD_KG_PARA_BASE_ADDR=(far WORD *)&port_flag[port_no];

    MOD_KG_begin_veriiedclk_time=Time_2048ms_Counter;
    MOD_KG_rxd_wait_time=Time_1ms_Counter;
    MOD_KG_transmit_task_busy=NO;
    MOD_KG_wait_replay=NO;
    MOD_KG_begin_addr=byte0(port_info[MOD_KG_PORT_NO].mirror_unit); 
    MOD_KG_end_addr=byte1(port_info[MOD_KG_PORT_NO].mirror_unit); 
    MOD_KG_now_poll_addr=byte0(port_info[MOD_KG_PORT_NO].scan_panel);	//-��ʼ����ʱ��ȡ��С�ӻ���ַ
    MOD_KG_rxd_head_flag=NO;
    MOD_KG_transmit_flag=YES;	//-���ֵ��ʾ���ڿ��������˿ڼ䴫����Ϣ������
    MOD_KG_transmit_wait_time=Time_1ms_Counter;
    MOD_KG_rec_OK=NO;
    MOD_KG_rec_frame_type=0;
    MOD_KG_YK_fx_flag=NO;
    MOD_KG_begin_zf_disable_time=Time_2048ms_Counter;
    disable();
    port_recv_pt[MOD_KG_PORT_NO]=0;
    port_recv_dl[MOD_KG_PORT_NO]=0;
    enable();
    MOD_KG_transmit_control=1;
    for(temp_axl=0;temp_axl<32;temp_axl++)
     MOD_KG_comm_err_counter[temp_axl]=0;   
}

/************************************************/
/* MOD_KG_Main     function                     */
/************************************************/
/*===========================================================================*/
void MOD_KG_Main()
{
    MOD_KG_PARA_BASE_ADDR=(far WORD *)&port_flag[port_no];	//-ÿһ���˿ڶ����Լ���һ��������

    MOD_KG_scheduler();  
    
    if(Judge_LongTime_In_MainLoop(MOD_KG_begin_zf_disable_time,15)==YES) // after initialed 1s, let ZF be enable.  
    {
        Portx_Poll_First[MOD_KG_PORT_NO]=YES;	//-��ʾ�����ڿ��Է���������,����������͵Ļ�
    }
    if(Judge_LongTime_In_MainLoop(MOD_KG_transmit_disable_begin_time,30)==YES)
    {
        MOD_KG_transmit_task_busy=NO;	//-��ʱ���ж�״̬
    }

}


/************************************************/
/* MOD_KG_Monitor     function                  */
/************************************************/
/*===========================================================================*/
void MOD_KG_Monitor()
{
  MOD_KG_PARA_BASE_ADDR=(far WORD *)&port_flag[port_no];	//-ָ��ָ��һ���̶���������,����ļ����������ֵ��ʹ����������ڵĴ洢��Ԫ
  
}
