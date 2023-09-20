#ifndef _DATA_H
#define _DATA_H

#include "list.h"
#include "main.h"

#define   BELT_ZONE_NUM                    200

#define   ZONE_NUM                         3

#define   MAX_PKG_NUM                      20

#define   VALUE                            1
#define   INVALUE                          0

#define   FINAL_CAR                       0xFFFF
#define   START_CAR                       0xFFFE


#define   PKGQUEUELEN                     30


enum {
	ZONE_TYPE_ONE = 0x01,
	ZONE_TYPE_TWO = 0x02,
	ZONE_TYPE_RISE = 0x03,
};


enum {
	TRANS_ALLOW = 0,                      //����ּ�
	TRANS_FORBID = 1,                     //��ֹ�ּ�
	TRANS_ABORT = 2,                      //ȡ���ּ�
	TRANS_SUCCESS = 3,                    //�ɹ��ּ�
};

#pragma pack (1)

//����
//����ڵ�
typedef struct {
	u16  zoneIndex;                        //������  �����վ�� �ޱ�Ȼ��ϵ�� Ĭ��0xFFFF ��Ч;
	u16  ctrlIndex;                        //������վ�� 
	u16  beltMoudleIndex;                  //�����ӻ�����  ��һ����(0x01) �ڶ�����(0x02) ��������0x03)
	u16  front_zone;                       //ǰ������          �涨ǰ������  
	u16  rear_zone;                        //������
	u16  left_zone;                        //������          �涨�����򣨵��Ͳ��ת ע�ⰲװλ�ã�   
	u16  right_zone;                       //�ҷ�����  
}sData_zone_node;

//��������
typedef struct {
	u16   flashcnt;                               //����flash
	u32   confid;                                 //���õ�id
	u16   pkgtotalnum;                            //�ܹ��ж��ٰ�����
	u16   pkgcurnum;                              //��ǰ�ǵڼ�������
	u16   pkgcal;                                 //���ݰ�ͳ��
	u16   zonetotalnum;                           //�ܹ��ж�������
	u16   zonecurnum;                             //��ǰ�������ж���
	sData_zone_node  zoneNode[BELT_ZONE_NUM];     //����ڵ���Ϣ
}sData_zone_config;

//���������Ϣ


//���ø����������
typedef struct {
	u16 zoneIndex;                        //ģ��1(0x01) ģ��2(0x02)  ����ģ��(0xA0)(����ģ�鲻��Ҫ����) 0xFFΪĬ��ֵ
	u16 FuncSeclet;                       //��������
	u16 speed;                            //�ٶȵ�λ 4��	
	u16 dir;                              //����  Ĭ��cw����
	u16 ramp;                             //�Ӽ��ٶȵ�λ
}sData_belt_node;
//ÿһ�����������
typedef struct {
	u8  version[4];                        //ģ��汾�ŵ��ֽ�
	u16 slaverIndex;                       //������վ��
	u16 slaver_zonenum;                    //�ôӻ���Ч�ĵ���������
	sData_belt_node  belt_node[ZONE_NUM];  //�ӻ��������
}sData_belt_msg;


//ʵʱ��Ϣ
typedef struct {
	u16 zoneIndex;                     //������
	u16 zoneState;                     // v1
									   // ����״̬ bit0����״̬   Bit1������1����  bit2 ������2���� 
	                                   // bit3 �޻� bit4�л� bit5 ���ڳ��� bit6�ȴ��ӻ�  bit7����ɹ����䵽λ bit8 ����������2-0����(��Ӧ���Ͳ1��
	                                   // bit9 ����������2-1���������Ͳ1��bit10 ����������2-2���������Ͳ2��bit11 ����������2-3���������Ͳ2��
	                                   // bit12 �������޹�紥�� bit13 �������޹�紥��
									   // v2 20230803
	                                   // ����״̬ bit0����״̬   Bit1������1���� (����������1-0����,��Ӧ���Ͳ1��  bit2 ������2���� (bit9 ����������1-1�������Ͳ1��
									   // bit3 ����������2-1���������Ͳ2��bit4 ����������2-2���������Ͳ2��
									   // bit5 �޻� bit6�л� bit7 ���ڳ��� bit8�ȴ��ӻ�  bit9 �������޹�紥�� bit10 �������޹�紥��
	u16 zoneAlarm;                     // ����״̬ Bit0  ����ģ����1 �������/δ����  bit1 Ƿѹ bit 2 ��ѹ bit3����
	                                   // bit4 ������1���� ����������1-0����(��Ӧ���Ͳ1��  Bit5������2����  ����������1-1����(��Ӧ���Ͳ1��
									   // bit6 ����������2-0����(��Ӧ���Ͳ2��  Bit7������2����  ����������2-1����(��Ӧ���Ͳ2��
									   // bit8 ��ģ��δ����  Bit9  ����ӻ���ʱ  Bit10  ���������ʱ Bit11 ����ģ����2/δ����  Bit12 ����ģ��������3/δ����  	                                  
	u32 zonePkg;                       // ������� Ԥ��  Ĭ��0
	u16 transsuccess;                  // �ɹ��ּ�
}sData_RealtimeState;

