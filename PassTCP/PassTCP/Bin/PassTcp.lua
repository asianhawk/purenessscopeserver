--����һ����׼��Lua PassTcp�ű�
--�ṩ�����¼��ķ�װ�ͽ��յ������¼��ķ�װ
--�������������֯��ķ������ݺ���֤��������
--����ӿ�������ƣ���Ҫ�ǿ��ǵ����̲߳������������ٲ���Ҫ���߳���
--������������ݿ鲻Ҫ�ڽű���������������㲻���ͷ�
--Ŀǰ���֧��100K�����ݿ��С
--add by freeeyes

--Ҫ�������ݵ��¼�����PassTCP���ã�����ʵ����ķ������ݷ��
function PassTcp_CreateSendData(szData, nLen)
    --������������
	--szDataΪchar* ����Ҫ���Ļ���顣
	--nLen�ǵ�ǰ�������󳤶�
	--���nNextIndex����-1����˵����ֵ����,���򷵻���һ�ε���ʼ��λ��
	
	--���Դ�ӡ
	LuaFn_Tcp_Print(szData, nLen)
	
	--����4�ֽڰ�ͷ
	nPacketSize = 10
	nStartIndex  = 0
	nNextIndex  = 0
	nNextIndex = Lua_Tcp_Buffer_In_Int32(szData, nPacketSize, nStartIndex, nLen)
	
	--����2�ֽڵ�������
	nCommandID = 4096
	nNextIndex = Lua_Tcp_Buffer_In_Int16(szData, nCommandID, nNextIndex, nLen)
	
	--�������ݿ�
	szBuff = '12345678'
	nNextIndex = Lua_Tcp_Buffer_In_Block(szData, szBuff, 8, nNextIndex, nLen)
	
	--������뷵��������ݿ�ĳ��ȣ���PassTCP֪�����������ܳ���
	return nNextIndex
end

--���յ��������¼�
function PassTcp_GetRecvData(szData, nLen)
    --�õ���������
	--szData�ǵ�ǰ�յ������ݿ�
	--nLen���յ������ݿ�鳤��
	--�����ķ��أ���һ��int����
	--Ŀǰ��ô���壬   0���������ݰ��ɹ�����֤�Ϸ����ɹ������ݰ�+1
	--                 1�����յİ�û�н����������������
	--                 2���������ݰ���֤ʧ�ܣ����Ǵ�������ݰ�+1
	
	--�жϽ������ݰ��ĳ���
	nCurrIndex = 0;
	nPacketLength = Lua_Tcp_Buffer_Out_Int32(szData, nCurrIndex, nLen);
	if nPacketLength ~= 10 then
		return 2;
	end
	
	--�жϽ��յķ�������ID�Ƿ���ȷ
	nCurrIndex = 4
	nCommandID = Lua_Tcp_Buffer_Out_Int16(szData, nCurrIndex, nLen);
	if nCommandID ~= 4096 then
		return 2;
	end
	
	--����֤�ɹ�
	return 0;
end

