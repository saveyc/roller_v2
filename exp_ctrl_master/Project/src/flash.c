#include "flash.h"
#include "main.h"


void flash_read_zone_conf(void)
{
    u16 i = 0;
    u32 tmp1 = 0;
    u32 tmp2 = 0;
    u16 num1 = 0;
    u16 num2 = 0;
    u16 totalnum1 = 0;
    u16 totalnum2 = 0;
    u16 index = 0;

    zonetmp.confid = 0;
    zonetmp.flashcnt = 0;
    zonetmp.pkgcal = 0;
    zonetmp.pkgcurnum = 0;


    tmp1 = *((u16*)UserParaAddressone) | ((*((u16*)(UserParaAddressone + 2))) << 16);
    num1 = *((u16*)(UserParaAddressone + 4) );
    totalnum1 = *((u16*)(UserParaAddressone + 6));


    tmp2 = *((u16*)UserParaAddresstwo) | ((*((u16*)(UserParaAddresstwo + 2))) << 16);
    num2 = *((u16*)(UserParaAddresstwo + 4));
    totalnum2 = *((u16*)(UserParaAddresstwo + 6));
    if (num1 == 0xFFFF) {
        num1 = 0;
    }
    if (num2 == 0xFFFF) {
        num2 = 0;
    }
    if ((tmp1 == 0xFFFF) || (tmp1 == 0)) {
        return;
    }
    if ((tmp2 == 0xFFFF) || (tmp2 == 0)) {
        return;
    }

    if ((tmp1 == tmp2) && (totalnum1 == totalnum2) && ((num2 + num1) == totalnum2)) {
        zoneConfig.zonetotalnum = totalnum2;
    }
    else {
        return;
    }

    if (zoneConfig.zonetotalnum > BELT_ZONE_NUM) {
        return;
    }

    for (i = 0; i< num1; i++) {
            zoneConfig.zoneNode[i].zoneIndex = *((u16*)(UserParaAddressone + 8 + i * 14 + 0));
            zoneConfig.zoneNode[i].ctrlIndex = *((u16*)(UserParaAddressone + 8 + i * 14 + 2));
            zoneConfig.zoneNode[i].beltMoudleIndex = *((u16*)(UserParaAddressone + 8 + i * 14 + 4));
            zoneConfig.zoneNode[i].front_zone = *((u16*)(UserParaAddressone + 8 + i * 14 + 6));
            zoneConfig.zoneNode[i].rear_zone = *((u16*)(UserParaAddressone + 8 + i * 14 + 8));
            zoneConfig.zoneNode[i].left_zone = *((u16*)(UserParaAddressone + 8 + i * 14 + 10));
            zoneConfig.zoneNode[i].right_zone = *((u16*)(UserParaAddressone + 8 + i * 14 + 12));
            index++;
    }

    for (i = 0; i < num2; i++) {
            zoneConfig.zoneNode[i + index].zoneIndex = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 0));
            zoneConfig.zoneNode[i + index].ctrlIndex = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 2));
            zoneConfig.zoneNode[i + index].beltMoudleIndex = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 4));
            zoneConfig.zoneNode[i + index].front_zone = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 6));
            zoneConfig.zoneNode[i + index].rear_zone = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 8));
            zoneConfig.zoneNode[i + index].left_zone = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 10));
            zoneConfig.zoneNode[i + index].right_zone = *((u16*)(UserParaAddresstwo + 8 + i * 14 + 12));
    }

    zoneConfig.confid = tmp1;
    zoneConfig.pkgcurnum = 0;

    funOnlineCtrl.confid = tmp1;

    zonetmp.confid = zoneConfig.confid;
    zonetmp.flashcnt = 0;
    zonetmp.pkgcal = 0;
}

void flash_write_zone_conf(void)
{
    u16 i, j;
    u16 tmp = 0;
    u16 num = 0;

    j = 0;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    if (FLASH_ErasePage(UserParaAddressone) == FLASH_COMPLETE)
    {
        tmp = zonetmp.confid & 0xFFFF;
        FLASH_ProgramHalfWord(UserParaAddressone, tmp);
        tmp = (zonetmp.confid >> 16) & 0xFFFF;
        FLASH_ProgramHalfWord(UserParaAddressone + 2, tmp);


        for (i = 0; i < BELT_ZONE_NUM / 2; i++) {
            if ((zonetmp.zoneNode[i].zoneIndex != 0xFFFF) && (zonetmp.zoneNode[i].zoneIndex !=0 ))
            {
                num++;
            }
        }
        FLASH_ProgramHalfWord(UserParaAddressone + 4, num);
        FLASH_ProgramHalfWord(UserParaAddressone + 6, zonetmp.zonetotalnum);
        

        for (i = 0; i < BELT_ZONE_NUM /2; i++) {
            if (zonetmp.zoneNode[i].zoneIndex != 0xFFFF)
            {
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].zoneIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].ctrlIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].beltMoudleIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].front_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].rear_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].left_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddressone + 2 * j + 8, zonetmp.zoneNode[i].right_zone);
                j++;
            }
        }
    }
    FLASH_Lock();


    j = 0;
    num = 0;
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
    if (FLASH_ErasePage(UserParaAddresstwo) == FLASH_COMPLETE)
    {
        tmp = zonetmp.confid & 0xFFFF;
        FLASH_ProgramHalfWord(UserParaAddresstwo, tmp);
        tmp = (zonetmp.confid >> 16) & 0xFFFF;
        FLASH_ProgramHalfWord(UserParaAddresstwo + 2, tmp);


        for (i = BELT_ZONE_NUM / 2; i < BELT_ZONE_NUM; i++) {
            if ((zonetmp.zoneNode[i].zoneIndex != 0xFFFF) && (zonetmp.zoneNode[i].zoneIndex !=0 ))
            {
                num++;
            }
        }
        FLASH_ProgramHalfWord(UserParaAddresstwo + 4, num);
        FLASH_ProgramHalfWord(UserParaAddresstwo + 6, zonetmp.zonetotalnum);


        for (i = BELT_ZONE_NUM / 2; i < BELT_ZONE_NUM; i++) {
            if (zonetmp.zoneNode[i].zoneIndex != 0xFFFF)
            {
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].zoneIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].ctrlIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].beltMoudleIndex);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].front_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].rear_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].left_zone);
                j++;
                FLASH_ProgramHalfWord(UserParaAddresstwo + 2 * j + 8, zonetmp.zoneNode[i].right_zone);
                j++;
            }
        }
    }
    FLASH_Lock();
}

void flash_upload(void)
{
    if (zonetmp.flashcnt != 0) {
        zonetmp.flashcnt--;
        if (zonetmp.flashcnt == 0) {
            flash_write_zone_conf();
            flash_read_zone_conf();
            AddSendMsgToQueue(SEND_MSG_BD2PC_BDONLINE_TYPE);
        }
    }
}