typedef struct {                                 //�������ñ�ʶ��
	u16 ctrlindex;
	u16 moudlenum;
	sData_RealtimeState  node[ZONE_NUM];
}sData_Realtime_statenode;

typedef struct {
	sData_Realtime_statenode  state[BELT_ZONE_NUM];
}sbelt_Moudle_state;


//��������ڵ�
typedef struct {
	u32   pkgid;
	u16   result;
}sPkgResultNode;

#pragma pack ()

//������Ϣ
typedef struct {
	struct xLIST_ITEM* index;
	u32 pkgId;                              //������
	u16 totalzoneNum;                       //������Ҫ;������������
	u16 zoneindex[BELT_ZONE_NUM];           //����;���ľ���·��
	u16 lastZonenum;                        //������һ��������������
	u16 curZonenum;                         //��ǰ����������������
	u16 curZoneIndex;                       //��ǰ��������
	u16 curZoneStatus;                      //�����ڵ�ǰ�����״̬
	u16 curZoneCnt;                         //�������ڵ�ǰ�����ʱ��
	u16 curZonedir;                         //�����ڵ�ǰ����Ĵ��䷽��
	u16 curZoneruntype;                     //��ǰ���������������
	u16 curZoneArrive;                      //���ﵱǰ����
	u16 nextZoneIndex;                      //��ǰ������һ������ı��
	u16 nextZoneStatus;                     //��������ǰ������һ�������״̬
	u16 nextZoneCnt;                        //��ǰ������һ������Ľӻ�ʱ��
	u16 nextZonedir;                        //������ǰ������һ������Ĵ��䷽��
	u16 nextZoneruntype;                    //ǰ������һ������İ�����������
//	u16 nextZonelastpkgstat;                //������ǰ������һ�������֮ǰ�İ���״̬
	u16 nextZonecurpkgstat;                 //������ǰ������һ������ĵ�ǰ�İ���״̬
	u16 nextZonepkgstatchange;              //������ǰ������һ������ĵ�ǰ�İ���״̬�仯
	u16 nextZonechangenum;                  //������ǰ������һ������ĵ�ǰ�İ���״̬�仯����
	u16 theThirdZone;                       //��һ���������һ������
    u16 allowState;                         //��ֹ���䡢�������͡�ȡ������
	u16 sendcmdcnt;                         //���Ӱ忨������ͣ�����ʱ
}sData_pkg_node;

typedef struct {
	sPkgResultNode* queue;     //������нڵ�
	u16 front, rear, len;     
	u16 maxsize;               
}sPkgResultQueue;

//����������Ϣ
extern sData_zone_config     zoneConfig;
extern sData_zone_config     zonetmp;
//��Ͳ���ò���
extern sData_belt_msg        beltMoudlepara;
//��Ͳģ��ʵʱ����
extern sbelt_Moudle_state    beltMoudlestate;


extern sData_pkg_node        pgkNodeDateItem[];
extern struct xLIST_ITEM     pkgNodeListItem[];


//�����������
extern sPkgResultNode        pkgresultnode[];
extern sPkgResultQueue       pkgresultqueue;

void data_msg_init(void);
void data_pkg_list_init(void);
void data_addto_pkg_list(sData_pkg_node x);
sData_zone_node* data_find_zone_moudlestate(u16 index);
sData_RealtimeState* data_find_ctrl_status(u16 ctrlindex, u16 moudleindex);
u8 data_find_nearzone_direction(sData_zone_node node, u16 nextzone);
u8 data_config_moudle_rundir(u8 predir, u8 nextdir);

void DatainitpkgQueue(void);
void DataaddPkgQueue(sPkgResultNode x);
sPkgResultNode* DatagetmsgfromQueue(void);


#endif