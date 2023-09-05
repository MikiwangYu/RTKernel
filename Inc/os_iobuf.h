#pragma once
#ifdef __cplusplus
extern "C" {
#endif
#include "os_def.h"
#include "os_evt.h"
#if OS_EVENT_EN
/**
 * @brief IO���塣
*/
typedef OS_Event OS_IOBuf;
/**
 * @brief ��ʼ��һ����������
 * @param iobuf ��Ҫ��ʼ���Ļ�������
 * @param name ���������ơ�
 * @param data ���������ݴ�Ŵ���
 * @param size ��������С��
*/
void OS_IOBuf_Init(OS_IOBuf iobuf, const char* name, uint8_t* data, uint32_t size);
/**
 * @brief ����һ����������
 * @param name ���������ơ�
 * @param data ���������ݴ�Ŵ���
 * @param size ��������С��
 * @return ��������ɹ�����Ȼ�Ỻ����ָ�룻���򷵻�NULL��
*/
OS_IOBuf OS_IOBuf_Create(const char* name, uint8_t* data, uint32_t size);
/**
 * @brief ����һ����������
 * @param iobuf ��Ҫ���ٵĻ�������
 * @return OS_OK��������ٳɹ���
*/
OS_Error_t OS_IOBuf_Release(OS_IOBuf iobuf);
/**
 * @brief ���Դӻ������ж�ȡ���ݡ�
 * @param iobuf ��Ҫ�����Ļ�������
 * @param data ��ȡ�����ݴ�Ŵ���
 * @param size ��ȡ�������������
 * @param ret_size ʵ�ʶ�ȡ�������������Դ���NULL��
 * @return OS_OK��������ٶ�ȡ��1�ֽ����ݡ�
*/
OS_Error_t OS_IOBuf_Try_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t *ret_size);
/**
 * @brief �ӻ������ж�ȡ���ݡ�
 * @param iobuf ��Ҫ�����Ļ�������
 * @param data ��ȡ�����ݴ�Ŵ���
 * @param size ��ȡ�������������
 * @param ret_size ʵ�ʶ�ȡ�������������Դ���NULL��
 * @param timeout ���Ϊ0�������޵ȴ�������ȴ�timeout��tick�󷵻ء�
 * @return OS_OK��������ٶ�ȡ��1�ֽ����ݣ�OS_TIMEOUT������ȴ���ʱ��
*/
OS_Error_t OS_IOBuf_Read(OS_IOBuf iobuf, uint8_t* data, uint32_t size, uint32_t* ret_size, OS_Tick timeout);
/**
 * @brief ������д�뻺���������Զ�ȡ���ݵ����񲻻������ȡ�����ݣ����ǻ�����ˢ�¡�
 * @param iobuf ��Ҫ�����Ļ�������
 * @param data ��Ҫд������ݡ�
 * @param size д�����������
 * @param ret_size ʵ��д��������������Դ���NULL��
 * @return OS_OK���������д����1�ֽ����ݡ�
*/
OS_Error_t OS_IOBuf_Write(OS_IOBuf iobuf, const uint8_t* data, uint32_t size, uint32_t *ret_size);
/**
 * @brief ˢ�»������������ѱ�������������������ж����ҿ������ж��ӳٷ�������Ὣ�˲������͵����̡߳�
 * @param iobuf ��Ҫˢ�µĻ�������
 * @return OS_OK�����ˢ�³ɹ���
*/
OS_Error_t OS_IOBuf_Flush(OS_IOBuf iobuf);
#endif
#ifdef __cplusplus
}
#endif