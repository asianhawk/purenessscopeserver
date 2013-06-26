/*
 * ����壬�Լ�һЩ��������
 * by w1w
 */

#pragma once

#ifndef _COMMANDDEFINE_H
#define _COMMANDDEFINE_H

//����ͻ�������(TCP)
#define COMMAND_REG    0x1000                    //ע��APP�ͻ���
#define COMMAND_REG_RESPONSE    0x1001           //ע������Ӧ��

#define COMMAND_KEEP_ALIVE    0x2000             //����
#define COMMAND_KEEP_ALIVE_RESPONSE    0x2001    //������Ӧ��
#define COMMAND_QUERY_APP    0x2010              //��ѯ��ǰ���ߵ�APP
#define COMMAND_QUERY_APP_RESPONSE    0x2011     //��ѯAPP��Ӧ��
#define COMMAND_SYN_APP    0x2012                // ����APP״̬��
#define COMMAND_QUERY_SERVER    0x2020           //��ѯ������״̬
#define COMMAND_QUERY_SERVER_RESPONSE    0x2021  //��ѯ��������Ӧ��

#define COMMAND_PACKET    0x3000                 //���ݰ�
#define COMMAND_PACKET_RECEIVED    0x3001        //���ݰ���ȷ����
#define COMMAND_PACKET_SERVERDOWN    0x3002      //������������
#define COMMAND_PACKET_APPDOWN    0x3003         //APP������
#define COMMAND_PACKET_REJECT    0x3004          //�ܾ�����
#define COMMAND_PACKET_ERR    0x3005             //���ݰ���֤����
#define COMMAND_PACKET_OUTTIME    0x3006         //���ݰ���ʱ

//��ʼ���ַ���Ϊȫ0
inline void iniChar(char *chars,int l)
{
	for (int i = 0; i<l;++i)
	{
		chars[i]='\0';
	}
}


#endif